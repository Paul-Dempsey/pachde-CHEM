#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../em/wrap-HakenMidi.hpp"
using namespace Haken;

namespace pachde {

inline float unipolar_7_to_rack(uint8_t value) {
    return (double)value / 127.0 * 10.0;
}

inline float unipolar_14_to_rack(uint16_t value) {
    return 10.0 * ((double)value * inv_max14);
}

inline float bipolar_14_to_rack(uint16_t value) {
    return 5.0 * (((double)value - (double)Haken::zero14) * inv_zero14);
}

inline uint8_t unipolar_rack_to_unipolar_7(float value) {
    auto v = clamp(value, 0.0, 10.0);
    return static_cast<uint8_t>(std::round(rescale(v, 0.f, 10.f, 0.f, 127.f))); 
}

inline uint16_t unipolar_rack_to_unipolar_14(float value) {
    auto r = clamp(value, 0.0, 10.0);
    return static_cast<uint16_t>(std::round(rescale(r, 0.f, 10.f, 0.f, Haken::max14)));
}

inline uint16_t bipolar_rack_to_bipolar_14(float value) {
    auto r = 0.2 * clamp(value, -5.0, 5.0) + zero14;
    return static_cast<uint16_t>(std::round(r));
}

inline int getParamIndex(ParamQuantity*pq) {
    return static_cast<int>(std::floor(pq->getValue() - pq->getMinValue()));
}

inline int getParamInt(Param& p) {
    return static_cast<int>(std::floor(p.getValue()));
}

// modulated value for knob track
// param_value =  unipolar knob value, 0v to 10v
// cv = bipolar control voltage, -5v to 5v
// amount = is bipolar amount in percent, -100% to 100%
inline float modulated_value(float param_value, float cv, float amount) {
    return clamp(param_value + (cv * (amount * .01)), 0.f, 10.f);
}

}