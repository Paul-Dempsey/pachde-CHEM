#pragma once
#include <rack.hpp>
#include "../services/svt_rack.hpp"
#include "tip-label-widget.hpp"
#include "../em/preset.hpp"

using namespace ::rack;
using namespace ::svg_theme;

namespace pachde {

constexpr const float GRIP_WIDTH = 10.f;
constexpr const float GRIP_HIT   = 15.f;
struct PresetWidget;

struct IPresetAction {
    virtual void onSetSelection(PresetWidget* source, bool on) = 0;
    virtual void onDelete(PresetWidget* source) = 0;
    virtual void onDropFile(const widget::Widget::PathDropEvent& e) = 0;
    virtual void onChoosePreset(PresetWidget* source) = 0;
    virtual PresetWidget* getDropTarget(Vec pos) = 0;

};

class PresetWidget : public OpaqueWidget, public IApplyTheme
{
    using Base = OpaqueWidget;

    TipLabel* preset_name;
    NVGcolor grip_color;
    NVGcolor dot_color;
    NVGcolor selected_color;
    NVGcolor selected_text_color;
    NVGcolor text_color;
    NVGcolor drag_color;
    NVGcolor drag_current_color;

    std::deque<std::shared_ptr<PresetDescription>>* preset_list;
    int preset_index;
    PresetId preset_id;
    IPresetAction* agent;
    
    bool hover_grip;
    bool button_down;
    bool live;
    bool selected;

    bool wire_style;

    bool drag_started;
    bool dragging;
    double drag_delay_timer;
    Vec drag_pos;
    PresetWidget* drop_target;

public:
    PresetWidget();
    TipLabel& get_label() { return *preset_name; }

    void clear_states();
    bool get_selected() { return selected; }
    void set_selected(bool select);
    int get_index() { return preset_index; }
    bool get_live();
    void set_live(bool state) { live = state; }
    PresetId get_preset_id() { return preset_id; }

    std::shared_ptr<PresetDescription> get_preset();
    void set_preset(int index, std::shared_ptr<PresetDescription> preset);
    void set_preset_list(std::deque<std::shared_ptr<PresetDescription>>* presets) {
        preset_list = presets;
    }
    void set_agent(IPresetAction* client) { agent = client; }

    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override;

    void appendContextMenu(ui::Menu* menu);
    void createContextMenu()
    {
        ui::Menu* menu = createMenu();
    	appendContextMenu(menu);
    }

    void onButton(const ButtonEvent&e) override;
    void onHover(const HoverEvent& e) override;
    void onHoverKey(const HoverKeyEvent& e) override;
    void onPathDrop(const PathDropEvent& e) override;
    void onLeave(const LeaveEvent& e) override;

    void begin_drag() {
        drag_started = true;
        dragging = false;
        drag_delay_timer = system::getTime();
    }
    void stop_drag() {
        drag_started = dragging = false;
    }
    void onDragStart(const DragStartEvent& e) override;
    void onDragEnd(const DragEndEvent& e) override;
    void onDragMove(const DragMoveEvent& e) override;
    void onDragHover(const DragHoverEvent& e) override;
    void onDragEnter(const DragEnterEvent& e) override;
    void onDragLeave(const DragLeaveEvent& e) override;

    void draw(const DrawArgs& args) override;
};

inline PresetWidget* createPresetWidget(std::deque<std::shared_ptr<PresetDescription>>* presets, float x, float y, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    auto o = createThemedWidget<PresetWidget>(Vec(x,y), engine, theme);
    o->set_preset_list(presets);
    return o;
}

}