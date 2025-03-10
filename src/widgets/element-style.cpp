#include "element-style.hpp"

namespace pachde {

ElementStyle::ElementStyle(const char* key, const char * text_color, float dx) :
    key(key),
    dx(dx)
{
    switch (*text_color) {
        case '#': fill_color = parseHexColor(text_color); break;
        case 'r': fill_color = parseRgbColor(text_color); break;
        case 'h': fill_color = parseHslaColor(text_color); break;
        default: break;
    }
}

ElementStyle::ElementStyle(const char *key, const char *text_color, const char *stroke, float dx) :
    key(key),
    dx(dx)
{
    switch (*text_color) {
    case '#': fill_color = parseHexColor(text_color); break;
    case 'r': fill_color = parseRgbColor(text_color); break;
    case 'h': fill_color = parseHslaColor(text_color); break;
    default: break;
    }

    switch (*stroke) {
    case '#': stroke_color = parseHexColor(stroke); break;
    case 'r': stroke_color = parseRgbColor(stroke); break;
    case 'h': stroke_color = parseHslaColor(stroke); break;
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