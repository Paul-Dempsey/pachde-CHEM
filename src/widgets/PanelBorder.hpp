// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
using namespace ::rack;

bool isPeerModule(Module* me, Module* candidate);

namespace pachde {
    
struct PartnerPanelBorder : PanelBorder {
    bool left = false;
    bool right = false;
    void setPartners(bool isLeft, bool isRight);
	void draw(const DrawArgs& args) override;
};

void removePanelBorder(SvgPanel* panel);
void replacePanelBorder(SvgPanel* panel, PanelBorder* border);

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