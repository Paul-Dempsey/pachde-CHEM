#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../services/colors.hpp"

namespace pachde {

struct ClickRegion : Widget
{
    using Base = Widget;
    int identifier;
    int mods;
    bool enabled;
    bool visible;
    NVGcolor color;
    float border_width;

    std::function<void(int, int)> handler;

    ClickRegion(int id) :
        identifier(id),
        mods(0),
        enabled(true),
        visible(false),
        color(Overlay(GetStockColor(StockColor::Coral), .5f)),
        border_width(.25f)
    {
        box.size.x = 0;
        box.size.y = 0;
    }

    void set_handler(std::function<void (int, int)> callback)
    {
        handler = callback;
    }

    int id() { return identifier; }

    void enable(bool enabled = true) {
        this->enabled = enabled;
    }

    void onButton(const ButtonEvent& e) override {
        Base::onButton(e);

        if (!enabled) return;
        // Dispatch ActionEvent on left click release
        if (e.action == GLFW_RELEASE && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            mods = e.mods;
            ActionEvent eAction;
            onAction(eAction);
        }
    }

    void onAction(const ActionEvent& e) override
    {
        if (enabled && handler) {
            handler(identifier, mods);
        } else {
            Base::onAction(e);
        }
    }

    void draw(const DrawArgs& args) override {
        Base::draw(args);
        if (!visible) return;
        FittedBoxRect(args.vg, 0, 0, box.size.x, box.size.y, color, Fit::Inside, border_width);
    }
};

template <typename TR = ClickRegion>
TR* createClickRegion(float x, float y, float width, float height, int id, std::function<void (int, int)> callback, bool enabled = true)
{
    TR* o = new TR(id);
    o->box.pos = Vec(x, y);
    o->box.size = Vec(width, height);
    o->set_handler(callback);
    o->enable(enabled);
    return o;
}

}