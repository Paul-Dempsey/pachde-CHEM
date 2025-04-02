# CHEM Core

The **Core** module is essential.
It maintains the MIDI connection and handles all communications with an Eagan Matrix device. All the other modules must be connected to a **Core**.

Sometimes MIDI connections don't work unless there is also one of the VCV Rack **Audio** modules in the patch that is connected to a driver or audio interface.
You don't have actually *use* the audio module, but it must be present and connected.

![Connected CHEM Core](./image/core.png)

**Fig 1.** *Core connected to an EaganMatrix Micro, with a ContinuuMini and a Korg nanoKONTROL2 as controllers.*

Press the **HAKEN** MIDI button to choose from the Eagan Matrix devices that are connected to the computer.
Once you've successfuly connected to a device,
the moving dots under the button indicate the transmission MIDI data to and from the device.

The name of the currently selected preset is shown under the **#d CHEM** logo.
The text in the footer shows the type of EM device and the firmware version number.

The **-** and **+** buttons under the preset name move to the previous and next presets, in System order.
You may be surprised that this isn't the same order as the preset movement buttons on the device itself for system presets. These buttons are sending simple preset movement commands provided by the EM firmware, and this is what they do.

The button above the **HAKEN** MIDI button is the **MIDI Reset** button.
It resets the connection, for use when there may be a connection glitch.
To clear all connections, Ctrl+Click the **MIDI Reset** button.

To control a second EM Device, add another **Core** to the patch.

## Connecting other CHEM modules

Once you have a **Core** in your patch, you're ready to add other CHEM modules.
When you place an unconnected CHEM module side-by-side with a connected one, it will automatically link to the connected **Core**.
Or, you can click the **Link** button at the bottom of the CHEM module to choose from a connected **Core** in the patch.

Once connected, you can move the module elsewhere in the patch or to a different row, and they will stay connected (unlike other Rack extender modules).

## MIDI Controllers

Use the **MIDI 1** and **MIDI 2** buttons to select a MIDI controller to use with the main **HAKEN** device.
MIDI data from the controller is used by CHEM modules to track the state of the EaganMatrix, and also passed through to the **HAKEN** device.
If you use an external MIDI controller without routing through CHEM, changes made by the external controller may not be reflected in the CHEM modules.
This isn't disastrous -- changing presets by any means will sync back up -- but it could be confusing or cause undesirable sound glitches.

If the controller is another EaganMatrix device, it can be used only for music data (notes and expression).
Use the **MIDI FIlter** button to the right of the controller connection to toggle the music data filter.
The button led will light when the filter is engaged.
**Core** attempts to automatically detect Eagan Matrix devices by their name, but this automatic detection may not always work.
When you're routing through a virtual cable, for example.
In these cases, you must make sure the filter is enabled to prevent your EM device from getting confused with data from another EM device.

The right-most button for a controller turns the midi stream for that controller off and on (filtered or not).
When pressed (red light), the controller is effectively "muted".

## Outs and Ins

The **Ready gate** output (purple port) can be used elsewhere in your patch to know when the EM device is ready and able to accept MIDI and produce sound.
The gate will be high when the EM is ready for input, and low when it is busy, such as while loading presets or enumerating presets on the device.

The **M1** and **M2** input ports (blue ports) are gate inputs to turn the corresponding controller's data stream off and on.
When the gate is high, the controller is blocked (muted), and the corresponding button led is lit.

---

// Copyright © Paul Chase Dempsey\
![pachde (#d) logo](./image/Logo.svg)
