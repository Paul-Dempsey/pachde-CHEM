#pragma once
#include <rack.hpp>
#include "../../services/svt_rack.hpp"
#include "../../widgets/element-style.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "../../em/preset.hpp"

using namespace ::rack;
using namespace ::svg_theme;
using namespace ::eaganmatrix;
namespace pachde {

constexpr const float GRIP_WIDTH = 10.f;
constexpr const float GRIP_HIT   = 15.f;
class PresetWidget;

struct IPresetAction {
    virtual void onClearSelection() = 0;
    virtual void onSetSelection(PresetWidget* source, bool on) = 0;
    virtual void onDelete(PresetWidget* source) = 0;
    virtual void onSetCurrrent(int index) = 0;
    virtual void onDropFile(const widget::Widget::PathDropEvent& e) = 0;
    virtual void onChoosePreset(PresetWidget* source) = 0;
    virtual PresetWidget* getDropTarget(Vec pos) = 0;
    virtual void onDropPreset(PresetWidget* target) = 0;
    virtual Widget* widget() = 0;
};

class PresetWidget : public OpaqueWidget, public IApplyTheme
{
    using Base = OpaqueWidget;

    std::string preset_name;

    NVGcolor grip_color;
    NVGcolor live_color;
    NVGcolor current_color;
    NVGcolor live_text_color;
    NVGcolor selected_color;
    NVGcolor selected_text_color;
    NVGcolor text_color;
    NVGcolor drag_color;
    NVGcolor drag_current_color;
    ElementStyle hover_element{"preset-hover", "hsla(0, 0%, 100%, 5%)", "hsl(120, 50%, 30%)", .5f};

    std::deque<std::shared_ptr<PresetInfo>>* preset_list;
    int preset_index;
    PresetId preset_id;
    IPresetAction* agent;
    
    bool hover_grip;
    bool button_down;
    bool live;
    bool selected;
    bool current;
    bool hovered;
    bool wire_style;

    bool drag_started;
    bool dragging;
    double drag_delay_timer;
    Vec drag_pos;
    PresetWidget* drop_target;

public:
    PresetWidget();
    void set_text(const std::string & name) {
        preset_name = name;
    }
    bool empty() {
        //assert(((preset_index >= 0 && preset_id.valid())) || ((-1 == preset_index) && !preset_id.valid()));
        return !preset_id.valid();
    }
    void clear_states();
    bool is_current() { return current; }
    void set_current(bool enable) { current = enable; }
    bool is_selected() { return selected; }
    void set_selected(bool select);
    int get_index() { return preset_index; }
    bool is_live() { return live; }
    void set_live(bool state) { live = state; }
    PresetId get_preset_id() { return preset_id; }
    std::shared_ptr<PresetInfo> get_preset();
    void set_preset(int index, bool is_current, bool is_live, std::shared_ptr<PresetInfo> preset);
    void clear_preset();
    void set_preset_list(std::deque<std::shared_ptr<PresetInfo>>* presets) {
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
    void onEnter(const EnterEvent& e) override;
    void onLeave(const LeaveEvent& e) override;

    void begin_drag() {
        drag_started = true;
        dragging = false;
        drag_delay_timer = system::getTime();
    }
    void stop_drag() {
        drag_started = dragging = false;
        glfwSetCursor(APP->window->win, NULL);
    }
    void onDragStart(const DragStartEvent& e) override;
    void onDragEnd(const DragEndEvent& e) override;
    void onDragMove(const DragMoveEvent& e) override;
    void onDragHover(const DragHoverEvent& e) override;
    void onDragEnter(const DragEnterEvent& e) override;
    void onDragLeave(const DragLeaveEvent& e) override;

    void render(const DrawArgs& args, bool pre_base);
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;
};

inline PresetWidget* createPresetWidget(IPresetAction* agent, std::deque<std::shared_ptr<PresetInfo>>* presets, float x, float y, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    auto o = createThemedWidget<PresetWidget>(Vec(x,y), engine, theme);
    o->set_agent(agent);
    o->set_preset_list(presets);
    return o;
}

}