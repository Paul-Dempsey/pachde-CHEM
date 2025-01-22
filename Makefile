RACK_DIR ?= ../..

ifdef BUILD_DEBUG
FLAGS += -O0
else
FLAGS += -O3
FLAGS += -DNDEBUG
endif

# plugin infra
SOURCES += src/plugin.cpp
SOURCES += src/chem.cpp

# services
SOURCES += src/services/colors.cpp
SOURCES += src/services/midi_devices.cpp
SOURCES += src/services/MidiDeviceBroker.cpp
SOURCES += src/services/MidiDeviceHolder.cpp
SOURCES += src/services/midi-message.cpp
SOURCES += src/services/misc.cpp
SOURCES += src/services/svgtheme.cpp
SOURCES += src/services/text.cpp

# widgets
SOURCES += src/widgets/PanelBorder.cpp
SOURCES += src/widgets/themed_widgets.cpp

# modules
SOURCES += src/Blank/Blank.cpp
SOURCES += src/Core/Core.cpp
SOURCES += src/Core/Core-ui.cpp
SOURCES += src/Preset/Preset.cpp
SOURCES += src/Play/Play.cpp

DISTRIBUTABLES += res
# DISTRIBUTABLES += presets
# DISTRIBUTABLES += selections

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk