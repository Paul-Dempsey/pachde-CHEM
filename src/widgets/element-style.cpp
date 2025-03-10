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