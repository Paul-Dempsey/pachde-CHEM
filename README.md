# pachde-CHEM

![CHEM Repository splash card](doc/image/repo-card.png)

CHEM (**C**ontroller for the **H**aken **E**agan **M**atrix).

CHEM supports EaganMatrix firmware 10.40 and greater on all EaganMatrix devices, including Osmose.
Today it is recommended to be running at least firmware 10.52 on all EaganMatrix devices, but anything 10.40 and later should be fine.
If you've been holding back on updating your Osmose to the latest release, now's the time!

Subscribe to the plugin at [CHEM - VCV Rack Library](https://library.vcvrack.com/pachde-CHEM).

To get started, be sure to read the [Documentation](./doc/index.md#pachde-chem).

**#d CHEM** is the successor to **HC One**, which targeted EaganMatrix firmware 10.09.

## Change log

See [CHANGELOG](./CHANGELOG.md#chem-change-log)

## Development builds

To download the latest development build, see the [Nightly Release](https://github.com/Paul-Dempsey/pachde-CHEM/releases/tag/Nightly) page.

[![Build Status](https://github.com/Paul-Dempsey/pachde-CHEM/actions/workflows/build-plugin.yml/badge.svg)](https://github.com/Paul-Dempsey/pachde--CHEM/actions/workflows/build-plugin.yml)

> **Note** –
> Pachde and **CHEM** are not officially affiliated with or sponsored by Haken Audio.

## Building CHEM from source

Building CHEM is like building any other vanilla VCV Rack plugin.
The only difference is that CHEM has proper debug/release builds supported by its makefile.
A normal `make`, or `make install` builds a release version.
To build CHEM for debugging with asserts enabled, use `make DEV_BUILD=1`.

I do not use the Rack SDK.
I build Rack from source with optimization off to make debugging through Rack easier.
The CHEM source is built and run in-place in it's plugin folder under Rack.

I do my primary development on Windows.
I use VSCode to write code and debug, but I build from the console.
I have two MSYS2 MINGW64 consoles in the new Windows terminal, one for building and running `./Rack -d`, and one for building CHEM (or one of my other Rack plugins).

I occasionally build and test on a Mac and Linux.

## Acknowledgements

Special thanks to Lippold Haken (and the Haken Audio team), for creating the Continuum family of devices.
Lippold personally has provided generous support by sharing internal technical details that make this software possible.

Special thanks to Richard Kram for encouragement, stellar docs, and testing.
And thanks everyone (you know who you are) for all the support and encouragement on the forums.

## Appreciation

The pachde line of software for Eagan Matrix devices is a labor of love. The products are free to use.
Creating them requires an large investment of time and $$.
You can show appreciation for the value these tools bring to your music-making, by making a donation or leaving a tip.

[Click to Show appreciation (VenMo)](https://venmo.com/u/pcdempsey). \
Thank you!

## License

This repo is nominally MIT-licensed.

If you make a derivative work using this source code for profit, a contribution back to the author would be appreciated so that this kind of work can continue.
Even though the license does not create a legal obligation to do so, it's just the right thing to do.

All graphics and documentation are Copyright (c) Paul Chase Dempsey.

This software uses the Haken Audio Midi Protocol Copyright (C) by Lippold Haken.

---

// Copyright © Paul Chase Dempsey\
![pachde (#d) logo](./doc/image/Logo.svg)
