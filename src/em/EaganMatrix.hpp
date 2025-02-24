#pragma once
#include <stdint.h>
#include "wrap-HakenMidi.hpp"
#include "midi-message.h"
#include "em-hardware.h"
#include "preset.hpp"
#include "FixedStringBuffer.hpp"

namespace pachde {

struct IHandleEmEvents {
    enum EventMask : uint16_t {
        LoopDetect       = 1,
        EditorReply      = 1 << 1,
        HardwareChanged  = 1 << 2,
        PresetChanged    = 1 << 3,
        UserBegin        = 1 << 4,
        UserComplete     = 1 << 5,
        SystemBegin      = 1 << 6,
        SystemComplete   = 1 << 7,
        TaskMessage      = 1 << 8,
        LED              = 1 << 9,

        All = LoopDetect
            + EditorReply
            + HardwareChanged
            + PresetChanged
            + UserBegin
            + UserComplete
            + SystemBegin
            + SystemComplete
            + TaskMessage
            + LED
    };
    uint16_t em_event_mask{EventMask::All};

    virtual void onLoopDetect(uint8_t cc, uint8_t value) {}
    virtual void onEditorReply(uint8_t reply) {}
    virtual void onHardwareChanged(uint8_t hardware, uint16_t firmware_version) {}
    virtual void onPresetChanged() {}
    virtual void onUserBegin() {}
    virtual void onUserComplete() {}
    virtual void onSystemBegin() {}
    virtual void onSystemComplete() {}
    virtual void onTaskMessage(uint8_t code) {}
    virtual void onLED(uint8_t led) {}
};

struct ChannelData
{
    uint8_t cc[128]{0};
    uint8_t pedal_fraction() { return cc[Haken::ccFracPed]; }
    uint8_t pedal_fraction_ex() { return cc[Haken::ccFracPedEx]; }
    void clear() { std::memset(cc, 0, 128); }
};

struct EaganMatrix
{
    bool ready;
    uint16_t firmware_version;
    //uint16_t cvc_version;
    uint8_t hardware;

    bool broken_midi;
    bool in_preset;
    bool in_user;
    bool in_system;
    bool pending_EditorReply;
    bool pending_config;
    bool frac_hi;
    uint8_t frac_lsb;
    int data_stream;

    // raw channel cc data
    ChannelData ch1;
    ChannelData ch2;
    ChannelData ch16;

    uint16_t macro[90];
    uint8_t mat[128]; // Haken::s_MatPoke

    uint16_t jack_1;
    uint16_t jack_2;

    float get_macro_voltage(int id);

    FixedStringBuffer<32> name_buffer;
    FixedStringBuffer<256> text_buffer;
    PresetDescription preset;

    bool is_ready() { return ready; }
    void reset();

    uint16_t get_jack_1() { return jack_1; }
    uint16_t get_jack_2() { return jack_2; }

    // simple cc value retreival
    uint8_t get_led() { return ch16.cc[Haken::ccEdState] & Haken::sLedBits; }
    uint8_t get_actuation() { return ch16.cc[Haken::ccActuation]; }
    uint8_t get_traditional_polyphony() { return ch16.cc[Haken::ccPolyTrad]; }
    uint8_t get_dsp_polyphony() { return ch16.cc[Haken::ccPolyDsp]; }
    uint8_t get_cvc_polyphony() { return ch16.cc[Haken::ccPolyCvc]; }
    uint8_t get_tuning() { return ch16.cc[Haken::ccGrid]; }
    uint8_t get_pedal_types() { return ch16.cc[Haken::ccPedType]; }
    uint8_t get_octave_shift() { return ch1.cc[Haken::ccOctShift]; }
    uint8_t get_mono_switch() { return ch1.cc[Haken::ccMonoSwitch]; }
    uint8_t get_fine_tune() { return ch1.cc[Haken::ccFineTune]; }

    uint8_t get_pre() { return ch1.cc[Haken::ccPre]; } // 14-bit?
    uint8_t get_post() { return ch1.cc[Haken::ccPost]; } // 14-bit?
    uint8_t get_attenuation() { return ch1.cc[Haken::ccAtten]; }

    uint8_t get_round_rate() { return ch1.cc[Haken::ccRoundRate]; }
    uint8_t get_round_initial() { return ch1.cc[Haken::ccRndIni]; }
    bool is_round_initial() { return ch1.cc[Haken::ccRndIni]; }

    uint8_t get_sustain() { return ch1.cc[Haken::ccSus]; }
    uint8_t get_sos() { return ch1.cc[Haken::ccSos]; }
    uint8_t get_sos2() { return ch1.cc[Haken::ccSos2]; }
    uint8_t get_stretch() { return ch1.cc[Haken::ccStretch]; }

    uint8_t get_headphone_level() { return ch1.cc[Haken::ccHpLevel]; }
    uint8_t get_line_level() { return ch1.cc[Haken::ccLineLevel]; }

    uint8_t get_pedal_1_min() { return ch1.cc[Haken::ccMin1]; }
    uint8_t get_pedal_1_max() { return ch1.cc[Haken::ccMax1]; }
    uint8_t get_pedal_2_min() { return ch1.cc[Haken::ccMin2]; }
    uint8_t get_pedal_2_max() { return ch1.cc[Haken::ccMax2]; }

    uint8_t get_eq_title() { return ch1.cc[Haken::ccEqTilt]; }
    uint8_t get_eq_freq() { return ch1.cc[Haken::ccEqFreq]; }
    uint8_t get_eq_mix() { return ch1.cc[Haken::ccEqMix]; }
    uint8_t get_thresh_drive() { return ch1.cc[Haken::ccThrDrv]; }
    uint8_t get_attack() { return ch1.cc[Haken::ccAtkCut]; }
    uint8_t get_ratio_makeup() { return ch1.cc[Haken::ccRatMkp]; }
    uint8_t get_cotan_mix() { return ch1.cc[Haken::ccCoThMix]; }

    uint8_t get_r1() { return ch1.cc[Haken::ccReci1]; }
    uint8_t get_r2() { return ch1.cc[Haken::ccReci2]; }
    uint8_t get_r3() { return ch1.cc[Haken::ccReci3]; }
    uint8_t get_r4() { return ch1.cc[Haken::ccReci4]; }
    uint8_t get_r5() { return ch1.cc[Haken::ccReci5]; }
    uint8_t get_r6() { return ch1.cc[Haken::ccReci6]; }
    uint8_t get_r_mix() { return ch1.cc[Haken::ccReciMix]; }

    //uint8_t get_xx() { return ch1.cc[Haken::]; }

    // S_Mat_Poke

    uint8_t get_base_polyphony() { return mat[Haken::idPoly]; }
    bool is_extend_polyphony() { return mat[Haken::idOkExpPoly]; }
    bool is_incresed_computation_rate() { return mat[Haken::idOkIncComp]; }
    uint8_t get_bend_range() { return mat[Haken::idBendRange]; }
    uint8_t get_y_assign() { return mat[Haken::idFrontBack]; }
    uint8_t get_z_assign() { return mat[Haken::idPressure]; }
    uint8_t get_mono_func() { return mat[Haken::idMonoFunc]; }
    uint8_t get_mono_interval() { return mat[Haken::idMonoInt]; }
    uint8_t get_note_priority() { return mat[Haken::idPrio]; }
    uint8_t get_note_Mode() { return mat[Haken::idNoteMode]; }
    uint8_t get_touch_area() { return mat[Haken::idTArea]; }
    uint8_t get_split_mode() { return mat[Haken::idSplitMode]; }
    uint8_t get_split_point() { return mat[Haken::idSplitPoint]; }
    uint8_t get_middle_c() { return mat[Haken::idMiddleC]; }
    bool is_reverse_surface() { return mat[Haken::idReverse]; }
    uint8_t get_round_mode() { return mat[Haken::idRoundMode]; }
    uint8_t get_cvc_config() { return mat[Haken::idCvcCfg]; }
    uint8_t get_midi_routing() { return mat[Haken::idRouting]; }
    uint8_t get_program() { return mat[Haken::idProgram]; }
    uint8_t get_jack_1_assign() { return mat[Haken::idPedal1]; }
    uint8_t get_jack_2_assign() { return mat[Haken::idPedal2]; }
    uint8_t get_jack_shift()  { return mat[Haken::idJackShift]; }
    uint8_t get_octave_pedal_mode() { return mat[Haken::idSwTogInst]; }
    bool is_disable_recirculator() { return mat[Haken::idNoRecirc]; }
    uint8_t get_recirculator_type() { return mat[Haken::idReciType]; }
    uint8_t get_recirculator_col_1() { return mat[Haken::idReciCol1]; }
    uint8_t get_recirculator_col_2() { return mat[Haken::idReciCol2]; }
    bool is_compressor() { return Haken::masComp == mat[Haken::idCompOpt]; }
    bool is_tanh() { return Haken::masTanh == mat[Haken::idCompOpt]; }
    uint8_t get_osc_filter_1() { return mat[Haken::idOscFilTyp1]; }
    uint8_t get_osc_filter_2() { return mat[Haken::idOscFilTyp2]; }
    uint8_t get_osc_filter_3() { return mat[Haken::idOscFilTyp3]; }
    uint8_t get_osc_filter_4() { return mat[Haken::idOscFilTyp4]; }
    uint8_t get_osc_filter_5() { return mat[Haken::idOscFilTyp5]; }
    uint8_t get_osc_filter_option_1() { return mat[Haken::idOscFilOpt1]; }
    uint8_t get_osc_filter_option_2() { return mat[Haken::idOscFilOpt2]; }
    uint8_t get_osc_filter_option_3() { return mat[Haken::idOscFilOpt3]; }
    uint8_t get_osc_filter_option_4() { return mat[Haken::idOscFilOpt4]; }
    uint8_t get_osc_filter_option_5() { return mat[Haken::idOscFilOpt5]; }
    bool is_filter_1_audio_rate() { return mat[Haken::idFilEx1]; }
    bool is_filter_2_audio_rate() { return mat[Haken::idFilEx2]; }
    bool is_filter_3_audio_rate() { return mat[Haken::idFilEx3]; }
    bool is_filter_4_audio_rate() { return mat[Haken::idFilEx4]; }
    bool is_filter_5_audio_rate() { return mat[Haken::idFilEx5]; }
    uint8_t get_bank_A_type() { return mat[Haken::idBankA]; }
    uint8_t get_bank_B_type() { return mat[Haken::idBankB]; }
    uint8_t get_bank_C_type() { return mat[Haken::idBankC]; }
	uint8_t get_A1_col_mode() { return mat[Haken::idColModeA1]; }
	uint8_t get_B1_col_mode() { return mat[Haken::idColModeB1]; }
	uint8_t get_A2_col_mode() { return mat[Haken::idColModeA2]; }
	uint8_t get_B2_col_mode() { return mat[Haken::idColModeB2]; }
	uint8_t get_bank_param_A() { return mat[Haken::idBankParamA]; }
	uint8_t get_bank_param_B() { return mat[Haken::idBankParamB]; }
    bool is_disable_anti_alias_delay() { return mat[Haken::idAliasDelay]; }
    uint8_t get_biq_extent_A() { return mat[Haken::idBqExtA]; }
    uint8_t get_biq_extent_B() { return mat[Haken::idBqExtB]; }
    uint8_t get_bank_A_spectral_set() { return mat[Haken::idSSetA]; }
    uint8_t get_bank_B_spectral_set() { return mat[Haken::idSSetB]; }
    uint8_t get_signal_generator_1_type() { return mat[Haken::idSgTyp1]; }
    uint8_t get_signal_generator_2_type() { return mat[Haken::idSgTyp2]; }
    uint8_t get_signal_generator_3_type() { return mat[Haken::idSgTyp3]; }
    uint8_t get_signal_generator_4_type() { return mat[Haken::idSgTyp4]; }
    uint8_t get_signal_generator_5_type() { return mat[Haken::idSgTyp5]; }
    uint8_t get_time_delay() { return mat[Haken::idTimeSel]; } //50 ms * 2^n
    uint8_t get_row_1_type(){ return mat[Haken::idRowTyp1]; }
    uint8_t get_row_2_type(){ return mat[Haken::idRowTyp2]; }
    uint8_t get_action() { return mat[Haken::idAction]; }
    uint8_t get_aes3() { return mat[Haken::idAes3]; }
    bool is_keep_surface_processing() { return mat[Haken::idPresSurf]; }
    bool is_keep_pedals() { return mat[Haken::idPresPed]; }
    bool is_keep_midi_encoding() { return mat[Haken::idPresEnc]; }
    bool is_big_popup_font() { return mat[Haken::idBigFontPop]; }
    uint8_t get_from_analysis_slot() { return mat[Haken::idFromAnlys]; }
    uint8_t get_to_analysis_slot() { return mat[Haken::idToAnlys]; }
    bool is_transmit_updates() { return mat[Haken::idCfgOut]; }

    //uint8_t get_xxx() { return mat[Haken::idxxxxx]; }

    std::vector<IHandleEmEvents*> clients;
    void clearClients() { clients.clear(); }
    void subscribeEMEvents(IHandleEmEvents* client);
    void unsubscribeEMEvents(IHandleEmEvents* client);

    void notifyLoopDetect(uint8_t cc, uint8_t value);
    void notifyEditorReply(uint8_t editor_reply);
    void notifyHardwareChanged();
    void notifyPresetChanged();
    void notifyUserBegin();
    void notifyUserComplete();
    void notifySystemBegin();
    void notifySystemComplete();
    void notifyTaskMessage(uint8_t code);
    void notifyLED(uint8_t led);

    EaganMatrix();

    void begin_preset();
    void clear_preset(bool notify);

    bool set_checked_data_stream(uint8_t stream_id);
    bool handle_macro_cc(uint8_t cc, uint8_t value);
    void onChannelOneCC(uint8_t cc, uint8_t value);
    void onChannelTwoCC(uint8_t cc, uint8_t value);
    void onChannel16CC(uint8_t cc, uint8_t value);
    void onMessage(PackedMidiMessage msg);

};

}