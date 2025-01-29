#pragma once
#include "label-widget.hpp"
#include "TipWidget.hpp"

namespace pachde {

struct TipLabel : StaticTextLabel
{
    using BaseClass = StaticTextLabel;

    TipHolder* tip_holder;

    TipLabel() {}
    virtual ~TipLabel() {
        if (tip_holder) delete tip_holder;
        tip_holder = nullptr;
    }
    void describe(std::string text)
    {
        if (!tip_holder) {
            tip_holder = new TipHolder();
        }
        tip_holder->setText(text);
    }
    void destroyTip() {
        if (tip_holder) { tip_holder->destroyTip(); }
    }

    void createTip() {
        if (tip_holder) { tip_holder->createTip(); }
    }

    void onHover(const HoverEvent& e) override
    {
        BaseClass::onHover(e);
        e.consume(this);
    }

    void onEnter(const EnterEvent& e) override {
        BaseClass:: onEnter(e);
        createTip();
    }

    void onLeave(const LeaveEvent& e) override {
        BaseClass:: onLeave(e);
        destroyTip();
    }

    void onDragLeave(const DragLeaveEvent& e) override {
        BaseClass::onDragLeave(e);
        destroyTip();
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        BaseClass::onDragEnd(e);
        destroyTip();
    }

    void onButton(const ButtonEvent& e) override
    {
        BaseClass::onButton(e);
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
            destroyTip();
            createContextMenu();
            e.consume(this);
        }
    }

    void createContextMenu() {
        ui::Menu* menu = createMenu();
    	appendContextMenu(menu);
    }

    virtual void appendContextMenu(ui::Menu* menu) {}

};

}