# CHEM To-Do

- Global settings module(s) (poly/mpe, mono, routing, surface, ...)
- Overlay macro system modules
- Reconcile host availability states across modules ( `connected()`/`ready()` ...)
- Decode category info for preset desciption tooltip (started)
- Undo for _Center knobs_ (5) and _Zero modulation_ (0)
- **Play**: sorting: alpha, id, category
- Allow shared controllers

## Before release

- Update doc images for final graphics/themes

- Clean up unused events

- Sync Dark theme with master SVGs (no delta in appearance between Null and Dark)

- Ship Null theme? I don't realistically expect anyone to make a custom theme.
  Better for the svg_theme project.

- Update svg_theme project to sync with all the improvements from CHEM.

## Consider

- Kinetic module (shelved for the moment - matrix may not be that useful in Rack scenario)
- Blinking or highlighted link button when not connected
- Play: Plugin scope for history (at least for last folder)
- Setting files (surface, rounding, polyphony) (Rack presets sufficient?)
- Preset-associated settings (re settings files) i.e. Customize sys preset - parameters per-module
