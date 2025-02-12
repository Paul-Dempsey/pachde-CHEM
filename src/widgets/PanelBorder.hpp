// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
using namespace ::rack;

#include "../services/svgtheme.hpp"
using namespace svg_theme;

bool isPeerModule(Module* me, Module* candidate);

namespace pachde {
    
struct PartnerPanelBorder : PanelBorder, IApplyTheme {
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

    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override;

    void draw(const DrawArgs& args) override;
};

void removePanelBorder(SvgPanel* panel);
void replacePanelBorder(SvgPanel* panel, PanelBorder* border);

PartnerPanelBorder* attachPartnerPanelBorder(rack::app::SvgPanel *panel, svg_theme::SvgThemeEngine& engine, std::shared_ptr<svg_theme::SvgTheme> theme);

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