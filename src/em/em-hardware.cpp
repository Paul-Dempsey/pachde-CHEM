#include "em-hardware.h"
#include "HakenMidi.h"
#include <stdint.h>

namespace pachde {

const char * HardwarePresetClass(uint8_t hw)
{
    switch (hw) {
        case hw_fL:
        case hw_hL:
        case hw_fC:
        case hw_hC:
        case hw_s22:
        case hw_s46:
        case hw_s70: return  "Continuum";
        case hw_Mini: return "CMini";
        case hw_o49: return  "Osmose";
        case hw_EMM: return  "EMM";
        case hw_Micro : return  "Micro";
        default: return "Unknown";
    }
}
const char * ShortHardwareName(uint8_t hw)
{
    switch (hw) {
    case hw_fL:return "CFL";
    case hw_hL:return "CHL"; 
    case hw_fC:return "CFC"; 
    case hw_hC:return "CHC";  
    case hw_Mini: return "CMini";
    case hw_o49: return "Osmose";
    case hw_s22:return "Slim22"; 
    case hw_s46: return "Slim46"; 
    case hw_s70: return "Slim70";
    case hw_EMM: return "EMM";
    case hw_Micro : return  "Micro";
    default: return "Unknown";
    }
}
const char * HardwareName(uint8_t hw)
{
    switch (hw) {
    case hw_fL:     return "Fill (Light)";
    case hw_hL:     return "Half (Light)"; 
    case hw_fC:     return "Full (Classic)"; 
    case hw_hC:     return "Half (Classic)";  
    case hw_Mini:   return "ContinuuMini";
    case hw_o49:    return "Osmose";
    case hw_s22:    return "Slim 22"; 
    case hw_s46:    return "Slim 46"; 
    case hw_s70:    return "Slim 70";
    case hw_EMM:    return "EaganMatrix Module";
    case hw_Micro : return "EaganMatrix Micro";
    default: return "Unknown";
    }
}

}