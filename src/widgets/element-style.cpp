#include "element-style.hpp"

namespace pachde {

ElementStyle::ElementStyle(const char* key, const char * color_spec, float dx) :
    key(key),
    dx(dx)
{
    switch (*color_spec) {
        case '#': fill_color = parseHexColor(color_spec); break;
        case 'r': fill_color = parseRgbColor(color_spec); break;
        case 'h': fill_color = parseHslaColor(color_spec); break;
        default: break;
    }
}

ElementStyle::ElementStyle(const char *key, const char *color_spec, const char *stroke_spec, float dx) :
    key(key),
    dx(dx)
{
    switch (*color_spec) {
    case '#': fill_color = parseHexColor(color_spec); break;
    case 'r': fill_color = parseRgbColor(color_spec); break;
    case 'h': fill_color = parseHslaColor(color_spec); break;
    default: break;
    }

    switch (*stroke_spec) {
    case '#': stroke_color = parseHexColor(stroke_spec); break;
    case 'r': stroke_color = parseRgbColor(stroke_spec); break;
    case 'h': stroke_color = parseHslaColor(stroke_spec); break;
    default: break;
    }
}

void ElementStyle::apply_theme(std::shared_ptr<SvgTheme> theme)
{
    assert(key);
    auto style = theme->getStyle(key);
    if (style) {
        if (style->isApplyStroke()) {
            stroke_color = style->stroke_color();
            dx = style->stroke_width;
        }
        if (style->isApplyFill()) {
            fill_color = style->fill_color();
        }
    }
    if (!isVisibleColor(fill_color) && isVisibleColor(stroke_color)) {
        fill_color = stroke_color;
    }
}

}