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
    uint8_t patties;
    NVGcolor patty_color;
    NVGcolor hover_color;
    bool hovered;

    Hamburger()
    :   patties(3),
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
        TipWidget::onHover(e);
        e.consume(this);
    }
    void onEnter(const EnterEvent& e) override
    {
        TipWidget::onEnter(e);
        hovered = true;
    }
    void onLeave(const LeaveEvent& e) override
    {
        TipWidget::onLeave(e);
        hovered = false;
    }

    void onButton(const ButtonEvent& e) override
    {
        TipWidget::onButton(e);
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
            createContextMenu();
            e.consume(this);
        }
    }

    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        patty_color = fromPacked(theme->getFillColor("patty"));
        hover_color = fromPacked(theme->getFillColor("patty-hover"));
        return true;
    }

    void draw(const DrawArgs& args) override
    {
        TipWidget::draw(args);

        auto vg = args.vg;

        if (hovered) {
            auto half_x = box.size.x * .5f;
            auto half_y = box.size.y * .5f;
            OpenCircle(vg, half_x, half_y, half_x-1.f, hover_color, .75f);
        }
        float y = 3.5f;
        const float step = 2.5f;
        const float line_width = 1.5f;
        for (auto n = 0; n < patties; ++n) {
            Line(vg, 2.f, y, box.size.x - 2.f, y, patty_color, line_width); y += step;
        }
    }
};

}
