#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/colors.hpp"
using namespace pachde;

namespace widgetry {

struct FancyLabel : MenuLabel
{
    NVGcolor co_bg;
    NVGcolor co_text;
    NVGcolor co_rule{nvgHSL(200.f/360.f,.5,.4)};
    NVGalign align{NVG_ALIGN_CENTER};

    FancyLabel();

    void backgroundColor(PackedColor color) { co_bg = fromPacked(color); }
    void textColor(PackedColor color) { co_text = fromPacked(color); }
    void setTextAlignment(NVGalign a) { align = (NVGalign)(a & (NVG_ALIGN_LEFT|NVG_ALIGN_CENTER|NVG_ALIGN_RIGHT)); }
    void draw(const DrawArgs& args) override;
};

struct OptionMenuEntry : rack::ui::MenuEntry {
    rack::ui::MenuItem* child_item;
    bool selected{false};

    OptionMenuEntry(rack::ui::MenuItem* item);
    OptionMenuEntry(bool selected, rack::ui::MenuItem* item);
    void step() override;
    void draw(const DrawArgs& args) override;
};

}