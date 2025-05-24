#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../../services/svt_rack.hpp"
using namespace ::svg_theme;
#include "../../widgets/element-style.hpp"

namespace pachde {

struct PadSelector : OpaqueWidget, IApplyTheme
{
    using Base = OpaqueWidget;

    // 28px x 28px (4x4:6px x 6px)
    int selected{-1};
    int hovered{-1};
    bool active{false};
    bool wire{false};

    ElementStyle pad{"pad", "hsl(0,0%,55%)", "", .25};
    ElementStyle pad_inactive{"pad-inactive", "hsl(0,0%,35%)", "hsl(0,0%,35%)", .25};
    ElementStyle pad_hover{"pad-hover", "hsl(42, 80%, 70%)", "hsl(42, 80%, 70%)", 1.25};
    ElementStyle pad_sel{"pad-sel", "hsl(42, 50%, 50%)", "hsl(42, 50%, 50%)", .25};

    std::function<void(int)> on_click{nullptr};

    PadSelector() {
        box.size = Vec(28,28);
    }

    void set_active(bool live)
    {
        active = live;
        if (!active) {
            selected = hovered = -1;
        }
    }

    void set_on_click(std::function<void(int)> handler)
    {
        on_click = handler;
    }

    Vec item_pos(int item)
    {
        float x = ((item % 4) * 6) + (item % 4);
        float y = ((item / 4) * 6) + (item / 4);
        return Vec(x,y);
    }
    
    int item_index(Vec pos)
    {
        return static_cast<int>(pos.x / 7) + (static_cast<int>(pos.y / 7) * 4);
    }

    void onAction(const ActionEvent& e) override {
        if (on_click) {
            on_click(selected);
        }
    }

    void onButton(const ButtonEvent& e) override
    {
        selected = hovered;
        Base::onButton(e);
    }

    void onLeave(const LeaveEvent& e) override
    {
        hovered = -1;
        Base::onLeave(e);
    }

    void onHover(const HoverEvent& e) override
    {
        hovered = item_index(e.pos);
        e.consume(this);
        Base::onHover(e);
    }

    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        wire = (0 == theme->name.compare("Wire"));
        pad.apply_theme(theme);
        pad_inactive.apply_theme(theme);
        pad_hover.apply_theme(theme);
        pad_sel.apply_theme(theme);
        return true;
    }

    void draw( const DrawArgs& args) override
    {
        //Base::draw(args);
        auto vg = args.vg;
        for (int i = 0; i < 16; ++i) {
            auto pos = item_pos(i);
            ElementStyle& style = active ? pad : pad_inactive;
            if (wire) {
                FittedBoxRect(vg, pos.x, pos.y, 6.f, 6.f, style.nvg_stroke_color(), Fit::Inside, style.width());
            } else {
                FillRect(vg, pos.x, pos.y, 6.f, 6.f, style.nvg_color());
            }
            if (active) {
                if (selected == i) {
                    if (wire) {
                        FittedBoxRect(vg, pos.x, pos.y, 6.f, 6.f, pad_sel.nvg_stroke_color(), Fit::Inside, pad_sel.width());
                    } else {
                        FillRect(vg, pos.x, pos.y, 6.f, 6.f, pad_sel.nvg_color());
                    }
                }
                if (hovered == i) {
                    FittedBoxRect(vg, pos.x, pos.y, 6.f, 6.f, pad_hover.nvg_stroke_color(), Fit::Inside, pad_hover.width());
                }
            }
        }
    }
};

}
