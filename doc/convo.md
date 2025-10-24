# CHEM Convolution

**Convolution** controls convolutions, both pre- and post- effects convolution.

![CHEM Convolution module](./image/convo.png)

The Blue knobs control the mix between the dry and processed signal for the pre-effects and post-effects convolutions.

The gray **Index** knobs select the dataset for the four convolution impulse responses (IRs).
An intermediary value provides an interpolation between the adjacent IRs.
Modulating the index can be an effective way to add movement to the timbres produced by a preset.

Right click an **IR** knob to see a menu of IR data, or turn (drag) the knob to move through the options.

For each of the 4 convolutions you can choose the parameters for applying the IR to the signal (*Length*, *Tuning*, and *Width*), the *Left* and *Right* strength, and which impule-response data to use.
Modulating *Length*, *Tuning*, and *Width* can introduce artifacts, so should be used sparingly or controlled using fixed voltages.

The **Phase** button enables phase cancellation.

The modulation inputs and **Modulation amount** knob work the same across all CHEM modules.
See [CHEM Modulation](./modulation.md#chem-modulation-voltage-control) for details.

Some of the convolution parameters were designed by Haken audio as set-and-forget in a preset.
Continuous modulation of parameters like _Length_, _Tuning_, _Width_, and the IR dataset will cause artifacts while audio is being produced.
The CV control can still be useful when driven by a sequencer, a fixed voltage, or a very low frequency sample and hold.

In some presets, convoluton parameters are controlled by macro assignments in the matrix.
This can create unintuitive effects or audio glitches when turning a knob or modulating the same parameter in CHEM.
In this case whatever happens, happens. If you don't like the audio results, then _dont do that_.
You can't damage anything but your ears or your relationshiop with your housemate.


---

// Copyright © Paul Chase Dempsey\
![pachde (#d) logo](./image/Logo.svg)
