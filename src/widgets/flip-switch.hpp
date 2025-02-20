#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../services/svgtheme.hpp"

using namespace svg_theme;

namespace pachde {

struct FlipSwitch: ::rack::app::Switch, IApplyTheme
{
    using Base = ::rack::app::Switch;

    NVGcolor hi;
    NVGcolor lo;
    float hi_width;
    float lo_width;
    bool pressed;

    FlipSwitch();

    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override;
    //TODO: ctrl+click for reverse
    void onDragStart(const DragStartEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
    void draw(const DrawArgs& args) override;
};

}