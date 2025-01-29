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
    // NVGcolor coLink = GetStockColor(StockColor::Coral);
    // NVGcolor coLink2 = nvgTransRGBAf(coLink, 0.f);
    auto vg = args.vg; 
    if (left || right) {
        Line(vg, 0.5f, 0.5f, box.size.x - 1.0f, 0.5f, panel_border_color);
        Line(vg, 0.5f, box.size.y - 1.f, box.size.x - 1.0f, box.size.y - 1.f, panel_border_color);
        if (left) {
            // gliss
            // float h = box.size.y *.1f;
            // float y1 = box.size.y *.2 - h;
            // float y2 = box.size.y *.2;
            // GradientRect(vg, 0.5f, y1, .75f, h, coLink2, coLink, y1, y2);
            // GradientRect(vg, 0.5f, y2, .75f, h, coLink, coLink2, y2, y2 + h);

            // stiches
            // float y = 7.5f;
            // Line(vg, 0.5f, y, 3.5, y, coLink, 1.f);
            // y += 1.5f;
            // Line(vg, 0.5f, 9.f, 3.5, 9.f, coLink, 1.f);

            // y = box.size.y *.5f;
            // Line(vg, 0.5f, y, 3.5, y, coLink, 1.f);
            // y += 1.5f;
            // Line(vg, 0.5f, y, 3.5, y, coLink, 1.f);

            // y = box.size.y - 7.5f;
            // Line(vg, 0.5f, y, 3.5, y, coLink, 1.f);
            // y += 1.5f;
            // Line(vg, 0.5f, y, 3.5, y, coLink, 1.f);

        } else {
            Line(args.vg, 0.5f, 0.5f, 0.5f, box.size.y - 1.0f, panel_border_color);
        }
        if (right) {
            // gliss
            // float h = box.size.y *.1f;
            // float y1 = box.size.y *.2 - h;
            // float y2 = box.size.y *.2;
            // GradientRect(vg, box.size.x - .75f, y1, .75f, h, coLink2, coLink, y1, y2);
            // GradientRect(vg, box.size.x - .75f, y2, .75f, h, coLink, coLink2, y2, y2 + h);

            // stiches
            // float y = 7.5f;
            // Line(vg, box.size.x - 3.5f, y, box.size.x - 1.f, y, coLink, 1.f);
            // y += 1.5f;
            // Line(vg, box.size.x - 3.5f, y, box.size.x - 1.f, y, coLink, 1.f);

            // y = box.size.y *.5f;
            // Line(vg, box.size.x - 3.5f, y, box.size.x - 1.f, y, coLink, 1.f);
            // y += 1.5f;
            // Line(vg, box.size.x - 3.5f, y, box.size.x - 1.f, y, coLink, 1.f);

            // y = box.size.y - 7.5f;
            // Line(vg, box.size.x - 3.5f, y, box.size.x - 1.f, y, coLink, 1.f);
            // y += 1.5f;
            // Line(vg, box.size.x - 3.5f, y, box.size.x - 1.f, y, coLink, 1.f);

        } else {
            Line(args.vg, box.size.x - 1.f, 0.5f, box.size.x - 1.f, box.size.y - 1.0f, panel_border_color);
        }
    } else {
        BoxRect(args.vg, 0.5f, 0.5f, box.size.x - 1.f, box.size.y - 1.f, panel_border_color);
    }
}

}