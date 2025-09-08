# HCL: Haken Control Language

The **MidiPad** module uses HCL - the _Haken Control Language_ to define the MIDI data stream that is sent.
HCL is a basic text notation designed specificallly for the Haken MIDI protocol understood by the EaganMatrix.

- When defiining MIDI pads, you will want to have the `HakenMidi.h` file or one of Richard Kram's guides at the ready for reference.
  If you need help coming up with a pad definition, please don't hesitate to ask on the community forums, and we'll help you out.
- HCL has no provision for notes or other music-related codes -- only EaganMatrix controls.
- Predefined `ccTask` Haken MIDI control code is really not useful for a pad control, so no special syntax has been provided for this part of the protocol.
  Generic CC is always available and can be used if you need to send one.

CHEM performs only minimal validation on the form of the data that is sent.
Each poke usually has a specific range or set of values that can be sent to it, but these values are not validated: it is up to you to send a correct value.

> **WARNING** —
> It is quite possible to temporarily bork your preset or crash your device by sending bad data.
> If your device stops making sound or strange things appear on the display,
> or you see "internal error" in CHEM Core, or errors on the device display,
> you may need to reload the preset or even power cycle your instrument.

The syntax for number values is context-dependent.
For CCs and macros, HCL is oriented to the generally higher resolution of the EaganMatrix, so the syntax favors 14-bit values.
CC and macro values are always sent as EM 14-bit values.
For CCs that support only 7-bit, the extra LSB MIDI message is ignored by the EaganMatrix.
Use the `'`_nnn_ syntax if you want to work in terms of 7-bit resolution for CCs and Macros 1-6.

For pokes and streams, all values must be 7-bit so you don't use the -1..+1 or the `'`_nnn_ syntax for stream data.

| Item | Syntax | Description |
| -- | -- | -- |
| Comment | `"`_text_`"` | Comments are enclosed in double quotes. Comments cannot contain a double quote mark. When the definition begins with a comment, that comment is displayed in the tooltip for a pad. |
| Variables | `{`_name_`=`_value_ (`;` _name_`=`_value_)* `}` | Semicolon-separated list of name-value pairs. Named values can be defined to make the MIDI definition more readable. Any place a _value_ appears, you can use the variable name. |
| Channel | `ch` 1-16 | Sets the channel for CCs that follow. |
| Control Code (CC) | `cc` 0-127 _value_ | Sends the corresponding CC and value. The value is always sent as an EM 14-bit value. For 7-bit CCs, the LSB message is ignored. |
| Macro value |  `m` 1-90 _value_ | Sends a macro value. |
| _value_ | _nnnn_ | If _nnnn_ begins with a digit, it is a raw value in the range 0..16_256 (14-bit MIDI). An underscore can be used as a visual separator in long numbers. If _nnnn_ begins with a signle quote (`'`), it is a "MIDI value" number in the range 0-127. Otherwise it is a signed value notated in the range -1 to 1. [`+-`](`1` or `zero` or `.`_nnn_). `zero` must be the spelled-out word "zero", rather than the digit `0`. Positive one-half is written as `.5`. The decimal point can be either `.` or `,`. |
| _pair_ | _value7_ _value7_ | Two 7-bit values to be sent in a poke or data stream. |
| _stream-data_ | `[` _pair_+ `]` | List of one or more pairs of 7-bit values for a poke or data stream. |
| Stream | `s` _stream-number_ `[` _value7_* `]` | The specified data stream, automatically padded with 0 to an even number of values. |
| Matrix poke | `mp` _stream-data_ | Sends a list of Matrix pokes. |
| Formula poke | `fp` _stream-data_ | Sends a list of Formula pokes. |
| Graph poke | `gp` or `gp1` or `gp2` | Sends a list of Graph pokes. |
| Kinetic poke  | `kp` _stream-data_ | Sends a list of Kinetic pokes. |
| Biquad/Sinebank poke | `bp` _stream-data_ | Sends a list of Biquad/Sinebank pokes. |
| Convolution poke | `cv` _stream-data_ | Sends a list of Convolution pokes |

Another way to think of the syntax is with the following pseudo-BNF notation.
Items in single quotes are literal values -- the things you type.
Something followed by `+` means "one or more", `*` means zero or more.
Pipe (`|`) means "or". Alternates are often grouped in parentheses.

```bnf
            comment : '"' nonquote* '"'
           variable : `{` name `=` value `}`

     channel-number : 1 .. 16
                 n7 : 0 .. 127
                n14 : 0 .. 16256
      stream-number : 0 .. 27
       macro-number : 1 .. 90
           cc-value : n7 | n14
        macro-value : n7 | n14 | signed-rational
             digits : '0' .. '9' | '_'
    signed-rational : '-1' | '+1' | 'zero' | ['+'|'-']('.'|',')digits
               pair : n7 n7
          pair-list : '[' pair+ ']'
               list : '[' n7+ ']'

            channel : 'ch' channel-number
       control-code : 'cc' n7 cc-value
     program-change : 'pc' n7
              macro : 'm'macro-number macro-value
             stream : 's'stream-number list
        matrix-poke : 'mp' pair-list
       formula-poke : 'fp' pair-list
         graph-poke : ('gp'|'gp1'|'gp2') pair-list
         kinet-poke : 'kp' pair-list
         bqsin-poke : 'bp' pair-list
          conv-poke : 'vp' pair-list
```