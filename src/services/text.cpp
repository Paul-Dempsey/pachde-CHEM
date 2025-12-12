// Copyright (C) Paul Chase Dempsey
#include "text.hpp"
#include "misc.hpp"

#include <cctype>

namespace pachde {

    std::string rgba_string(PackedColor color) {
    uint8_t r = color & 0xff; color >>= 8;
    uint8_t g = color & 0xff; color >>= 8;
    uint8_t b = color & 0xff; color >>= 8;
    uint8_t a = color & 0xff;
    if (a < 255) {
        return format_string("rgba(%d, %d, %d, %d)", r, g, b, a);
    } else {
        return format_string("rgb(%d, %d, %d)", r, g, b);
    }
}

std::string hsla_string(float hue, float saturation, float lightness, float alpha)
{
    char buffer[25];
     std::string result{"hsl"};
     int ihue = static_cast<int>(hue * 360.f);
     if (alpha < 1.0f) {
        result.append("a(");
        auto len = format_buffer(buffer, sizeof(buffer), "%d, ", ihue);
        result.append(buffer, len);
        if (saturation > .01f) {
            len = format_buffer(buffer, sizeof(buffer), "%.1f, ", saturation);
            result.append(buffer, len);
        } else {
            result.append("0");
        }
        if (lightness > .01f) {
            len = format_buffer(buffer, sizeof(buffer), "%.1f, ", lightness);
            result.append(buffer, len);
        } else {
            result.append("0,");
        }
        if (alpha > .01f) {
            len = format_buffer(buffer, sizeof(buffer), "%.1f)", alpha);
            result.append(buffer, len);
        } else {
            result.append("0)");
        }
     } else {
        result.push_back('(');
        auto len = format_buffer(buffer, sizeof(buffer), "%d, ", ihue);
        result.append(buffer, len);
        if (saturation > .01f) {
            len = format_buffer(buffer, sizeof(buffer), "%.1f, ", saturation);
            result.append(buffer, len);
        } else {
            result.append("0,");
        }
        if (lightness > .01f) {
            len = format_buffer(buffer, sizeof(buffer), "%.1f)", lightness);
            result.append(buffer, len);
        } else {
            result.append("0)");
        }
     }
     return result;
}

void SetTextStyle(NVGcontext *vg, std::shared_ptr<window::Font> font, NVGcolor color, float height)
{
    assert(FontOk(font));
    nvgFillColor(vg, color);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 0.f);
    nvgFontSize(vg, height);
}

void CenterText(NVGcontext *vg, float x, float y, const char * text, const char * end)
{
    // nvg offers a variety of text alignment options
    nvgTextAlign(vg, NVG_ALIGN_CENTER);
    nvgText(vg, x, y, text, end);
}

void RightAlignText(NVGcontext *vg, float x, float y, const char * text, const char * end, BaselineCorrection correction)
{
    float bounds[4] = { 0, 0, 0, 0 };
    nvgTextBounds(vg, 0, 0, text, end, bounds);
    auto descent = correction == BaselineCorrection::Baseline ? bounds[3] : 0.;
    nvgText(vg, x - bounds[2], y - descent, text, end);
}

}