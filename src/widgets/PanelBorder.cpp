// Copyright (C) Paul Chase Dempsey
#include "../services/colors.hpp"
#include "PanelBorder.hpp"

namespace pachde {

void removePanelBorder(SvgPanel* panel)
{
    if (panel->panelBorder) {
        auto child = panel->panelBorder;
        panel->panelBorder = nullptr;
        panel->fb->removeChild(child);
        panel->fb->setDirty();
    }
}

void replacePanelBorder(SvgPanel* panel, PanelBorder* border) {
    removePanelBorder(panel);
	border->box.size = panel->fb->box.size;
    panel->panelBorder = border;
    panel->fb->addChild(border);
    panel->fb->setDirty();
}

void PartnerPanelBorder::setPartners(bool isLeft, bool isRight)
{
    left = isLeft;
    right = isRight;
    auto parent = reinterpret_cast<FramebufferWidget*>(getParent());
    if (parent) {
        parent->setDirty();
    }
}

void PartnerPanelBorder::draw(const DrawArgs& args)
{
    if (left || right) {
        Line(args.vg, 0.5f, 0.5f, box.size.x - 1.0f, 0.5f, panel_border_color);
        Line(args.vg, 0.5f, box.size.y - 1.f, box.size.x - 1.0f, box.size.y - 1.f, panel_border_color);
        if (!left) {
            Line(args.vg, 0.5f, 0.5f, 0.5f, box.size.y - 1.0f, panel_border_color);
        }
        if (!right) {
            Line(args.vg, box.size.x - 1.f, 0.5f, box.size.x - 1.f, box.size.y - 1.0f, panel_border_color);
        }
    } else {
        BoxRect(args.vg, 0.5f, 0.5f, box.size.x - 1.f, box.size.y - 1.f, panel_border_color);
    }
}

}