# CHEM To-Do

- **Preset** Strip list building and rewire to get preset lists from Core
- Should Preset spin while Core is scanning?

## General

- **Core** Process() usually isn't called when there is no audio device selecteed.
  We can check for this and display a message to tell user they need to connect an Audio device to Rack.
- Preset and Play (Core?): onRandomize select random preset
- Bug: rounding lights aren't quite right
- Reconcile host availability states across modules ( `connected()`/`ready()` ...)
- color picker
  - choose text syntax?
  - 100% transparent option, or transparency slider

## Midi pad

- Implement hover/push ui feedback
- What might be some good factory presets?

## Overlay macros

- add/remove
- implement control & dispatch
- Proper host proxying: Overlay module should forward chem_host events, and clients do not independently register chem host?

## Osmose

- Live preset tracking can't use PresetId: must use something else (crc)
- Play:
  - Import user/system sets
  - Append from Playlist block

## Before release

- Update doc images for final graphics/themes

- Clean up unused events

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
