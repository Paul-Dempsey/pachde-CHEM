#include "Play.hpp"
#include <ghc/filesystem.hpp>
#include "../../services/colors.hpp"
#include "../../services/open-file.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
namespace fs = ghc::filesystem;
using namespace svg_theme;
using namespace pachde;


// -- Play Menu ---------------------------------

struct PlayMenu : Hamburger
{
    using Base = Hamburger;

    PlayUi* ui;
    PlayMenu() : ui(nullptr) { }

    void setUi(PlayUi* w) { ui = w; }

    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override {
        return Base::applyTheme(engine, theme);
    }

    void appendContextMenu(ui::Menu* menu) override
    {
        menu->addChild(createMenuLabel<HamburgerTitle>("Playlist Actions"));
        menu->addChild(createMenuItem("Open...", "",    [this](){ ui->open_playlist(); }, false));
        menu->addChild(createMenuItem("Close", "",      [this](){ ui->close_playlist(); }, false));
        menu->addChild(createMenuItem("Save", "",       [this](){ ui->save_playlist(); } , false));
        menu->addChild(createMenuItem("Save as...", "", [this](){ ui->save_as_playlist(); } , false));
        menu->addChild(createMenuItem("Clear", "", [this](){ ui->clear_playlist(false); }, ui->presets.empty()));
    
        if (ui->my_module) {
            menu->addChild(new MenuSeparator);
            menu->addChild(createSubmenuItem("Open recent", "", [=](Menu* menu) {
                if (ui->my_module->playlist_mru.empty()) {
                    menu->addChild(createMenuLabel("(empty)"));
                } else {
                    for (auto path : ui->my_module->playlist_mru) {
                        auto name = system::getStem(path);
                        menu->addChild(createMenuItem(name, "", [=]() {
                            ui->load_playlist(path, true);
                        }));
                    }
                }
            }));
            menu->addChild(createMenuItem("Clear recents", "", [this](){ ui->my_module->clear_mru(); }, false));
        }
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Sort alphabetically", "", [this](){ ui->sort_presets(PresetOrder::Alpha); }, false));
        menu->addChild(createMenuItem("Sort by category", "", [this](){ ui->sort_presets(PresetOrder::Category); }, false));
        menu->addChild(createMenuItem("Sort by Natural (system) order", "", [this](){ ui->sort_presets(PresetOrder::Natural); }, false));
        if (ui->chem_host) {
            auto em = ui->chem_host->host_matrix();
            if (em) {
                if (! em->is_osmose()) {
                    menu->addChild(createSubmenuItem("Append", "", [=](Menu* menu) {
                        menu->addChild(createMenuItem("User presets", "", [this](){ ui->fill(FillOptions::User); }));
                        menu->addChild(createMenuItem("System presets", "", [this](){ ui->fill(FillOptions::System); }));
                        menu->addChild(createMenuItem("All presets", "", [this](){ ui->fill(FillOptions::All); }));
                    }));
                } else {
                    menu->addChild(createMenuLabel("Append (unavailble on Osmose)"));
                    //TODO:get playlist....
                }
            } else {
                menu->addChild(createMenuLabel("Append [no EM connection]"));
            }
        } else {
            menu->addChild(createMenuLabel("Append [no connection]"));
        }

        menu->addChild(new MenuSeparator);
        bool no_selection =  ui->selected.empty();
        bool first = (0 == ui->first_selected());
        bool last = (ui->last_selected() == static_cast<int>(ui->presets.size())-1);
        menu->addChild(createMenuLabel<HamburgerTitle>("Selected"));
        menu->addChild(createMenuItem("Duplicate", "",     [this](){ ui->clone(); }, no_selection));
        menu->addChild(createMenuItem("Remove", "",        [this](){ ui->remove_selected(); }, no_selection));
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Move to first", "", [this](){ ui->to_first(); }, first || no_selection));
        menu->addChild(createMenuItem("Move up", "",       [this](){ ui->to_up(); }, first || no_selection));
        menu->addChild(createMenuItem("Move down", "",     [this](){ ui->to_down(); }, last || no_selection));
        menu->addChild(createMenuItem("Move to last", "",  [this](){ ui->to_last(); }, last || no_selection));
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Select none", "Esc",  [this](){ ui->select_none(); }, no_selection));
    }
    
    void onHoverKey(const HoverKeyEvent& e) override
    {
        Base::onHoverKey(e);
        switch (e.key) {
        case GLFW_KEY_ENTER:
        case GLFW_KEY_MENU:
            if (e.action == GLFW_RELEASE) {
                e.stopPropagating();
                createContextMenu();
            }
        }
    }

};


// -- Play UI -----------------------------------

constexpr const float ONEU = 15.f;
constexpr const float RIGHT_MARGIN_CENTER = 186.f;
constexpr const float PRESETS_TOP = 32.f;
constexpr const float PRESETS_LEFT = 12.f;

PlayUi::PlayUi(PlayModule *module) :
    my_module(module)
{
    setModule(module);
    IHandleEmEvents::em_event_mask = (EventMask)(
        SystemComplete + 
        UserComplete
    );
    IHandleEmEvents::module_id = ChemId::Play;

    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);
    if (S::show_screws()) {
        addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
    }

    float y = PRESETS_TOP;
    for (int i = 0; i < PAGE_CAPACITY; ++i) {
        auto pw = createPresetWidget(this, &presets, PRESETS_LEFT, y, theme_engine, theme);
        if (!module) {
            pw->set_text(in_range(i, 4, 14) ? "..." : format_string("[preset #%d]", 1 + i));
        }
        preset_widgets.push_back(pw);
        addChild(pw);
        y += 20;
    }
    addChild(haken_device_label = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 13.f), 200.f, S::NotConnected, theme_engine, theme, S::haken_label));

    addChild(warning_label = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 22.f), box.size.x, "", theme_engine, theme, S::warning_label));
    warning_label->describe("[warning/status]");
    warning_label->glowing(true);

    LabelStyle style{"dytext", TextAlignment::Left, 14.f};
    addChild(playlist_label = createLabel<TipLabel>(
        Vec(ONEU, 16), 148.f, "My Favorites", theme_engine, theme, style));
    playlist_label->glowing(true);

    addChild(blip = createBlipCentered(7.5f, 24.f, "Playlist unsaved when lit"));
    blip->set_radius(2.5f);
    blip->set_rim_color(nvgTransRGBAf(RampGray(G_65), .4f));
    blip->set_light_color(ColorFromTheme(getThemeName(), "warning", nvgRGB(0xe7, 0xe7, 0x45)));
    blip->set_brightness(0.f);

    style.height = 9.f;
    style.align = TextAlignment::Center;
    addChild(page_label = createLabel<TextLabel>(
        Vec(RIGHT_MARGIN_CENTER, 35.f),  35.f, "1 of 1", theme_engine, theme, style));

    auto heart = createThemedButton<HeartButton>(Vec(ONEU, 342.f), theme_engine, theme, "Add to playlist");
    heart->setHandler([this](bool c, bool s){ 
        add_live();
    });
    addChild(heart);

    style.key = "curpreset";
    style.height = 14.f;
    style.bold = true;
    addChild(live_preset_label = createLabel<TipLabel>(
        Vec(87, 340.f), 150.f, "[current device preset]", theme_engine, theme, style));
    live_preset_label->glowing(true);

    play_menu = createThemedWidget<PlayMenu>(Vec(RIGHT_MARGIN_CENTER, 148.f), theme_engine, theme);
    play_menu->setUi(this);
    play_menu->describe("Play actions menu");
    addChild(Center(play_menu));

    up_button = createWidgetCentered<UpButton>(Vec(RIGHT_MARGIN_CENTER, 52.f));
    up_button->describe("Page up");
    up_button->setHandler([this](bool c, bool s){ 
        page_up(c, s);
    });
    up_button->applyTheme(theme_engine, theme);
    addChild(up_button);

    down_button = createWidgetCentered<DownButton>(Vec(RIGHT_MARGIN_CENTER, 67.f));
    down_button->describe("Page down");
    down_button->setHandler([this](bool c, bool s){
        page_down(c, s);
    });
    down_button->applyTheme(theme_engine, theme);
    addChild(down_button);

    auto prev = createWidgetCentered<PrevButton>(Vec(RIGHT_MARGIN_CENTER - 9.5f, 98.f));
    prev->describe("Select previous preset");
    prev->setHandler([this](bool c, bool s){ prev_preset(); });
    prev->applyTheme(theme_engine, theme);
    addChild(prev);

    auto next = createWidgetCentered<NextButton>(Vec(RIGHT_MARGIN_CENTER + 9.f, 98.f));
    next->describe("Select next preset");
    next->setHandler([this](bool c, bool s){ next_preset(); });
    next->applyTheme(theme_engine, theme);
    addChild(next);

    addChild(Center(createThemedColorInput(Vec(RIGHT_MARGIN_CENTER - 9.f, RACK_GRID_HEIGHT - 26.f), my_module, PlayModule::IN_PRESET_PREV, S::InputColorKey, PORT_CORN, theme_engine, theme)));
    addChild(Center(createThemedColorInput(Vec(RIGHT_MARGIN_CENTER + 9.f, RACK_GRID_HEIGHT - 26.f), my_module, PlayModule::IN_PRESET_NEXT, S::InputColorKey, PORT_CORN, theme_engine, theme)));

    if (!module) {
        auto logo = new WatermarkLogo(1.25f);
        logo->box.pos = Vec(90.f, box.size.y*.5);
        addChild(Center(logo));
    }

    // Footer

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y-ONEU), theme_engine, theme, "Core link");
    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    if (my_module) {
        my_module->set_chem_ui(this);
        if (!chem_host) {
            onConnectHost(my_module->chem_host);
        }
        if (!my_module->playlist_file.empty()) {
            load_playlist(my_module->playlist_file, false);
        }
    } else {
        update_up_down();
    }
    set_modified(false);
}

void PlayUi::setThemeName(const std::string& name, void * context)
{
    blip->set_light_color(ColorFromTheme(getThemeName(), "warning", nvgRGB(0xe7, 0xe7, 0x45)));
    Base::setThemeName(name, context);
}

void PlayUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    menu->addChild(new MenuSeparator);
    menu->addChild(createCheckMenuItem("Track live preset", "", 
        [this](){ return my_module->track_live; },
        [this](){ set_track_live(!my_module->track_live); }
    ));
    Base::appendContextMenu(menu);
}
