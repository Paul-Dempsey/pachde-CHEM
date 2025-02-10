![](./image/repo-banner.svg)

# pachde CHEM

**CHEM** is the successor to pachde **HC One**, for EaganMatrix devices running firmware 10.4x.

**Note** – Pachde and **CHEM** are not officially affiliated with or sponsored by Haken Audio or Expressive E.

If you own more than one Eagan Matrix device, add an instance of **Core** for each device you want to control.

| Module | Description |
| -- | -- |
| **Core** | An instance of Core must exist in the patch. It provides the connection to an EaganMatrix device for the rest of the modules. Certain Eagan Matrix utility functions are available in the right-click menu. |
| **Play** | Manages playlists -- files of shortcuts to your favorite presets. Each playlist is for a specific device, so take care when you own more than one. |
| **Macro** | Tha main i-vi macro controls. |
| **Pre** | The pre-master section controls, with general pre-level, and controls for the Compressor/Tanh. Pre-convolution controls are in the Convolution module. |
| **Fx** | The "effects", a.k.a. Recirculator section. |
| **Post** | Th post-master section controls, including a global Mute, post level (Volume), and EQ. Post-convolution controls are in the Convolution module.|
| **Convolution** | Controls for pre- and post- convolutions. |

More modules are planned to cover other important controls, such as rounding and polyphony,
and a means to address macros 7-90 (which has interesting challenges in the context of VCV Rack).

We take requests! Open an issue on the GitHub repository,
 or get in touch with me on the VCV Rack Community forum or on Discord — where I go by "pachde" — or on Facebook — where I am Paul Dempsey.

---

// Copyright © Paul Chase Dempsey\
![pachde (#d) logo](./image/Logo.svg)