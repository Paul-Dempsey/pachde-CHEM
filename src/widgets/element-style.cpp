#include "element-style.hpp"

using namespace pachde;
namespace widgetry {

ElementStyle::ElementStyle(const char* key, const char * color_spec, float dx) :
    key(key),
    fill_color(parseColor(color_spec, colors::NoColor)),
    stroke_color(0),
    dx(dx)
{
}

ElementStyle::ElementStyle(const char *key, PackedColor co, const char *stroke_spec, float dx) :
    key(key),
    fill_color(co),
    stroke_color(parseColor(stroke_spec, colors::NoColor)),
    dx(dx)
{
}


ElementStyle::ElementStyle(const char *key, const char *color_spec, const char *stroke_spec, float dx) :
    key(key),
    fill_color(parseColor(color_spec, colors::NoColor)),
    stroke_color(parseColor(stroke_spec, colors::NoColor)),
    dx(dx)
{
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
    // if (!isVisibleColor(fill_color) && isVisibleColor(stroke_color)) {
    //     fill_color = stroke_color;
    // }
}

}