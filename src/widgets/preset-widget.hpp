#pragma once
#include <rack.hpp>
#include "../services/svt_rack.hpp"
#include "tip-label-widget.hpp"
#include "../em/preset.hpp"

using namespace ::rack;
using namespace ::svg_theme;

namespace pachde {
//"preset-grip": { "fill":"#9d9131", "opacity": 0.2 }
struct PresetWidget : OpaqueWidget, IApplyTheme
{
    using Base = OpaqueWidget;

    TipLabel* preset_name;
    NVGcolor grip_color;
    NVGcolor dot_color;
    bool hover_grip;

    std::vector<std::shared_ptr<PresetDescription>>* preset_list;
    int preset_index;
    PresetId preset_id;
    bool live;
    bool selected;

    PresetWidget();
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override;
    void set_preset(int index, std::shared_ptr<PresetDescription> preset);
    std::shared_ptr<PresetDescription> get_preset();

    void onHover(const HoverEvent& e) override
    {
        hover_grip = (preset_index >= 0) && get_preset()->id.valid() && (e.pos.x > box.size.x - 10.f);
        Base::onHover(e);
        e.consume(this);
    }
    void onLeave(const LeaveEvent& e) override
    {
        hover_grip = false;
        Base::onLeave(e);
    }

    void draw(const DrawArgs& args) override;
};

inline PresetWidget* createPresetWidget(std::vector<std::shared_ptr<PresetDescription>>* presets, float x, float y, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    auto o = createThemedWidget<PresetWidget>(Vec(x,y), engine, theme);
    o->preset_list = presets;
    return o;
}

}