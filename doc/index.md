# pachde CHEM

![CHEM banner](./image/repo-banner.svg)

**CHEM** is the successor to pachde (#d) **HC One**, for EaganMatrix devices running firmware 10.4x.

**Note** – Pachde and **CHEM** are not officially affiliated with or sponsored by Haken Audio or Expressive E.

If you own more than one Eagan Matrix device, add an instance of **Core** for each device you want to control.
Every other module must be connected to a **Core** module, using the link button in the module footer.

| Module | Description |
| -- | -- |
| [**Core**](./core.md)  | An instance of Core must exist in the patch. It provides the connection to an EaganMatrix device for the rest of the modules. Certain Eagan Matrix utility functions are available in the right-click menu. |
| [**Play**](./play.md)  | Manages playlists -- files of shortcuts to your favorite presets. Each playlist is for a specific device, so take care when you own more than one. |
| [**Macro**](./macro.md) | Tha main i-vi macro controls. |
| [**Pre**](./pre.md)   | The pre-master section controls, with general pre-effects level, and controls for the Compressor/Tanh. Pre-section convolution controls are in the Convolution module. |
| [**Fx**](./fx.md)    | The "effects", a.k.a. Recirculator section. |
| [**Post**](./post.md)  | The post- master section controls, including a global Mute, post level (Volume), and EQ. Post-section convolution controls are in the Convolution module.|
| [**Convolution**](./convo.md) | Controls for pre- and post- convolutions. |
| [**Jack**](./jack.md)  | Pedal assignment, min/max settings and jack output CV. |
| [**Sustain**](./sus.md)  | On-screen Sustain |
| [**Sostenuto**](./sus.md)  | On-screen Sostenuto |
| [**Sostenuto 2**](./sus.md)  | On-screen Sostenuto 2 |
| [**Settings**](./settings.md) | Surface direction, XYZ MIDI assignments, Polyphony, Note processing, Rounding, MIDI routing. |

More modules are planned to address macros 7-90 (which has interesting challenges in the context of VCV Rack).

We take requests! Open an issue on the GitHub repository,
 or get in touch with me on the VCV Rack Community forum or on Discord — where I go by "pachde" — or on the Haken Continuum and Expressive E Osmose Facebook groups where I am Paul Dempsey.

## Modulation (voltage control)

The user interface for modulation using voltage control works the same across all the CHEM modules.
See [CHEM Modulation](./modulation.md) for details on how the common **Modulation amount** knob works.

## Themes

**CHEM** is thoroughly themeable, and comes with the following themes, selectable by right click in the module menu.
Any set of adjacent CHEM panels change themes together in unison.
Here, we show the **Pre** module modeling the themes against a Light-themed Rack.

| **Dark** | **Light** | **Ice** | **Katy** | **Mellow** |**Wire**   |
| -- | -- |-- |-- |-- |-- |
| Standard default Dark\* | Standard Light\* | Cool deep blue-green | Cotton-candy Pink |  Warm cozy tones | Dark wireframe |
| ![Dark theme](./image/dark.png) | ![Light theme](./image/light.png) | ![Ice theme](./image/ice.png) | ![Katy theme](./image/katy.png) | ![Mellow theme](./image/mellow.png)  | ![Wire theme](./image/wire.png) |

\* The setting **Follow Rack theme** in the module menu does just that: follows the Rack panel preference setting with the **Dark** and **Light** themes.

### Custom themes

If you're adventurous, you can try creating a custom theme.
Each theme is defined a JSON file in the `pachde-CHEM/res/themes` folder.
These are plain-text files you can edit in any text editor.

To make your own custom theme, make a copy of one of the predefined themes and edit away.
**Dark** is the best one to start from becuase it contains an entry for every theme-able element in CHEM.
The name of your theme file should match the "name" item at the top of the JSON file.

If you make a mistake while editing like a JSON syntax error,
Rack will crash on startup with a diagnostic error message in the log with a hint of what went wrong.
Don't worry, Rack and your computer are not permanently damaged. Just fix the error and try again.
When working on a theme, it can be helpful to run Rack from the console using -d to see the log on the console isntead of in the log file.

You don't have to restart Rack to see results of each change as you work.
Choose **Hot-reload themes** in the right click menu or press F5 after clicking on a CHEM module.

## Lights Down

For when the lights are down, choose **Glowing knobs** in the right click menu for the knobs to stay bright.

---

// Copyright © Paul Chase Dempsey\
![pachde (#d) logo](./image/Logo.svg)
