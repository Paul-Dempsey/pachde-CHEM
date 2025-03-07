# CHEM To-Do

- Implement CV
- Get Convo sync/knobs working
- Global settings module (poly/mpe, mono, routing, surface, ...)
- Refactor labels for just the requirements
- Reconcile host availability states across modules ( `connected()`/`ready()` ...)
- Ensure logging off completely silences logging and log-file creation.
- Listen for new 10.50 Mahling start/end signals for busy detection
- Decode category info for tooltip

## Before release

- Clean up unused events

- Sync Dark theme with master SVGs (no delta in appearance between Null and Dark)

- Ship Null theme? I don't realistically expect anyone to make a custom theme.
  Better for the svg_theme project.

- Update svg_theme project to sync with all the improvements from CHEM.

## Consider

- Blinking link button when not connected
- Play: Plugin scope for history (at least for last folder)
- Setting files (surface, rounding, polyphony)
- Preset-associated settings (re settings files) i.e. Customize sys preset - parameters per-module
