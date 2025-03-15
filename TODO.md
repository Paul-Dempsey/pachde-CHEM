# CHEM To-Do

- Global settings module(s) (poly/mpe, mono, routing, surface, ...)
- Kinetic module
- Overlay macro system modules
- Reconcile host availability states across modules ( `connected()`/`ready()` ...)
- Listen for new 10.50 Mahling start/end signals for busy detection
- Decode category info for preset desciption tooltip
- Undo for _Center knobs_ (5) and _Zero modulation_ (0)

## Before release

- Update doc images for final graphics/themes

- Clean up unused events

- Sync Dark theme with master SVGs (no delta in appearance between Null and Dark)

- Ship Null theme? I don't realistically expect anyone to make a custom theme.
  Better for the svg_theme project.

- Update svg_theme project to sync with all the improvements from CHEM.

## Consider

- Blinking or highlighted link button when not connected
- Play: Plugin scope for history (at least for last folder)
- Setting files (surface, rounding, polyphony)
- Preset-associated settings (re settings files) i.e. Customize sys preset - parameters per-module
