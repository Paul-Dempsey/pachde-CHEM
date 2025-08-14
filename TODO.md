# CHEM To-Do

## General

- Bug: rounding lights aren't quite same as Continuum
- Reconcile host availability states across modules ( `connected()`/`ready()` ...)
- color picker
  - choose text syntax?
  - 100% transparent option
  - transparency slider

## Core

- Option for per-connection preset lists to support multiple devices.
- Bug: Switching devices always requires a reset.

## Preset

## Play

- Osmose: Append playlist page

## MidiPad

- What might be some good factory presets?

## Overlay macros

- Load Rack preset dies
- Test! Needs some banging on to verify it works
- Exclusion: Allow only one knob/macro across XM instances (currently excluded only in menu)

## Osmose

## Before release

- Update doc images for final graphics/themes

- Clean up unused events

- Open issues for unresolved TODOs.

- Sync Dark theme with master SVGs (no delta in appearance between Null and Dark)

- Ship Null theme? I don't realistically expect anyone to make a custom theme.
  Better for the svg_theme project.

- Update svg_theme project to sync with all the improvements from CHEM.

## Consider

- Undo for _Center knobs_ (5) and _Zero modulation_ (0)?
- Kinetic module (shelved for the moment - matrix may not be that useful in Rack scenario)
- Blinking or highlighted link button when not connected
- Setting files (surface, rounding, polyphony) (Rack presets sufficient?)
- Preset-associated settings (re settings files) i.e. Customize sys preset - parameters per-module
