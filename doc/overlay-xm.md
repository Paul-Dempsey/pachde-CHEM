# CHEM Overlay and XM

In CHEM you can build the user interface of simple Overlay synths without the need for general programming or user interface design expertise.

An Overlay synth is a combination of a specific EM preset with a software or hardware user interface (UI) designed for that specific preset.
The user interface communicates with the Eagan Matrix using the Haken MIDI protocol.

The _EaganMatrix Overlay Developer's Guide_ is essential for anyone wanting to create or understand  Overlay presets.
This document is included in the Haken Audio firmware downloads for 10.52 and beyond.
Creating overlay presets are beyond the scope of CHEM documentation.

Extended macros are an Eagan Matrix feature designed specifically for the creation of "Overlay Synths".
They are used as the link between a user interface element and parameters in the preset.
A macro is used in a preset formula where you want to provide a knob or other UI element.
The CHEM modules for extended macros are [**Overlay**](./overlay.md#chem-overlay), [**XM**](./xm.md#chem-xm).

An overlay synth may also change things that are not able to be paramaterized by a Macro in a formula.
For example, buttons may be used to change the waveform of an oscillators, or change the type of an oscillator or filter.
Access to non-macro elements can by programmed in [**MidiPad**](./midipad.md#chem-midipad).
MidiPad can send any fixed MIDI phrase at the press of a button or CV trigger.
Midipad is specialized for sending EaganMatrix macro values, graph pokes, and stream pokes, so you don't have to break it down to basic MIDI.

An Overlay is associated 1:1 with a Core.

`Core <=> Overlay <- XM <- XM <- XM`

- Core manages the connection and communications with the Eagan Matrix hardware.
- Overlay manages the specific overlay preset that the patch is designed for.
- XM controls up to 8 extended macros.

Each XM instance must be paired with an Overlay.
This happens automaticically unless you are controlling two EaganMatrix devices in the same Rack patch.
XM automatically links to an Overlay to it's immediate left or through a set of adjacent XMs to a common Overlay.
Otherwise, XM scans all the modules in the patch and links to the first Overlay it finds.
If it doesn't find the right Overlay, move it to be immediately on the right of the Overlay you want.

Once you've defined the control UI for an Overlay synth it can be useful to save the group of overlay preset modules as a Rack selection.
This allows you to import the overlay synth controls into any Rack patch where you want to inorporate that overlay synth.

---

// Copyright © Paul Chase Dempsey\
![pachde (#d) logo](./image/Logo.svg)
