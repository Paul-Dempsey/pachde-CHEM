#pragma once
#include <rack.hpp>
#include "services/colors.hpp"
#include "services/svg-theme.hpp"
#include "widgets/tip-widget.hpp"
using namespace ::rack;
using namespace svg_theme;
namespace widgetry {

struct InfoSymbol: OpaqueWidget, IThemed
{
    using Base = OpaqueWidget;
    TipHolder* tip_holder{nullptr};

    PackedColor color{colors::G85};
    PackedColor co_hover{0xff06809a};
    bool hovered{false};
    std::function<void()> click_handler{nullptr};

    InfoSymbol() {
        box.size.x = box.size.y = 15.f;
    }

    void ensureTipHolder() { if (!tip_holder) { tip_holder = new TipHolder(); } }
    void describe(std::string text) { ensureTipHolder(); tip_holder->setText(text); }
    void destroyTip() { if (tip_holder) { tip_holder->destroyTip(); } }
    void createTip() { ensureTipHolder(); if (tip_holder) { tip_holder->createTip(); }}

    void set_handler(std::function<void()> fn) { click_handler = fn; }
    void applyTheme(std::shared_ptr<SvgTheme> theme) override;
    void onHover(const HoverEvent& e) override { hovered = true; Base::onHover(e); e.consume(this); }
    void onEnter(const EnterEvent& e) override { hovered = true; createTip(); Base::onEnter(e); }
    void onLeave(const LeaveEvent& e) override { hovered = false; destroyTip(); Base::onLeave(e); }
    void onButton(const ButtonEvent& e) override;
    void draw(const DrawArgs& args) override;
};

}