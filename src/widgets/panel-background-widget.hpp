#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../services/colors.hpp"
#include "../services/svgtheme.hpp"
using namespace svg_theme;

struct PanelBackgroundWidget : OpaqueWidget, IApplyTheme
{
    PackedColor g_0{0xff282828};
    PackedColor g_1{0xff181818};

    void expand_to_parent() {
        auto p = getParent();
        if (p) {
            box.size = p->box.size;
        }
    }
    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        auto style = theme->getStyle("g-panel");
        if (style) {
            auto g = style->fill.getGradient();
            if (g) {
                g_0 = g->stops[0].color;
                g_1 = g->stops[1].color;
            }
            else {
                g_0 = 0xff282828;
                g_1 = 0xff181818;
            }
        } else {
            g_0 = 0xff282828;
            g_1 = 0xff181818;
        }
        return true;
    }

    void draw(const DrawArgs& args) override
    {
        auto vg = args.vg;

        nvgBeginPath(vg);
        // TODO: adjust when not filling parent
        nvgFillPaint(vg, nvgLinearGradient(vg, 0, 0, box.size.x, box.size.y, fromPacked(g_0), fromPacked(g_1)));
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFill(vg);
    }
};

