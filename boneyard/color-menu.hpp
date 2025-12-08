#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/colors.hpp"
#include "element-style.hpp"

namespace pachde {

struct SwatchesMenuItem : rack::ui::MenuItem
{
    using Base = rack::ui::MenuItem;
    static const int BASE_INDEX = static_cast<int>(StockColor::Black);

    ElementStyle swatch_box{"sb", "#181818", "hsl(0,0%,75%)", 2.f};
    ElementStyle swatch_selection{"ss", "#000000", "hsl(42,60%,65%)", 2.f};
    ElementStyle swatch_hover{"sh", "#000000", "hsl(0,0%,95%)", 1.f};

    Vec swatch_size;
    int index{-1};
    int hover_item{-1};
    std::function<void(int)> click_fn{nullptr};

    void set_on_click(std::function<void(int)> click) { click_fn = click; }
    void set_swatch_size(Vec size) {
        swatch_size = size;
        box.size.x = 4.f + (16 * size.x) + 30.f;
        box.size.y = 4.f + (10 * size.y) + 18.f;
    }

    void onHover(const HoverEvent& e) override
    {
        if (e.pos.x < 2.f) return;
        if (e.pos.y < 2.f) return;
        if (e.pos.x > box.size.x - 2.f) return;
        if (e.pos.y > box.size.y - 2.f) return;
        int column = std::floor(((e.pos.x) - 2.f) / (swatch_size.x + 2.f));
        int row  = std::floor(((e.pos.y) - 2.f) / (swatch_size.y + 2.f));
        hover_item = (row * 16) + column;
        if (hover_item > static_cast<int>(StockColor::STOCK_COLOR_COUNT) - BASE_INDEX) {
            hover_item = -1;
        }
    }

    void onButton(const ButtonEvent& e) override
    {
        if (e.button != GLFW_MOUSE_BUTTON_LEFT) return;
        if (0 != (e.mods & RACK_MOD_MASK)) return;
        e.consume(this);
    }

    void onAction(const ActionEvent& e) override
    {
        index = hover_item;
        if (click_fn) click_fn(index);
        e.unconsume(); // don't close menu
    }

    void step() override
    {
        Widget::step();
    }

    void draw(const DrawArgs& args) override
    {
        auto vg = args.vg;
        FillRect(vg, 0, 0, box.size.x, box.size.y, swatch_box.nvg_color());
        FittedBoxRect(vg, 0, 0, box.size.x, box.size.y, swatch_box.nvg_stroke_color(), Fit::Inside, swatch_box.width());
        float x = 2.f;
        float y = 2.f;
        for (int i = 0; i < static_cast<int>(StockColor::STOCK_COLOR_COUNT) - BASE_INDEX; ++i) {
            if ((i > 0) && (0 == (i % 16))) {
                y += swatch_size.y + 2.f;
                x = 2.f;
            }
            FillRect(vg, x, y, swatch_size.x, swatch_size.y, GetStockColor(static_cast<StockColor>(i + BASE_INDEX)));
            if (index == i) {
                FittedBoxRect(vg, x, y, swatch_size.x, swatch_size.y, swatch_selection.nvg_stroke_color(), Fit::Inside, swatch_selection.width());
            }
            if (hover_item == i) {
                FittedBoxRect(vg, x, y, swatch_size.x, swatch_size.y, swatch_hover.nvg_stroke_color(), Fit::Outside, swatch_hover.width());
            }
            x += swatch_size.x + 2.f;
        }
    }
};


}