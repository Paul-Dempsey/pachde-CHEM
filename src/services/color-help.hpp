#pragma once
#include "svgtheme.hpp"
using namespace svg_theme;

namespace pachde {

PackedColor parse_color(const char * color_spec, PackedColor error_color = 0);

inline PackedColor parse_color(const std::string& spec, PackedColor error_color = 0) {
    return parse_color(spec.c_str(), error_color);
}
const PackedColor RARE_COLOR{PackRGBA(uint32_t(0x73), uint32_t(0x21), uint32_t(0x17), uint32_t(0))};

}