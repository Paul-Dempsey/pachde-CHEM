#pragma once
#include <rack.hpp>
#include "../services/colors.hpp"
#include "../services/svgtheme.hpp"
#include "TipWidget.hpp"
using namespace ::rack;
using namespace svg_theme;
namespace pachde {

struct HamData : IApplyTheme
{
    uint8_t patties;
    float patty_width;
    NVGcolor patty_color;
    NVGcolor hover_color;
    bool hovered;

    HamData()
    :   patties(3),
        patty_width(1.5f),
        patty_color(RampGray(G_90)),
        hover_color(RampGray(G_65)),
        hovered(false)
        {}

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

    void draw(Widget* host, const Widget::DrawArgs& args)
    {
        auto vg = args.vg;
        auto co  = patty_color;

        if (hovered) {
            co = hover_color;
            auto half_x = host->box.size.x * .5f;
            auto half_y = host->box.size.y * .5f;
            OpenCircle(vg, half_x, half_y, half_x + .25f, co, .75f);
        }
        float y = 3.5f;
        float step = std::max(2.5f, patty_width + 1);
        for (auto n = 0; n < patties; ++n) {
            Line(vg, 2.f, y, host->box.size.x - 2.f, y, co, patty_width); y += step;
        }
    }

};

struct Hamburger : TipWidget, IApplyTheme
{
    using base = TipWidget;
    HamData data;

    Hamburger() {
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
        data.hovered = true;
        base::onEnter(e);
    }

    void onLeave(const LeaveEvent& e) override
    {
        data.hovered = false;
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
        return data.applyTheme(engine, theme);
    }

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);
        data.draw(this, args);
    }
};

template <class TBaseWidget>
struct HamburgerUi : TBaseWidget, IApplyTheme
{
    using Base = TBaseWidget;
    HamData data;

    HamburgerUi()
    {
        Base::box.size.x = 12.f;
        Base::box.size.y = 12.f;
    }

    void onHover(const ::rack::Widget::HoverEvent& e) override
    {
        e.consume(this);
        Base::onHover(e);
    }

    void onEnter(const ::rack::Widget::EnterEvent& e) override
    {
        data.hovered = true;
        Base::onEnter(e);
    }

    void onLeave(const ::rack::Widget::LeaveEvent& e) override
    {
        data.hovered = false;
        Base::onLeave(e);
    }

    void onButton(const ::rack::Widget::ButtonEvent& e) override
    {
        if (e.action == GLFW_PRESS && (e.mods & RACK_MOD_MASK) == 0) {
            switch (e.button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                Base::createContextMenu();
                e.consume(this);
                return;
            case GLFW_MOUSE_BUTTON_RIGHT:
                e.consume(this);
                return;
            }
        }
        Base::onButton(e);
    }

    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        return data.applyTheme(engine, theme);
    }

    void draw(const ::rack::Widget::DrawArgs& args) override
    {
        Base::draw(args);
        data.draw(this, args);
    }
};

}
