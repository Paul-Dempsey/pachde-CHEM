# CHEM To-Do

Starting a new refactor to address limitations, issues exposed by Osmose support

- Central preset database in Core
- Job queue to serialize EM operations
- Differentiate preset changes from config request/select preset/device update?
- Replace multiple notification schemes (some with few clients) with job completion callback?
- Prototype preset check-summing to compensate for Osmose lack of exposed preset ids.

## General

- **Core** Process() usually isn't called when there is no audio device selecteed.
  We can check for this and display a message to tell user they need to connect an Audio device to Rack.

- Overlay macros
  - add/remove
  - implement control & dispatch
  - Proper host proxying: Overlay module should forward chem_host events, and clients do not independently register chem host?

- Preset and Play: onRandomize select random preset
- Bug: rounding lights aren't quite right
- Reconcile host availability states across modules ( `connected()`/`ready()` ...)
- color picker
  - choose text syntax?
  - 100% transparent option, or transparency slider

## Osmose

- Live preset tracking can't use PresetId: must use something else
- Preset: Implement loading Prebuilt System (factory) preset list from res/cache
- Play:
  - Append from User block
  - Exclude EM-style Append
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
