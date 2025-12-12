#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/svg-theme.hpp"

using namespace svg_theme;

namespace widgetry {

struct FlipSwitch: ::rack::app::Switch, IThemed
{
    using Base = ::rack::app::Switch;

    NVGcolor hi;
    NVGcolor lo;
    float hi_width;
    float lo_width;
    bool pressed;

    FlipSwitch();

    bool applyTheme(std::shared_ptr<svg_theme::SvgTheme> theme) override;
    //TODO: ctrl+click for reverse
    void onDragStart(const DragStartEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
    void draw(const DrawArgs& args) override;
};

}