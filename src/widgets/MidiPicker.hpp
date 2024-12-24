// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
#include "../plugin.hpp"
#include "../services/em_device.hpp"
#include "TipWidget.hpp"

using namespace ::rack;
namespace pachde {

struct MidiPicker : TipWidget
{
    MidiPicker & operator=(const MidiPicker &) = delete;
    MidiPicker(const MidiPicker&) = delete;

    widget::FramebufferWidget* fb;
    widget::SvgWidget* sw;
    IMidiDeviceHolder* setter;
    std::shared_ptr<MidiDeviceConnection> connection;
    bool is_em;

    MidiPicker() : setter(nullptr), is_em(false)
    {
        fb = new widget::FramebufferWidget;
        addChild(fb);
        sw = new widget::SvgWidget;
        fb->addChild(sw);
        sw->setSvg(Svg::load(asset::plugin(pluginInstance, "res/widgets/midi-button.svg")));
        box.size = sw->box.size;
        fb->box.size = sw->box.size;
        fb->setDirty(true);
    }

    void setCallback(IMidiDeviceHolder * callback) {
        assert(callback);
        setter = callback;
    }
    void setConnection(std::shared_ptr<MidiDeviceConnection> conn) {
        connection = conn;
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

        menu->addChild(createMenuLabel( is_em ? "Eagan Matrix device" : "MIDI controller"));
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Reset (auto)", "", [=](){ setter->setMidiDeviceClaim(""); }));

        auto broker = MidiDeviceBroker::get();
        broker->sync();

        auto current_claim = connection ? connection->info.spec() : "";

        broker->scan_while(
            [=](const std::shared_ptr<MidiDeviceConnection> conn) {
                if (!is_em || is_EMDevice(conn->info.input_device_name)) {
                    auto item_claim = conn->info.spec();
                    bool mine = (0 == current_claim.compare(item_claim));
                    bool unavailable = mine ? false : !broker->is_available(item_claim);

                    menu->addChild(createCheckMenuItem(conn->info.friendly(TextFormatLength::Long), "",
                        [=](){ return mine; },
                        [=](){ setter->setMidiDeviceClaim(item_claim); }, unavailable));
                }
                return true;
            }
        );
        if (is_em) {
            menu->addChild(new MenuSeparator);
            menu->addChild(createSubmenuItem("Any MIDI device (advanced)", "", [=](Menu * menu){
                broker->scan_while(
                    [=](const std::shared_ptr<MidiDeviceConnection> conn) {
                        auto item_claim = conn->info.spec();
                        bool mine = (0 == current_claim.compare(item_claim));
                        bool unavailable = mine ? false : !broker->is_available(item_claim);

                        menu->addChild(createCheckMenuItem(conn->info.friendly(TextFormatLength::Long), "",
                            [=](){ return mine; },
                            [=](){ setter->setMidiDeviceClaim(item_claim); }, unavailable));
                        return true;
                    }
                );
            }));
        }
    }
};


}