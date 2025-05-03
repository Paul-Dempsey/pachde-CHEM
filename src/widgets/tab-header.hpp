#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../services/text.hpp"
#include "../services/svt_rack.hpp"
#include "element-style.hpp"
#include "TipWidget.hpp"

using namespace svg_theme;

namespace pachde {

struct TabHeader: OpaqueWidget, IApplyTheme
{
    using Base = OpaqueWidget;
    const float cell = 12.f;

    int n_items{8};
    int current_item{0};
    int hovered_item{-1};

    ElementStyle item_style{"tabh-item", "", "#a6a6a6", .75f};
    ElementStyle text_style{"tabh-text", "#a6a6a6"};
    ElementStyle current_style{"tabh-sel", "hsl(60,80%,75%)", "hsl(60,80%,75%)", .25f};
    ElementStyle current_text_style{"tabh-sel-text", "#000000"};
    ElementStyle hover_style{"tabh-hover", "hsl(260,50%,80%)", "", .75f};
    ElementStyle hover_text_style{"tabh-hover-text", "#000000"};

    TipHolder* tip_holder{nullptr};

    TabHeader(Vec pos, int count) : n_items(count) {
        box.pos = pos;
    }
    ~TabHeader() {
        if (tip_holder) delete tip_holder;
        tip_holder = nullptr;
    }
    void set_current_item(int index) {
        current_item = ::rack::math::clamp(index, 0, n_items-1);
    }
    std::function<void(int item)> on_item_change{nullptr};
    void set_on_item_change(std::function<void(int item)> handler) { on_item_change = handler; }

    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override;

    void ensure_tip_holder() { if (!tip_holder) tip_holder = new TipHolder(); }
    void set_tip_text(std::string text) { ensure_tip_holder(); tip_holder->setText(text); }
    void destroy_tip() { if (tip_holder) { tip_holder->destroyTip(); } };
    void create_tip() { ensure_tip_holder(); if (tip_holder) tip_holder->createTip(); }
    int index_of_pos(Vec pos);

    void onHover(const HoverEvent& e) override;
    void onEnter(const EnterEvent& e) override;
    void onLeave(const LeaveEvent& e) override ;
    void onDragLeave(const DragLeaveEvent& e) override ;
    void onDragEnd(const DragEndEvent& e) override ;
    void onButton(const ButtonEvent& e) override ;

    void draw(const DrawArgs& args) override;
};

}