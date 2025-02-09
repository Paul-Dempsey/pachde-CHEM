#include "em-hardware.h"
#include "wrap-HakenMidi.hpp"
#include <stdint.h>

namespace pachde {

const char * HardwarePresetClass(uint8_t hw)
{
    switch (hw) {
        case Haken::hw_fL:
        case Haken::hw_hL:
        case Haken::hw_fC:
        case Haken::hw_hC:
        case Haken::hw_s22:
        case Haken::hw_s46:
        case Haken::hw_s70: return  "Continuum";
        case Haken::hw_Mini: return "CMini";
        case Haken::hw_o49: return  "Osmose";
        case Haken::hw_EMM: return  "EMM";
        case Haken::hw_Micro : return  "Micro";
        default: return "Unknown";
    }
}
const char * ShortHardwareName(uint8_t hw)
{
    switch (hw) {
    case Haken::hw_fL:return "CFL";
    case Haken::hw_hL:return "CHL"; 
    case Haken::hw_fC:return "CFC"; 
    case Haken::hw_hC:return "CHC";  
    case Haken::hw_Mini: return "CMini";
    case Haken::hw_o49: return "Osmose";
    case Haken::hw_s22:return "Slim22"; 
    case Haken::hw_s46: return "Slim46"; 
    case Haken::hw_s70: return "Slim70";
    case Haken::hw_EMM: return "EMM";
    case Haken::hw_Micro : return  "Micro";
    default: return "Unknown";
    }
}
const char * HardwareName(uint8_t hw)
{
    switch (hw) {
    case Haken::hw_fL:     return "Fill (Light)";
    case Haken::hw_hL:     return "Half (Light)"; 
    case Haken::hw_fC:     return "Full (Classic)"; 
    case Haken::hw_hC:     return "Half (Classic)";  
    case Haken::hw_Mini:   return "ContinuuMini";
    case Haken::hw_o49:    return "Osmose";
    case Haken::hw_s22:    return "Slim 22"; 
    case Haken::hw_s46:    return "Slim 46"; 
    case Haken::hw_s70:    return "Slim 70";
    case Haken::hw_EMM:    return "EaganMatrix Module";
    case Haken::hw_Micro : return "EaganMatrix Micro";
    default: return "Unknown";
    }
}

}