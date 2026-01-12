#pragma once
#include "Preset.hpp"
#include "widgets/menu-widgets.hpp"

struct PresetMenu : Hamburger {
    using Base = Hamburger;

    PresetUi* ui;
    PresetMenu() : ui(nullptr) { }
    void setUi(PresetUi* w) { ui = w; }

    void onHoverKey(const HoverKeyEvent& e) override
    {
        switch (e.key) {
            case GLFW_KEY_ENTER:
            case GLFW_KEY_MENU:
            if (e.action == GLFW_RELEASE) {
                e.consume(this);
                createContextMenu();
                return;
            }
        }
        Base::onHoverKey(e);
    }
};

struct ActionsMenu : PresetMenu
{
    void appendContextMenu(ui::Menu* menu) override {
        auto module = ui->my_module;
        menu->addChild(createMenuLabel<HamburgerTitle>("Preset Actions"));

        Tab & tab = ui->active_tab();
        PresetOrder order = tab.list.preset_list ? tab.list.preset_list->order : PresetOrder::Alpha;

        auto entry = new OptionMenuEntry(PresetOrder::Alpha == order,
            createMenuItem("Sort alphabetically", "", [=](){ ui->sort_presets(PresetOrder::Alpha); }));
        menu->addChild(entry);

        entry = new OptionMenuEntry(PresetOrder::Category == order,
            createMenuItem("Sort by category", "", [this](){ ui->sort_presets(PresetOrder::Category); }));
        menu->addChild(entry);

        entry = new OptionMenuEntry(PresetOrder::Natural == order,
            createMenuItem("Sort by preset number", "", [this](){ ui->sort_presets(PresetOrder::Natural); }));
        menu->addChild(entry);

        menu->addChild(new MenuSeparator);

        menu->addChild(createMenuItem("Clear filters", "",
            [this](){ ui->clear_filters(); },
            !ui->filtering()
        ));
        menu->addChild(createCheckMenuItem("Keep search filters", "",
            [module]() { return module->keep_search_filters; },
            [module]() { module->keep_search_filters = !module->keep_search_filters; },
            !module
        ));

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Show live preset", "",
            [this](){ ui->scroll_to_live(); },
            !ui->get_live_id().valid()
        ));
        menu->addChild(createCheckMenuItem("Track live preset", "",
            [this](){ return ui->my_module->track_live; },
            [this](){ ui->set_track_live(!ui->my_module->track_live); },
            !module
        ));
        menu->addChild(createMenuItem("Set live preset as current", "",
            [this](){ ui->set_live_current(); },
            !module
        ));
    }
};


struct SearchMenu : PresetMenu
{
    void appendContextMenu(ui::Menu* menu) override {

        if (!ui->my_module) return;

        menu->addChild(createMenuLabel<HamburgerTitle>("Search Options"));

        menu->addChild(createCheckMenuItem("Search preset Name", "",
            [this](){ return ui->my_module->search_name; },
            [this](){
                ui->my_module->search_name = !ui->my_module->search_name;
                if (!ui->my_module->search_name) {
                    ui->my_module->search_meta = true;
                }
                ui->on_search_text_enter();
            }
        ));
        menu->addChild(createCheckMenuItem("Search preset Metadata", "",
            [this](){ return ui->my_module->search_meta; },
            [this](){
                ui->my_module->search_meta = !ui->my_module->search_meta;
                if (!ui->my_module->search_meta) {
                    ui->my_module->search_name = true;
                }
                ui->on_search_text_enter();
            },
            !ui->my_module
        ));
        menu->addChild(createCheckMenuItem("Match at Word start", "",
            [this](){ return ui->my_module->search_anchor; },
            [this](){
                ui->my_module->search_anchor = !ui->my_module->search_anchor;
                ui->on_search_text_enter();
            },
            !ui->my_module
        ));

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Clear filters", "",
            [this](){ ui->clear_filters(); },
            !ui->filtering()
        ));

        menu->addChild(new MenuSeparator);

        auto entry = new OptionMenuEntry(!ui->my_module->search_incremental,
            createMenuItem("Search on ENTER", "",
                [this](){ ui->my_module->search_incremental = false; }));
        menu->addChild(entry);

        entry = new OptionMenuEntry(ui->my_module->search_incremental,
            createMenuItem("Search as you type", "",
                [this](){ ui->my_module->search_incremental = true; }));
        menu->addChild(entry);
    }
};

