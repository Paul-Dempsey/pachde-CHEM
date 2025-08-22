// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
#include "../plugin.hpp"
#include "../services/midi-devices.hpp"
#include "../services/svt_rack.hpp"
#include "TipWidget.hpp"

using namespace ::rack;
namespace pachde {

struct MidiPicker : TipWidget, IApplyTheme
{
    MidiPicker & operator=(const MidiPicker &) = delete;
    MidiPicker(const MidiPicker&) = delete;

    widget::FramebufferWidget* fb;
    widget::SvgWidget* sw;
    MidiDeviceHolder* setter;
    MidiDeviceHolder* em_holder;

    bool is_em() { return setter == em_holder; }

    MidiPicker() : setter(nullptr)
    {
        fb = new widget::FramebufferWidget;
        addChild(fb);
        sw = new widget::SvgWidget;
        fb->addChild(sw);
    }

    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        sw->setSvg(theme_engine.loadSvg(asset::plugin(pluginInstance, "res/widgets/midi-button.svg"), theme));
        box.size = sw->box.size;
        fb->box.size = sw->box.size;
        fb->setDirty(true);
        return true;
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

        menu->addChild(createMenuLabel( is_em() ? "Eagan Matrix device" : "MIDI controller"));
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