#include "preset-widget.hpp"
#include "../../services/colors.hpp"
#include "../../services/misc.hpp"
#include "../../widgets/hamburger.hpp"

namespace pachde {

constexpr const double DRAG_DELAY_INTERVAL = 0.35;

PresetWidget::PresetWidget()
:   grip_color(nvgRGBAf(0x9d, 0x91, 0x31, 0.2f)),
    live_color(nvgRGB(0x46, 0x40, 0x60)),
    current_color(nvgRGB(0xf9, 0xa5, 0x4b)),
    
    selected_color(nvgRGB(0x1e, 0x66, 0x2a)),
    selected_text_color(RampGray(G_75)),
    text_color(RampGray(G_65)),
    drag_color(nvgRGB(0x9d, 0x91, 0x31)),
    drag_current_color(nvgRGBAf(0x9d, 0x91, 0x31, 0.2f)),

    preset_list(nullptr),
    preset_index(-1),
    agent(nullptr),
    hover_grip(false),
    button_down(false),
    live(false),
    selected(false),
    current(false),
    hovered(false),
    wire_style(false),

    drag_started(false),
    dragging(false),
    drop_target(nullptr)
{
    box.size.x = 150.f;
    box.size.y = 20.f;
}

bool PresetWidget::applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme)
{
    wire_style = theme->name == "Wire";
    grip_color =          fromPacked(theme->getFillColor("preset-grip", true));
    live_color =          fromPacked(theme->getFillColor("preset-live", true));
    current_color =       fromPacked(theme->getFillColor("preset-current", true));
    live_text_color =     fromPacked(theme->getFillColor("preset-livetext", true));
    selected_color =      fromPacked(theme->getFillColor("preset-selected", true));
    selected_text_color = fromPacked(theme->getFillColor("preset-seltext", true));
    text_color =          fromPacked(theme->getFillColor("preset", true));
    drag_color =          fromPacked(theme->getFillColor("preset-drag", true));
    drag_current_color =  fromPacked(theme->getFillColor("preset-dragcur", true));
    hover_element.apply_theme(theme);
    return true;
}

void PresetWidget::clear_preset()
{
    preset_index = -1;
    live = selected = current = false;
    preset_id.invalidate();
    preset_name = "";
    //preset_name->describe("");
}

void PresetWidget::set_preset(int index, bool is_current, bool is_live, std::shared_ptr<PresetInfo> source)
{
    preset_index = index;
    live = is_live;
    current = is_current;
    if (index >= 0) {
        assert(static_cast<size_t>(index) < preset_list->size());
        preset_id = source->id;
        preset_name = source->name;
        //preset_name->describe(source->summary());
    } else {
        clear_preset();
    }
}

void PresetWidget::set_selected(bool select)
{
    selected = select;
}

std::shared_ptr<PresetInfo> PresetWidget::get_preset()
{
    if (preset_index < 0) return nullptr;
    auto pit = preset_list->cbegin() + preset_index;
    return *pit;
}

void PresetWidget::clear_states()
{
    hover_grip = false;
    button_down = false;
    live = false;
    selected = false;
    current = false;
}

void PresetWidget::onHover(const HoverEvent& e)
{
    hover_grip = (preset_index >= 0) && preset_id.valid() && (e.pos.x > box.size.x - GRIP_WIDTH);
    Base::onHover(e);
}

void PresetWidget::onEnter(const EnterEvent &e)
{
    Base::onEnter(e);
    hovered = preset_id.valid();
}

void PresetWidget::onLeave(const LeaveEvent& e)
{
    hover_grip = false;
    button_down = false;
    hovered = false;
    Base::onLeave(e);
}

void PresetWidget::onHoverKey(const HoverKeyEvent& e)
{
    Base::onHoverKey(e);
    if (preset_id == PresetId::InvalidKey) return;
    switch (e.key) {

    case GLFW_KEY_ENTER:
        if (drag_started && !dragging) {
            e.consume(this);
            stop_drag();
            agent->onChoosePreset(this);
            APP->event->setSelectedWidget(agent->widget());
        }
        break;

    case GLFW_KEY_SPACE:
        if (e.action == GLFW_RELEASE) {
            e.consume(this);
            set_selected(!selected);
            agent->onSetSelection(this, selected);
            APP->event->setSelectedWidget(agent->widget());
        }
        break;

    case GLFW_KEY_MENU:
        if (e.action == GLFW_RELEASE) {
            e.consume(this);
            createContextMenu();
        }
    }
}

void PresetWidget::onButton(const ButtonEvent&e)
{
    Base::onButton(e);
    if (preset_id == PresetId::InvalidKey) return;
    auto mod = e.mods & RACK_MOD_MASK;
    switch (e.button) {
    case GLFW_MOUSE_BUTTON_LEFT: {
        switch (e.action) {
        case GLFW_RELEASE:
            if (button_down)
            {
                if (mod) {
                    e.consume(this);
                    set_selected(!selected);
                    agent->onSetSelection(this, selected);
                    stop_drag();
                } else {
                    if (e.pos.x < box.size.x - GRIP_HIT) {
                        if (drag_started && !dragging) {
                            e.consume(this);
                            agent->onChoosePreset(this);
                            stop_drag();
                        }
                    } else {
                        e.consume(this);
                        set_selected(!selected);
                        agent->onSetSelection(this, selected);
                        stop_drag();
                    }
                }
                button_down = false;
                APP->event->setSelectedWidget(agent->widget());
            }
            break;

        case GLFW_PRESS:
            if (0 == mod) {
                e.consume(this);
                button_down = true;
                drag_pos = e.pos;
            }
            break;
        }
    } break;

    case GLFW_MOUSE_BUTTON_RIGHT: {
        if (0 == mod) {
            e.consume(this);
            switch (e.action) {
                case GLFW_RELEASE: {
                    createContextMenu();
                } break;
            }
        }
        break;

    } break;
    }
}

bool has_elapsed(double start, double interval)
{
    return (system::getTime() - start) > interval;
}

void PresetWidget::onDragStart(const DragStartEvent& e)
{
    if (drag_pos.x < box.size.x - GRIP_HIT) {
        e.consume(this);
        begin_drag();
    }
}

void PresetWidget::onDragEnd(const DragEndEvent& e)
{
    if (dragging) {
        if (drop_target != this) {
            agent->onDropPreset(drop_target);
        }
        stop_drag();
    }
}

void PresetWidget::onDragMove(const DragMoveEvent& e)
{
    drag_pos = drag_pos.plus(e.mouseDelta.div(getAbsoluteZoom()));
    if (dragging) {
        drop_target = agent->getDropTarget(this->box.pos + drag_pos);
    }
}

void PresetWidget::onDragHover(const DragHoverEvent& e)
{
    if (drag_started && !dragging) {
        if (has_elapsed(drag_delay_timer, DRAG_DELAY_INTERVAL)) {
            dragging = true;
            glfwSetCursor(APP->window->win, glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR));
            if (!selected) {
                agent->onClearSelection();
                set_selected(true);
                agent->onSetSelection(this, true);
            }
        }
    }
    e.consume(this); // allow DragEnter and DragLeave
}

void PresetWidget::onDragEnter(const DragEnterEvent& e)
{
}

void PresetWidget::onDragLeave(const DragLeaveEvent& e)
{
}

void PresetWidget::onPathDrop(const PathDropEvent& e)
{
    e.consume(this);
    agent->onDropFile(e);
}

void PresetWidget::appendContextMenu(ui::Menu* menu)
{
    bool disable = empty() || !agent;
    menu->addChild(createMenuLabel<HamburgerTitle>(preset_name));
    menu->addChild(createMenuItem("Remove", "", [this](){ agent->onDelete(this); }, disable));
    if (selected) {
        menu->addChild(createMenuItem("Un-select", "", [this](){ agent->onSetSelection(this, false); }, disable));
    } else {
        menu->addChild(createMenuItem("Select", "", [this](){ agent->onSetSelection(this, true); }, disable));
    }
    menu->addChild(createCheckMenuItem("Toggle current", "",
        [this](){ return is_current(); },
        [this](){ 
            set_current(!is_current());
            agent->onSetCurrrent(is_current() ? get_index() : -1);
        },
        disable
    ));
}

void PresetWidget::render(const DrawArgs& args, bool pre_base)
{
    auto vg = args.vg;

    if (pre_base) {
        if (current) {
            nvgBeginPath(vg);
            nvgMoveTo(vg, -6.f, box.size.y*.5f - 3.5f);
            nvgLineTo(vg,  0.f, box.size.y*.5f);
            nvgLineTo(vg, -6.f, box.size.y*.5f + 3.5f);
            nvgClosePath(vg);
            nvgFillColor(vg, current_color);
            nvgFill(vg);
        }

        if (button_down) {
            FillRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - 1.f, drag_current_color);
        } else if (hovered) {
            FillRect(vg, 0, 0, box.size.x, box.size.y, hover_element.nvg_color());
            FittedBoxRect(vg, 0, 0, box.size.x, box.size.y, hover_element.nvg_stroke_color(), Fit::Inside, hover_element.width());
        }

        if (dragging) {
            if (drop_target == this) {
                FillRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - 1.f, drag_current_color);
            }
            BoxRect(vg, 0, 0, box.size.x, box.size.y, drag_color);
        }

        if (selected && !dragging) {
            if (wire_style) {
                FillRect(args.vg, box.size.x - 8.f, box.size.y*.5f - 2.f, 4.25f, 4.25f, selected_color);
            } else {
                FillRect(args.vg, .25f, .25f, box.size.x - .5f, box.size.y - .5f, selected_color);
            }
        }

    } else {
 
        auto font = live ? GetPluginFontSemiBold() : GetPluginFontRegular();
        nvgSave(vg);
        auto co_text = selected ? selected_text_color : (live ? live_text_color : text_color);
        SetTextStyle(vg, font, co_text, 14.f);
        nvgTextAlign(vg, NVG_ALIGN_TOP|NVG_ALIGN_LEFT);
        nvgText(vg, 2.5f, 3.5f, preset_name.c_str(), nullptr);
        nvgRestore(vg);

        
        if (live) {
            nvgBeginPath(vg);
            float cx = box.size.x - .5f;
            nvgMoveTo(vg, cx, .5f);
            nvgLineTo(vg, cx - 8.5f, .5f);
            nvgLineTo(vg, cx - .5f, box.size.y *.35);
            nvgClosePath(vg);
            nvgFillColor(vg, live_color);
            nvgFill(vg);
        }
    
        if (hover_grip) {
            nvgBeginPath(vg);
            nvgMoveTo(vg, box.size.x - GRIP_WIDTH, 0.f);
            nvgLineTo(vg, box.size.x, 0.f);
            nvgLineTo(vg, box.size.x, box.size.y);
            nvgLineTo(vg, box.size.x - GRIP_WIDTH, box.size.y);
            nvgLineTo(vg, box.size.x - (GRIP_WIDTH + 4.f), box.size.y*.5f);
            nvgClosePath(vg);
            if (wire_style) {
                nvgStrokeColor(vg, grip_color);
                nvgStrokeWidth(vg, 0.5f);
                nvgStroke(vg);
            } else {
                nvgFillColor(vg, grip_color);
                nvgFill(vg);
            }
        }
    
        if (dragging) {
            if (drop_target && drop_target != this) {
                float cx = 3.f;
                float cy = 4.f;
                Vec pos = drop_target->box.pos.minus(box.pos);;
    
                nvgFillColor(vg, drag_color);
                nvgStrokeColor(vg, drag_color);
                nvgStrokeWidth(vg, 1.5f);
    
                nvgBeginPath(vg);
                nvgMoveTo(vg, pos.x - cx,  pos.y - cy);
                nvgLineTo(vg, pos.x + cx,  pos.y);
                nvgLineTo(vg, pos.x - cx,  pos.y + cy);
                nvgClosePath(vg);
                nvgFill(vg);
    
                nvgBeginPath(vg);
                nvgMoveTo(vg, pos.x,              pos.y);
                nvgLineTo(vg, pos.x + box.size.x, pos.y);
                nvgStroke(vg);
    
                nvgBeginPath(vg);
                nvgMoveTo(vg, pos.x + box.size.x + cx, pos.y + cy);
                nvgLineTo(vg, pos.x + box.size.x - cx, pos.y);
                nvgLineTo(vg, pos.x + box.size.x + cx, pos.y - cy);
                nvgClosePath(vg);
                nvgFill(vg);
            }
        }
   
    }
}

void PresetWidget::drawLayer(const DrawArgs& args, int layer)
{
    bool render_here = rack::settings::rackBrightness < .95f;
    if (render_here) {
        render(args, true);
    }

    Base::drawLayer(args, layer);

    if (render_here) {
        render(args, false);
    }

}

void PresetWidget::draw(const DrawArgs& args)
{
    bool render_here = rack::settings::rackBrightness >= .95f;
    if (render_here) {
        render(args, true);
    }
 
    Base::draw(args);
 
    if (render_here) {
        render(args, false);
    }
}

}