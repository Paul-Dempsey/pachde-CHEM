// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "my-plugin.hpp"
#include "services/midi-devices.hpp"
#include "services/svg-theme.hpp"
using namespace svg_theme;
#include "tip-widget.hpp"

namespace widgetry {
struct BasicMidiPicker : TipWidget
{
    BasicMidiPicker & operator=(const BasicMidiPicker &) = delete;
    BasicMidiPicker(const BasicMidiPicker&) = delete;

    widget::FramebufferWidget* fb{nullptr};
    widget::SvgWidget* sw{nullptr};
    MidiDeviceHolder* device{nullptr};
    std::function<void()> handle_configure{nullptr};

    BasicMidiPicker() {
        fb = new widget::FramebufferWidget;
        addChild(fb);
        sw = new widget::SvgWidget;
        fb->addChild(sw);
    }

    void set_configure_handler(std::function<void()> handler) {
        handle_configure = handler;
    }
    void setDeviceHolder(MidiDeviceHolder * holder) {
        assert(holder);
        device = holder;
    }

    void loadSvg(ILoadSvg* loader) {
        sw->setSvg(loader->loadSvg(asset::plugin(pluginInstance, "res/widgets/DIN.svg")));
        box.size = sw->box.size;
        fb->box.size = sw->box.size;
        fb->setDirty(true);
    }

    void onButton(const ButtonEvent& e) override
    {
        TipWidget::onButton(e);
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
            createContextMenu();
            e.consume(this);
        }
    }

    void appendContextMenu(Menu* menu) override {
        if (!device) return;
        auto broker = MidiDeviceBroker::get();
        broker->sync();

        menu->addChild(createMenuLabel<HamburgerTitle>("MIDI controller"));
        menu->addChild(createMenuItem("Reset (none)", "", [=](){ device->clear(); }));

        if (handle_configure) {
            menu->addChild(createMenuItem("Configure...", "", [=](){ handle_configure(); }));
        }

        menu->addChild(new MenuSeparator);
        auto current_claim = device->get_claim();
        auto connections = EnumerateMidiConnections(false);
        for (auto it = connections.cbegin(); it != connections.cend(); ++it) {
            auto conn = *it;
            auto item_claim = conn->info.claim();
            bool mine = (0 == current_claim.compare(item_claim));
            //if (conn->input_device_id == -1) continue;

            menu->addChild(createCheckMenuItem(conn->info.friendly(NameFormat::Long), "",
                [=](){ return mine; },
                [=](){ device->connect(conn); }));
        }
    }
};

struct CoreMidiPicker : TipWidget
{
    CoreMidiPicker & operator=(const CoreMidiPicker &) = delete;
    CoreMidiPicker(const CoreMidiPicker&) = delete;

    widget::FramebufferWidget* fb{nullptr};
    widget::SvgWidget* sw{nullptr};
    MidiDeviceHolder* setter{nullptr};
    MidiDeviceHolder* em_holder{nullptr};

    bool is_em() { return em_holder && (setter == em_holder); }

    CoreMidiPicker() : setter(nullptr)
    {
        fb = new widget::FramebufferWidget;
        addChild(fb);
        sw = new widget::SvgWidget;
        fb->addChild(sw);
    }

    void loadSvg(ILoadSvg* loader) {
        sw->setSvg(loader->loadSvg(asset::plugin(pluginInstance, "res/widgets/midi-button.svg")));
        box.size = sw->box.size;
        fb->box.size = sw->box.size;
        fb->setDirty(true);
    }

    void setDeviceHolder(MidiDeviceHolder * holder, MidiDeviceHolder* main_holder) {
        assert(holder);
        assert(main_holder && main_holder->device_role == ChemDevice::Haken);

        setter = holder;
        em_holder = main_holder;
    }

    void onButton(const ButtonEvent& e) override
    {
        TipWidget::onButton(e);
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
            createContextMenu();
            e.consume(this);
        }
    }

    void appendContextMenu(Menu* menu) override
    {
        if (!setter) return;
        auto broker = MidiDeviceBroker::get();
        broker->sync();

        menu->addChild(createMenuLabel<HamburgerTitle>( is_em() ? "Eagan Matrix device" : "MIDI controller"));
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Reset (none)", "", [=](){
            setter->clear();
        }));

        auto current_claim = setter->get_claim();

        auto connections = EnumerateMidiConnections(false);
        if (is_em()) {
            for (auto it = connections.cbegin(); it != connections.cend(); ++it) {
                auto conn = *it;
                if (is_EMDevice(conn->info.input_device_name)) {
                    auto item_claim = conn->info.claim();
                    bool mine = (0 == current_claim.compare(item_claim));
                    bool unavailable = mine ? false : !broker->available(item_claim);

                    menu->addChild(createCheckMenuItem(conn->info.friendly(NameFormat::Long), "",
                        [=](){ return mine; },
                        [=](){ setter->connect(conn); }, unavailable));
                }
            }
            menu->addChild(new MenuSeparator);
            menu->addChild(createSubmenuItem("Any MIDI device (advanced)", "", [=](Menu * menu){
                for (auto it = connections.cbegin(); it != connections.cend(); ++it) {
                    auto conn = *it;
                    auto item_claim = conn->info.claim();
                    bool mine = (0 == current_claim.compare(item_claim));
                    bool unavailable = mine ? false : !broker->available(item_claim);

                    menu->addChild(createCheckMenuItem(conn->info.friendly(NameFormat::Long), "",
                        [=](){ return mine; },
                        [=](){ setter->connect(conn); }, unavailable));
                }
            }));
        } else {
            for (auto it = connections.cbegin(); it != connections.cend(); ++it) {
                auto conn = *it;
                auto item_claim = conn->info.claim();
                bool mine = (0 == current_claim.compare(item_claim));
                bool unavailable = mine ? false : 0 == item_claim.compare(em_holder->get_claim());

                menu->addChild(createCheckMenuItem(conn->info.friendly(NameFormat::Long), "",
                    [=](){ return mine; },
                    [=](){ setter->connect(conn); }, unavailable));
            }
        }
    }
};


}