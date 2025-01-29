RACK_DIR ?= ../..

ifdef BUILD_DEBUG
FLAGS += -O0
else
FLAGS += -O3
FLAGS += -DNDEBUG
endif
FLAGS += -Wno-comment

# plugin infra
SOURCES += src/plugin.cpp
SOURCES += src/chem.cpp

# EaganMatrix
SOURCES += src/em/EaganMatrix.cpp
SOURCES += src/em/em-hardware.cpp
SOURCES += src/em/midi-message.cpp
SOURCES += src/em/preset.cpp

# services
SOURCES += src/services/colors.cpp
SOURCES += src/services/haken-midi.cpp
SOURCES += src/services/midi-devices.cpp
SOURCES += src/services/midi-io.cpp
SOURCES += src/services/MidiDeviceBroker.cpp
SOURCES += src/services/MidiDeviceHolder.cpp
SOURCES += src/services/ModuleBroker.cpp
SOURCES += src/services/misc.cpp
SOURCES += src/services/open-file.cpp
SOURCES += src/services/svgtheme.cpp
SOURCES += src/services/text.cpp

# widgets
SOURCES += src/widgets/blip-widget.cpp
SOURCES += src/widgets/PanelBorder.cpp
SOURCES += src/widgets/themed-widgets.cpp

# modules
SOURCES += src/Blank/Blank.cpp

SOURCES += src/Core/Core.cpp
SOURCES += src/Core/Core-ui.cpp
SOURCES += src/Core/haken-task.cpp
# SOURCES += src/Core/midi-input-worker.cpp

SOURCES += src/Preset/Preset.cpp
SOURCES += src/Preset/Preset-ui.cpp

SOURCES += src/Play/Play.cpp
SOURCES += src/Play/Play-ui.cpp

DISTRIBUTABLES += res
# DISTRIBUTABLES += presets
# DISTRIBUTABLES += selections

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

