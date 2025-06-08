# CHEM To-Do

- Overlay macros
  - add/remove
  - implement control & dispatch
  - Proper host proxying: Overlay module should forward chem_host events, and clients do not independently register chem host?

- Preset and Play: onRandomize select random preset
- Bug: rounding lights aren't quite right
- Option to shut off looking for hardware (so that you can jump into the HE without issues).
- Reconcile host availability states across modules ( `connected()`/`ready()` ...)
- Load tab: attempt gathering only once (prevent looping).
- color picker
  - choose text syntax?
  - 100% transparent option, or transparency slider

## Osmose

- Preset: Implement loading Prebuilt System (factory) preset list from res/cache
- Play:
  - Append from User block
  - Exclude EM-style Append

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
