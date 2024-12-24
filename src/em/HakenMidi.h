// Haken Audio Midi Protocol by Lippold Haken, (C) Copyright 1999-2024.
// These definitions are used by the Haken Editor, as well as third-party EaganMatrix Overlays.
// In this file, "tx" means transmitted by DSP, "rx" means received by DSP.	

#ifndef HAKENMIDI_H
#define HAKENMIDI_H

// Table of Contents
// 
//  Line#	  Topic											  Midi Channels
//	  32	Overview		
//	  54W	Status Bytes    Definitions for Continuum		(ch1, ch2, ch16)
//   155	SSL_ch1_ch2     List of all cc on ch1/ch2       (ch1/ch2)
//	 180	SSL_ch16        List of all cc on ch16		    (ch16)
//   206    SSL_streams		List of all stream names		(ch16)
//   216    SSL_Form_Poke	List of all id in s_Form_Poke	(ch16)
//   236    SSL_Mat_Poke	List of all id in s_Mat_Poke	(ch16)
//   260	SSL_ccTask		List of values for ccTask		(ch16)
//	 288	Preset Select   Preset Load and Store			(ch2/ch16)
//	 395	Note cc         For Encoding Surface Activity	(ch2..ch15)
//	 424	Macro cc        Macros i..vi and 14' m7..m90	(ch1 and ch2)
//	 467	Global cc       Absolute ch1 and Relative ch2	(ch1/ch16 and ch2)
//	 509	Setup cc        General Configuration			(ch16)
//	 925	ccStream        Select a stream					(ch16 "sData" follow)
//	 998	s_Form_Poke     Formula Configuration stream	(ch16 "sData" pairs: id,value)
//	1203	s_Mat_Poke      Matrix  Configuration stream	(ch16 "sData" pairs: id,value)
//	1486	s_Kinet_Poke    Kinetic Properties stream		(ch16 "sData" pairs: id,value)
//	1541	s_BiqSin_Poke   BiqSin  Properties stream		(ch16 "sData" pairs: id,value)
//	1600	s_Conv_Poke     Convolution Config stream 		(ch16 "sData" pairs: id,value)
//	1644	Thick Continuum config strip locations


// Overview of Haken Audio Midi Protocol:
//		Channel 2-16 for MPE notes (see "Note cc" section below for more information)
//			Bn 86 ff		(1) *optional* low 7' for subsequent Z, Y, or X
//			En xx xx		(2) X 14' bend offset from keyOn nn (optionally preceded by cc86 for 21')
//			Dn zz			(3) Z 7' (when Z changes are slow this is preceded by cc86 for 14')
//			Bn 74 yy		(4) Y 7' (optionally preceded by cc86 for 14')
//			8n nn 127		(5) start note at pitch of nn + immediately preceding X bend
//			9n nn 127		(6) end of note (not sent until end of sustain/sostenuto if active)
//		Channel 1 for Absolute 7-bit Control (general use) and 14-bit Macros (for EaganMatrix Overlays)
//			B0 cc# value	(1) Absolute 7' controllers and 7' macros
//							(2)  m7-m48 14' controllers (lsb in preceding "FracPed" cc86)
//							(3) m49-m90 14' controllers (lsb in preceding "FracPedEx" cc97)
//		Channel 2 for Relative 7-bit Control (general use) and Bank/Preset Select
//			B1 cc# value	(1) Relative 7' controllers and 7' macros
//							(2) Select preset: B1 32 bank, B1 00 presetHi, C1 presetLow
//		Channel 16 for Configuration (0xBF and 0xAF Midi status bytes, AF for id# & value or value pair)
//			BF cc# value	Configuration cc# descriptions below
//			AF id# value	Configuration id# meaning determined by preceding cc56 "stream_poke" select
//			AF val1 val2	Pairs of values in stream, meaning determined by preceding cc56 "stream" select


enum {
	// Midi "status byte" definitions.(8)
	keyOn 			= 0x90,		// default: Continuum uses constant 127 for keyOn velocity
	keyOff 			= 0x80,
		nnPerOctave		= 12,
		nnMiddleC 		= 60,
		nnMiddleD		= 62,	// middle of playing surface for Continuums
		nnMiddleA 		= 69,
		nnHi			= 119,	// half step below 10th octave (for display, good to avoid 10th oct)
		nnLowFull		= 15,	// lowest note number (without transpose) on full size
		nnHighFull		= 109,	// highest note number (without transpose) on full size
		nnLow70			= 28,	// nominal lowest note number (without transpose) on slim70
		nnLow70fsh		= 30,	// lowest f# on a slim70
		nnHigh70		= 96,	// nominal highest note number (without transpose) on slim70
		nnHigh70c		= 96,	// highest c on a slim70
		nnLow46			= 40,	// nominal lowest note number (without transpose) on slim46 and half size
		nnLow46fsh		= 42,	// lowest f# on a slim46
		nnHigh46		= 85,	// nominal highest note number (without transpose) on slim46 and half size
		nnHigh46c		= 84,	// highest c on a slim46
		nnLowMini		= 52,	// lowest note number (without transpose) on ContinuuMini
		nnHighMini		= 80,	// highest note number (without transpose) on ContinuuMini
		nnMidMini		= 66,	// (nnLowMini+nnHighMin)/2
		koSusSos		= 1,	// internal keyOff value for sustain/sostenuto pedal up		10.10 [CCon23]
		koSteal			= 127, 	// internal keyOff value for voice steal					10.10 [CCon23]
	polyKeyPres 	= 0xa0,		// polyphonic key pressure (2 data bytes) meaning depends on channel
								// Channel bits = 0000 rx:    MPE ch1 Z, used by rechannelMidiKeyboard()
								// Channel bits = 10XY rx:    X is msb of 1st download 8' byte, Y of 2nd
		sData			= 0xaf,	// Channel bits = 1111 rx&tx: two sData 7' bytes in Stream	10.33
								//   - sData is preceded by ccStream to select a stream	 	 6.16 [NAMM]
								//	 - always an even number of sData bytes in a stream		10.34 [Bday]
								//   - "poke" streams encode id# & value
								//   - other streams sData are pairs of values (e.g. two ascii characters)
	ccStat 			= 0xb0,		// control change "status byte"; controllers defined below
		max14		   = 0x3f80,// 1.0 value for 14' 0..1 controllers (max high 7', zero low 7': 127<<7)
		#define inv_max14  0.00006151574 // 1/max14 = 1/(127<<7); ccFrac/ccFracPed/ccFracPedEx = low 7'
		zero14		   = 0x2000,// 0.0 value for 14' -1..1 controllers
		#define inv_zero14 0.00012207031 // 1/zero14 = 1/(64<<7); ccFrac/ccFracPed/ccFracPedEx = low 7'
	progChg 		= 0xc0,		// program change (1 data byte follows) has two distinct uses:
								// (1) Continuum preset select sequence ccBankH ccBankL progChg:
								//	   ccBankH on ch2 or ch16 is category (see below),
								//	   ccBankL is high 7 bits of preset within category (can omit if 0),
								//	   progChg on ch2 or ch16 to read (use ccStore instead to write).
								// (2) On preset load, can transmit a user-selected progChg on ch1.
	chanPres 		= 0xd0,		// channel pressure (1 data byte); used by MPE Z encode
								//   - our MPE+ uses chanPres for Z with ccFrac supplying optional lsb
	pitchWheel 		= 0xe0,		// pitch wheel (pitch bend) 14 bits follow for normal Midi,
								// we sometimes send additional 7 bits with preceding ccFrac
		bendMax		= 0x3fff,	// max for encoding 14-bit normal Midi PitchWheel
		extBendMax 	= 0x1fffff,	// max for encoding 21-bit extended Midi PitchWheel using ccFrac
		bendOffset 	= 0x2000,	// offset for encoding 14-bit normal Midi PitchWheel 
		#define inv_bendOffset    .0001220703125
		extBendOffset=0x100000,	// offset for encoding 21-bit extended PitchWheel (using ccFrac)
		#define inv_extBendOffset .000000953674316

	// System Messages.
	sysMes 			= 0xf0,		// system messages   
	SysEx 			= 0xf0,		// sysex for old Preset files: now use sData  				 5.40 [BPK]
	MidiTimingCode	= 0xf1,		// Midi Time Code quarter-frame message (together with Time Code SysEx)
	SongPosPtr 		= 0xf2,		// 2 bytes of data follow
	SongSelect 		= 0xf3,		// 1 byte of data follows
	//				= 0xf4,		//
	//				= 0xf5,		//
	TuneRequest	 	= 0xf6,		// no data follows
	SysExEnd 		= 0xf7,		// end system exclusive; see also s_StreamEnd 				 5.40 [BPK]
	TimingCk 		= 0xf8,		// Midi Clock: message 1 of 4 (real time, 24 per quarter note, no data)
	MidiTick		= 0xf9,		// 10 ms clock (rarely used, not part of Midi Clock)
	StartSeq 		= 0xfa,		// Midi Clock: message 2 of 4 (Midi Time Code is different, uses SysEx)
	ContinueSeq 	= 0xfb,		// Midi Clock: message 3 of 4 
	StopSeq 		= 0xfc,		// Midi Clock: message 4 of 4 
	//				= 0xfd,		// [undefined]
	actSense 		= 0xfe,		// active sensing (Continuum no longer transmits, as of 8.11)
	ResetAllReceivers = 0xff,

	// Masks.
	realTimeMask 	= 0xf8,		// real-time Midi bytes have these bits set
	statFlag 		= 0x80,		// top bit of status nybble indicating a status byte   
	statMask 		= 0xf0,		// status nybble   
	chanMask 		= 0x0f,		// channel nybble  

	// System controllers.
	ccSoundOff		= 120,		// All Sound Off (stops all notes on channel)
	ccDefault	 	= 121,		// All Controllers Default Value channel mode message
	ccLocal			= 122,		// local control on/off
	ccAllOff	 	= 123,		// All Notes Off channel mode message (keeps playing if sus pedal)
	ccOmniOff		= 124,
	ccOmniOn		= 125,
	ccMonoOn		= 126,		// mono on (poly off)
	ctPolyOn		= 127,		// poly on (mono off); redefined by MPE
	ccMpe			= 127,		// ... taken over by MPE: data is polyphony, or 0 to end MPE

	// Channel number.
	mpeMastChan		= 0,		// channel 1 is MPE's default "master" channel
	ch1				= 0,		// channel 1 tx/rx absolute peds, ch1Note, utility			 8.67
		ccStat1=ccStat+ch1,		// NB: Continuum echoes absolute ch1 for relative rx'ed on ch2
		progChg1=progChg+ch1,	// tx only: user-selected value to external synth
	mpeFirstChan	= 1,		// channel 2 is MPE's default first channel for notes
	ch2				= 1,		// channel 2 rx preset select, macros, globals (see below)	 9.94
		ccStat2=ccStat+ch2,		// rx only: relative position, echo absolute on ch1			 8.67
	ch16	 		= 15,		// channel 16 cc (tx and rx): general Continuum setup
		ccStat16=ccStat+ch16,	// NB: ccI..VI and globals aliased (same function) on ch16	10.22


// -------------------------------------- List of all cc on ch1/ch2 ---------------------------------------
// Space-separated list of names of Continuum cc on ch1/ch2.  
	#define SSL_ch1_ch2            /* see sections below for definitions of these cc */					  \
			 /* ch1/ch2 cc0   */   "BankH Mod Breath Undef Foot (5) DataH Vol OctShift MonoSw FineTune "  \
			 /* ch1/ch2 cc11  */   "Express i ii iii iv v vi Post AudIn Reci1 Reci2 Reci3 Reci4 "		  \
			 /* ch1/ch2 cc24  */   "ReciMix RoundRate Pre Atten RndIni Jack1 Jack2 Advance BankL "		  \
			 /* ch1/ch2 cc33  */     "(33) (34) (35) (36) (37) "										  \
			 /* ch1/ch2 cc38  */   "DataL "																  \
			 /* ch1/ch2 cc39  */     "(39) m7/m49 m8/m50 m9/m51 m10/m52 m11/m53 m12/m54 m13/m55 m14/m56 " \
			 /* ch1/ch2 cc48  */     "m15/m57 m16/m58 m17/m59 m18/m60 m19/m61 m20/m62 m21/m63 m22/m64 "	  \
			 /* ch1/ch2 cc56  */     "m23/m65 m24/m66 m25/m67 m26/m68 m27/m69 m28/m70 m29/m71 m30/m72 "	  \
			 /* ch1/ch2 cc64  */   "Sus Wide Sos HpLevel LineLevel Sos2 Actuatn "						  \
			 /* ch1/ch2 cc71  */     "(71) (72) (73) "													  \
			 /* ch1/ch2 cc74  */   "ccY "																  \
			 /* ch1/ch2 cc75  */     "(75) (76) (77) (78) (79) (80) (81) LpDetect "						  \
			 /* ch1/ch2 cc83  */   "EqTilt EqFrq EqMix FracPed Frac MSpL MSpH ThrDrv AtkCut "			  \
			 /* ch1/ch2 cc92  */   "RatMkp CoThMix (94) Reci5 Reci6 FracPedEx NrpnL NrpnH RpnL RpnH "	  \
			 /* ch1/ch2 cc102 */     "m31/m73 m32/m74 m33/m75 m34/m76 m35/m77 m36/m78 m37/m79 m38/m80 "   \
			 /* ch1/ch2 cc110 */     "m39/m81 m40/m82 m41/m83 m42/m84 m43/m85 m44/m86 m45/m87 m46/m88 "	  \
			 /* ch1/ch2 cc118 */     "m47/m89 m48/m90 "													  \
			 /* ch1/ch2 cc120 */   "SoundOff "															  \
			 /* ch1/ch2 cc121 */     "(121) (122) (123) (124) (125) (126) "								  \
			 /* ch1/ch2 cc127 */   "MPE "


// ---------------------------------------- List of all cc on ch16 ----------------------------------------
// Space-separated list of names of Continuum cc on ch16.
// NB: cc in brackets are reserved on ch16 for preset files, but will be translated to ch1/ch2.
#define SSL_ch16				  /* see sections below for definitions of these cc */					  \
			   /* ch 16 cc0   */   "[BankH] [1] [2] [3] [4] [5] [DataH] [7] [8] [MonoSw] [FineTune] "	  \
			   /* ch 16 cc11  */   "[Express] [12] [13] [14] [15] [16] [17] [Post] [AudIn] "			  \
			   /* ch 16 cc20  */   "[Reci1] [Reci2] [Reci3] [Reci4] [ReciMix] [RoundRate] [Pre] "		  \
			   /* ch 16 cc27  */   "[Atten] [RndIni] [29] [30] [31] [BankL] "							  \
			   /* ch 16 cc33  */     "(33) FormSel FormCopy MacrUses PedTyp "							  \
			   /* ch 16 cc38  */   "[DataL] "															  \
			   /* ch 16 cc39  */     "(39) Row Col MatDecLo MatDecHi MatVal MatOp Neighbor ModMatInd "    \
			   /* ch 16 cc48  */     "ModMatVal (49) (50) Grid (52) (53) (54) (55) "					  \
			   /* ch 16 cc56  */     "Stream (57) (58) Mini_LB (60) (61) (62) (63) "					  \
			   /* ch 16 cc64  */   "[64] [65] [66] [HpLevel] [LineLevel] [Sos2] [Actuatn] "				  \
			   /* ch 16 cc71  */     "PolyTrad PolyDsp PolyCvc "										  \
			   /* ch 16 cc74  */   "ccY "																  \
			   /* ch 16 cc75  */     "Test Min1 Max1 Min2 Max2 QBias (81) LpDetect "					  \
			   /* ch 16 cc83  */   "[EqTilt] [EqFrq] [EqMix] [86] Frac [MSpL] [MSpH] [ThrDrv] [AtkCut] "  \
			   /* ch 16 cc92  */   "[RatMkp] [CoThMix] (94) [Reci5] [Reci6] [97] [98] [99] [100] [101] "  \
			   /* ch 16 cc102 */     "VersHi VersLo CVCHi CVCMed CVCLo Cont (108) Task "				  \
			   /* ch 16 cc110 */     "DInfo LedAes Store SNBN UsgDisp LogDump Editor (117) "			  \
			   /* ch 16 cc118 */     "EdReply ArcCnt "													  \
			   /* ch 16 cc120 */   "SoundOff "															  \
			   /* ch 16 cc121 */     "(121) CRC0 CRC1 CRC2 CRC3 CRC4 "									  \
			   /* ch 16 cc127 */   "MPE "


// ----------------------------------------- List of all Streams -----------------------------------------
// Space-separated list of all stream names, selected by cc56.         
	#define SSL_streams		   	   /* see "ccStream" section below for stream formats */				  \
			 /*  streams  0- 8 */  "Name ConText Graph GraphO1 GraphO2 GraphT0 GraphT1 Log UpdF2 "		  \
			 /*  streams  9-17 */  "DemoAst Float Kinetic BiqSin Sys Conv Thumbnail MacroH MacroL "		  \
			 /*  streams 18-23 */  "MacrUses Form_Poke Mat_Poke Graph_Poke GraphO1_Poke GraphO2_Poke "	  \
			 /*  streams 24-26 */  "Kinet_Poke BiqSin_Poke Conv_Poke "	  


// ------------------------------------ List of all id in s_Form_Poke ------------------------------------
// Space-separated list of id names in s_Form_Poke streams (preceded by cc56 19).
	#define SSL_Form_Poke		    /* see s_Form_Poke section below for definitions of these id */		  \
			/* stream 19 id 0   */ "(0) MultX MultY MultZ InitX InitY InitZ "							  \
			/* stream 19 id 7   */ "AncDom AncLookup AncLimit AncNN AncTAorAll TransBl "				  \
			/* stream 19 id 13  */ "NegW NegW2 NegBegX NegBegX2 NegEndX NegEndX2 NegBegY "				  \
			/* stream 19 id 20  */ "NegBegY2 NegEndY NegEndY2 NegBegZ NegBegZ2 NegEndZ NegEndZ2 "		  \
			/* stream 19 id 27  */ "XMacr YMacr ZMacr (30) (31) (32) (33) (34) (35) (36) (37) "			  \
			/* stream 19 id 38  */ "FormOp Blend SgW MultW HeightW HBegX ZeroX VBegX VEndX (47) "		  \
			/* stream 19 id 48  */ "Interp HEndX ShapeW VBegY VEndY HBegY HEndY TransY "				  \
			/* stream 19 id 56  */ "AncOp AncVal VBegZ VEndZ HBegZ HEndZ TransZ Persist (64) (65) "		  \
			/* stream 19 id 66  */ "ScaleW SWMacr BlMacForm (69) (70) (71) (72) HeightW2 (74) "			  \
			/* stream 19 id 75  */ "VBegX2 VEndX2 VBegY2 VEndY2 VBegZ2 VEndZ2 Persist2 Interp2 "		  \
			/* stream 19 id 83  */ "(83) (84) TransX (86) (87) (88) (89) (90) (91) (92) (93) "			  \
			/* stream 19 id 94  */ "(94) (95) Reluct Reluct2 (98) (99) AncOp2 AnvVal2 (102) "			  \
			/* stream 19 id 103 */ "(103) (104) (105) (106) (107) (108) (109) (110) (111) QuantX "		  \
			/* stream 19 id 114 */ "(113) (114) (115) (116) SgM LoW HiW (120) BegBl (122) EndBl " 		  \
			/* stream 19 id 124 */ "AllW InitW (126) (127) "


// ------------------------------------- List of all id in s_Mat_Poke -------------------------------------
// Space-separated list of id names in s_Mat_Poke streams (preceded by cc56 20).
	#define SSL_Mat_Poke	       /* see s_Mat_Poke section below for definitions of these id */		  \
			/* stream 20 id 0   */ "(0) SplitMode NoteMode ReciCol1 ReciCol2 OkIncComp "				  \
			/* stream 20 id 6   */ "Prio SwTogInst (8) Reverse RoundMode OkExpPoly Action "				  \
			/* stream 20 id 13  */ "Aes3 BigFontPop RecircDisab CompOpt (17) (18) (19) (20) "			  \
			/* stream 20 id 21  */ "(21) (22) (23) (24) (25) (26) (27) (28) (29) (30) (31) "		      \
			/* stream 20 id 32  */ "(32) (33) (34) Program Routing (37) (38) Poly BendRange "			  \
			/* stream 20 id 41  */ "FrontBack Pressure (43) MiddleC SplitPoint MonoFunc (47) "			  \
			/* stream 20 id 48  */ "MonoInterv (49) (50) (51) Pedal1 Pedal2 JackShift (55) "		      \
			/* stream 20 id 56  */ "PreservSurf PreservPed PreservEncod ConfigToMidi TArea (61) "		  \
			/* stream 20 id 62  */ "ReciType CvcCfg SSetA SSetB (66) (67) (68) (69) BqExtA BqExtB "		  \
			/* stream 20 id 72  */ "OscFilTyp1 OscFilTyp2 OscFilTyp3 OscFilTyp4 OscFilTyp5 "		      \
			/* stream 20 id 77  */ "OscFilOpt1 OscFilOpt2 OscFilOpt3 OscFilOpt4 OscFilOpt5 "		      \
			/* stream 20 id 82  */ "FilExtrm1 FilExtrm2 FilExtrm3 FilExtrm4 FilExtrm5 "					  \
			/* stream 20 id 87  */ "(87) (88) (89) (90) BankA BankB BankC ColModeB1 ColModeA1 "			  \
			/* stream 20 id 96  */ "ColModeB2 ColModeA2 BankParamB BankParamA "							  \
			/* stream 20 id 100 */ "SgTyp1 SgTyp2 SgTyp3 SgTyp4 SgTyp5 TimeSel RowTyp1 RowTyp2 "	      \
			/* stream 20 id 108 */ "AliasDelay (109) (110) (111) (112) (113) (114) (115) (116) "	      \
			/* stream 20 id 117 */ "(117) (118) (119) (120) (121) (122) (123) (124) "					  \
			/* stream 20 id 125 */ "(125) FromAdd ToAdd "


// ------------------------------------ List of all values for ccTask ------------------------------------
// Space-separated list of data values for ccTask.
	#define SSL_ccTask 			   /* see ccTask below for definitions of these values */				  \
			/* ccTask value 0   */ "downloadOkBoot downloadFail downloadOkChained doneCopy "              \
			/* ccTask value 4   */ "downloadOkData archiveOk archiveFail value7available curGloToFlash "  \
			/* ccTask value 9   */ "reduceGain reducePoly inFactCalib eraseMessage noSync cvcPower "	  \
			/* ccTask value 15  */ "dspMismatch configToMidi startFirmware startData burnUserArea "		  \
			/* ccTask value 20  */ "endDataDownload midiLoopbackDetected txtToMidi helloWorld begSync "   \
			/* ccTask value 25  */ "endSync doneSync syncFail doUpdate2 createLed testBegin testErr "	  \
			/* ccTask value 32  */ "namesToMidi PresetRequiresManualUpdate resetCalib refineCalib "		  \
			/* ccTask value 36  */ "midiTxFullRate midiTxOneThirdRate midiTx5PercentRate sysToMidi "	  \
			/* ccTask value 40  */ "endSysNames factCalib doUpdateFile1 burnUser489 dspReboot surfAlign " \
			/* ccTask value 46  */ "addTrim delTrim resetTrim beginSysNames (50) storeFactSetup "		  \
			/* ccTask value 52  */ "decPreset incPreset beginUserNames endUserNames clearTopo "			  \
			/* ccTask value 57  */ "preEraseUpdF1 preEraseUpdF2 preEraseUpdF3 remakeQSPI "      		  \
			/* ccTask value 61  */ "doneFactProg failFactProg usbTxNoAck MidiRxOver MidiTxOver "		  \
			/* ccTask value 66  */ "MidiRxSyntax MidiRxBitErr sensComm nan dspSeq burnUserMini MidiLog0 " \
			/* ccTask value 73  */ "MidiLog1 MidiLog2 MidiLog3 burnRecov489 burnRecov364 burnRecovMini "  \
			/* ccTask value 79  */ "loadsToMidi pitchGrid1 pitchGrid2 pitchGrid3 pitchGrid4 pitchGrid5 "  \
			/* ccTask value 85  */ "pitchGrid6 pitchGrid7 pitchGrid8 numDecMat numIncMat mendDisco "	  \
			/* ccTask value 91  */ "rebootRecov stageUp stageDown stageDok1 stageDok2 stageDok3 "		  \
			/* ccTask value 97  */ "stageDfail1 stageDfail2 stageDfail3 rebootFinaliz gridToFlash "		  \
			/* ccTask value 102 */ "mendDiv startUpdF2 (104) (105) (106) DemoUset1 DemoUset2 DemoUset3 "  \
			/* ccTask value 110 */ "DemoUset4 DemoUset5 DemoUset6 DemoUset7 DemoUset8 EmptyUset1 "		  \
			/* ccTask value 116 */ "EmptyUset2 EmptyUset3 EmptyUset4 EmptyUset5 EmptyUset6 EmptyUset7 "	  \
			/* ccTask value 122 */ "EmptyUset8 burnUser593 burnRecov593 (125) (126) rebootUser "			  


// ---------------------------------------- Preset Load and Store ----------------------------------------\
// Midi order for preset load:	   (on ch2 or ch16)
//	 ccBankH category; ccBankL presetHi; progChg presetLo	   (presetLo/Hi are preset# within category)
// Midi order for preset store:	   (on ch16)
//	 ccBankH category; ccBankL presetHi; ccStore presetLo									10.33

	ccBankH			= 0,		// category select; ccBankL and progChg follow.				 9.70 [COVID19]
								// Aliased (same function) on ch2 and ch16.					 9.95
								// Categories 0..91 are for "general use,"
								//      including local preset selection.
								// Categories 126, 127 are for Editor and program interfaces.
								// 
		catUser 		= 0,	// Category# for User Preset Slots; System Presets categories are 1..91
								// 	NB: Preset numbers start from 1 for internal "catpre" encoding
								// 		(offset 1 from progChg2/progChg16/ccStore Midi encoding)
								// 
								// Categories 1..91 and associated category tags: 
								// All of these are used by the Editor's preset lister;
								// different subsets are used by the menu systems of
								// the Mini, EaganMatrix Module, and Slim Continuum.
		catDrone		= 28,	// "DO" drone category (cat28 see below)
		#define SSL_Cat1_Cat91		/* space-separated list of category tags, cat1..cat91 */		\
								   "AC AD AG AI AN AT BA BB " 	/* tags for cat1  - cat8  */		\
								   "BG BH BI BM BO BR C1 CH " 	/* tags for cat9  - cat16 */		\
								   "CL CM CN CV DA DF DM DP " 	/* tags for cat17 - cat24 */		\
								   "DS DV DI DO DT DY EC EL " 	/* tags for cat25 - cat32 */		\
								   "EM EN EP EV FL FM HM HY " 	/* tags for cat33 - cat40 */		\
								   "IC IN KI KY LE LF LP LY " 	/* tags for cat41 - cat48 */		\
								   "MD MI MM MO MT NA NO OR " 	/* tags for cat49 - cat56 */		\
								   "OS OJ OP OT PA PE PT PL " 	/* tags for cat57 - cat64 */		\
								   "PO PR RD RS RN RO RV SB " 	/* tags for cat65 - cat72 */		\
								   "SD SE SH SI SO SP SR SS " 	/* tags for cat73 - cat80 */		\
								   "ST SU SV SY TA UT VO WA " 	/* tags for cat81 - cat88 */		\
								   "WB WI WO " 					/* tags for cat89 - cat91 */
								//   MainType			 SubType			 
								//  CL : Classic		AT : Atonal			 
								//  CV : CVC			BA : Bass			 
								//  DO : Drone			BO : Bowed			 
								//  KY : Keyboard		BR : Brass			 
								//  MD : Midi			DP : Demo Preset	 
								//  OT : Other			EP : Elec Piano		 
								//  PE : Percussion		FL : Flute			 
								//  PR : Processor		LE : Lead			 
								//  PT : Perc Tuned		OR : Organ			 
								//  ST : Strings		PA : Pad			 
								//  UT : Utility		PL : Plucked		 
								//  VO : Vocal 			RD : Reed Double	 
								//  WI : Winds			RS : Reed Single	 
								// 						SU : Struck			 
								// 
								//   Character ............................................
								// 	AC : Acoustic	    EN : Ensemble	   RV : Reverberant
								// 	AG : Aggressive     EV : Evolving	   SD : Snd Design
								// 	Al : Airy		    FM : FM			   SE : Stereo
								// 	AN : Analog	        HY : Hybrid		   SH : Shaking
								// 	BG : Big		    IC : Icy		   SI : Simple
								// 	BI : Bright	        IN : Intimate      SO : Soft
								// 	CH : Chords	        LF : Lo - fi	   SR : Strumming
								// 	CN : Clean		    LP : Looping	   SY : Synthetic
								// 	DA : Dark		    LY : Layered	   WA : Warm
								// 	DI : Digital	    MO : Morphing      WO : Woody
								// 	DT : Distorted	    MT : Metallic   
								// 	DY : Dry		    NA : Nature
								// 	EC : Echo		    NO : Noise
								// 	EL : Electric	    RN : Random
								// 
								// 	 EaganMatrix .....................	    Setting
								// 	AD : Additive		HM : HarMan		   C1 : Channel 1
								// 	BB : BiqBank		KI : Kinetic	   MI : Mono Interval
								// 	BH : BiqGraph		MM : ModMan		   PO : Portamento
								// 	BM : BigMouth		OJ : Osc Jenny	   RO : Rounding
								// 	CM : Cutoff Mod		OP : Osc Phase	   SP : Split Voice
								// 	DF : Delay Formula	OS : Osc DSF	   SV : Single Voice
								// 	DM : Delay Micro	SB : SineBank	   TA : Touch Area
								// 	DS : Delay Sum		SS : SineSpray		 
								// 	DV : Delay Voice	WB : WaveBank	
								// 	EM : Ext Midi Clk
								// 
		catEdBuf 		= 126, 	// Category# for Midi tx and rx, not used internally: Preset Edit Buffer
								// 	NB: Internally, catUser with userPreset0 is used instead
								//  NB: For Editor highlighting, source preset slot encoded as follows:
								// 		0 no source preset slot ("from disk" or similar state)
								// 		1..128 User Preset Slot 0..127 is source preset
								// 		129+ System Preset Slot 0+ is source preset
								// 
		catSSlot 		= 127,	// Category# for tx/rx, not used internally: System Preset Slot "sysSlot"
								// 
	ccBankL			= 32,		// optional preset# MSB; follows ccBankH
								// Aliased (same function) on ch2 and ch16.					 9.95
	progChg2  = progChg+ch2,	// ch2:  preset load LSB; follows ccBankH and ccBankL.		 9.94
								//		 NB: Preset numbers start from 1 for internal catpre,
								// 		     offset 1 from progChg2/progChg16/ccStore Midi encoding.
								//		 NB: rx for ccBankH = any valid category,
								//			 Continuum tx *only* ccBankH = catSSlot/catEdBuf/catUser
	progChg16 = progChg+ch16,	// ch16: preset load LSB; progChg2 alias (same function)	 9.70 [COVID19]
	ccStore			= 112,		// ch16: store flash; ccBankH = catEdBuf/catUser/catSSlot	10.33
								//       NB: ccStore with catEdBuf or catUser may be done at any time,
								//           but catSSlot only in sequence as part of Update File 2.
								//		 NB: ccStore with catSSlot saves compressed preset in flash
								//		   - only parameters that differ from default are included;
								//		     this is also true for "archives" (presets in .mid file)
								//		   - unused formulas/graphs/properties are omitted;
								//		     this is also true for "archives" (presets in .mid file)
								//		   - in flash, one ch16 cc or sData pair encode in one 16' word
								//		   - in flash, runs of identical sData encode in one 16' word


// -------------------------------------- Note cc: All MPE Channels --------------------------------------
// See MPE+ web page for more discussion of Continuum's note encoding.
// (1) Do not use Key Velocity “MPE Strike Value”; instead use a sequence of values that preserve 
//     details of the attack trajectory. MPE+ always has Midi Key Velocity at 127.
// (2) Notes may start at any pitch; the initial pitch of an MPE+ note is Midi Note Number
//     plus preceding bend. Encourage performers to learn to play without "rounding";
//     do not dumb down the Continuum to starting notes at one-of-12 pitches like a keyboard.
// (3) Temporal resolution in Z updates is paramount in attacks, value resolution in sustains.
//     Use MPE+ glitch-free 14-bit Y and Z updates (see "ccFrac" below), except in attack
//     use as many 7-bit Z updates as possible (skip "ccFrac" for Y and Z during attack).
// (4) 21-bit X (Bend) is available but rarely necessary, even in long sustains.
// (5) The Continuum extends notes during sustain and sostenuto, 
//     delaying keyOff until end sustain or sostenuto pedal is released.
// Notes are encoded with keyOn, keyOff, chanPres, pitchWheel, and certain ccStat (see "Overview").
// The following are cc ("ccStat") that may be used to encode notes.
	ccMod			= 1,		// modulation depth		[alt choice for cc74 Y or chanPres Z]
	ccBreath		= 2,		// breath				[alt choice for cc74 Y or chanPres Z]
	ccUndef			= 3,		// undefined 			[alt choice for cc74 Y or chanPres Z]
	ccFoot			= 4,		// foot pedal			[alt choice for cc74 Y or chanPres Z]
	ccVol			= 7,		// volume 				[alt choice for cc74 Y or chanPres Z]
	ccExpres		= 11,		// expression			[cc11 Z is synonym for chanPres Z]
	ccBrightness	= 74,		// brightness/cutoff 	[default cc for Y]
	ccFrac			= 87,		// MPE+: 7' lsb for next bend/y/z; assumed 0 when not present;
								// ccFrac has no effect until bend/y/z received.
								// Internal: also before keyOn to indicate ch1Note.			 7.44
	ccMSpL			= 88,		// multi split: low nn for channel  						 3.38
	ccMSpH			= 89,		// multi split: high nn for channel  						 3.38


// ------------------------------------- Macros i..vi and 14' m7..m90 -------------------------------------
	// Continuum MPE+ 14' macros incorporate preceding cc86 (ccFracPed) or cc97 (ccFracPedEx) for low 7'.
	// Avoids Midi 14' glitch because cc86 and cc97 values are not used until subsequent macro cc arrives.
	// 
	// i..vi    are  0..1 (see "max14"  0x3F80) and are *optionally* 14' preceded by cc86 ccFracPed,
	// m7..m48  are -1..1 (see "zero14" 0x2000) and are   *always*   14' preceded by cc86 ccFracPed,
	// m49..m90 are -1..1 (see "zero14" 0x2000) and are   *always*   14' preceded by cc97 ccFracPedEx.
	// 
	// NB: *Identical* cc for macros m7..m48 and m49..m90 are differentiated by preceding cc86 *or* cc97.
	// 
	// Examples: (see cc definitions below)
	//	   (1) ch1 cc86 108, ch1 cc42 57 ==> m9  = -0.09619 = (0x1CEC-0x2000)/0x2000
	//	   (2) ch1 cc97 108, ch1 cc42 57 ==> m51 = -0.09619 = (0x1CEC-0x2000)/0x2000
	//	   (3)               ch1 cc14 57 ==> iii =  0.45    =  0x1C80/0x3F80  (no cc86, so low 7' zero)
	//	   (4) ch1 cc86 108, ch1 cc14 57 ==> iii =  0.45446 =  0x1CEC/0x3F80
	// 
	// NB: Macros i..vi on ch16 are aliases for i..vi on ch1 (same function on ch16 as ch1).
	// NB: On the EaganMatrix Module (EMM) only, macros i..iv are a special case. 
	//     The EMM receives ccI..ccIV to provide offsets for the EMM i..iv CV inputs (like rotary).
	//     When the EMM transmits ccI..ccIV, EMM's i..iv CV inputs are added in to the offsets.
	ccFracPed 		= 86,		// 7' lsb for subsequent macro i..m48 or ccJack1,ccJack2
	ccFracPedEx		= 97,		// 7' lsb for subsequent macro m49..m90								
	// cc12..cc17 are i..vi (top 7' of 14' 0..1); 
	//     if there is no preceding cc86 ccFracPed (low 7') then low 7' are zero
	ccI=12,ccII=13,ccIII=14,ccIV=15,ccV=16,ccVI=17,
	// When preceded by cc86 ccFracPed (low 7'), cc40..cc63 are m7..m30 (14' -1..1)   
	ccM7=40,ccM8=41,ccM9=42,ccM10=43,ccM11=44,ccM12=45,ccM13=46,ccM14=47,
	ccM15=48,ccM16=49,ccM17=50,ccM18=51,ccM19=52,ccM20=53,ccM21=54,ccM22=55,
	ccM23=56,ccM24=57,ccM25=58,ccM26=59,ccM27=60,ccM28=61,ccM29=62,ccM30=63,
	// When preceded by cc86 ccFracPed (low 7'), cc102..cc119 are m31..m48 (14' -1..1)
	ccM31=102,ccM32=103,ccM33=104,ccM34=105,ccM35=106,ccM36=107,ccM37=108,ccM38=109,ccM39=110,
	ccM40=111,ccM41=112,ccM42=113,ccM43=114,ccM44=115,ccM45=116,ccM46=117,ccM47=118,ccM48=119,
	// When preceded by cc97 ccFracPed (low 7'), cc40..cc63 are m49..m72 (14' -1..1)
	ccM49=40,ccM50=41,ccM51=42,ccM52=43,ccM53=44,ccM54=45,ccM55=46,ccM56=47,
	ccM57=48,ccM58=49,ccM59=50,ccM60=51,ccM61=52,ccM62=53,ccM63=54,ccM64=55,
	ccM65=56,ccM66=57,ccM67=58,ccM68=59,ccM69=60,ccM70=61,ccM71=62,ccM72=63,
	// When preceded by cc97 ctlFracPedEx (low 7'), cc102..cc119 are m73..m90 (14' -1..1) 
	ccM73=102,ccM74=103,ccM75=104,ccM76=105,ccM77=106,ccM78=107,ccM79=108,ccM80=109,ccM81=110,
	ccM82=111,ccM83=112,ccM84=113,ccM85=114,ccM86=115,ccM87=116,ccM88=117,ccM89=118,ccM90=119,
	// The following definitions are internally used for indexing into arrays of macro values.
	idI=0,idIV=3,idM7=6,idM31=30,idM48=47,idM90=89,idMex=42,


// ---------------------------- Global cc: Absolute ch1/ch16 or Relative ch2  ----------------------------
// For all controllers assignable to pedals (including 7' and 14' controllers), see routine pedVal7().
// NB: ch1 also has cc82 ccLoopDetect, a random pattern for loopback detect (defined for ch16 below).
	ccOctShift		= 8,		// octave pedal: 60=normal, 48/72=oct down/up  				 7.75 [JoshuaT]
								// tx oct buttons ch1 for Editor, tx oct peds ch16			10.33
	ccMonoSwitch	= 9,		// mono footswitch, enable mono interval  					 1.71
	ccFineTune		= 10,		// fine tune  4..124 = -60..+60 cents   					 4.32 6.15 9.08
	ccPost 			= 18,		// samples generation output level after Master Section
	ccAudIn			= 19,		// audio in level  											 4.31 6.15
	ccReci1			= 20,		// recirc dial 1 											 6.14[BPK] 7.35
	ccReci2			= 21,		// recirc dial 2											 6.14[BPK] 7.35
	ccReci3			= 22,		// recirc dial 3, in nn units				 				 6.15 7.35
	ccReci4			= 23,		// recirc dial 4 (see also ccReci5 and ccReci6)				 3.10 7.36
	ccReciMix	 	= 24,		// recirc mix control  										 7.36
	ccRoundRate 	= 25,		// round rate  
	ccPre	 		= 26,		// samples generation level before Master Section
	ccAtten			= 27,		// output attenuation, only saved on Slim  					 9.10 [COVID19]
	ccRndIni		= 28,		// Round Initial
	ccJack1			= 29,		// 0..1 14' (with ccFracPed) from Pedal Jack 1 or Midi in	 7.73 8.28 8.81
	ccJack2			= 30,		// 0..1 14' (with ccFracPed) from Pedal Jack 2 or Midi in 
	ccAdvance		= 31,		// advance to next upreset									 7.70 [HockngH]
	ccSus			= 64,		// sustain pedal; down = b0 40 7f; up = b0 40 00  
	ccStretch		= 65,		// 0..127 = normal..twice stretched intervals				10.34 [Suprbth]
	ccSos			= 66,		// sostenuto pedal, used as first sostenuto on Continuum
	ccHpLevel		= 67,		// headphone level (for OS) 								 8.12 [COVID19]
	ccLineLevel		= 68,		// line level  	  (for OS)									 8.12 [COVID19]
	ccSos2			= 69,		// hold2 pedal, used as second sostenuto					 1.67
	ccMin1			= 76,		// minimum data value for pedal 1							 4.15 
	ccMax1			= 77,		// max data value for pedal 1			
	ccMin2			= 78,		// minimum data value for pedal 2		
	ccMax2			= 79,		// max data value for pedal 2			
	ccEqTilt		= 83,		// Eq Tilt, 64=flat											 9.10 [COVID19]
	ccEqFreq	    = 84,		// Eq Frequency   0..127 = 120Hz..15kHz						 9.10 [COVID19]
	ccEqMix			= 85,		// Eq Mix, 0=dry, 127=wet									 9.10 [COVID19]
	ccThrDrv		= 90,		// Compressor Thresh	/ Master tanh Drive					 9.93 10.33
	ccAtkCut		= 91,		// Compressor Attack	/ Master tanh not used				 9.93
	ccRatMkp		= 92,		// Compressor Ratio		/ Master tanh Makeup				 9.93
	ccCoThMix		= 93,		// Compressor Mix		/ Master tanh Mix					 9.93 10.33
	ccReci5			= 95,		// recirc dial 5
	ccReci6			= 96,		// recirc dial 6


// ---------------------------------------- General Setup cc: ch16 ----------------------------------------
	ccDataH			= 6,		// data entry  tx: display, log, MPE bend range				 7.39 [Fruita]
		// The following at ccDataH definitions for tx utility display; ccDataL has sensor data.
		hiSensNo		= 0x00,	// tx utility display: 2/0,5/ msbs of sensor number
		loSensNo_norm	= 0x20,	// tx utility display: 2/1,5/ lsbs of sens# using normalized display
		loSensNo_dual	= 0x40,	// tx utility display: 2/2,5/ lsbs of sens# using dual-rail display
		newScanHalf 	= 0x7e,	// tx utility display: start of new scan for half-size fingerboard
		newScanFull		= 0x7f,	// rx utility display: start of new scan for full-size or slim70
		newLogDump  	= 0x7d,	// log dump: start of log dump over traditional Midi		
	ccFormSel		= 34,		// select a formula for Editor display and/or modification
	ccFormCopy		= 35,		// formula to copy into currently selected; 0 primary to secondary
	    cpyA = 1 /*matA*/, cpyW = 45 /*matW*/, cpyX, cpyXnn, cpyY, cpyZ, cpy0p1,
	ccMacrUses		= 36,		// request list of formulas using specified macro (via s_MacrUses)
	ccPedType		= 37,		// type of pedals at pedal jacks (tx only)
		bPedType0		= 0x07,	// bits specify type of pedal in jack 0, values below		 7.76
		bPedType1		= 0x38,	// bits specify type of pedal in jack 1, values below
		sPedType1		= 3,	// shift value for bPedType1
		  unPlugged 	  = 0, 	// no pedal plugged in
		  isSwitch		  = 1, 	// Yamaha FC4/FC5   (two-value switch: 0 or 127)
		  isExpr		  = 2, 	// Yamaha FC7 pedal (continuous-valued)
		  isDamper		  = 3,	// Yamaha FC3 pedal (continuous-valued)
		  isTri		 	  = 4,	// Yamaha FC5+FC5   (three-value switch: 0, 64, 127)		 7.76
		  	  	  	  	  	  	// 8x: only in i2c because 1/4" jacks have no ring pullup
		  isCV			  = 5,	// Mini limited CV input -2.5v..+2.5v via special cable		 8.68
		  isPot			  = 6,	// Slim Continuum pot (continuous-valued)
	ccDataL 		= 38, 		// log dumps, utility disp, cvc verify, custom tuning		 6.19 re-add
								// For sensor utility display:
								// 	 Dual Rail ccDataL has three bytes (min, max, cur) per sensor.
								// 	 Normalized Sensor ccDataL has one byte per sensor.
		maxGridEntries 	= 128,	// tuning grid: fills one flash page						 6.19 [GulfSP]
		ccsPerGridPoint = 6,	// tuning grid: number of ccDataL messages per grid point
	ccRow			= 40,		// select row in matrix
		rNoise			=  0,	// noise samples source row
		rOscFil1		=  1,	// 1st oscillator/filter's output row
		rOscFil2		=  2,	// 2nd oscillator/filter's output row
		rOscFil3		=  3,	// 3rd oscillator/filter's output row
		rOscFil4		=  4,	// 4th oscillator/filter's output row
		rOscFil5		=  5,	// 5th oscillator/filter's output row
		rA				=  6,	// Bank A output row
		rB				=  7,	// Bank B output row
		rC				=  8,	// Bank C output row
		rAudioL			=  9,	// External Audio Input Signal L
		rAudioR			= 10,	// External Audio Input Signal R
		rSubL			= 11,	// Submix Audio L
		rSubR			= 12,	// Submix Audio R
		rDirect1		= 13,	// 1st Direct Row
		rDirect2		= 14,	// 2nd Direct Row
		rNegDirect		= 15,	// Negative Direct Row
		rMultiply		= 16,	// Multiply Row
	     nRows,					// number of rows in matrix
	ccCol			= 41,		// select column in matrix
		cSL				=  0,	// Master Section left input
		cSR				=  1,	// Master Section right input
		cCnvI1			=  2,	// 1st convolution index
		cCnvM1			=  3,	// 1st convolution mix
		cReciR1			=  4,	// recirculator first column
		cReciR2			=  5,	// recirculator second column
		cCnvI2			=  6,	// 2nd convolution index
		cCnvM2			=  7,	// 2nd convolution mix
		cSM				=  8,	// Master / Dry mix
		cL				=  9,	// Dry left 
		cR				= 10,	// Dry Right
		cSep			= 11,	// separator column (used for "no column selected")
		cOscFilIn1		= 12,	// 1st Oscillator/Filter Input
		cOscFilF1		= 13,	// 1st Oscillator/Filter Frequency
		cOscFilB1		= 14,	// 1st Oscillator/Filter Bandwidth
		cOscFilIn2		= 15,	// 2nd Oscillator/Filter Input
		cOscFilF2		= 16,	// 2nd Oscillator/Filter Frequency
		cOscFilB2		= 17,	// 2nd Oscillator/Filter Bandwidth
		cOscFilIn3		= 18,	// 3rd Oscillator/Filter Input
		cOscFilF3		= 19,	// 3rd Oscillator/Filter Frequency
		cOscFilB3		= 20,	// 3rd Oscillator/Filter Bandwidth
		cOscFilIn4		= 21,	// 4th Oscillator/Filter Input
		cOscFilF4		= 22,	// 4th Oscillator/Filter Frequency
		cOscFilB4		= 23,	// 4th Oscillator/Filter Bandwidth
		cOscFilIn5		= 24,	// 5th Oscillator/Filter Input
		cOscFilF5		= 25,	// 5th Oscillator/Filter Frequency
		cOscFilB5		= 26,	// 5th Oscillator/Filter Bandwidth
		cA1				= 27,	// BankA first column	idBankA selects bankA function
		cA2				= 28,	//    
		cA3				= 29,	//    
		cA4				= 30,	//    
		cA5				= 31,	//    
		cA6				= 32,	//    
		cA7				= 33,	//    
		cA8				= 34,	//    
		cB1				= 35,	// BankB first column	idBankB selects bankB function
		cB2				= 36,	//    
		cB3				= 37,	//    
		cB4				= 38,	//    
		cB5				= 39,	//    
		cB6				= 40,	//    
		cB7				= 41,	//    
		cB8				= 42,	//    
		cC1				= 43,	// BankC first column   idBankC (delays, or alias for certain BankB)
		cC2				= 44,	//    
		cC3				= 45,	//    
		cC4				= 46,	//    
		cC5				= 47,	//    
		cC6				= 48,	//    
		cC7				= 49,	//    
		cC8				= 50,	//    
		cSgF1			= 51,	// 1st Shape Generator Frequency
		cSgT1			= 52,	// 1st Shape Generator Trigger
		cSgF2			= 53,	// 2nd Shape Generator Frequency
		cSgT2			= 54,	// 2nd Shape Generator Trigger
		cSgF3			= 55,	// 3rd Shape Generator Frequency
		cSgT3			= 56,	// 3rd Shape Generator Trigger
		cSgF4			= 57,	// 4th Shape Generator Frequency
		cSgT4			= 58,	// 4th Shape Generator Trigger
		cSgF5			= 59,	// 5th Shape Generator Frequency
		cSgT5			= 60,	// 5th Shape Generator Trigger
	     nCols,					// number of columns in matrix [61]
		 nCnv			= 2,	// number of convolutions (both in Master) 					 5.41 [BPK]
		 oCnv			= 4,	// number of matrix columns offset between convolutions
	     nOFil			= 5,	// number of oscillators/filters
		 oFilt			= 3,	// number of matrix columns per filter
		 nBank			= 3,	// number of multipurpose banks
	     oBank			= 8,	// number of matrix columns per bank
	     nBiq			= 2,	// number of simultaneous biquad banks		(in BankA and BankB/C)
	     nGran			= 2,	// number of simultaneous granulator banks	(in BankA and BankB/C)
	     nSin			= 2,	// number of simultaneous sine banks		(in BankA and BankB/C)
		 nKin			= 2,	// number of simultaneous kinetic banks		(in BankA and BankB/C)
	     nSg			= 5,	// number of shape generators (2*nSg SG max with dualSG)	 8.22
		 oSg			= 2,	// number of matrix columns per SG
	ccMatDecLo		= 42,		// 1..100: .00xx digits  (for MatDec)						 6.17
								// omit if value 0; precedes ccMatVal, follows ccMatDecHi
	ccMatDecHi		= 43,		// 0..100: tenth and hundredth digits (for matDec)			 6.17
								// precedes ccMatDecLo (if ccMatDecLo is present) and ccMatVal
	ccMatVal		= 44,		// value in matrix - optionally preceded by ccMatDecHi/ccMatDecLo
		matNC			= 0,	// 0 = no connection
		matA			= 1,	// A = first formula
		matV			= 22,	// V = user-defined formula
		matA_			= 23,	// A' = user-defined formula 								10.30
		matV_			= 44,	// V' = last user-defined formula							 6.17
		 nUForms,				// number of user-definable formulas (including placeholder matNC)
		 fSpan = matV_ - matA,	// last definable formula's index from first index			10.30 [SF]
		matW			= 45,	// w gated W
		matX			= 46,	// x concert pitch
		matY			= 47,	// y
		matZZ			= 48,	// z squared
		 nForms,				// number of matrix formulas (including WXYZ and placeholder matNC)
		matDec			= 49,	// 0.0001<=value<=.9999, ccMatDecHi/ccMatDecLo have data	 6.17
		mat1			= 50,	// constant value 1
		mat9			= 58,	// constant value 9
		matSq			= 59,	// square row value
		matCu			= 60,	// cube row value
		matDisBit 		= 64, 	// bit set if matrix point disabled 						 8.10
	ccMatOp			= 45,		// matrix operation
		// 				= 100..105 avail
		colFloat		= 106,	// rx: Show column value (via s_Float)						 8.61 [DanielB]
		noFloat			= 107,	// rx: Hide debug value (no s_Float)						 8.10
		formFloat		= 108,	// rx: Show formula value (s_Float)							 8.10
		//				= 109,	// [no longer used 9.13 COVID19]
		nyqDis			= 110,	// tx: Delay nyquist filter disabled						 5.41
		nyqOff			= 111,	// tx: Delay nyquist filter not in use						 5.41
		nyqOn			= 112,	// tx: Delay nyquist filter is in use
		thumbRefresh	= 113,	// rx: send all thumbnails									10.30 [SF]
		matSendArrays	= 114,	// rx: request to send graph/props data						 9.75
		matSelPosAlt	= 115,	// rx: matSelPos with Alt key down 							 5.13
		matBegPos		= 116,	// rx: user mousedown at ccRow,ccCol 						 5.13
		matHiliteWide	= 117,	// tx: wide highlight current position						 6.17
		matHilite		= 118,	// tx: normal highlight current position					 6.17
		matNoHilite		= 119,	// tx: no highlighted position in matrix					 6.17
		matAtHilite		= 120,	// tx: at matrix data for highlit point						 6.17
		matTogMute		= 121,	// rx: toggle mute on ccRow,ccCol matrix point				 8.10
		matSetMute		= 122,	// rx: set mute on ccRow,ccCol matrix point 				10.33
		matClrMute		= 123,	// rx: clear mute on ccRow,ccCol matrix point 				10.33
		matRefresh		= 124,	// rx: retransmit matrix, current formula, text
		matClear		= 125,	// rx,tx: clear matrix (tx to Editor at new preset load)
		matStart		= 126,	// rx,tx: matrix data follows
		matSelPos		= 127,	// rx: user mouseup in matrix, select it;
								// if matBegPos elsewhere and valid, drag value to here		 5.13
	ccNeighborInd	= 46,		// matrix index to right of matHiliteWide (if any)			 6.17
	ccModMatInd		= 47,		// matrix list index for edit, preceded by ccModMatVal		 6.01
	ccModMatVal		= 48,		// numeric in-place matrix edit, ccModMatInd follows		 6.01
	ccGrid 			= 51,		// tuning: 0 default,  1-50 n-tone equal, 60-71 just
		equalTempered 	= 0,	// 0 is "normal" equal tempered scale
		oneToneEqual 	= 1,	// 1..50 select n-tone equal scales
		fiftyToneEqual 	= 50,	// do not use 51..59
		justC 			= 60,	// 60..71 select just tempered scales
		justB 			= 71,	// do not use 72..79
		firstUserTuning = 80,	// 80..87 are user grids
		lastUserTuning 	= 87,
		userTuningGrids = lastUserTuning - firstUserTuning + 1,
	ccMini_LB		= 59,		// two purposes: menu-diving options for Mini, and loopback detect:
								// only sent, never received								 9.08 [COVID19]
								// for loopback detect see also companion mechanism ccLoopDetect
		bDim			= 0x07,	// Mini dim = -6,-12,-18,-24dB
		bPedExt			= 0x18,	// set to use Mini's ped/ext jack for pedal / i2c / cv		 8.59
			pedX=0,i2cX,cvX,	// bPedExt field values: jack is for pedal / i2c / cv		 8.68
		bBrD			= 0x60,	// Mini display brightness
	ccActuation		= 70,		// sensor scanning modifications							 8.55
	ccPolyTrad		= 71,		// Continuum out: total _TRAD polyphony						 6.00
	ccPolyDsp		= 72,		// Continuum out: total _DSP polyphony						 6.00
	ccPolyCvc		= 73,		// Continuum out: total _CVC polyphony						 6.00
	ccTest			= 75,		// for stress test of Midi to/from Editor; data 64..127		 5.64 
	ccQBias			= 80,		// obsolete (saved but no longer does anything)				10.07
	ccLoopDetect	= 82,		// random pattern for loopback detect   also on *ch1*		 8.62 [RoanMtn]
								// see also companion mechanism ccMini_LB
	ccNrpnL  		= 98,		// low 7' nrpn select										 9.08 [NAMM]
	ccNrpnH  		= 99,		// high 7' nrpn select										 9.08 [NAMM]
	ccRpnL			= 100,		// low 7' rpn select										 7.38 [Fruita]
	ccRpnH			= 101,		// high 7' rpn select										 7.38 [Fruita]
	ccVersHi	 	= 102,		// firmware version top 7 bits (tx only)
	ccVersLo 		= 103,		// firmware version low 7 bits (tx only)
	ccCVCHigh 		= 104,		// 5/hardw,2/cvcSerialMsb (bits 15..14) (tx only)
		hw_fL=1,hw_hL=2,		// hardw field: light action full/half						 8.84
		hw_fC=3,hw_hC=4,		// hardw field: classic action full size and half size		 8.84
		hw_Mini=5,hw_o49=6,		// hardw field: ContinuuMini and Osmose						 8.84
		hw_s22=7,hw_s46,hw_s70,	// hardw field: slim 22L6x, slim 46L6x, slim70L6x			 9.08
		hw_EMM=10,hw_Micro=11,	// hardw field: EaganMatrix Module, EaganMatrix Micro		 9.70 10.34
	ccCVCMed 		= 105,		// cvc serial number med 7 bits (bits 13..7) (tx only)
	ccCVCLow 		= 106,		// cvc serial number low 7 bits (bits 6..0)  (tx only)
	ccTxSNBN 		= 107,		// for SNBN: once per second (tx only)						 3.26 3.39
		snbnModeBit		= 0x40,	// data is polyphony, plus top data bit indicates ccRxSNBN received	
	ccTask			= 109,		// task request or report
		// NB: tx to Editor's Center/Left Message Bars use report() or postReport(). 		 5.60
		// NB: "download" is firmware or data, "archive" is preset in .mid file; despite separate 
		//     messages for the two cases here, Editor Message Bar often uses same text for both.
		// NB: ccTask not stored within an "archive" (preset in .mid file); but some ccDInfo are.
		downloadOkBoot 	= 0,	// tx: center msg "Please Power Cycle"						 9.31 [COVID19]
		downloadFail 	= 1,	// tx: center msg "Operation Failed.  Try again."
		downloadOkChained=2,	// tx: center msg n/c (leave up "Download in Progress")		 5.60
		doneCopy		= 3,	// tx: copy operation has completed							10.34
		downloadOkData	= 4,	// tx: center msg "Data download completed."
		archiveOk		= 5,	// tx: erase message bar; at end of Preset Group
		archiveFail		= 6,	// tx: msg bar red "Failed"; File2 and User Preset Groups retry
		//				= 7,	//     retired Killamix query								10.10
		curGloToFlash	= 8,	// rx: write global/calib/Map/current to flash				 5.12
								//     Special case if Archive Retrieve: end of Retrieve
		reduceGain		= 9,	// tx: center msg "Reduce Gain"
		reducePoly		= 10,	// tx: center msg "Reduce Polyphony"
		inFactCalib		= 11,	// tx: center msg "Factory Calibration In Progress"			 6.38
		eraseMessage	= 12,	// tx: erase message bar
		aesSyncFail		= 13,	// tx: center msg "AES3 Sync Failure"  						 5.40
		cvcPower		= 14,	// tx: center msg "Turn On or Disconnect CVC"  1x dsp only	 5.40
		ceeMismatch		= 15,	// tx: firmware version mismatch between MultiDsps			 5.60
		configToMidi	= 16,	// rx: current config to Midi								 5.21
		startFirmware 	= 17,	// rx: begin firmware download
		startData		= 18,	// rx: begin synthesis data download
		burnUser364 	= 19,	// rx: done with firmware 1x3x download, burn flash (if crc ok)
		endDataDownload = 20,	// rx: end data firmware/data download
		midiLoopback	= 21,	// tx: Midi-loopback-detected error message
		contTxtToMidi	= 22,	// rx: preset's control text to Midi						10.36
		helloWorld		= 23,	// tx: sent once ~10 seconds after powerup					10.40
		begTxDsp		= 24,	// internal: begin config send    (tx dsp1, rx dsp2,dsp3)	 5.60
		endTxDsp		= 25,	// internal: end   config send    (tx dsp1, rx dsp2,dsp3)	 5.60
		doneTxDsp		= 26,	// internal: end   config receive (tx dsp2,dsp3, rx dsp1) 
		txDspFail		= 27,	// tx: config send txDsp fail, log in Max window
		doUpdF2			= 28,	// tx: did UpdateFile1 reboot, "Do Update File 2"
								// NB: This is also used by Osmose.
		createLed		= 29,	// rx: turn on yellow LED for archive create
		testBegin		= 30,	// rx: begin Midi stress test
		testErr			= 31,	// tx: error in Midi rx sequence
		userToMidi		= 32,	// rx: uPreset names to Midi, then current config			 6.19
		manualUpdate	= 33,	// tx: old preset requires manual update 					 6.20
		doResetCalib	= 34,	// rx: replace at-rest sensor levels						 6.38
		doRefineCalib	= 35,	// rx: incorporate new at-rest								 6.38
		midiTxFull		= 36,	// rx: use full midi transmission rate						 6.41
		midiTxThird		= 37,	// rx: use one-third midi transmission rate 				 6.41 7.57 8.55
		midiTxTweenth	= 38,	// rx: use one-twentieth midi transmission rate 			 6.41 7.57 7.59
		sysToMidi		= 39,	// rx: sysPreset names to Midi, then current config			 6.19
		endSysNames		= 40,	// tx: end list of system presets				  			 9.70
		doFactCalib		= 41,	// rx: do factory calibration
		doUpdate1		= 42,	// tx: booted up in recovery mode, "Do Update File 1"
								// NB: This is also used by Osmose to detect Recovery Mode.
		burnUser489 	= 43,	// rx: done firm 2x6x download, write flash (if crc ok)
		rebootUpdF1		= 44,	// rx: reboot after UpdateFile1 (if all ok)					 7.56
		surfAlign		= 45,	// rx: toggle slim's Surface Alignment mode 				 9.08
		addTrim			= 46,	// rx: add currently-playing finger to Pitch Trim array		 7.57
		delTrim			= 47,	// rx: remove Pitch Trim point nearest to finger			 7.57
		resetTrim		= 48,	// rx: remove all Pitch Trim data  							 7.57
		beginSysNames	= 49,	// tx: begin list of system presets 			 			 9.70
		//				= 50,	//     retired exitCombi									10.10 [SuprBth]
		storeFactSetup	= 51,	// rx: copy calib/global/userPre to factory setup backup
		decPreset		= 52,	// rx: go to prev sysPreset									 8.59 [SuprBth]
		incPreset		= 53,	// rx: go to next sysPreset									 8.59 [SuprBth]
		beginUserNames	= 54,	// tx: begin list of user presets 			 				 7.78
		endUserNames	= 55,	// tx: end list of user presets				  				 7.78 9.70
		clearTopo		= 56,	// rx: clear topology information for thick continuums		10.35
		preEraseUpdF1	= 57,	// rx: erase flash UpdFile1; dsp1 echoes after				 9.74
		preEraseUpdF2	= 58,	// rx: erase flash UpdFile2; dsp1 echoes after				 9.74
		preEraseUpdF3	= 59,	// rx: erase flash UpdFile3; dsp1 echoes after				 9.74
		remakeSRMahl	= 60,	// rx: remake synthesis-ready Mahling data, tx: done		 9.83
		doneFactProg	= 61,	// tx: done copying to/from Factory Program Board 			 9.91
		failFactProg	= 62,	// tx: failed copy to/from  Factory Program Board 			 9.91
		usbTxNoAck		= 63,	// tx: Usb-Midi out from Mini did not get Ack				 8.70
		rxOver 			= 64,	// tx: Midi rx queue overflow								 8.50
		txOver 			= 65,	// tx: Midi tx queue overflow								 8.50
		rxSynErr 		= 66,	// tx: Midi rx syntax error									 8.50
		rxBitErr 		= 67,	// tx: Midi rx bad bit widths								 8.50
		sensComm		= 68,	// tx: serial sensors errors								 8.50
		nanErr			= 69,	// tx: output has nan										 8.50
		ceeSeq			= 70,	// tx: multi-dsp comm glitch								 8.50
		burnUserMini	= 71,	// rx: done Mini firmware download, write flash
		doMidiLog0		= 72,	// rx: end scrolling ascii log via Midi						 7.40 8.81
		doMidiLog1		= 73,	// rx: dsp1 scrolling ascii log via Midi					 7.40 8.81
		doMidiLog2		= 74,	// rx: dsp2 scrolling ascii log via Midi 					 7.40 8.81
		doMidiLog3		= 75,	// rx: dsp3 scrolling ascii log via Midi 					 7.40 8.81
		burnRecovery489 = 76,	// rx: done recovery firm 1x3x download, write flash
		burnRecovery364 = 77,	// rx: done recovery firm 2x6x download, write flash
		burnRecoveryMini= 78,	// rx: done recovery firm Mini download, write flash
		loadsToMidi 	= 79,	// rx: do "configToMidi" for future preset loads			10.10
		defFirstTuning	= 80,	// rx: begin first user tuning grid; ccDataL data follows 
			// 80..87 are for defining user tuning grids					 
		defLastTuning 	= 87,	// rx: begin last  user tuning grid; ccDataL data follows 
		numDecMat		= 88,	// rx: decrement numeric matrix point						 9.70
		numIncMat		= 89,	// rx: increment numeric matrix point						 9.70
		mendDisco		= 90,	// rx: mend discontinuity at note (Sensor Map)				 9.74
		rebootRecov		= 91,	// rx: reboot in Recovery Mode, no delay					 9.81
		stageUp			= 92,	// rx: upload monolithic update								 9.82
		stageDown		= 93,	// rx: download monolithic update							 9.82
		stageDownOk1	= 94, stageDownOk2, stageDownOk3,		// tx: at end of StageDown
		stageDownFail1	= 97, stageDownFail2, stageDownFail3,	// tx: at end of StageDown
		rebootFinaliz	= 100,	// rx: reboot in 10s, led matrix "finalize"					 9.82
		gridToFlash 	= 101,	// rx: save tuning grids to flash
		mendDivided		= 102,	// rx: mend divided note (add to Sensor Map)				 9.74
		startUpdF2		= 103,	// rx: beginning of Update File 2							 9.70 [COVID19]
		//				= 104,	//     notFirst combi retired								10.10 [SuprBth]
		pullConv		= 105,	// internal: get convolution data (DSP593 tx dsp1, rx dsp2)	10.39
		pullSSet		= 106,	// internal: get spectral set     (DSP593 tx dsp1, rx dsp2)	10.39
		Demo1			= 107,	// rx: Assortment to first set-of-16 user presets.			 9.70 [COVID19]
			// 107..114 are for loading demo assortment into user presets
		Demo8			= 114,	// rx: Assortment to last set-of-16 user presets.			 9.70 [COVID19]
		Empty1			= 115,	// rx: Empty to first set-of-16 user presets.				 9.70 [COVID19]
			// 115..122 are for emptying set-of-16 user presets
		Empty8			= 122,	// rx: Empty to last set-of-16 user presets.				 9.70 [COVID19]
		burnUser593 	= 123,	// rx: done firmware 8x download, write flash (if crc ok)
		burnRecovery593 = 124,	// rx: done recovery firm 8x download, write flash
		rebootUser		= 127,	// rx: reboot in User (normal) Mode, no delay				10.33
	ccDInfo			= 110,		// for Continuum Editor: additional info for download
		// NB: ccTask not stored within an "archive" (preset in .mid file); but some ccDInfo are.
		profileEnd		= 0,	// tx: erase Editor's Message Bar, Editor puts up Save dialog
		percentFirst	= 1,	// tx: left msg "1%"  in Editor's Message Bar
		percentLast		= 99,	// tx: left msg "99%" in Editor's Message Bar
		cfCreateArch0	= 100,	// rx: start archive *creation* (preset to archive)		 5.12
								//     value 100 selects edit buffer (userPreset0) as source
		cfCreateArch1	= 101,	// rx: start archive *creation* (preset to archive)
								//     value 101..116 select within current set-of-16 user presets
		cfCreateArch16	= 116,	// rx: start archive *creation* (preset to archive)	
		inProgress		= 118,	// tx: center msg "Download in progress. Please wait."
		archiveNop		= 119,	// rx: no-op, used as padding in midi files
		edRecordArchive	= 120,	// tx: sent to Editor when cfCreateArchive is received;
								//     Editor starts recording (which excludes edRecordArchive)
		cfRetrieveArch  = 121,	// tx: sent to Editor along with edRecordArchive -- 
								// rx: played back from Editor at start of *retrieval* from archive;
								//     Editor itself does not use this (except to save in archive)
		archiveEof		= 123,	// tx: last data sent to Editor in archive create; Editor does not
								// rx: echoed back from Editor as last data in file during *retrieval* 
								//     -- Continuum verifies message count
								//     (but does not exit archRetrieve until later at curGloToFlash)
		archiveToFile	= 124,	// tx: archive *creation*: Editor stops recording, saves to file system
								// rx: archive retrieval:  Continuum does nothing with this
		finalizing		= 125,	// tx: left msg "finalizing flash"
		//				= 126,	// tx: left msg "initializing" [no longer used, except in old archives]
		profileStart	= 127,	// tx: center msg "Profile is being generated. Please wait."
	ccEdState		= 111,		// Continuum state info (to Editor) 						 3.47
		sLedBits		= 0x0f,	// select led color bits
		  ledOff		  = 0,	// led color
		  ledBlue		  = 1,	// led color
		  ledRed		  = 2,	// led color
		  ledBrightGreen  = 3,	// led color
		  ledGreen		  = 4,	// led color
		  ledWhite		  = 5,	// led color 				 calib		
		  ledYellow		  = 6,	// led color				 download, etc
		  ledPurple		  = 7,	// led color				 no aes sync, or flashing if cvc0 power off
		  ledBlueGreen	  = 8,	// led color				 [retired from flawed sensor]	 4.02
		sAesBits		= 0x70,	// aes detected rate bits
		sAesShift		= 4,	// shift for aes detected bits
		  aesInputNone	  = 0,	// no aes input stream
		  aesInputNonStd  = 1,	// aes input nonstandard rate
		  aesInput44p1	  = 2,	// aes input  44.1 kHz										 4.15 [Ottawa]
		  aesInput48	  = 3,	// aes input  48.0 kHz
		  aesInput88p2	  = 4,	// aes input  88.2 kHz
		  aesInput96	  = 5,	// aes input  96.0 kHz
		  aesInput176p4	  = 6,	// aes input 176.4 kHz
		  aesInput192	  = 7,	// aes input 192.0 kHz
		  #define StdRate 44100,48000,88200,96000,176400,192000
	ccRxSNBN 		= 113,		// Continuum rx: response to ccTxSNBN						 3.26
								// sets poly, multi-split, and Midi special encoding:
								// high channel number hack, assume bend 96, default y z cc
	cc_txUsg_rxDis  = 114, 		// Tx: Processing usage; Rx: Utility sensor-display mode
								// Tx: 32*dsp# + 4%units (0-25)								 9.08 [COVID19]
		drawAttacks 	= 1, 	// rx sensor utility display mode: "Start of Notes"
		drawNormalized	= 2,	// rx sensor utility display mode: "Continuous"
		drawDualRaw		= 3,	// rx sensor utility display mode: "Raw Sensors"
		drawDualZoomRaw	= 10,	// rx sensor utility display mode: "Zoom Raw Sensors"
		drawDualZoomNxt = 9,	// rx sensor utility display mode: "Zoom Next Start"
		drawDualNorm	= 4,	// rx sensor utility display mode: "Normalized Sensors"
		drawDualSumDiff	= 7, 	// rx sensor utility display mode: "Difference and Sum"
		drawDualBarZ	= 8,	// rx sensor utility display mode: "Intermediate and Final"
		drawDualHist	= 5, 	// rx sensor utility display mode: "Midi History Z,-Y"
		drawDualHist14	= 6, 	// rx sensor utility display mode: "Midi History Zhi,Zlo"
	ccLogDump 		= 115, 		// begin or continue continuum log dump
		ordBits			= 0x03,	// 0 starts, 1..3 role.ord request for next block of data in log
		//				= 0x7c,	// available (nonzero to avoid initiating log dump)
	ccEditor 		= 116,		// Continuum input: Continuum Editor is out there 			 3.33
	ccEditorReply	= 118,		// Respond to ccEditor LightAct,doubleSR,3/ProcPow,SNBN,edCvcMatrix
		edsCvcMatrix	= 0,	// bit 0: cvc determined by EaganMatrix						 5.20 7.44
		edsSNBN			= 1,	// bit 1: SNBN active
		edsProcPow		= 2,	// bits 2-4: 3-bit procPowMult: 1..6=1x..6x, 7=8x			 7.82
		  edmProcPow	= 0x1c,	// bits 2-4 mask
		edsDoubleSR		= 5,	// bit 5: doubleSR active									 8.24
		//edsLightAction= 6,	// bit 6: available
	ccArchCnt		= 119,		// count of controller messages in archive definition
	//ccSoundOff	= 120,		// ccSoundOff is special, cannot use for data communications
	//				= 121		   
	ccCRC0			= 122,		// internal dsp1 -> dsp2,dsp3: 7 lsbs of config CRC			 6.20
	ccCRC1			= 123,		// internal dsp1 -> dsp2,dsp3: next 7 bits of config CRC
	ccCRC2			= 124,		// internal dsp1 -> dsp2,dsp3: next 7 bits of config CRC
	ccCRC3			= 125,		// internal dsp1 -> dsp2,dsp3: next 7 bits of config CRC
	ccCRC4			= 126,		// internal dsp1 -> dsp2,dsp3: 4 msbs of config CRC
	//ccMPE			= 127,		// ccMPE is special, cannot use for data communications


// -------------------------------------- ccStream: Select a stream -------------------------------------
	ccStream		= 56,		// select a stream for ch16 "sData" that follow				 6.16 [NAMM]
		// Midi order for sData streams:    (stream = s_Xxx defined below)
		//	 ccStream stream; sData,sData,...sData; ccStream s_StreamEnd	(*even* number of sData bytes)
		// Midi order for sData pokes:		(stream_Poke = s_Xxx_Poke defined below)
		//	 ccStream stream_Poke; sData,sData; [sData,sData; ...]			(sData pairs = id#,value)
		// Two sData 7' bytes are transmitted by each Midi ch16 polyKeyPres message.
		s_Name			= 0,	// Ascii Preset Name						    (max  32 sData)
								//   zero-pad if odd number of ascii characters
		s_ConText		= 1,	// Ascii Preset Control Text					(max 256 sData)
								//   zero-pad if odd number of ascii characters
		s_Graph			= 2,	// Main Graph (48 values of range 0..96)		(max  48 sData)
		  ini_dB0		  = 96,	// s_Graph: 0dB amplitude value						 		 5.61
		s_GraphO1		= 3,	// 1st Offset Graph (48 values of range 0..96)	(max  48 sData)
		s_GraphO2		= 4,	// 2nd Offset Graph (48 values of range 0..96)	(max  48 sData)
		  off_dB0		  = 48,	// s_GraphO1 and s_GraphO2: 0dB amplitude value				 5.61
		s_GraphT0		= 5,	// Up to 16 Pitch Trim Y=0 Points			    (max  48 sData)
		s_GraphT1		= 6,	// Up to 16 Pitch Trim Y=1 Points			    (max  48 sData)
								// Each s_GraphT0/s_GraphT1 Trim point is three sData:
								//   nn  note number, multiple of 6, increasing order, 0 terminates
								//   lo  least significant 7' of cents offset
								//   hi  most  significant 7' of cents offset; 14' offset 0x2000
		s_Log			= 7,	// Hardware Log line of ascii text (to Editor)
								//   zero-pad if odd number of ascii characters
		s_UpdF2			= 8,	// Rx before Update File 2; #sysPresets			(2  sData)
		s_DemoAssort	= 9,	// Rx end of Update File 2; Demo Assortment		(34 sData)
								// 17 catSSlot values, for EmptyPreset then 16 demo assortment
		s_Float			= 10,	// Floating Debug Value							(6  sData)
								// 32' IEEE float, 6' per sData, lsb 1st, "finger down" bit above msb
		s_Kinet			= 11,	// Kinetic Properties							(max 48 sData)
								// >> see s_Kinet_Poke section below for stream's sData ordering
		s_BiqSin		= 12,	// BiqBank/SineBank Properties					(max 48 sData)
								// >> see s_BiqSin_Poke section below for stream's sData ordering
		s_Sys			= 13,	// Tx System Info:								(20 sData)
								//   2 sData: Count of System Presets
								//   4 sData: SysPresets Sum Check
								//   1 sData: % slots
								//   1 sData: % SysPre data
								//   4 sData: 28' Synthesis Data Sum Check, dsp1
								//   4 sData: 28' Synthesis Data Sum Check, dsp2
								//   4 sData: 28' Synthesis Data Sum Check, dsp3
		s_Conv			= 14,	// Convolution Parameters						(max 30 sData)
								// >> see s_Conv_Poke section below for stream's sData ordering
		s_Thumb			= 15,	// Tx Thumbnails:								(sets of 4 sData)
								//   1: formula number 1..48
								//   2: lowest macro 0..89 formula, 127 if none				10.30 [SF]
									    thNoMacr=127,
								//   3: SG/Tap in low 4', then 3' idFormOp value			10.30 [SF]
			                            thSg1=1,thSg5=5,thTap1=10,thTap4=13,thOpShift=4,
								//   4: dimming bits and friend-of-selected bit				 6.17 10.30
									    thDimZ=1,thDimY=2,thDimX=4,thDimW=8,thFriend=32,thDimOp=64,
		s_MacroHi		= 16,	// High 7' of Macros i..vi,m7..m90				(max 90 sData)
								// access via ccFracPed/ccFracPedEx with ccI..ccVI,ccM8..ccM48
								// for encoding of i..vi see "max14", for m7..m90 see "zero14"
		s_MacroLo		= 17,	// Low  7' of Macros i..vi,m7..m90   			(max 90 sData)
								// access via ccFracPed/ccFracPedEx with ccI..ccVI,ccM8..ccM48
		s_MacrUses		= 18,	// Internally used to respond to ccMacrUses (to Editor)
		s_Form_Poke		= 19,	// Formula Configuration poke					(id,value pairs)
								// >> see s_Form_Poke section below for id assignments		10.32
		s_Mat_Poke		= 20,	// Matrix Configuration poke					(id,value pairs)
								// >> see s_Mat_Poke section below for id assignments		10.32
		s_Graph_Poke	= 21,	// Graph poke     (id 0..47, value 0..96)		(id,value pairs)
		s_GraphO1_Poke	= 22,	// First Offset Graph poke	(0..47,0..96)		(id,value pairs)
		s_GraphO2_Poke	= 23,	// Second Offset Graph poke	(0..47,0..96)		(id,value pairs)
		s_Kinet_Poke	= 24,	// Kinetic property poke						(id,value pairs)
								// >> see s_Kinet_Poke section below for id assignments
		s_BiqSin_Poke	= 25,	// BiqBank/SineBank property poke				(id,value pairs)
								// >> see s_BiqSin_Poke section below for id assignments
		s_Conv_Poke		= 26,	// Convolution parameters poke					(id,value pairs)
								// >> see s_Conv_Poke section below for id assignments
		s_StreamEnd		= 127,	// Marks end of sData stream (not required with stream_Poke)


// ------------------------------------- s_Form_Poke id Assignments -------------------------------------
// Assignments of id for s_Form_Poke; these configure formula parameters. 
// These affect the formula selected by ccFormSel; ccFormSel must precede use of id below. 
// For more information, see ccStream and s_Form_Poke definition above.
	// Formula's operator	   
	idFormOp		= 38,		// formula's operator
		fAdd			= 0,	// W+X+Y+Z
		fWMul			= 1,	// W*(X+Y+Z)
		fZMul			= 2,	// (W+X+Y)*Z
		fPairMul		= 3,	// (W+X)*(Y+Z)
		fPairAdd		= 4,	// (W*X)+(Y*Z)
	// W subcomponent of the formula
	idSgW			= 40,		// W shape generator selection, must precede idHeightW
		wSgConstant		= 0,	// always 1          (idShapeW not used)
		wSgGated		= 1,	// 1 if finger down  (idShapeW not used)
		wSg1			= 2,	// Shape Generator 1 (idShapeW specifies SG shape)
		wSg2			= 3,	// SG 2
		wSg3			= 4,	// SG 3
		wSg4			= 5,	// SG 4
		wSg5			= 6,	// SG 5
		wSg1b			= 7,	// SG 1b (for dualSG)										 8.22 
		wSg2b			= 8,	// SG 2b
		wSg3b			= 9,	// SG 3b
		wSg4b			= 10,	// SG 4b
		wSg5b			= 11,	// SG 5b
		wDelay			= 12,	// FDelay tap        (idShapeW specifies tap#)				 5.40
		wRMS			= 13,	// RMS value of matrix row (moving window 8 ms)				 9.84
	idShapeW		= 50,		// accessory for idSgW: select delay tap# or sg shape
		wLastTap		= 3,	// clip value if sg shape is selecting tap; 4 FDelay taps total
		wRampUp			= 0,	// sg shape: ramp 0..1
		wRampDn			= 1,	// sg shape: ramp 1..0
		wPulse			= 2,	// sg shape: pulse (1 on, 0 off)
		wPulseEnd		= 3,	// sg shape: pulse at end        							 7.80
		wTriangle		= 4,	// sg shape: triangle (0..1..0)	  							 4.37 [20Anniv]
		wHann			= 5,	// sg shape: Hann (raised cosine)
		wGentleUp		= 6,	// sg shape: S with gentle start
		wSteepUp		= 7,	// sg shape: S with steep start
		wGentleDn		= 8,	// sg shape: S with gentle start
		wSteepDn		= 9,	// sg shape: S with steep start
		wSquare			= 10,	// sg shape: square (1 then -1)
		wSine			= 11,	// sg shape: sine (-1..1)
		#define			ENDHI(a) ((a)==wRampUp || (a)==wGentleUp || (a)==wSteepUp)
		wSampHold		= 12,	// sg shape: sample&hold matrix row 						 4.36 [Sylvnia]
								// Note: s&h n/a for dualSG; contCycle or singleCycle.		 8.22
		wSampHoldA		= 13,	// sg shape: sample&hold formula A 							 4.38 [Tifton]
		wSampHoldV		= 34,	// sg shape: sample&hold formula V 							 6.37
		wSampHoldA_		= 35,	// sg shape: sample&hold formula A'							10.22 [SF]
		wSampHoldV_		= 56,	// sg shape: sample&hold formula V'
	idSgM			= 117,		// accessory for idSgW: sg phase modulation  				 5.26 [Toronto]
		sg90=1,sg180,sg270,sgModA,sgModV=25,sgModA_,sgModV_=47,
	idNegW			= 13,		// true for -idHeightW; no effect until idHeightW received
	idHeightW		= 42,		// height for W		    with idNegW -1.00..1.27 in 0.01 steps
	idMultW			= 41,		// multiplier for W (order-of-magnitude: m0p001..m1000p0)
		m0p001			= 0,	// multiply by 0.001 for idMultW,idMultX,idMultY,idMultZ
		m0p01			= 1,	// multiply by 0.01
		m0p1			= 2,	// multiply by 0.1
		m1p0			= 3,	// multiply by 1.0
		m10p0			= 4,	// multiply by 10.0
		m100p0			= 5,	// multiply by 100.0
		m1000p0			= 6,	// multiply by 1000.0
		mavail			= 7,	// multiplier available (last one in 3-bit field)
		#define			FormMults .001,.01,.1,1,10,100,1000,1 // for idMultW/X/Y/Z
	idScaleW		= 66,		// 
		wNone			= 0,	// no scaler (1.0)
		wMacro			= 1,	// scale with macro idSWMacr; result=0..1 (i..vi) or -1..1 (m7..m90)
		wMacro127		= 2,	// scale with macro idSWMacr; result=0..127 (i..vi) or -127..127
		wDivFings		= 3,	// 1 / fingersDown    										 8.73
		wVoiceNum		= 4,	// scale by current voice number (1..n)
		wIfVoice1		= 5,	// 1 if voice 1, else 0
		wDivPoly		= 6,	// 1 / thisDSPpoly  										 7.60
	idSWMacr		= 67,		// accessory for idScaleW: macro select (0..95)  			10.22
	idLoW			= 118,		// low nn for W  											 5.25 [Intrlcn]
	idHiW			= 119,  	// high nn for W 											 5.25
	idAllW			= 124,		// true for w Zone repeats every octave  					 8.10 8.22
	idInitW		= 125,			// true for idHiW/idLoW based on initial pitch 				 5.25 [Intrlcn]
	// X subcomponent of the formula
	idInitX			= 4,		// cContinuous, cInitial, cDerivative, cRelative
	idNegBegX		= 15,		// true for -idVBegX; no effect until idVBegX received
	idVBegX			= 45,		// beginning X (vertical)   0..100; with idNegBegX -1..1 in 0.01 steps
	idNegEndX		= 17,		// true for -idVEndX; no effect until idVEndX received
	idVEndX			= 46,		// ending X    (vertical)   0..100; with idNegEndX -1..1 in 0.01 steps
	idHBegX			= 43,		// beginning X (horizontal) 0..12;  end of left shelf
		hMax			= 12,	// max value for hBegX,hEndx,hBegY,hEndY,hBegZ,hEndZ
		#define			inv_pMax  0.0833333333
	idHEndX			= 49,		// ending X    (horizontal) 0..12;  start of right shelf
	idTransX		= 85,		// X transfer function
		xOctaves=0,xKHz,xLinear,xS,xSquared,xSqRoot,xTwoStep,xThreeStep,xGraph,xOff1,xOff2,
	idMultX			= 1,		// multiplier for X (order-of-magnitude: m0p001..m1000p0)
	idXMacr			= 27,		// macro assignment for X component of formula (0 none, or 1..96)
		xyzNoM=0,xyzI=1,		// 0 = no macro, 1..90 = i..vi,m7..m90; for idXMacr/idYMacr/idZMacr
	idZeroX			= 44,		// note number for zero X component (60=middleC is default)
	idQuantX		= 112,		// x quantization
		#define			Quant 0,1,2,3,4,5,7,12,24,36,48,96 // list of values for idQuantX
	// Y subcomponent of the formula
	idInitY			= 5,		// cContinuous, cInitial, cDerivative, cRelative 			 5.01 7.19 8.21
	idNegBegY		= 19,		// true for -idVBegY; no effect until idVBegY received
	idVBegY			= 51,		// beginning Y (vertical)   0..100; with idNegBegY -1..1 in 0.01 steps
	idNegEndY		= 21,		// true for -idVEndY; no effect until idVEndY received
	idVEndY			= 52,		// ending Y    (vertical)   0..100; with idNegEndY -1..1 in 0.01 steps
	idHBegY			= 53,		// beginning Y (horizontal) 0..12;  end of left shelf
	idHEndY			= 54,		// ending Y    (horizontal) 0..12;  start of right shelf
	idTransY		= 55,		// transfer function and polarity (see values below)
		yzLinear=0,yzS,yzSquared,yzSqRoot,yzTwoStep,yzThreeStep,yzGraph,yzOff1,yzOff2,
	idMultY			= 2,		// multiplier for Y (order-of-magnitude: m0p001..m1000p0)
	idYMacr			= 28,		// macro assignment for Y component of formula (0 none, or 1..96)
	// Z subcomponent of the formula
	idInitZ			= 6,		// see definitions below									 7.18 8.20 8.21
		cContinuous		= 0,	// definitions for idInitX/idInitY/idInitZ
		cInitial		= 1,	   
		cDerivative		= 2,	// 															 7.19
		cRelative		= 3,	// for formula's X and Y components only
		cReleaseF		= 3,	// for formula's Z component only 							 8.20
		cReleaseP		= 4,	// for formula's Z component only 							 8.21
		cContinuousNT	= 4,	// for formula's X component only - no trim	X				10.36
		cInitialNT		= 5,	// for formula's X component only - no trim	X				10.36
		cDerivativeNT	= 6,	// for formula's X component only - no trim	X				10.36
		cRelativeNT		= 7,	// for formula's X component only - no trim	X				10.36
	idNegBegZ		= 23,		// true for -idVBegZ; no effect until idVBegZ received
	idVBegZ			= 58,		// beginning Z (vertical)   0..100; with idNegBegZ -1..1 in 0.01 steps
	idNegEndZ		= 25,		// true for -idVEndZ; no effect until idVEndZ received
	idVEndZ			= 59,		// ending Z    (vertical)   0..100; with idNegEndZ -1..1 in 0.01 steps
	idHBegZ			= 60,		// beginning Z (horizontal) 0..12;  end of left shelf
	idHEndZ			= 61,		// ending Z    (horizontal) 0..12;  start of right shelf
	idTransZ		= 62,		// transfer function (see values defined at idTransY)
	idMultZ			= 3,		// multiplier for Z (order-of-magnitude: m0p001..m1000p0)
	idZMacr			= 29,		// macro assignment for Z component of formula (0 none, or 1..96)
	// Ancillary Formula
	idAncOp			= 56,		// first ancillary operation, values listed below			 5.11 7.04
	idAncOp2		= 100,		// second ancillary operation, values listed below		 	 9.73
		aMul			=  0,	// multiply by ancillary value
		aAbsMul			=  1,	// absolute value then multiply by ancillary value
		aInvMul			=  2,	// invert then multiply by ancillary value
		aDivide			=  3,	// divide by ancillary value
		aAdd			=  4,	// add ancillary value
		aSub			=  5,	// subtract ancillary value
		aPow			=  6,	// to power of ancillary value
		aLog			=  7,	// log base ancillary value
		aMod			=  8,	// modulo ancillary value
		aQuant			=  9,	// quantize by ancillary value
		aMin			= 10,	// result is never more than ancillary value
		aMax			= 11,	// result is never less than ancillary value
		aCross			= 12,	// crossing detect: below ancillary value to above = +1,
								// 				    above ancillary value to below = -1
	idAncVal		= 57,		// selects first ancillary value					 		 5.11 10.31
	idAncVal2		= 101,		// selects second ancillary value							 9.73 10.31
		a0p0			=   0,	// ancillary value is 0.0
		a0p25			=   1,	// ancillary value is 0.25
		a0p5			=   2,	// ancillary value is 0.5
		a1p0			=   3,	// ancillary value is 1.0
		a1p5			=   4,	// ancillary value is 1.5
		a2p0			=   5,	// ancillary value is 2.0
		a3p0			=   6,	// ancillary value is 3.0
		a4p0			=   7,	// ancillary value is 4.0
		a5p0			=   8,	// ancillary value is 5.0
		a10p0			=   9,	// ancillary value is 10.0
		aA				=  10,	// ancillary value is formula A
		aV				=  31,	// ancillary value is formula V
		aA_				=  32,	// ancillary value is formula A'
		aV_				=  53,	// ancillary value is formula V'
		aI				=  54,	// ancillary value is macro i   (0..1)
		aVI				=  59,	// ancillary value is macro vi  (0..1)
		aM7				=  60,	// ancillary value is macro m7  (-1..1)
		aM72			= 125,	// ancillary value is macro m72 (-1..1) last macro choice in ancVal
		aTimeCk			= 126,	// ancillary value is Midi time clock
	idAncDom		= 7,		// domain of Ancillary Formula, values listed below
		aWXYZ=0,aW,aX,aY,aZ,
	idAncTAorAll	= 11,		// false = normal, true = Touch Area / All Fingers; required in Master
	idAncLookup		= 8,		// use first ancillary result to index lookup; then do second ancillary
		aLookNo=0,aLookGrph,aLook1st,aLook2nd,aLookGrph48,aLook1st48,aLook2nd48,
		#define			LookGrph(a) (((a) - 1) % 3)  // 0 = Graph, 1 = Offset Graph 1, 2 = Offset Graph 2
	idAncNN			= 10,		// false = normal, true = do conversion from kHz to nn
	idAncLimit		= 9,		// false = normal, true = engage matrix column limiter
	// Interpolation, Persistence, Reluctance   
	idInterp		= 48,		// interpolation value 0..100 								 4.37 [20Anniv]
		interpDef		= 40,	// default interpolation
	idPersist		= 63,		// formula persistence 0..100 								 4.36 [Sylvnia]
	idReluct		= 96,		// formula reluctance 0..100 								10.22
	// Blend.					   
	idBlend			= 39,		// blend control for primary/secondary formula values		 6.10 10.22
		// idBlend values:		   (NB: psForm and psMacro use value of idBlMacForm)
		ps0=0,ps1,psForm,psMacro,psCh1Note,psDuplex,psSus,psSos,psSos2,
	idBlMacForm		= 68,		// accessory for idBlend: macro 0..95 or formula 0..44		10.22
	idHBegBl		= 121,		// beginning blend (horizontal) 0..12;  end of left shelf
	idHEndBl		= 123,		// ending blend    (horizontal) 0..12;  start of right shelf
	idTransBl		= 12,		// blend transfer function (see values defined at idTransY)
	// Secondary Formula Values (used when Blend active)				   
	idNegW2			= 14,		// true for -idHeightW2; no effect until idHeightW2 received
	idHeightW2		= 73,		// secondary idHeightW										 6.19
	idNegBegX2		= 16,		// true for -idVBegX2; no effect until idVBegX2 received
	idVBegX2		= 75,		// secondary idVBegX  									 6.19
	idNegEndX2		= 18,		// true for -idVEndX2; no effect until idVEndX2 received
	idVEndX2		= 76,		// secondary idVEndX
	idNegBegY2		= 20,		// true for -idVBegY2; no effect until idVBegY2 received
	idVBegY2		= 77,		// secondary idVBegY
	idNegEndY2		= 22,		// true for -idVEndY2; no effect until idVEndY2 received
	idVEndY2		= 78,		// secondary idVEndY
	idNegBegZ2		= 24,		// true for -idVBegZ2; no effect until idVBegZ2 received
	idVBegZ2		= 79,		// secondary idVBegZ
	idNegEndZ2		= 26,		// true for -idVEndZ2; no effect until idVEndZ2 received
	idVEndZ2		= 80,		// secondary idVEndZ
	idPersist2		= 81,		// secondary idPersist
	idInterp2		= 82,		// secondary idInterp
	idReluct2		= 97,		// secondary idReluct		 								10.22


// ------------------------------------- s_Mat_Poke id Assignments --------------------------------------
// Assignment of id for s_Mat_Poke; these configure matrix / preset parameters.
// For more information, see ccStream and s_Mat_Poke definitions above.
	// Polyphony
	idPoly		 	= 39,		// base polyphony (lowest common denominator 2x dsp), values 1..8
								// See also idOkExpPoly										10.32
		maxMpePoly		= 15,	// max poly for MPE: not notes on mpeMastChan 				10.34 [Bday]
		maxMidiPoly		= 16,	// number of Midi channels limits number of voices
		emVoices		= 8, 	// max EaganMatrix voices per dsp (see "DSP489 emVoices=4.jpg")
		maxPoly8x		= 16, 	// 8x systems have two dsp (C1 and C2), 8 voices each		10.10
		scale8xDSP		= 2,	// per-CORE base polyphony multiplier for 8x systems
		maxPoly6x		= 24, 	// 6x systems have three dsp, 8 voices per dsp				 8.81
		scale6xDSP		= 1,	// per-dsp base polyphony multiplier
	idOkExpPoly		= 11,		// true to use scaled-up base polyphony when possible		 7.04
	idOkIncComp     = 5,		// true to allow increased computation rate (affects poly)
	// Midi encoding
	idBendRange 	= 40, 		// bend range
		defBendRange	= 96,	// default bend range is max; good for surface display in Editor
		maxBendRange 	= 96,	// max bend always used internally independent of ccBendRange 
								// values > 96 select MPE+ ch1's bend range					 8.60
		#define				ch1Bend(b) MAX(1,(b)-maxBendRange+1)
		#define				inv_maxBendRange 0.0104166667
	idFrontBack 	= 41, 		// select controller number for front-back position
		xmitNoY			= 0,	// do not transmit y info
		xmitNoShelf		= 127,	// avoid shelf on y, use defaultYcc
		defaultYcc = ccBrightness,
	idPressure		= 42, 		// select controller for finger pressure; 127 to use channel pressure
        xmitNoZ		 	= 0,	// do not transmit z info									 5.40
		useChanPr		= 69,	// use channel pressure to encode Z (but do not use MPE)	 7.56
		useMPEplus		= 70,	// MPE+: our 14-bit MPE, externally remap channels to n+1	 7.44
								// chanPres, cc74 (Ymsb), cc87 (ccFrac)						 8.59 [Paris]
								// MPE+ on ch1 channelized to DSP, see psCh1Note.			 8.60
		useMPE		 	= 127,	// MPE: channel pressure for Z, externally remap channels to n+1 [7.44]
	// Mono function and voice assignment
	idMonoFunc		= 46,		// mono function (fingers combine to make single pitch)		 1.82
		MonoPortZ		= 0,	// pressure-based pitch		Mini substitutes MonoLegatoT	 8.65
		MonoLegatoZ		= 1,	// 							Mini substitutes MonoLegatoT	 8.65
		MonoRetrigZ		= 2,	// 							Mini substitutes MonoLegatoT	 8.65
		MonoLegatoT		= 3,	// time-based pitch, use most recent finger's Z
		MonoRetrigT		= 4,	   
		MonoRetrigTL	= 5,	   
		MonoLegatoTZ	= 6,	// time-based pitch, use max finger's Z
		MonoRetrigTZ	= 7,
		MonoRetrigTLZ	= 8,
		#define				isRetrig(mf)  ((0b110110100 >> (mf)) & 1) // 8,7,5,4,2 retrig	 8.10
		#define				isRetrigL(mf) ((0b100100100 >> (mf)) & 1) // 8,5,2 retrig lift	 8.10
		#define				isSmooth(mf)  ((0b001001011 >> (mf)) & 1) // 6,3,1,0 are smooth
	idMonoInt		= 48,		// set mono interval
	idPrio			= 6,		// voice-assignment priority rule
		LRU				= 0,	// least recently, used
		LRR				= 1,	// least recently used with repeated pitch reuse			 6.20
		LCN				= 2,	// lowest channel number
		HIGH1			= 3,	// use only highest channel
		HIGH2			= 4,	// use only 2 highest channels
		HIGH3			= 5,	// use only 3 highest channels
		HIGH4			= 6,	// use only 4 highest channels
	idNoteMode		= 2,		// midi keyOn encoding
		MStatic			= 0,	// always use Midi key velocity 127
		MVeloc			= 1,	// encode velocity in Midi keyOn message
		MFormula		= 2,	// encode formula V for velocity in Midi keyOn message		 7.35
		MNoNote			= 3,	// do not output keyOn,keyOff,bends -- for CVC+Midi control of Voyager
		MMidC			= 4,	// nn 60 and static velocity all notes (Moog Theremin)		 7.84
		MAnnounce		= 5,	// announce continuum presence for SNBN
	// Touch Area and Split
	idTArea		 	= 60, 		// nn center of Touch Area; < nnLowFull if disabled			 7.76
		nnTWid			= 2,	// +/- width of tarea area (unless extended by split SplitTArea)
		nnTOff			= 16,	// less or equal nnTOff means no Touch Area defined (use All Fingers)
		nnTMax			= 108,	// highest Touch Area value
	idSplitMode		= 1,
		SplitOff 		= 0,	// surface split disabled
		SplitPoly 		= 1,	// equal number of channels below and above split point
		SplitMonoL 		= 2,	// single channel (1st ch) below split point, all other channels above
		SplitMonoH 		= 3,	// single channel (1st ch) above split point, all other channels below
		SplitIntL 		= 4,	// internal sound below, all Midi above.
		SplitIntH 		= 5,	// internal sound above split, all Midi below
		SplitTArea		= 6,	// Touch Area Extends Throughout Split Area					 9.28
	idSplitPoint 	= 45, 		// note number for split [see also ccMSpL and ccMSpH]
	// Transpose and Rounding
	idMiddleC 		= 44,		// middle C transpose; nnMiddleC = normal, 59 = sound half step lower
	idReverse		= 9,		// true to reverse pitches
	idRoundMode		= 10,		// rounding mode
		rNormal			= 0,	// normal rounding
		rRelease		= 1,	// release-only rounding
		rViaY			= 2,	// rounding via Y position
		rViaYinv		= 3,	// rounding via 1-Y position
	// CVC, Midi routing, and user-selectable program
	idCvcCfg		= 63,		// cvc standard voltage configuration (if no CVC bank in use)
		cvcStdBits		= 0x07,	// cvc standard voltage range selection	
		cvcZsqBit		= 0x08,	// cvc z squared select										 7.44
		cvcLayerBit		= 0x10,	// cvc layer bit											 7.44
		cvcBaseBits		= 0x60, // cvc base voice bits										 7.44
	idRouting		= 36,		// Midi routing
		bSurfaceTrad	= 0x01,	// bit routes playing surface to Midi Out
		bSurfaceDsp		= 0x02,	// bit routes playing surface to internal sounds
		bSurfaceCvc		= 0x04,	// bit routes playing surface to CVC
		bMidiInTrad		= 0x08,	// bit routes Midi In to Midi Out
		bMidiInDsp		= 0x10,	// bit routes Midi In to internal sounds
		bMidiInCvc		= 0x20,	// bit routes Midi In to CVC
		defaultRoute    = 63,	// default is to set all routing bits
	idProgram 		= 35,		// User program# tx to third party on ch1 at preset load 	 6.12
	// Pedals
	idPedal1 		= 52,		// select cc number for pedal jack 1; MINI: 0 means i2c instead
	idPedal2 		= 53,		// select cc number for pedal jack 2
	idJackShift		= 54,		// amount to shift when pedal jack is ccOctShift ch1,ch16
		jackShiftDefault = 48,	// octave down is default (unfortunate default!)
	idSwTogInst		= 7,		// oct ped mode											4.39 7.73 [Conifer]
		octPedSw		= 0,	// switch
		octPedTog		= 1,	// toggle
		octPedInst		= 2,	// instant
	// Master Section
	idNoRecirc		= 15,		// true to disable recirculator 							 5.01 5.12
	idReciType		= 62,		// recirculator type										 7.36
		R_shortRev		= 0, 	// short reverb		 										 7.44 10.20
		R_modDel		= 1,	// modulated delay	
		R_swepEcho		= 2, 	// swept echo												 7.35 [Lechwth]
		R_anaEcho		= 3,	// analog echo	
		R_digEchoLPF	= 4,	// digital echo	
		R_digEchoHPF	= 5,	// high pass digital echo									 7.36
		R_longRev		= 6,	// long reverb												 10.12 10.20
		R_mask			= 0x7,	// this must be > R_lastone, for switch optimization
	idReciCol1		= 3,		// recirc columns function select (see below)				 8.62 [DanielB]
	idReciCol2		= 4,		// (Note: rDials is reciParams+1, includes rMix at end)
		rDial1=0,rDial2,rDial3,rDial4,rDial5,rDial6, rMix,    rDials,
	idCompOpt		= 16,		// 0=compressor, 1=masterTanh								10.35
		masComp=0,masTanh,
	// Oscillators/Filters 
	idOscFilTyp1	= 72,		// oscillator/filter type, see values below
	idOscFilTyp2	= 73,
	idOscFilTyp3	= 74,
	idOscFilTyp4	= 75,
	idOscFilTyp5	= 76,
		// Sources:
		OSCn			=  0,	// oscillator DSF
		OSCi			=  1,	// oscillator integrated DSF
		OSCpwm			= 18,	// oscillator PWM						oscFilOpt specifies variant
		OSCj			=  2,	// oscillator Jenny						oscFilOpt specifies variant
		OSCp			=  3,	// oscillator phase (not bandlimited)	oscFilOpt specifies variant
		SNois			=  4,	// seeded noise source
		// Filters:
		LP				=  5,	// filter low pass						oscFilOpt specifies cascade
		LS				= 10,	// filter low shelf						oscFilOpt specifies cascade
		LP1				= 12,	// filter low pass single pole			oscFilOpt specifies cascade
		LPladder		= 19,	// filter lowpass ladder				oscFilOpt specifies variant
		HP				=  6,	// filter high pass						oscFilOpt specifies cascade
		HS				= 11,	// filter high shelf					oscFilOpt specifies cascade
		HP1				= 13,	// filter high pass single pole			oscFilOpt specifies cascade
		BP				=  7,	// filter band pass						oscFilOpt specifies cascade
		BR				=  8,	// filter band reject					oscFilOpt specifies cascade
		AP				=  9,	// filter all pass						oscFilOpt specifies cascade
		// Others:
		MUL				= 14,	// signal multiplier
		TANH			= 15,	// tanh function
		CRSH			= 16,	// bit crush
		GDEL			= 17,	// granulate delay tap (pitch shift)
	idOscFilOpt1	= 77,		// filterCascade/oscillatorOption/ladderVariant, see values below
	idOscFilOpt2	= 78,		   
	idOscFilOpt3	= 79,		   
	idOscFilOpt4	= 80,		   
	idOscFilOpt5	= 81,		// if oscFilTyp = LP..HP1,  oscFilOpt=0, or 1..3 to cascade filters
		m_lin			= 0,	// if oscFilTyp = OSCpwm,	oscFilOpt=0 linear pwm			10.40
		m_exp			= 1,	// if oscFilTyp = OSCpwm,	oscFilOpt=1 exponential pwm		10.40
		p_sum7 			= 0,	// if oscFilTyp = OSCp,     oscFilOpt=0 sum of seven Phase Generators
		p_rampup 		= 1,	// if oscFilTyp = OSCp,     oscFilOpt=1 phase generator 0..1
		p_rampdn 		= 2,	// if oscFilTyp = OSCp,     oscFilOpt=2 phase generator 1..0
		p_triangle 		= 3,	// if oscFilTyp = OSCp,     oscFilOpt=3 triangle phase 0..1..0
		j_SharpDecay	= 0,	// if oscFilTyp = OSCj,     oscFilOpt=0 post-mult by (1-masterPhase)
		j_SmoothDecay	= 1,	// if oscFilTyp = OSCj,     oscFilOpt=1 post-mult by bandlimited decay
		j_HannDecay 	= 2,	// if oscFilTyp = OSCj,     oscFilOpt=2 post-mult by symmetric window
		j_NoDecay		= 3,	// if oscFilTyp = OSCj,     oscFilOpt=3 to skip post-multiply
		l_Trans			= 0,	// if oscFilTyp = LPladder, oscFilOpt=0 evocative of transistor ladder
		l_Diode			= 1,	// if oscFilTyp = LPladder, oscFilOpt=1 evocative of diode ladder
	idFilEx1		= 82,		// true to enable audio-rate coef update for filter ("extreme")
	idFilEx2		= 83,
	idFilEx3		= 84,
	idFilEx4		= 85,
	idFilEx5		= 86,
	// Banks
	idBankA		= 91,			// set type for bankA in matrix (see values below)			 4.89
	idBankB		= 92,			// set type for bankB in matrix (see values below)			 4.89
	idBankC		= 93,			// set type for bankC in matrix (see values below)			 5.41 [BPK]
		BiqBank		= 0,		// for idBankA and idBankB
		BiqGraph	= 1,		// for idBankA and idBankB
		BiqMouth	= 2,		// for idBankA and idBankB
		SineBank	= 3,		// for idBankA and idBankB
		Additive	= 10,		// for idBankA and idBankB
		Kinetic		= 9,		// for idBankA and idBankB
		ModMan		= 7,		// for idBankA and idBankB
		HarMan		= 6,		// for idBankA and idBankB/idBankC (C is alias for B)
		WaveBank	= 5,		// for idBankA and idBankB/idBankC (C is alias for B)
		SineSpray	= 4,		// for idBankA and idBankB/idBankC (C is alias for B)
		CvcBank		= 8,		// for             idBankB,idBankC (B+C for dual CVC)		10.33
		VDelay		= 0,		// for                     idBankC
		SDelay		= 1,		// for                     idBankC
		MDelay		= 2,		// for                     idBankC
		FDelay		= 3,		// for                     idBankC
		#define			IsGran(a) IN(SineSpray, a, HarMan)
	idColModeA1		= 95,		// first (1') column mode select (bankA)    see cm1_xxx below
	idColModeB1		= 94,		// first (1') column mode select (bankB/C)  see cm1_xxx below  
		// idColModeA1/idColModeB1 values by bank type
		cm1_g2Amp=0,cm1_g2Band=1,			// BiqGraph: graph2 does Amp or BW
		cm1_aSF=0,cm1_aQF=1,				// Additive: select sF or qF
		cm1_mmSB=0,cm1_mmQB=1,				// ModMan:   select sB or qB
		cm1_wbOD=0,cm1_wbEO=1,				// WaveBank: select offset duty cycle or even/odd
		cm1_bbDBP=0,cm1_bbDBO=1,			// BiqBank:  select dB/partial or dB/octave
	idColModeA2		= 97,		// second (2') column mode select (bankA)   see cm2_xxx below
	idColModeB2		= 96,		// second (2') column mode select (bankB/C) see cm2_xxx below
		// idColModeA2/idColModeB2 values by bank type
		cm2_g1Amp=0,cm2_g1Frq=1,			// BiqGraph: graph1 does Amp or Freq
		cm2_aIP=0,cm2_aDF,cm2_aPA,cm2_aN,	// Additive: I' or dF or pA or N
		cm2_mmSF=0,cm2_mmQF=1,cm2_mmSA=2,	// ModMan:   sF or qF or sA
		cm2_bbFull=0,cm2_bbHead=1,			// BiqBank:  modes full scale or leave headroom
	idBankParamA	= 99,		// deflate (biq) or wave (WaveBank) for bankA
	idBankParamB	= 98,		// deflate (biq) or wave (WaveBank) for bankB/C
		wave_saw		= 0,	// waveBank: sawtooth
		wave_square		= 1,	// waveBank: square
		wave_triangle	= 2,	// waveBank: triangle			
		wave_LeCaine	= 3,	// waveBank: LeCaine square
		defBankParam	= 1,	// default deflate 1 (biqBank) or square (waveBank)			 7.60
	idAliasDelay	= 108,		// true to disable nyquist filtering on delay
	idBqExtA		= 70,		// size of biquad bankA	(see bqVar comment below)			 8.62 [DanielB]
	idBqExtB		= 71,		// size of biquad bankB	(see bqVar comment below)			 5.63
		bqVar			= 6,	// idBqExtA/B 0..5 means 8..48 modes; 6 means variable		 10.09
	idSSetA			= 64,		// HarMan/ModMan/Additive BankA:   select Set				 5.41 [BPK]
	idSSetB			= 65,		// HarMan/ModMan/Additive BankB/C: select Set				 5.41 [BPK]
		Live			= 0,	// HarMan: granulate data recorded by VDelay/SDelay, or flash items below
		  ss_Bell = 1, Drum = 2, ss_KalimbaHard = 3, ss_KalimbaSoft = 4, 
		  ss_MarimbaHard = 5, ss_MarimbaSoft = 6, ss_mBiraski = 7, ss_Metal = 8, 
		  ss_Sprong = 9, ss_Noise1 = 10, ss_Noise2 = 11, ss_StringTap1 = 12, 
		  ss_StringTap2 = 13, ss_StringTap3 = 14, ss_ViolaLegato = 15, ss_ViolaMarcato = 16,
		  ss_PizzicatoLoud = 17, ss_PizzicatoSnap = 18, ss_PizzicatoSoft = 19, 
		  ss_SpiccatoLoud = 20, ss_SpiccatoSoft = 21, ss_AltoFlute = 22, ss_AltoSax = 23,
		  ss_Brass = 24, ss_Clarinet1 = 25, ss_Clarinet2 = 26, ss_Flute1 = 27, 
		  ss_Flute2 = 28, ss_Oboe1 = 29, ss_Oboe2 = 30, ss_Ukulele1 = 31, ss_Ukulele2 = 32, 
		  ss_Ukulele3 = 33, ss_ToyPiano1Strike = 34, ss_ToyPiano1Release = 35, 
		  ss_ToyPiano2Strike = 36, ss_Crystal = 37,
		  nSSet,				// HarMan and ModMan: number of flash-based Spectral Sets
		addM			= 0,	// Additive: Mahling Phrase
		add1stDecade	= 1,	// Additive: 1st decade (1..10)  of Additive Analyses
		add2ndDecade	= 2,	// Additive: 2nd decade (11..20) of Additive Analyses
		add3rdDecade	= 3,	// Additive: 3rd decade (21..30) of Additive Analyses
		add4thDecade	= 4,	// Additive: 4th decade (31..40) of Additive Analyses
	// Shape Generators   
	idSgTyp1		= 100,		// two bits shape generator type, see below
	idSgTyp2		= 101,		// 
	idSgTyp3		= 102,		// 
	idSgTyp4		= 103,		// 
	idSgTyp5		= 104,		// 
		contCycle		= 0,	// idSgTypN: sg continuous cycle
		singleCycle		= 1,	// idSgTypN: sg single cycle
		phaseInput		= 2,	// idSgTypN: sg direct phase input
		dualSG			= 3,	// idSgTypN: dual shape generator, Na and Nb
	idTimeSel		= 105,		// select time delay			 50 ms * 2^n    			 5.01 5.40
	// Matrix Row options		   
	idRowTyp1		= 106,		// matrix row type (AudioLR rows)
		rt1Aud			= 0,	// idRowTyp1: Audio L and R 
		rt1Tap			= 1,	// idRowTyp1: Tap1 and Tap2 
		rt1AudTap		= 2, 	// idRowTyp1: Mono Mix Audio, Mono Mix Tap1+Tap2
	idRowTyp2		= 107,		// matrix row type (SubLR rows)
		rt2Sub			= 0,	// idRowTyp2: Submix Return L and R
		rt2Tap			= 1,	// idRowTyp2: Tap3 and Tap4
		rt2SubTap		= 2,	// idRowTyp2: Mono Mix Submix, Mono Mix Tap3+Tap4
		rt2Aud			= 3,	// idRowTyp2: Audio L and R 
		rt2AudTap		= 4,	// idRowTyp2: Mono Mix Audio, Mono Mix Tap3+Tap4
	// Midi and Global Options - for all presets
	idAction		= 12,		// select global options Action								 4.33
		ActnMiniClassic = 0,	// pre-CF600 or ContinuuMini
		ActnMedium		= 1,	// medium action (thresholds same as Classic)
		ActnLight		= 2, 	// light action for wide intervals
		ActnLightN		= 3, 	// light action for narrow intervals						 8.55
	idAes3			= 13,		// select aestx clock
		AES96k			= 0,	// aestx at standard 96k rate, internally clocked
		AEShouse		= 1,	// use aes3 house clock (aesrx clock) for xmit, if aesrx available
		AES48k 			= 2,	// aestx at 48k clock rate, internally clocked
		AES108k			= 3,	// this can't be user-configured; for multi-dsp only  		 5.55
	idPresSurf		= 56,		// true = preserve surface processing (round, split, mono, etc)
	idPresPed		= 57,		// true = preserve Pedal assignments (Ped1/2,jackShift,min,max)
	idPresEnc		= 58,		// true = preserve encoding (Bend, Ycc, Zcc, routing) 		 6.16 10.33
	idBigFontPop	= 14,		// true for large popup menu font in Editor					 7.21 10.33
	// Copy additive analyses from one location to another.
	idFromAdd		= 126,		// source additive slot#
	idToAdd			= 127,		// destination additive slot#								10.34
	// Session options - active until power cycle
	idCfgOut		= 59,		// true = output config to Midi whenever it changes			 5.40


	// ---------------------------------- s_Kinet_Poke id Assignments -----------------------------------
	// Assignment of id for s_Kinet_Poke; these configure Kinetic properties.
	// These id assignments correspond to the sData ordering in s_Kinet.
	// For more information, see ccStream and s_Kinet_Poke definitions above.
	id_k_NOP		=  0,		// data not used; id means "unused column" for id_k_SelN and id_k2_SelN
	id_k_A			=  1,		// BankA property: anchor position
	id_k_K			=  2,		// BankA property: spring constant
	id_k_M			=  3,		// BankA property: mass
	id_k_G			=  4,		// BankA property: gravity
	id_k_H			=  5,		// BankA property: height
	id_k_V			=  6,		// BankA property: viscosity
	id_k_E			=  7,		// BankA property: elasticity
	id_k_P			=  8,		// BankA property: padding
	id_k_S			=  9,		// BankA property: slipping friction
	id_k_Q			= 10,		// BankA property: quit-slipping friction
	id_k_not1		= 11,		// BankA property: reserved for future
	id_k_not2		= 12,		// BankA property: reserved for future
	id_k_Sel5		= 13,		// BankA property: variable column 5 selection id_k_NOP..id_k_Q
	id_k_Sel6		= 14,		// BankA property: variable column 6 selection id_k_NOP..id_k_Q
	id_k_Sel7		= 15,		// BankA property: variable column 7 selection id_k_NOP..id_k_Q
	id_k_Sel8		= 16,		// BankA property: variable column 8 selection id_k_NOP..id_k_Q
	id_k_KLin		= 17,		// BankA property: spring constant nonlinearity
	id_k_VLin		= 18,		// BankA property: viscocity nonlinearity
	id_k_nn			= 19,		// BankA property: step frequency trim note number
	id_k_cent		= 20,		// BankA property: step frequency trim cents
	id_k_not3		= 21,		// BankA property: reserved for future
	id_k_not4		= 22,		// BankA property: reserved for future
	id_k_not5		= 23,		// BankA property: reserved for future
	  kBankBytes		= 24,	// # sData for each bank; 48 sData total
	id_k2_NOP		= 24,		// data value not used
	id_k2_A			= 25,		// BankB property: anchor position
	id_k2_K			= 26,		// BankB property: spring constant
	id_k2_M			= 27,		// BankB property: mass
	id_k2_G			= 28,		// BankB property: gravity
	id_k2_H			= 29,		// BankB property: height
	id_k2_V			= 30,		// BankB property: viscosity
	id_k2_E			= 31,		// BankB property: elasticity
	id_k2_P			= 32,		// BankB property: padding
	id_k2_S			= 33,		// BankB property: slipping friction
	id_k2_Q			= 34,		// BankB property: quit-slipping friction
	id_k2_not1		= 35,		// BankB property: reserved for future
	id_k2_not2		= 36,		// BankB property: reserved for future
	id_k2_Sel5		= 37,		// BankB property: variable column 5 selection id_k_NOP..id_k_Q
	id_k2_Sel6		= 38,		// BankB property: variable column 6 selection id_k_NOP..id_k_Q
	id_k2_Sel7		= 39,		// BankB property: variable column 7 selection id_k_NOP..id_k_Q
	id_k2_Sel8		= 40,		// BankB property: variable column 8 selection id_k_NOP..id_k_Q
	id_k2_KLin		= 41,		// BankB property: spring constant nonlinearity
	id_k2_VLin		= 42,		// BankB property: viscocity nonlinearity
	id_k2_nn		= 43,		// BankB property: step frequency trim note number
	id_k2_cent		= 44,		// BankB property: step frequency trim cents
	id_k2_not3		= 45,		// BankB property: reserved for future
	id_k2_not4		= 46,		// BankB property: reserved for future
	id_k2_not5		= 47,		// BankB property: reserved for future
	

	// ---------------------------------- s_BiqSin_Poke id Assignments -----------------------------------
	// Assignment of id for s_BiqSin_Poke; these configure BiqBank and SinBank properties.
	// These id assignments correspond to the sData ordering in s_BiqSin.
	// For more information, see ccStream and s_BiqSin_Poke definitions above.
	id_q_NOP		=  0,		// data not used; id means "unused column" for id_q_SelN and id_q2_SelN
	id_q_sF			=  1,		// BankA property: mode/partial freq linear spread from fundamental
	id_q_qF			=  2,		// BankA property: mode/partial freq nonlin spread from fundamental
	id_q_B			=  3,		// BankA property: mode/partial bandw
	id_q_R			=  4,		// BankA property: mode/partial amps even/odd ratio
	id_q_C			=  5,		// BankA property: center of focus
	id_q_sB			=  6,		// BankA property: BiqBank mode bandw linear increase from center,
								//              or SinBank partial noise amp spread
	id_q_qBsP		=  7,		// BankA property: BiqBank mode bandw nonlin increase from center,
								//				or SinBank partial phase spread
	id_q_sA			=  8,		// BankA property: mode/partial amps decrease from center
	id_q_F1			=  9,		// BankA property: freq  spectral peak 1 (nToHz x optional matrix column)
	id_q_F2			= 10,		// BankA property: freq  spectral peak 2 (nToHz x optional matrix column)
	id_q_F3			= 11,		// BankA property: freq  spectral peak 3 (nToHz x optional matrix column)
	id_q_B1			= 12,		// BankA property: bandw spectral peak 1 ( + optional matrix column)
	id_q_B2			= 13,		// BankA property: bandw spectral peak 2 ( + optional matrix column)
	id_q_B3			= 14,		// BankA property: bandw spectral peak 3 ( + optional matrix column)
	id_q_A1			= 15,		// BankA property: amp   spectral peak 1 ( + optional matrix column)
	id_q_A2			= 16,		// BankA property: amp   spectral peak 2 ( + optional matrix column)
	id_q_A3			= 17,		// BankA property: amp   spectral peak 3 ( + optional matrix column)
	id_q_Sel3		= 18,		// BankA property: variable column 3 selection id_q_NOP..id_q_A3
	id_q_Sel4		= 19,		// BankA property: variable column 4 selection id_q_NOP..id_q_A3
	id_q_Sel5		= 20,		// BankA property: variable column 5 selection id_q_NOP..id_q_A3
	id_q_Sel6		= 21,		// BankA property: variable column 6 selection id_q_NOP..id_q_A3
	id_q_Sel7		= 22,		// BankA property: variable column 7 selection id_q_NOP..id_q_A3
	id_q_Sel8		= 23,		// BankA property: variable column 8 selection id_q_NOP..id_q_A3
	  bBankBytes		= 24,	// # sData for each bank; 48 sData total
	id_q2_NOP		= 24,		// data value not used
	id_q2_sF		= 25,		// BankB property: mode/partial freq linear spread from fundamental
	id_q2_qF		= 26,		// BankB property: mode/partial freq nonlin spread from fundamental
	id_q2_B			= 27,		// BankB property: mode/partial bandw
	id_q2_R			= 28,		// BankB property: mode/partial amps even/odd ratio
	id_q2_C			= 29,		// BankB property: center of focus
	id_q2_sB		= 30,		// BankB property: BiqBank mode bandw linear increase from center,
								//              or SinBank partial noise amp spread
	id_q2_qBsP		= 31,		// BankB property: BiqBank mode bandw nonlin increase from center,
								//		 		or SinBank partial phase spread
	id_q2_sA		= 32,		// BankB property: mode/partial amps decrease from center
	id_q2_F1		= 33,		// BankB property: freq  spectral peak 1 (nToHz x optional matrix column)
	id_q2_F2		= 34,		// BankB property: freq  spectral peak 2 (nToHz x optional matrix column)
	id_q2_F3		= 35,		// BankB property: freq  spectral peak 3 (nToHz x optional matrix column)
	id_q2_B1		= 36,		// BankB property: bandw spectral peak 1 ( + optional matrix column)
	id_q2_B2		= 37,		// BankB property: bandw spectral peak 2 ( + optional matrix column)
	id_q2_B3		= 38,		// BankB property: bandw spectral peak 3 ( + optional matrix column)
	id_q2_A1		= 39,		// BankB property: amp   spectral peak 1 ( + optional matrix column)
	id_q2_A2		= 40,		// BankB property: amp   spectral peak 2 ( + optional matrix column)
	id_q2_A3		= 41,		// BankB property: amp   spectral peak 3 ( + optional matrix column)
	id_q2_Sel3		= 42,		// BankB property: variable column 3 selection id_q_NOP..id_q_A3
	id_q2_Sel4		= 43,		// BankB property: variable column 4 selection id_q_NOP..id_q_A3
	id_q2_Sel5		= 44,		// BankB property: variable column 5 selection id_q_NOP..id_q_A3
	id_q2_Sel6		= 45,		// BankB property: variable column 6 selection id_q_NOP..id_q_A3
	id_q2_Sel7		= 46,		// BankB property: variable column 7 selection id_q_NOP..id_q_A3
	id_q2_Sel8		= 47,		// BankB property: variable column 8 selection id_q_NOP..id_q_A3


	// ---------------------------------- s_Conv_Poke id Assignments -------------------------------------
	// Assignment of id for s_Conv_Poke; parameters for first and second convolution in Master Section.
	// These id assignments correspond to the sData ordering in s_Conv.
	// For more information, see ccStream and s_Conv_Poke definitions above.
	id_c_idx1		=  0,		// set index for 1st convolution (add in to 1st I column)
	id_c_mix1		=  1,		// set mix   for 1st convolution (add in to 1st M column)
	id_c_idx2		=  2,		// set index for 2nd convolution (add in to 2nd I column)
	id_c_mix2		=  3,		// set mix   for 2nd convolution (add in to 2nd M column)
	id_c_dat0		=  4,		// select data for 1st convolution IR (see choices below)
	id_c_dat1		=  5,		// select data for 2nd convolution IR (see choices below)
	id_c_dat2		=  6,		// select data for 3rd convolution IR (see choices below)
	id_c_dat3		=  7,		// select data for 4th convolution IR (see choices below)
		cd_Waterphone1 = 0, cd_Waterphone2 = 1, cd_Autoharp1 = 2, cd_Autoharp2 = 3,
		cd_GuitarDark = 4, cd_FingerSnap = 5, cd_Wood = 6, cd_MetalBright = 7,
		cd_Fiber = 8, cd_Leather = 9, cd_MetalDamped = 10, cd_Nylon = 11,
		cd_Cloth = 12, cd_EowaveGongSnapshot = 13, cd_LVDLPyramidSnapshot = 14,
		cd_LVDLGongSnapshot = 15, cd_LVDLOndeSnapshot = 16, cd_White = 17, cd_Grey = 18,
	id_c_lth0		=  8,		// length for 1st convolution IR
	id_c_lth1		=  9,		// length for 2nd convolution IR
	id_c_lth2		= 10,		// length for 3rd convolution IR
	id_c_lth3		= 11,		// length for 4th convolution IR
	id_c_shf0		= 12,		// shift  for 1st convolution IR							10.01
	id_c_shf1		= 13,		// shift  for 2nd convolution IR
	id_c_shf2		= 14,		// shift  for 3rd convolution IR
	id_c_shf3		= 15,		// shift  for 4th convolution IR
	id_c_wid0		= 16,		// width  for 1st convolution IR							10.10
	id_c_wid1		= 17,		// width  for 2nd convolution IR
	id_c_wid2		= 18,		// width  for 3rd convolution IR
	id_c_wid3		= 19,		// width  for 4th convolution IR
	id_c_atL0		= 20,		// attenL for 1st convolution IR	
	id_c_atL1		= 21,		// attenL for 2nd convolution IR
	id_c_atL2		= 22,		// attenL for 3rd convolution IR
	id_c_atL3		= 23,		// attenL for 4th convolution IR
	id_c_atR0		= 24,		// attenR for 1st convolution IR	
	id_c_atR1		= 25,		// attenR for 2nd convolution IR
	id_c_atR2		= 26,		// attenR for 3rd convolution IR
	id_c_atR3		= 27,		// attenR for 4th convolution IR
	id_c_phc		= 28,		// enable phase cancellation compensation (true/false)
	id_c_pad		= 29		// zero pad to make even number of bytes in s_Conv
};




// Definitions for Thick Continuum's config strip (for configuration from playing surface).
enum {

	// Internal sound parameter selection nn locations on Thick Continuum's config strip.
	// These are placed in ConField's "side" field by str_xxx macros
	optCalibrate = 48, optPost, optIntSound, optMidiProgram, optCVC,
	optRouting, optPoly, optChannelPrio, optBendRange, optY, optZ, optVelocity,
	optMiddleC, optSplitPoint, optSplitMode, optMonoFunc, optMonoInt, 
	optRoundInitial, optRoundRate, optTuning, optPedal1, optPedal2, 
	opti, optii, optiii, optOptions, optSend, optLoad, optStore,

	firstOpt = optCalibrate, lastOpt = optStore,

	// Data entry.
	optValue120 = 48, optValue10 = 59, optValue0 = 60, optValue16 = 76,

	// Options category.
	optReset = 0,				// reset options selections
	optMedAction = 1,			// medium action 
	optAesHouse = 2,			// allow slaving of aestx clock to aesrx rate
	optAes48k = 3,				// 48k internal clock for aestx
	optTwoOct = 4,				// octave pedal does two octaves (instead of normal 1 octave)
	optFourOct = 5,				// octave pedal does four octaves (instead of normal 1 octave)
	optReverse = 6,				// reverse pitches
	optNoRecirc = 7,			// avoid recirculator										 4.40
	optTxThird = 8,				// one-third Midi transmit rate								 6.41 7.57
	optTxTweenth = 9,			// one-twentieth Midi transmit rate							 7.59
	optDemoAssort = 10,			// demo presets												 7.00

	optResetAll = 127,			// reset all saved presets as well as current configuration

	// Send category.
	sendConfig	= 1,			// send current config over midi
};


// MINI: Indices into calibration arrays.
enum { L0 = 0, L1, R0, R1, rL, rR, calSiz=6, halls=4 };
enum { calMLo = 0, calMMid, calMHi, calPts=3 };



#endif
	

