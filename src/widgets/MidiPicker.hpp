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
    bool is_em;

    MidiPicker() : setter(nullptr), is_em(false)
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


    void setDeviceHolder(MidiDeviceHolder * holder) {
        assert(holder);
        setter = holder;
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


        menu->addChild(createMenuLabel( is_em ? "Eagan Matrix device" : "MIDI controller"));
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Reset (none)", "", [=](){
            auto claim = setter->getClaim(); 
            if (!claim.empty()) {
                broker->revokeClaim(claim);
            }
            setter->clear();
        }));

        auto current_claim = setter->getClaim();

        auto connections = EnumerateMidiConnections(false);
        if (is_em) {
            for (auto it = connections.cbegin(); it != connections.cend(); ++it) {
                auto conn = *it;
                if (is_EMDevice(conn->info.input_device_name)) {
                    auto item_claim = conn->info.claim();
                    bool mine = (0 == current_claim.compare(item_claim));
                    bool unavailable = mine ? false : !broker->available(item_claim);

                    menu->addChild(createCheckMenuItem(conn->info.friendly(TextFormatLength::Long), "",
                        [=](){ return mine; },
                        [=](){
                            if (!current_claim.empty()) {
                                broker->revokeClaim(current_claim);
                            }
                            if (broker->claimDevice(item_claim, setter)) {
                                setter->connect(conn);
                            }
                        }, unavailable));
                }
            }
            menu->addChild(new MenuSeparator);
            menu->addChild(createSubmenuItem("Any MIDI device (advanced)", "", [=](Menu * menu){
                for (auto it = connections.cbegin(); it != connections.cend(); ++it) {
                    auto conn = *it;
                    auto item_claim = conn->info.claim();
                    bool mine = (0 == current_claim.compare(item_claim));
                    bool unavailable = mine ? false : !broker->available(item_claim);

                    menu->addChild(createCheckMenuItem(conn->info.friendly(TextFormatLength::Long), "",
                        [=](){ return mine; },
                        [=](){
                            if (!current_claim.empty()) {
                                broker->revokeClaim(current_claim);
                            }
                            if (broker->claimDevice(item_claim, setter)) {
                                setter->connect(conn);
                            }
                        }, unavailable));
                }
            }));
        } else {
            for (auto it = connections.cbegin(); it != connections.cend(); ++it) {
                auto conn = *it;
                auto item_claim = conn->info.claim();
                bool mine = (0 == current_claim.compare(item_claim));
                bool unavailable = mine ? false : !broker->available(item_claim);

                menu->addChild(createCheckMenuItem(conn->info.friendly(TextFormatLength::Long), "",
                    [=](){ return mine; },
                    [=](){
                        if (!current_claim.empty()) {
                            broker->revokeClaim(current_claim);
                        }
                        if (broker->claimDevice(item_claim, setter)) {
                            setter->connect(conn);
                        }
                    }, unavailable));
            }
        }
    }
};


}