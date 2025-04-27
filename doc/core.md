# CHEM Core

The **Core** module is essential.
It maintains the MIDI connection and handles all communications with an Eagan Matrix device. All the other modules must be connected to a **Core**.

Sometimes MIDI connections don't work unless there is also one of the VCV Rack **Audio** modules in the patch that is connected to a driver or audio interface.
You don't have actually *use* the audio module, but it must be present and connected.
If you're starting a new Rack patch, add one of the VCV **Audio** modules first, then add a **Core**.

After **Core**, you'll want a **CHEM Preset**, but that's getting ahead of ourselves. Let's focus on **Core**.

![Connected CHEM Core](./image/core.png)

**Fig 1.** *Core connected to an Slim70, with a Korg nanoKONTROL2 on C2.*

When added to the patch CHEM automatically tries to find a connected EaganMatrix device, by name, connecting to the first one it finds.
Press the **HAKEN** MIDI button to choose from the Eagan Matrix devices that are connected to the computer.
Once you've successfuly connected to a device,
the moving dots under the button indicate the transmission of MIDI data to and from the device.
The text in the footer shows the type of EM device and the firmware version number.

The name of the currently selected preset is shown under the **#d CHEM** logo.

The **-** and **+** buttons under the preset name move to the previous and next presets, in System order.
You may be surprised that this isn't the same order as the preset movement buttons on the device itself for system presets. These buttons are sending simple preset movement commands provided by the EM firmware, and this is what they do.

The Big blue knob is Volume (aka Attenuation).

The button above the **HAKEN** MIDI button is the **MIDI Reset** button.
It resets the connection, for use when there may be a connection glitch.
To clear all connections, Ctrl+Click the **MIDI Reset** button. **Core** will immediately start scanning for EM devices to connect to.

To control a second EM Device, add another **Core** to the patch.

## Connecting other CHEM modules

Once you have a **Core** in your patch, you're ready to add other CHEM modules.
When you place an unconnected CHEM module side-by-side with a connected one, it will automatically link to the connected **Core**.
Or, you can click the **Link** button at the bottom of the CHEM module to choose from a connected **Core** in the patch.

Once connected, you can move the module elsewhere in the patch or to a different row, and they will stay connected (unlike other Rack extender modules).

## MIDI Controllers

Use the **C1** and **c2** buttons to select a MIDI controller to use with the main **HAKEN** device.
MIDI data from the controller is used by CHEM modules to track the state of the EaganMatrix and also passed through to the **HAKEN** device.
If you use an external MIDI controller _without_ routing through CHEM, changes made by the external controller may not be reflected in the CHEM modules.
This isn't disastrous -- changing presets will sync back up -- but it could be confusing or cause undesirable sound glitches, so best to route the controller through **Core**.

The three round buttons to the right of the controller, are **Channel map**, **Midi Filter**, and **Data**.

- If the controller is another EaganMatrix device with a playing surface, it can be used only for music data,
and if configured for MPE/MPE+ it is necessary to engage the **Channel Map**.
When engaged, music information on channels 2+ are reflected to channels 15-.
This helps avoid unwanted note collisions and allows the playing of both surfaces simultaneously.
The button led will light when the filter is engaged.

  **Core** attempts to automatically detect Eagan Matrix devices by their name, and enable both the **Channel map** and **Music filter**, but this automatic detection may not always work; when you're routing through a virtual cable, for example.
  In these cases, you must make sure the filter is enabled to prevent your EM device from getting confused with data from another EM device.

- If a controller is an instrument like a Keyboard or other Haken playing surface, it is usually necessary to allow only music data (notes and expression) through to the EM to prevent unwanted side effects from MIDI controls not intended for the EM.
Engage **Music Data** to pass only music data to the EM.
The button led will light when the filter is engaged.

- When engaged (the red light on the button is on), **Data** blocks the controller and does not past MIDI to the EM.
This effectively "mutes" that controller, so you can turn it on and off as needed without haveing to reconnect.

## Outs and Ins

The **Ready gate** output (purple port) can be used elsewhere in your patch to know when the EM device is ready and able to accept MIDI and produce sound.
The gate will be high when the EM is ready for input, and low when it is busy, such as while loading presets or enumerating presets on the device.

The **M1** and **M2** input ports (blue ports) are gate inputs to turn the corresponding controller's data stream off and on.
When the gate is high, the controller is blocked (muted), and the corresponding button led is lit.

---

// Copyright © Paul Chase Dempsey\
![pachde (#d) logo](./image/Logo.svg)
