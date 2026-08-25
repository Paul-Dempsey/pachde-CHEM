#include "em-hardware.h"
#include "wrap-HakenMidi.hpp"
#include <stdint.h>

namespace eaganmatrix {

const char * PresetClassName(uint8_t hardware)
{
    switch (hardware) {
    case Haken::hw_fL:
    case Haken::hw_hL:
    case Haken::hw_fC:
    case Haken::hw_hC:
    case Haken::hw_s21:
    case Haken::hw_s46:
    case Haken::hw_s70: return "Continuum";
    case Haken::hw_Mini: return "Mini";
    case Haken::hw_o49: return "Osmose";
    case Haken::hw_E2x: return "Emm";
    case Haken::hw_u2x: return "Micro";
    default: return "Unknown";
    }
}

const char * ShortHardwareName(uint8_t hardware)
{
    switch (hardware) {
    case Haken::hw_fL: return "CFL";
    case Haken::hw_hL: return "CHL";
    case Haken::hw_fC: return "CFC";
    case Haken::hw_hC: return "CHC";
    case Haken::hw_Mini: return "CMini";
    case Haken::hw_o49: return "Osmose";
    case Haken::hw_s21: return "Slim21";
    case Haken::hw_s46: return "Slim46";
    case Haken::hw_s70: return "Slim70";
    case Haken::hw_E2x: return "EMM";
    case Haken::hw_u2x: return  "Micro";
    default: return "Unknown";
    }
}
const char * HardwareName(uint8_t hardware)
{
    switch (hardware) {
    case Haken::hw_fL: return "Full (Light)";
    case Haken::hw_hL: return "Half (Light)";
    case Haken::hw_fC: return "Full (Classic)";
    case Haken::hw_hC: return "Half (Classic)";
    case Haken::hw_Mini: return "ContinuuMini";
    case Haken::hw_o49: return "Osmose";
    case Haken::hw_s21: return "Slim 21";
    case Haken::hw_s46: return "Slim 46";
    case Haken::hw_s70: return "Slim 70";
    case Haken::hw_E2x: return "EaganMatrix Module";
    case Haken::hw_u2x: return "EaganMatrix Micro";
    default: return "Unknown";
    }
}

}