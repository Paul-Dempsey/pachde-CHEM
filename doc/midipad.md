# CHEM MidiPad

*MidiPad** (a.k.a. **4x4**) lets you define up to 16 Pads that can send a wide range of EM configuration commands.
This is useful for things not covered in a dedicated CHEM modules, and designed as part of basic support for Overlay Synths.

The Eagan Matrix is controlled entirely by MIDI, and this module has special knowledge of the uniquer MIDI protocol used in the EM.

This is an advanced module, so you'll need to get technical to define the MIDI stream that will be sent to your EM device.
You will need to have the _Continuum User Guide_, the `HakenMidi.h` or `HakenMidi.pdf`, or one of Richard Kram's other technical guides at hand to define useful MIDI streams for your pads.

The MIDI definition is text-based using what I've dubbed the "Haken Control Language", or HCL.
The reference for HCL is here: [HCL: Haken Control Language](.\hcl.md#hcl_haken_control_language).


---

// Copyright © Paul Chase Dempsey\
![pachde (#d) logo](./image/Logo.svg)
