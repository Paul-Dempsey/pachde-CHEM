#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "element-style.hpp"

namespace pachde {
struct LevelWidget: TransparentWidget, IApplyTheme
{
    uint8_t level{0};
    ElementStyle style{"level", "hsl(30, 80%, 50%)", "hsla(200, 50%, 25%, 100%)", .25f};
    bool wire{false};

    LevelWidget() {
        box.size = Vec(1.5f, 26.f);
    }

    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override {
        wire = 0 == theme->name.compare("Wire");
        style.apply_theme(theme);
        return true;
    }

    void drawLayer(const DrawArgs& args, int layer) override {
        if (layer != 1) return;
        float extent = level * (box.size.y/127.f);
        FillRect(args.vg, 0, box.size.y - extent, box.size.x, extent, style.nvg_color());
    }

    void draw(const DrawArgs& args) override {
        if (wire) {
            BoxRect(args.vg, 0, 0, box.size.x, box.size.y, style.nvg_stroke_color(), style.width());
        } else {
            FillRect(args.vg, 0, 0, box.size.x, box.size.y, style.nvg_stroke_color());
        }
    }
};

}