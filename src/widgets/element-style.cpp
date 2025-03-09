#include "element-style.hpp"

namespace pachde {

ElementStyle::ElementStyle(const char* key, const char * text_color, float dx) :
    key(key),
    dx(dx)
{
    switch (*text_color) {
        case '#': color = parseHexColor(text_color); break;
        case 'r': color = parseRgbColor(text_color); break;
        case 'h': color = parseHslaColor(text_color); break;
        default: break;
    }
}    

void ElementStyle::apply_theme(std::shared_ptr<SvgTheme> theme)
{
    assert(key);
    auto style = theme->getStyle(key);
    if (style) {
        if (style->isApplyStroke()) {
            color = style->stroke_color();
            dx = style->stroke_width;
        }
        if (style->isApplyFill()) {
            color = style->fill_color();
        }
    }
}

}