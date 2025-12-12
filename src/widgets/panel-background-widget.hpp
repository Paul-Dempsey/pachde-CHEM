#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/colors.hpp"
#include "services/svg-theme.hpp"
using namespace svg_theme;
using namespace pachde;

namespace widgetry {

struct PanelBackgroundWidget : Widget, IThemed
{
    bool track_parent_size{false};

    PackedColor g_0{0xff282828};
    PackedColor g_1{0xff181818};

    void track() { track_parent_size = true; }

    void expand_to_parent() {
        auto p = getParent();
        if (p) {
            box.size = p->box.size;
        }
    }

    void onButton(const ButtonEvent& e) override {
        e.unconsume();
    }

    bool applyTheme(std::shared_ptr<SvgTheme> theme) override
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

    void step() override
    {
        if (track_parent_size) {
            expand_to_parent();
        }
    }

    void draw(const DrawArgs& args) override
    {
        auto vg = args.vg;

        nvgBeginPath(vg);
        // TODO: adjust when not filling parent
        nvgFillPaint(vg, nvgLinearGradient(vg, 0, 0, 0, box.size.y, fromPacked(g_0), fromPacked(g_1)));
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFill(vg);

        // Line(vg, 0, 0, box.size.x, box.size.y, SCHEME_RED, 2);
        // Line(vg, box.size.x, 0, 0, box.size.y, SCHEME_RED, 2);
    }
};

}