#include "Preset.hpp"
#include "Preset-menu.hpp"
#include "em/preset-meta.hpp"
#include "services/colors.hpp"
#include "services/svg-query.hpp"
#include "widgets/click-region-widget.hpp"
#include "widgets/logo-widget.hpp"
#include "widgets/menu-widgets.hpp"
#include "widgets/hamburger.hpp"
#include "widgets/spinner.hpp"

using PM = PresetModule;

PresetUi::PresetUi(PresetModule *module) :
    my_module(module)
{
    setModule(module);
    IHandleEmEvents::module_id = ChemId::Preset;
    IHandleEmEvents::em_event_mask = (IHandleEmEvents::EventMask)(
        SystemBegin +
        SystemComplete +
        UserBegin +
        UserComplete
    );
    auto theme = getSvgTheme();
    auto panel = createThemedPanel(panelFilename(), &module_svgs);
    panelBorder = attachPartnerPanelBorder(panel);
    setPanel(panel);
    ::svg_query::BoundsIndex bounds;
    svg_query::addBounds(panel->svg, "k:", bounds, true);

    search_entry = createThemedTextInput(bounds["k:search-edit"], "",
        [=](const std::string& text) { on_search_text_changed(text); },
        [=](const std::string&) { on_search_text_enter(); },
        "preset search");
    addChild(search_entry);

    menu = Center(createWidget<SearchMenu>(bounds["k:search-menu"].getCenter()));
    menu->setUi(this);
    menu->describe("Search options");
    addChild(menu);

    addChild(createLightCentered<SmallLight<RedLight>>(bounds["k:l-filter"].getCenter(), my_module, PresetModule::L_FILTER));;

    // tab labels
    addChild(user_label = createLabel(bounds["k:user-tab"], "User", &tab_style));
    addChild(createClickRegion(RECT_ARGS(user_label->box.grow(Vec(6,2))), (int)PresetTab::User, [=](int id, int mods) { set_tab((PresetTab)id, true); }));

    addChild(system_label = createLabel(bounds["k:system-tab"], "System", &current_tab_style));
    addChild(createClickRegion(RECT_ARGS(system_label->box.grow(Vec(6,2))), (int)PresetTab::System, [=](int id, int mods) { set_tab((PresetTab)id, true); }));

    // right controls

    addChild(page_label = createLabel<TextLabel>(bounds["k:pager"], "1 of 1", &dytext_style));

    up_button = createWidgetCentered<UpButton>(bounds["k:up"].getCenter());
    up_button->describe("Page up");
    up_button->applyTheme(theme);
    up_button->setHandler([this](bool c, bool s){
        page_up(c,s);
        APP->event->setSelectedWidget(this);
    });
    addChild(up_button);

    down_button = createWidgetCentered<DownButton>(bounds["k:down"].getCenter());
    down_button->describe("Page down");
    down_button->applyTheme(theme);
    down_button->setHandler([this](bool c, bool s){
        page_down(c,s);
        APP->event->setSelectedWidget(this);
    });
    addChild(down_button);

    auto prev = createWidgetCentered<PrevButton>(bounds["k:prev"].getCenter());
    prev->describe("Select previous preset");
    prev->applyTheme(theme);
    prev->setHandler([this](bool c, bool s) {
        previous_preset(c,s);
        APP->event->setSelectedWidget(this);
    });
    addChild(prev);

    auto next = createWidgetCentered<NextButton>(bounds["k:next"].getCenter());
    next->describe("Select next preset");
    next->applyTheme(theme);
    next->setHandler([this](bool c, bool s){
        next_preset(c,s);
        APP->event->setSelectedWidget(this);
    });
    addChild(next);

    menu = createWidgetCentered<ActionsMenu>(bounds["k:action-menu"].getCenter());
    menu->setUi(this);
    menu->describe("Preset actions menu");
    addChild(menu);

    {   // Filter buttons
        FilterButton* filter{nullptr};

        addChild(Center(filter = makeCatFilter(bounds["k:cat"].getCenter(), &module_svgs, theme, [=](uint64_t state){ on_filter_change(FilterId::Category, state); })));
        filter_buttons.push_back(filter);

        addChild(Center(filter = makeTypeFilter(bounds["k:type"].getCenter(), &module_svgs, theme, [=](uint64_t state){ on_filter_change(FilterId::Type, state); })));
        filter_buttons.push_back(filter);

        addChild(Center(filter = makeCharacterFilter(bounds["k:char"].getCenter(), &module_svgs, theme, [=](uint64_t state){ on_filter_change(FilterId::Character, state); })));
        filter_buttons.push_back(filter);

        addChild(Center(filter = makeMatrixFilter(bounds["k:mat"].getCenter(), &module_svgs, theme, [=](uint64_t state){ on_filter_change(FilterId::Matrix, state); })));
        filter_buttons.push_back(filter);

        addChild(Center(filter = makeSettingFilter(bounds["k:set"].getCenter(), &module_svgs, theme, [=](uint64_t state){ on_filter_change(FilterId::Setting, state); })));
        filter_buttons.push_back(filter);
    }

    addChild(filter_off_button = Center(makeFilterStateButton(bounds["k:state"].getCenter(), &module_svgs, [=]() {
        if (filtering()) {
            clear_filters();
            filter_off_button->button_down = false;
        }
    })));

    auto nav = createChemKnob<EndlessKnob>(bounds["k:nav-knob"].getCenter(), &module_svgs, my_module, PresetModule::P_NAV);
    nav->set_handler([=](){
        Tab& tab = active_tab();
        if ((tab.current_index >= 0) && host_available() && (tab.list.count() > 0)) {
            chem_host->request_preset(ChemId::Preset, tab.list.nth(tab.current_index)->id);
        }
    });
    addChild(nav);

    addChild(Center(createThemedParamButton<DotParamButton>(bounds["k:nav-sel"].getCenter(), &module_svgs, my_module, PresetModule::P_SELECT)));

    auto picker = Center(createThemedWidget<BasicMidiPicker>(bounds["k:midi"].getCenter(), &module_svgs));
    picker->describe("Preset Midi controller");
    if (my_module) {
        picker->setDeviceHolder(&my_module->preset_midi.midi_device);
        picker->set_configure_handler( [=](){ configure_midi(); });
    }
    addChild(picker);

    if (my_module) {
        user_tab.list.init_filters(my_module->user_filters);
        system_tab.list.init_filters(my_module->system_filters);
        auto filters = my_module->filters();
        for (auto fb: filter_buttons) {
            fb->set_state(*filters++);
        }
    }

    // preset grid
    float x = 9.f; float y = PRESET_TOP;
    for (int i = 0; i < PAGE_CAPACITY; ++i) {
        auto entry = PresetEntry::create(Vec(x,y), preset_grid, this);
        preset_grid.push_back(entry);
        addChild(entry);
        y += 16.f;
        if (i == PAGE_CAPACITY/2 - 1) {
            x = 172;
            y = PRESET_TOP;
        }
    }

    addChild(help_label = createLabel<TextLabel>(bounds["k:help"], "", &help_style));

    // footer
    link_button = createThemedButton<LinkButton>(Vec(15.f, box.size.y - S::U1), &module_svgs, "Core link");
    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    addChild(haken_device_label = createLabel<TipLabel>(
        Vec(32.f, box.size.y - 13.f), S::NotConnected, &S::haken_label, 150.f));

    addChild(live_preset_label = createLabel<TipLabel>(bounds["k:live"], "[preset]", &cp_style));
    live_preset_label->glowing(true);

    if (S::show_screws()) {
        createScrews();
    }

    if (!module && S::show_browser_logo()) {
        auto logo = new WatermarkLogo(1.8f);
        logo->box.pos = Vec(84.f, 180.f - logo->box.size.y*.6);
        addChild(logo);
    }

    module_svgs.changeTheme(theme);
    applyChildrenTheme(this, theme);

    if (module) {
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
        set_tab(PresetTab(my_module->active_tab), false);
    }
}

PresetUi::~PresetUi()
{
    if (my_module) my_module->set_chem_ui(nullptr);
    if (chem_host) {
        auto em = chem_host->host_matrix();
        if (em) em->unsubscribeEMEvents(this);
        auto ipl = chem_host->host_ipreset_list();
        if (ipl) {
            ipl->unregister_preset_list_client(this);
        }
    }
}

void PresetUi::createScrews()
{
    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, 0), &module_svgs));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0), &module_svgs));

    addChild(createThemedWidget<ThemeScrew>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), &module_svgs));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), &module_svgs));
}

