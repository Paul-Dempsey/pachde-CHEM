#include "tab-header.hpp"

namespace pachde {

static const char * _digits = "123456789";

bool TabHeader::applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    item_style.apply_theme(theme);
    current_style.apply_theme(theme);
    current_text_style.apply_theme(theme);
    hover_style.apply_theme(theme);
    hover_text_style.apply_theme(theme);
    box.size.x = (cell * n_items) + ((2 + n_items-1) * item_style.width());
    box.size.y = cell + (2 * item_style.width());
    return true;
}

int TabHeader::index_of_pos(Vec pos)
{
    auto dx = item_style.width();
    if (pos.x < dx || pos.x > box.size.x - dx) return -1;
    if (pos.y < dx || pos.y > box.size.y - dx) return -1;
    return pos.x / (cell + dx);
}

void TabHeader::onHover(const HoverEvent& e) {
    Base::onHover(e);
    
    int item = index_of_pos(e.pos);
    if (item != hovered_item) {
        hovered_item = item;
        // if (hovered_item >= 0) {
        //     destroy_tip();
        // } else {
        //     set_tip_text("");
        //     destroy_tip();
        // }
    }
    e.consume(this);
}

void TabHeader::onEnter(const EnterEvent& e) {
    if (hovered_item == -1) {
        Base::onEnter(e);
    } else {
        create_tip();
        e.consume(this);
    }
}

void TabHeader::onLeave(const LeaveEvent& e) {
    destroy_tip();
    hovered_item = -1;
    Base::onLeave(e);
}

void TabHeader::onDragLeave(const DragLeaveEvent& e) {
    destroy_tip();
    hovered_item = -1;
    Base::onDragLeave(e);
}

void TabHeader::onDragEnd(const DragEndEvent& e) {
    destroy_tip();
    hovered_item = -1;
    Base::onDragEnd(e);
}

void TabHeader::onButton(const ButtonEvent& e)
{
    Base::onButton(e);
    auto mods = (e.mods & RACK_MOD_MASK);
    if ((0 == mods) && (e.action == GLFW_RELEASE)) {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
            destroy_tip();
            hovered_item = -1;
            current_item = index_of_pos(e.pos);
            if (current_item >= 0 && on_item_change) {
                on_item_change(current_item);
            }
            e.consume(this);
        } else if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
            destroy_tip();
            hovered_item = -1;
            //createContextMenu();
            e.consume(this);
        } 
    }
}
void TabHeader::draw(const DrawArgs& args)
{
    Base::draw(args);
    auto vg = args.vg;

    if (item_style.fill_color) {
        FillRect(vg, 0, 0, box.size.x, cell, item_style.nvg_color());
    }
    auto stroke = item_style.nvg_stroke_color();
    auto stroke_width = item_style.width();
    auto dx = cell + stroke_width;
    float x = dx + stroke_width*.5f;
    for (int item = 0; item < n_items - 1; ++item, x += dx) {
        Line(vg, x, stroke_width, x, dx, stroke, stroke_width);
    }
    FittedBoxRect(vg, 0, 0, box.size.x, box.size.y, stroke, Fit::Inside, stroke_width);

    auto font = GetPluginFontSemiBold();
    if (!FontOk(font)) return;

    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 0.f);
    nvgFontSize(vg, cell);

    nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
    float middle = box.size.y*.5;
    x = stroke_width;
    for (int item = 0; item < n_items; ++item, x += (cell + stroke_width)) {
        if (current_item == item) {
            if (current_style.fill_color) {
                FillRect(vg, x, stroke_width, cell, cell, current_style.nvg_color());
            } else if (current_style.stroke_color) {
                FittedBoxRect(vg, x, stroke_width, cell, cell, current_style.nvg_stroke_color(), Fit::Inside, current_style.width());
            }
            nvgFillColor(vg, current_text_style.nvg_color());
        } else if (hovered_item == item) {
            if (hover_style.fill_color) {
                FillRect(vg, x, stroke_width, cell, cell, hover_style.nvg_color());
            } else if (hover_style.stroke_color) {
                FittedBoxRect(vg, x, stroke_width, cell, cell, hover_style.nvg_stroke_color(), Fit::Inside, hover_style.width());
            }
            nvgFillColor(vg, hover_text_style.nvg_color());
        } else {
            nvgFillColor(vg, text_style.nvg_color());
        }
        nvgText(vg, x + cell*.5f, middle, _digits + item, _digits + item + 1);
    }
}

}