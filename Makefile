RACK_DIR ?= ../..

ifdef BUILD_DEBUG
FLAGS += -O0
FLAGS += -DLAYOUT_HELP

# template not listed in plugin.json, etc.
SOURCES += src/modules/_template/template.cpp
SOURCES += src/modules/_template/template-ui.cpp

else
FLAGS += -O3
FLAGS += -DNDEBUG
endif

# plugin infra
SOURCES += src/plugin.cpp
SOURCES += src/chem.cpp

# EaganMatrix
SOURCES += src/em/EaganMatrix.cpp
SOURCES += src/em/em-hardware.cpp
SOURCES += src/em/midi-message.cpp
SOURCES += src/em/preset.cpp
SOURCES += src/em/preset-list.cpp
SOURCES += src/em/preset-macro.cpp
SOURCES += src/em/preset-meta.cpp
SOURCES += src/em/preset-sort.cpp

# services
SOURCES += src/services/colors.cpp
SOURCES += src/services/color-help.cpp
SOURCES += src/services/em-midi-port.cpp
SOURCES += src/services/haken-midi.cpp
SOURCES += src/services/HakenMidiOutput.cpp
SOURCES += src/services/kv-store.cpp
SOURCES += src/services/midi-devices.cpp
SOURCES += src/services/midi-io.cpp
SOURCES += src/services/midi-log.cpp
SOURCES += src/services/MidiDeviceBroker.cpp
SOURCES += src/services/MidiDeviceHolder.cpp
SOURCES += src/services/misc.cpp
SOURCES += src/services/ModuleBroker.cpp
SOURCES += src/services/open-file.cpp
SOURCES += src/services/svgtheme.cpp
SOURCES += src/services/text.cpp

# widgets
SOURCES += src/widgets/blip-widget.cpp
SOURCES += src/widgets/bits-widget.cpp
SOURCES += src/widgets/color-picker.cpp
SOURCES += src/widgets/element-style.cpp
SOURCES += src/widgets/flip-switch.cpp
SOURCES += src/widgets/knob-track-widget.cpp
SOURCES += src/widgets/label-widget.cpp
SOURCES += src/widgets/PanelBorder.cpp
SOURCES += src/widgets/pic.cpp
SOURCES += src/widgets/selector-widget.cpp
SOURCES += src/widgets/slider-widget.cpp
SOURCES += src/widgets/slider-h-widget.cpp
SOURCES += src/widgets/tab-header.cpp
SOURCES += src/widgets/text-input.cpp
SOURCES += src/widgets/themed-widgets.cpp
SOURCES += src/widgets/uniform-style.cpp

# modules
ifdef BUILD_DEBUG
SOURCES += src/modules/_proto/proto.cpp
SOURCES += src/modules/_proto/proto-ui.cpp
SOURCES += src/modules/_template/template.cpp
SOURCES += src/modules/_template/template-ui.cpp
endif

SOURCES += src/modules/Core/Core.cpp
SOURCES += src/modules/Core/Core-ui.cpp
SOURCES += src/modules/Core/chem-task.cpp
SOURCES += src/modules/Core/preset-enum.cpp
SOURCES += src/modules/Core/preset-file-info.cpp

SOURCES += src/modules/Play/Play.cpp
SOURCES += src/modules/Play/Play-ui.cpp
SOURCES += src/modules/Play/Play-ui-create.cpp
SOURCES += src/modules/Play/Play-ui-file.cpp
SOURCES += src/modules/Play/preset-widget.cpp

SOURCES += src/modules/Macro/Macro.cpp
SOURCES += src/modules/Macro/Macro-ui.cpp

SOURCES += src/modules/Pre/Pre.cpp
SOURCES += src/modules/Pre/Pre-ui.cpp

SOURCES += src/modules/Fx/Fx.cpp
SOURCES += src/modules/Fx/Fx-ui.cpp

SOURCES += src/modules/Post/Post.cpp
SOURCES += src/modules/Post/Post-ui.cpp

SOURCES += src/modules/Convolution/Convo.cpp
SOURCES += src/modules/Convolution/Convo-ui.cpp
SOURCES += src/modules/Convolution/convolution.cpp

SOURCES += src/modules/Jack/Jack.cpp
SOURCES += src/modules/Jack/Jack-ui.cpp

SOURCES += src/modules/Sustain/Sustain.cpp
SOURCES += src/modules/Sustain/Sustain-ui.cpp

SOURCES += src/modules/Settings/SettingsModule.cpp
SOURCES += src/modules/Settings/Settings-ui.cpp
SOURCES += src/modules/Settings/widgets/tuning.cpp

SOURCES += src/modules/Preset/Preset.cpp
SOURCES += src/modules/Preset/Preset-ui.cpp
SOURCES += src/modules/Preset/Preset-ui-create.cpp
SOURCES += src/modules/Preset/Preset-ui-events.cpp
SOURCES += src/modules/Preset/preset-tab.cpp
SOURCES += src/modules/Preset/widgets/filter-widget.cpp
SOURCES += src/modules/Preset/widgets/preset-entry.cpp

SOURCES += src/modules/XM-shared/macro-data.cpp
SOURCES += src/modules/XM-shared/macro-usage.cpp
SOURCES += src/modules/XM-shared/xm-overlay.cpp

SOURCES += src/modules/XM-Overlay/Overlay.cpp
SOURCES += src/modules/XM-Overlay/Overlay-ui.cpp

SOURCES += src/modules/XM/XM.cpp
SOURCES += src/modules/XM/XM-ui.cpp

SOURCES += src/modules/MidiPad/MidiPad.cpp
SOURCES += src/modules/MidiPad/MidiPad-ui.cpp
SOURCES += src/modules/MidiPad/midi-pad.cpp
SOURCES += src/modules/MidiPad/hcl.cpp

DISTRIBUTABLES += res
# DISTRIBUTABLES += presets
# DISTRIBUTABLES += selections

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

