#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/colors.hpp"
#include "services/text.hpp"
#include "services/svg-theme.hpp"
#include "services/theme.hpp"
#include "tip-widget.hpp"
using namespace svg_theme;
using namespace pachde;
namespace widgetry {

struct HamData
{
    float patty_width{1.5f};
    float circle_width{.75f};
    NVGcolor co_patty{RampGray(G_50)};
    PackedColor co_patty_hover{colors::G50};
    PackedColor co_hover{colors::G65};
    bool hovered{false};

    void applyTheme(std::shared_ptr<SvgTheme> theme) {
        if (theme) {
            PackedColor color;
            if (theme->getFillColor(color, "patty", true)) {
                co_patty = fromPacked(color);
            } else {
                co_patty = RampGray(G_50);
            }
            auto style = theme->getStyle("patty-hover");
            if (style) {
                co_patty_hover = style->fillWithOpacity();
                co_hover = style->strokeWithOpacity();
                circle_width = style->stroke_width;
            } else {
                co_hover = colors::G65;
            }
        } else {
            co_patty = RampGray(G_50);
            co_hover = colors::G65;
            circle_width = .75f;
        }
    }

    void draw(Widget* host, const Widget::DrawArgs& args) {
        auto vg = args.vg;

        if (hovered) {
            auto half_x = host->box.size.x * .5f;
            auto half_y = host->box.size.y * .5f;
            OpenCircle(vg, half_x, half_y, half_x + .25f, fromPacked(co_hover), circle_width);
        }
        float y = 3.5f;
        float step = std::max(2.5f, patty_width + 1);
        float right = host->box.size.x - 2.f;

        nvgBeginPath(vg);
        nvgMoveTo(vg, 2.f, y); nvgLineTo(vg, right, y); y += step;
        nvgMoveTo(vg, 2.f, y); nvgLineTo(vg, right, y); y += step;
        nvgMoveTo(vg, 2.f, y); nvgLineTo(vg, right, y);
        nvgStrokeColor(vg, hovered ? fromPacked(co_patty_hover) : co_patty);
        nvgStrokeWidth(vg, patty_width);
        nvgStroke(vg);
    }

};

struct HamburgerTitle : MenuLabel
{
    NVGcolor co_bg;
    NVGcolor co_text;

    HamburgerTitle() {
        using namespace theme;
        switch (get_rack_ui_theme()) {
        case RackUiTheme::Light:
            co_bg = nvgHSL(200.f/360.f,.5,.4);
            co_text = nvgRGB(250, 250, 250);
            break;
        case RackUiTheme::Dark:
            co_bg = nvgHSL(200.f/360.f,.5,.4);
            co_text = nvgRGB(250, 250, 250);
            break;
        case RackUiTheme::HighContrast:
            co_bg = nvgRGB(254, 254, 254);
            co_text = nvgRGB(0,0,0);
            break;
        }
    }

    void draw(const DrawArgs& args) override{
        auto vg = args.vg;
        auto font = GetPluginFontSemiBold();
        if (!FontOk(font)) return;
        FillRect(vg, 0, 0, box.size.x, box.size.y - 1.f, co_bg);
        SetTextStyle(vg, font, co_text, 16.f);
        CenterText(vg, box.size.x*.5, 14.f, text.c_str(), nullptr);
    }
};

struct Hamburger : TipWidget, IThemed
{
    using Base = TipWidget;
    HamData data;

    Hamburger() {
        box.size.x = 12.f;
        box.size.y = 12.f;
    }

    void createContextMenu() {
        ui::Menu* menu = createMenu();
    	appendContextMenu(menu);
    }

    void onHover(const HoverEvent& e) override {
        e.consume(this);
        Base::onHover(e);
    }

    void onEnter(const EnterEvent& e) override {
        data.hovered = true;
        Base::onEnter(e);
    }

    void onLeave(const LeaveEvent& e) override {
        data.hovered = false;
        Base::onLeave(e);
    }

    void onButton(const ::rack::Widget::ButtonEvent& e) override {
        if ((e.action == GLFW_PRESS) && ((e.mods & RACK_MOD_MASK) == 0)) {
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

    void applyTheme(std::shared_ptr<SvgTheme> theme) override {
        data.applyTheme(theme);
    }

    void onHoverKey(const HoverKeyEvent& e) override {
        switch (e.key) {
            case GLFW_KEY_ENTER:
            case GLFW_KEY_MENU:
            if (e.action == GLFW_RELEASE) {
                e.consume(this);
                createContextMenu();
                return;
            }
        }
        Base::onHoverKey(e);
    }

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);
        data.draw(this, args);
    }
};

template <class TBaseWidget>
struct HamburgerUi : TBaseWidget
{
    using Base = TBaseWidget;
    HamData data;

    HamburgerUi() {
        Base::box.size.x = 12.f;
        Base::box.size.y = 12.f;
    }

    void onHover(const ::rack::Widget::HoverEvent& e) override {
        e.consume(this);
        Base::onHover(e);
    }

    void onEnter(const ::rack::Widget::EnterEvent& e) override {
        data.hovered = true;
        Base::onEnter(e);
    }

    void onLeave(const ::rack::Widget::LeaveEvent& e) override {
        data.hovered = false;
        Base::onLeave(e);
    }

    void onButton(const ::rack::Widget::ButtonEvent& e) override {
        if ((e.action == GLFW_PRESS) && ((e.mods & RACK_MOD_MASK) == 0)) {
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

    void applyTheme(std::shared_ptr<SvgTheme> theme) { data.applyTheme(theme); }

    void draw(const ::rack::Widget::DrawArgs& args) override {
        Base::draw(args);
        data.draw(this, args);
    }
};

using HamParam = HamburgerUi<ParamWidget>;

}
