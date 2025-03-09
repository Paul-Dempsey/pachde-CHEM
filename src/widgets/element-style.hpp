#pragma once
#include "../services/svgtheme.hpp"
#include "../services/colors.hpp"
using namespace ::svg_theme;

namespace pachde {

struct ElementStyle
{
    const char* key;
    PackedColor color;
    float dx;

    ElementStyle(const char* key) : key(key), color(0), dx(1.0f) {}
    ElementStyle(const char* key, const char * text_color, float dx = 1.f);

    ElementStyle(const char* key, PackedColor co, float dx = 1.f) :
        key(key), color(co), dx(dx) {}

    NVGcolor nvg_color() { return fromPacked(color); }
    float width() { return dx; }

    void apply_theme(std::shared_ptr<SvgTheme> theme);
};

}