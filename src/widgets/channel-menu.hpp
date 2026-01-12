#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/colors.hpp"
#include "services/svg-theme.hpp"
using namespace svg_theme;
#include "hamburger.hpp"
#include "menu-widgets.hpp"

namespace widgetry {

struct ChannelMenu : Hamburger {
    using Base = Hamburger;
    uint8_t* poke{nullptr};

    void appendContextMenu(ui::Menu* menu) override {
        menu->addChild(createMenuLabel<HamburgerTitle>("Channel"));
        menu->addChild(new OptionMenuEntry( *poke == 0xFF, createMenuItem("[ any ]", "", [=](){ *poke = 0xFF; })));
        menu->addChild(new OptionMenuEntry( *poke == 1, createMenuItem("1", "", [=](){ *poke = 1; })));
        menu->addChild(new OptionMenuEntry( *poke == 2, createMenuItem("2", "", [=](){ *poke = 2; })));
        menu->addChild(new OptionMenuEntry( *poke == 3, createMenuItem("3", "", [=](){ *poke = 3; })));
        menu->addChild(new OptionMenuEntry( *poke == 4, createMenuItem("4", "", [=](){ *poke = 4; })));
        menu->addChild(new OptionMenuEntry( *poke == 5, createMenuItem("5", "", [=](){ *poke = 5; })));
        menu->addChild(new OptionMenuEntry( *poke == 6, createMenuItem("6", "", [=](){ *poke = 6; })));
        menu->addChild(new OptionMenuEntry( *poke == 7, createMenuItem("7", "", [=](){ *poke = 7; })));
        menu->addChild(new OptionMenuEntry( *poke == 8, createMenuItem("8", "", [=](){ *poke = 8; })));
        menu->addChild(new OptionMenuEntry( *poke == 9, createMenuItem("9", "", [=](){ *poke = 9; })));
        menu->addChild(new OptionMenuEntry( *poke == 10, createMenuItem("10", "", [=](){ *poke = 10; })));
        menu->addChild(new OptionMenuEntry( *poke == 11, createMenuItem("11", "", [=](){ *poke = 11; })));
        menu->addChild(new OptionMenuEntry( *poke == 12, createMenuItem("12", "", [=](){ *poke = 12; })));
        menu->addChild(new OptionMenuEntry( *poke == 13, createMenuItem("13", "", [=](){ *poke = 13; })));
        menu->addChild(new OptionMenuEntry( *poke == 14, createMenuItem("14", "", [=](){ *poke = 14; })));
        menu->addChild(new OptionMenuEntry( *poke == 15, createMenuItem("15", "", [=](){ *poke = 15; })));
        menu->addChild(new OptionMenuEntry( *poke == 16, createMenuItem("16", "", [=](){ *poke = 16; })));
    }
};




}