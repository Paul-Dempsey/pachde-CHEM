#pragma once
#include <rack.hpp>
using namespace ::rack;
using namespace pachde;

namespace widgetry
{

struct ILayoutHelp {
    bool layout_hints{false};
    NVGcolor layout_hint_color{Overlay(nvgRGB(0xbf, 0x60, 0xbe), .5f)};

    virtual void enable_layout_help(bool enable) { layout_hints = enable; }

    void draw_widget_bounds(Widget* widget, const rack::widget::Widget::DrawArgs& args) {
        FittedBoxRect(args.vg, 0, 0, widget->box.size.x, widget->box.size.y, layout_hint_color, Fit::Inside, 0.35f);
    }
};

namespace layout_help {
    inline void enable_children(Widget* widget, bool help) {
        for (auto child: widget->children) {
            auto lh = dynamic_cast<ILayoutHelp*>(child);
            if (lh) lh->enable_layout_help(help);
            enable_children(child, help);
        }
    }
}

}
