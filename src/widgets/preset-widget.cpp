#include "preset-widget.hpp"
#include "../services/colors.hpp"
#include "../services/misc.hpp"
namespace pachde {

constexpr const double DRAG_DELAY_INTERVAL = 0.35;

PresetWidget::PresetWidget()
:   preset_name(nullptr),
    grip_color(nvgRGBAf(0x9d, 0x91, 0x31, 0.2f)),
    dot_color(nvgRGB(0x46, 0x40, 0x60)),
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
    wire_style(false),

    drag_started(false),
    dragging(false),
    drop_target(nullptr)
{
    box.size.x = 150.f;
    box.size.y = 21.f;
}

bool PresetWidget::applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme)
{
    if (!preset_name) {
        LabelStyle style{"preset", TextAlignment::Left, 14.f};
        preset_name = createStaticTextLabel<TipLabel>(Vec(2.5f, 2.5f), 145.f, "", theme_engine, theme, style);
        addChild(preset_name);
    }
    wire_style = theme->name == "Wire";
    grip_color =          fromPacked(theme->getFillColor("preset-grip", true));
    dot_color =           fromPacked(theme->getFillColor("cur-dot", true));
    selected_color =      fromPacked(theme->getFillColor("preset-selected", true));
    selected_text_color = fromPacked(theme->getFillColor("preset-seltext", true));
    text_color =          fromPacked(theme->getFillColor("preset", true));
    drag_color =          fromPacked(theme->getFillColor("preset-drag", true));
    drag_current_color =  fromPacked(theme->getFillColor("preset-dragcur", true));

    preset_name->color(selected ? selected_text_color : text_color);
    return true;
}

void PresetWidget::set_preset(int index, std::shared_ptr<PresetDescription> source)
{
    preset_index = index;
    if (index >= 0  && static_cast<size_t>(index) < preset_list->size()) {
        preset_id = source->id;
        preset_name->text(source->name);
        preset_name->describe(source->summary());
    } else {
        preset_id.invalidate();
        preset_name->text("");
        preset_name->describe("");
    }
}

void PresetWidget::set_selected(bool select)
{
    selected = select;
    preset_name->color(select ? selected_text_color : text_color);
}

std::shared_ptr<PresetDescription> PresetWidget::get_preset()
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
    preset_name->color(text_color);
}

void PresetWidget::onHover(const HoverEvent& e)
{
    hover_grip = (preset_index >= 0) && get_preset()->id.valid() && (e.pos.x > box.size.x - GRIP_WIDTH);
    Base::onHover(e);
}

void PresetWidget::onLeave(const LeaveEvent& e)
{
    hover_grip = false;
    button_down = false;
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
            if (agent) agent->onChoosePreset(this);
        }
        break;

    case GLFW_KEY_SPACE:
        if (e.action == GLFW_RELEASE) {
            e.consume(this);
            set_selected(!selected);
            if (agent) agent->onSetSelection(this, selected);
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
    switch (e.button) {
    case GLFW_MOUSE_BUTTON_LEFT: {
        switch (e.action) {
        case GLFW_RELEASE:
            if (button_down)
            {
                if (e.pos.x < box.size.x - GRIP_HIT) {
                    if (drag_started && !dragging) {
                        e.consume(this);
                        stop_drag();
                        if (agent) agent->onChoosePreset(this);
                    }
                } else {
                    e.consume(this);
                    set_selected(!selected);
                    if (agent) agent->onSetSelection(this, selected);
                }
                button_down = false;
            }
            break;

        case GLFW_PRESS:
            e.consume(this);
            button_down = true;
            drag_pos = e.pos;
            break;
        }
    } break;

    case GLFW_MOUSE_BUTTON_RIGHT: {
        switch (e.action) {
            case GLFW_RELEASE: {
                e.consume(this);
                createContextMenu();
            } break;
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
    stop_drag();
}

void PresetWidget::onDragMove(const DragMoveEvent& e)
{
    drag_pos = drag_pos.plus(e.mouseDelta.div(getAbsoluteZoom()));
    if (dragging && agent) {
        drop_target = agent->getDropTarget(this->box.pos + drag_pos);
    }
}

void PresetWidget::onDragHover(const DragHoverEvent& e)
{
    if (drag_started && !dragging) {
        if (has_elapsed(drag_delay_timer, DRAG_DELAY_INTERVAL)) {
            dragging = true;
        }
    }
    e.consume(this); // allow DragEnter and DragLeave
}

void PresetWidget::onDragEnter(const DragEnterEvent& e)
{
}

void PresetWidget::onDragLeave(const DragLeaveEvent& e)
{
    stop_drag();
}

void PresetWidget::onPathDrop(const PathDropEvent& e)
{
    e.consume(this);
    if (agent) agent->onDropFile(e);
}


void PresetWidget::appendContextMenu(ui::Menu* menu)
{
    menu->addChild(createMenuLabel(format_string("— %s —", preset_name->getText().c_str())));
    menu->addChild(createMenuItem("Remove", "", [this](){ agent->onDelete(this); }, !agent));
    //menu->addChild(createMenuItem("", "", [this](){  }, !agent));
    if (selected) {
        menu->addChild(createMenuItem("Un-select", "", [this](){ agent->onSetSelection(this, false); }, !agent));
    } else {
        menu->addChild(createMenuItem("Select", "", [this](){ agent->onSetSelection(this, true); }, !agent));
    }
}

void PresetWidget::draw(const DrawArgs& args)
{
    auto vg = args.vg;

    if (live) {
        FillRect(vg, -6.f, box.size.y*.5f - 2.f, 4.f, 4.f, dot_color);
    }
    if (selected && !dragging) {
        if (wire_style) {
            FillRect(vg, box.size.x - 8.f, box.size.y*.5f - 2.f, 4.f, 4.f, selected_color);
        } else {
            FillRect(vg, .25f, .25f, box.size.x - .5f, box.size.y - .5f, selected_color);
        }
    }
    if (button_down) {
        FillRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - 1.f, drag_current_color);
    }
    if (dragging) {
        if (drop_target == this) {
            FillRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - 1.f, drag_current_color);
        }
        BoxRect(vg, 0, 0, box.size.x, box.size.y, drag_color);
    }
 
    Base::draw(args);

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