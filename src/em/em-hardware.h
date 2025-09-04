#pragma once
#include <stdint.h>
namespace eaganmatrix {

// enum class HardwarePresetClass { Unknown, Continuum, Mini, Micro, Emm, Osmose };
// HardwarePresetClass preset_class_from_hardware(uint8_t hw);
// const char * PresetClassName(HardwarePresetClass preset_class);

const char * PresetClassName(uint8_t hardware);
const char * ShortHardwareName(uint8_t hardware);
const char * HardwareName(uint8_t hardware);

}