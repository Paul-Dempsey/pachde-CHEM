#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "click-region-widget.hpp"
#include "label-widget.hpp"

namespace pachde {

struct BitsWidget : OpaqueWidget, IApplyTheme
{
    using Base = OpaqueWidget;

    std::string name;
    uint64_t state{0};
    size_t count;
    int rows;
    float label_width;
    std::vector<ClickRegion*> hot_spots;
    std::vector<TextLabel*> labels;
    TextLabel* title;

    std::function<void(uint64_t item)> change_fn{nullptr};

    LabelStyle title_style{"options-title", TextAlignment::Center, 10.f, true};
    ElementStyle envelope{"options-box", "#282828", "hsl(0, 0%, 65%)", 1.25f};
    ElementStyle control_frame{"ctl-frame", "hsl(0, 0%, 55%)", "hsl(0, 0%, 55%)", .5f};
    ElementStyle control_glyph{"ctl-glyph", "hsl(0, 0%, 65%)", "hsl(0, 0%, 65%)", .25f};
    ElementStyle check_style{"option-check", "hsl(120, 50%, 50%)", "hsl(120, 50%, 50%)", .25f};

    BitsWidget(
        const std::string& name,
        int rows,
        float item_width,
        const std::vector<std::string>& items,
        SvgThemeEngine& theme_engine,
        std::shared_ptr<svg_theme::SvgTheme> theme,
        std::function<void(uint64_t state)> on_change
    );
    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override;
    uint64_t get_state() { return state; }
    void set_state(uint64_t the_state) { state = the_state; }
    std::string make_summary();
    Vec check_pos(int i);
    Rect exit_box_rect();
    void onSelectKey(const SelectKeyEvent& e) override;
    void onShow(const ShowEvent& e) override;
    void onHide(const HideEvent& e) override;
    void draw(const DrawArgs& args) override;
};

}