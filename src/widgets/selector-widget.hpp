// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../services/colors.hpp"
#include "../services/svt_rack.hpp"
using namespace svg_theme;
#include "TipWidget.hpp"

namespace pachde {

struct SelectorWidget: rack::app::ParamWidget, IApplyTheme
{
    using Base = rack::app::ParamWidget;

    bool bright;
    bool wire;
    int minimum;
    int maximum;
    float radius;
    float stem_width;
    NVGcolor head_color;
    NVGcolor stem_color;
    NVGcolor active_color;
    NVGcolor inactive_color;
    int hovered_item;
    TipHolder* tip_holder;

    SelectorWidget();
    virtual ~SelectorWidget();

    // IApplyTheme
    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override;

    // ParamWidget
    void initParamQuantity() override;

    int indexOfPos(Vec(pos));
    void ensure_tip_holder();
    void set_tip_text(std::string text);
    void destroy_tip();
    void create_tip();

    // rack
    void onHover(const HoverEvent& e) override;
    void onEnter(const EnterEvent& e) override;
    void onLeave(const LeaveEvent& e) override ;
    void onDragLeave(const DragLeaveEvent& e) override ;
    void onDragEnd(const DragEndEvent& e) override ;
    void onButton(const ButtonEvent& e) override ;
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;
};

}
