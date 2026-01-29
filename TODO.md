# CHEM To-Do

## General

- Update Preset scan images for 2.03 Core/Preset

- 10.6x HakenMidi
  - New Pedal assignments
  - New Y and Z ccs
  - MPE on all channels `MLegacyCh1out` -- affects WXYZ
  - check for other new/deprecated features

- Document CHEM-kv.txt, Midilog, and all files in plugin folder.

## Preset

- keyboard and cc auto-repeat prev/next
- save/load controller configurations independent of module preset
  which includes too much info, like the connection

## Play

- Document poly triggers

- Midi nav? That was complicated enough for Preset, but not sure how it would work for both. Managing focus would be tricky, so probably best if each have disjoint controls configured.

## Core

- Revisit mpe_channels for new ability to do MPE on ch 1 `MLegacyCh1out`
  This should be done when we add support for CH1out in Settings.

- Bug: rounding lights aren't always same as Continuum

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
