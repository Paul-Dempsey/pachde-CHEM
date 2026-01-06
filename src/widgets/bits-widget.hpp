#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "click-region-widget.hpp"
#include "label.hpp"

namespace widgetry {

struct BitsWidget : OpaqueWidget, IThemed
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

    ElementStyle envelope{"options-box", colors::G18, colors::G55, 1.25f};
    ElementStyle control_frame{"ctl-frame", colors::G55, colors::G55, .5f};
    ElementStyle control_glyph{"ctl-glyph", colors::G65, colors::G65, .25f};
    ElementStyle check_style{"option-check", "hsl(120, 50%, 50%)", "hsl(120, 50%, 50%)", .25f};

    LabelStyle title_style{"options-title", HAlign::Center, 14.f, true};
    LabelStyle center_style{"choice", HAlign::Center, 12.f, false};
    LabelStyle left_style{"choice", HAlign::Left, 12.f, false};

    BitsWidget(
        const std::string& name,
        int rows,
        float item_width,
        std::vector<const char *>& items,
        std::shared_ptr<svg_theme::SvgTheme> theme,
        std::function<void(uint64_t state)> on_change
    );
    void applyTheme(std::shared_ptr<svg_theme::SvgTheme> theme) override;
    uint64_t get_state() { return state; }
    void set_state(uint64_t the_state) { state = the_state; }
    std::string make_summary();
    Vec check_pos(int i);
    Rect exit_box_rect();
    void close();
    void select_item(int id, int mods);
    void onSelectKey(const SelectKeyEvent& e) override;
    void onShow(const ShowEvent& e) override;
    void onHide(const HideEvent& e) override;
    void draw(const DrawArgs& args) override;
};

}