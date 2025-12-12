// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
using namespace ::rack;

#include "services/svg-theme.hpp"
using namespace svg_theme;

bool isPeerModule(::rack::engine::Module* me, ::rack::engine::Module* candidate);

namespace widgetry {

struct PartnerPanelBorder : ::rack::app::PanelBorder, IThemed
{
    bool left{false};
    bool right{false};

    NVGcolor link_color{nvgRGB(0xf9, 0xa5, 0x4b)}; //  "brand": "#f9a54b"
    float line_stroke_width{0.5f};
    float ring_stroke_width{0.5f};
    void set_appearance(NVGcolor color, float line, float ring) {
        link_color = color;
        line_stroke_width = line;
        ring_stroke_width = ring;
    }
    void setPartners(bool isLeft, bool isRight);

    bool applyTheme(std::shared_ptr<SvgTheme> theme) override;

    void draw(const DrawArgs& args) override;
};

void removePanelBorder(::rack::app::SvgPanel* panel);
void replacePanelBorder(::rack::app::SvgPanel* panel, ::rack::app::PanelBorder* border);

PartnerPanelBorder* attachPartnerPanelBorder(::rack::app::SvgPanel *panel, std::shared_ptr<svg_theme::SvgTheme> theme);

template<class TModuleWidget>
void setPartnerPanelBorder(TModuleWidget* me)
{
    if (me->module && me->panelBorder) {
        bool leftPartner = isPeerModule(me->module, me->module->getLeftExpander().module);
        bool rightPartner = isPeerModule(me->module, me->module->getRightExpander().module);
        me->panelBorder->setPartners(leftPartner, rightPartner);
    }
}

}