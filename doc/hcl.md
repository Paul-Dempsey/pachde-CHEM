# HCL: Haken Control Language

The **MidiPad** (aka **4x4**) module uses HCL - the Haken Control Language to define the MIDI data stream that will be sent when a pad is clicked.

- No notes (there is no clock) or other music-related codes -- only EaganMatrix controls.
- Predefined ccTask is not useful for pad controls. Generic cc is always available.
- Values out of range of 7-bit automatically generate the 14-bit MIDI.

For the specific values to send, see the `HakenMidi.h` file included with the User Guides in the Haken Editor download.
CHEM performs only minimal validation on the form of the data that is sent.

| Item | Syntax | Description |
| -- | -- | -- |
| Comment | `[`_text_`]` | Comments are enclosed in square brackets. Comments cannot contain an `]`. |
| Variable | `{`_name_`=`_value_`}` | Named values can be defined to make the MIDI definition more readable. Any place a _value_ appears, you can use a variable name. |
| Channel | `ch` 1-16 | Sets the channel for the MIDI that follows. Macros, Pokes, and Streams are always sent on channel 16, ignoring the channel set by `ch`. |
| Control Code (CC) | `cc` 0-127 _value_ | Sends the corresponding CC and value. If the CC supports 14-bits and the value is greater than 127, the corresponding LSB MIDI code is automatically sent. |
| Macro value |  `m` 1-90 _value_ | Sends a macro value. |
| _value_ | _nnnn_ | If first char is a digit, it is a raw value in the range 0...16_256. Otherwise a signed value in the range -1 to 1. `[+-](1 or .nnn)`, that is: `-1` .. `+1`: `-1`, `-.nnn`, `+1` or `.nnn`. An underscore can be used as a visual separator. The decimal point can be either `.` or `,`. For CCs and macros 1 .. 6, only raw 14-bit values are supported. If a value greater than 7 bits is used with a CC that is 7-bit only, the least-significant byte (LSB) is ignored. |
| _pair_ | _value7_ _value7_ | Two 7-bit values to be sent in a poke or data stream. |
| _stream-data_ | _pair_ [_pair_]* | List of pairs of 7-bit values for a poke or data stream. |
| Begin stream | `s` _stream-number_ | Begins the specified data stream. |
| End Stream | `end` | Optional for poke streams. |
| Matrix poke | `mp` _stream-data_ | Sends a list of Matrix pokes. |
| Formula poke | `fp` _stream-data_ | Sends a list of Formula pokes. |
| Graph poke | `gp` or `gp1` or `gp2` | Sends a list of Graph pokes. |
| Kinetic poke  | `kp` _stream-data_ | Sends a list of Kinetic pokes. |
| Biquad/Sinebank poke | `bp` _stream-data_ | Sends a list of Biquad/Sinebank pokes. |
| Convolution poke | `cv` _stream-data_ | Sends a list of Convolution pokes |

