#pragma once
#include <rack.hpp>
using namespace ::rack;

namespace pachde
{

// set displayPrecision = 4
inline rack::engine::ParamQuantity* dp4(rack::engine::ParamQuantity* p) {
    p->displayPrecision = 4;
    return p;
}

// set displayPrecision = 2
inline rack::engine::ParamQuantity* dp2(rack::engine::ParamQuantity* p) {
    p->displayPrecision = 2;
    return p;
}

}