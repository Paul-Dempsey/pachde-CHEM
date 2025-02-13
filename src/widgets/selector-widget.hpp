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

    SelectorWidget();

    // IApplyTheme
    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override;

    // ParamWidget
    void initParamQuantity() override;

    // rack
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;
};

}
