#include "color-help.hpp"
namespace pachde {

PackedColor parse_color(const char * color_spec, PackedColor error_color)
{
    switch (*color_spec) {
        case '#': return parseHexColor(color_spec, error_color); break;
        case 'r': return parseRgbColor(color_spec, error_color); break;
        case 'h': return parseHslaColor(color_spec, error_color); break;
        default: return error_color;
    }
}

}