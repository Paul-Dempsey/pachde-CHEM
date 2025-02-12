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

PartnerPanelBorder* attachPartnerPanelBorder(rack::app::SvgPanel *panel, svg_theme::SvgThemeEngine& engine, std::shared_ptr<svg_theme::SvgTheme> theme)
{
    auto panelBorder = new PartnerPanelBorder();
    panelBorder->applyTheme(engine, theme);
    replacePanelBorder(panel, panelBorder);
    return panelBorder;
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

bool PartnerPanelBorder::applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme)
{
    auto style = theme->getStyle("panel-border");
    if (style) {
        link_color = fromPacked(style->strokeWithOpacity());
        line_stroke_width = ring_stroke_width = style->stroke_width;
    }
    if (!style || !isColorVisible(link_color)) {
        link_color = nvgRGB(0xf9, 0xa5, 0x4b);
    }
    if (!style) {
        line_stroke_width = ring_stroke_width = 0.5f;
    }
    return true;
}

void PartnerPanelBorder::draw(const DrawArgs& args)
{
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

            // dotted
            // float y = 10.f;
            // while (y < 380.f) {
            //     Line(vg, 0.5f, y, 0.5f, y + 1.f, coLink, 1.f);
            //     y += 10.f;
            // }
            
            OpenCircle(vg, 0.f, 6.25, 2.5f, link_color, ring_stroke_width);
            Line(vg, .5f, 6.25f, 4.f, 6.25f, link_color, line_stroke_width);

        } else {
            Line(args.vg, 0.5f, 0.5f, 0.5f, box.size.y - 1.0f, panel_border_color);
        }
        if (right) {
            OpenCircle(vg, box.size.x, 6.25, 2.5f, link_color, ring_stroke_width);
            Line(vg, box.size.x - 4.f, 6.25f, box.size.x - .5f, 6.25f, link_color, line_stroke_width); 

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