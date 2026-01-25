# CHEM To-Do

## General

- MIDI-friendly control for navigating presets
  - keyboard working
  - consider relative mode: center rather than first for variable size steps
  - implement cc
  - save/load controller configurations independent of module preset
    which includes too much info, like the connection

- 10.6x HakenMidi
  - New Pedal assignments
  - New Y and Z ccs
  - MPE on all channels `MLegacyCh1out` -- affects WXYZ
  - check for other new/deprecated features

- Document CHEM-kv.txt, Midilog, and all files in plugin folder.

## Preset

- Add trigger param for send
- Keyboard graphic display for note config

## Core

- Revisit mpe_channels for new ability to send MPE on ch 1 `MLegacyCh1out`
- Bug: rounding lights aren't always same as Continuum

## After release

- Update svg_theme project to sync with all the improvements from CHEM/pachde1.

## Ideas

- Factory presets for MidiPad
- More controllers in Core
- Save unsaved Play list in patch
- Undo for _Center knobs_ (5) and _Zero modulation_ (0)
- Kinetic module (shelved for the moment - matrix may not be that useful in Rack scenario)
- Blinking or highlighted link button when not connected
- Setting files (surface, rounding, polyphony) (Rack presets sufficient?)
- Preset-associated settings (re settings files) i.e. Customize sys preset - parameters per-module
- Something for pitch tables?
