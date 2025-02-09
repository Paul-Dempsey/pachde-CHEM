#pragma once
#include <rack.hpp>
#include "../services/colors.hpp"
#include "../services/svgtheme.hpp"
#include "TipWidget.hpp"
using namespace ::rack;
using namespace svg_theme;
namespace pachde {

struct Hamburger : TipWidget, IApplyTheme
{
    using base = TipWidget;
    uint8_t patties;
    float patty_width;
    NVGcolor patty_color;
    NVGcolor hover_color;
    bool hovered;

    Hamburger()
    :   patties(3),
        patty_width(1.5f),
        patty_color(RampGray(G_90)),
        hover_color(RampGray(G_65)),
        hovered(false)
    {
        box.size.x = 12.f;
        box.size.y = 12.f;
    }

    void createContextMenu() {
        ui::Menu* menu = createMenu();
    	appendContextMenu(menu);
    }

    void onHover(const HoverEvent& e) override
    {
        e.consume(this);
        base::onHover(e);
    }

    void onEnter(const EnterEvent& e) override
    {
        hovered = true;
        base::onEnter(e);
    }

    void onLeave(const LeaveEvent& e) override
    {
        hovered = false;
        base::onLeave(e);
    }

    void onButton(const ButtonEvent& e) override
    {
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
            createContextMenu();
            e.consume(this);
        }
        base::onButton(e);
    }

    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        auto style = theme->getStyle("patty");
        if (style) {
            patty_width = style->isApplyStrokeWidth() ? style->stroke_width : 1.5f;
            patty_color = fromPackedOrDefault(style->strokeWithOpacity(), RampGray(G_50));
        } else {
            patty_width = 1.5f;
            patty_color = RampGray(G_50);
        }
        hover_color = fromPackedOrDefault(theme->getStrokeColor("patty-hover", true), RampGray(G_40));
        return true;
    }

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);

        auto vg = args.vg;

        if (hovered) {
            auto half_x = box.size.x * .5f;
            auto half_y = box.size.y * .5f;
            OpenCircle(vg, half_x, half_y, half_x-1.f, hover_color, .75f);
        }
        float y = 3.5f;
        float step = std::max(2.5f, patty_width + 1);
        for (auto n = 0; n < patties; ++n) {
            Line(vg, 2.f, y, box.size.x - 2.f, y, patty_color, patty_width); y += step;
        }
    }
};

}
