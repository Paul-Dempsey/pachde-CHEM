#include "Preset.hpp"
#include "../../em/preset-meta.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"
#include "../../widgets/hamburger.hpp"
#include "../../widgets/spinner.hpp"

using PM = PresetModule;

PresetUi::~PresetUi()
{
    if (user_tab.list.dirty()) {
        save_presets(PresetTab::User);
    }
    if (user_tab.list.dirty()) {
        save_presets(PresetTab::System);
    }
}

void no_preset(TipLabel* label) {
    label->text("");
    label->describe("(none)");
}

void PresetUi::build_database(PresetTab which)
{
    if (!host_available()) return;
    start_spinner();
    Tab& tab = active_tab();
    db_builder = new DBBuilder();
    PresetId dummy;
    db_builder->init(tab.list.preset_list, live_preset ? live_preset->id : dummy);
    db_builder->next(chem_host->host_haken());
}

void PresetUi::request_presets(PresetTab which)
{
    if (!host_available()) return;

    switch (which) {
    case PresetTab::Unset:
        assert(false);
        break;

    case PresetTab::User:
        user_tab.clear();
        gathering = PresetTab::User;
        chem_host->host_haken()->request_user(ChemId::Preset);
        break;

    case PresetTab::System:
        system_tab.clear();
        gathering = PresetTab::System;
        chem_host->host_haken()->request_system(ChemId::Preset);
        break;
    }
}

std::string PresetUi::preset_file(PresetTab which)
{
    assert(PresetTab::Unset != which);
    const char * s_u = which == PresetTab::System ? "system" : "user";
    auto em = chem_host->host_matrix();
    auto preset_filename = format_string("%s-%s-%d.json", PresetClassName(em->hardware), s_u, em->firmware_version);
    return user_plugin_asset(preset_filename);
}

bool PresetUi::load_presets(PresetTab which)
{
    assert(PresetTab::Unset != which);
    if (!host_available()) return false;

    if ((PresetTab::User == which) && !my_module->use_cached_user_presets) return false;

    auto em = chem_host->host_matrix();
    if (!em || (0 == em->firmware_version) || (0 == em->hardware)) return false;

    auto path = preset_file(which);
    if (!system::exists(path)) return false;

    FILE* file = std::fopen(path.c_str(), "r");
	if (!file) return false;

    DEFER({std::fclose(file);});

    json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
		WARN("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        return false;
    }
    Tab& tab = get_tab(which);
    tab.clear();
    //tab.list.set_device_info(em->firmware_version, em->hardware);
    tab.list.order = which == PresetTab::User ? my_module->user_order : my_module->system_order;
    return tab.list.from_json(root, path);
}

bool PresetUi::save_presets(PresetTab which)
{
    if (!host_available()) return false;

    Tab& tab = get_tab(which);
    if (0 == tab.count()) return false;

    auto root = json_object();
    if (!root) { return false; }
	DEFER({json_decref(root);});

    auto path = preset_file(which);
    if (system::exists(path)) {
        system::remove(path);
    }
    auto dir = system::getDirectory(path);
    system::createDirectories(dir);

    FILE* file = std::fopen(path.c_str(), "wb");
    if (!file) return false;

    uint8_t hardware = chem_host->host_matrix()->hardware;

    tab.list.to_json(root, hardware, my_module->device_claim);
    auto e = json_dumpf(root, file, JSON_INDENT(2));
	std::fclose(file);
    return e >= 0;
}

void PresetUi::sort_presets(PresetOrder order)
{
    if (!my_module) return;
    
    Tab & tab = active_tab();
    if (tab.list.order == order) return;
    my_module->set_order(active_tab_id, order);
    if (0 == tab.count()) return;

    PresetId current_id;
    if ((tab.current_index >= 0) && tab.list.count()) {
        current_id = tab.list.nth(tab.current_index)->id;
    }

    tab.list.sort(order);
    save_presets(active_tab_id);

    if (my_module->track_live) {
        scroll_to_live();
    } else {
        if (current_id.valid()) {
            size_t index = tab.list.index_of_id(current_id);
            tab.current_index = index;
        }
        scroll_to_page_of_index(tab.current_index);
    }
}

void PresetUi::onSystemBegin()
{
    if (PresetTab::Unset == gathering) {
        other_system_gather = true;
    } else {
        start_spinner();
        gather_start = true;
    }
}

void PresetUi::onSystemComplete()
{
    stop_spinner();
    gather_start = false;
    if (other_system_gather) {
        other_system_gather = false;
        return;
    }
    if (gathering == PresetTab::System) {
        gathering = PresetTab::Unset;

        //auto em = chem_host->host_matrix();
        //system_tab.list.set_device_info(em->firmware_version, em->hardware);
        system_tab.list.sort(my_module->system_order);
        save_presets(PresetTab::System);
        system_tab.list.refresh_filter_view();
        if (active_tab_id == PresetTab::System) {
            scroll_to(0);
        }
    }
}

void PresetUi::onUserBegin()
{
    if (PresetTab::Unset == gathering) {
        other_user_gather = true;
    } else {
        start_spinner();
        gather_start = true;
    }
}

void PresetUi::onUserComplete()
{
    stop_spinner();
    gather_start = false;
    if (other_user_gather) {
        other_user_gather = false;
        return;
    }
    if (gathering == PresetTab::User) {
        gathering = PresetTab::Unset;

        user_tab.list.sort(my_module->user_order);
        save_presets(PresetTab::User);
        user_tab.list.refresh_filter_view();

        if (active_tab_id == PresetTab::User) {
            scroll_to(0);
        }
    }
}

void PresetUi::onPresetChange()
{
    // ignore preset changes while some other module is gathering presets
    if (other_user_gather || other_system_gather) return;

    if (db_builder) {
        bool done = false;
        if (host_available()) {
            auto preset = chem_host->host_preset();
            if (preset) {
                Tab& tab = active_tab();
                auto n = tab.list.index_of_id(preset->id);
                if (n >= 0) {
                    auto p = tab.list.nth(n);
                    if (p) {
                        p->set_text(preset->text);
                        tab.list.set_dirty();
                    }
                }
                if (!db_builder->next(chem_host->host_haken())) {
                    save_presets(active_tab_id);
                    done = true;
                }
            }
        } else {
            done = true;
        }

        if (done) {
            delete db_builder;
            db_builder = nullptr;
            stop_spinner();
        }
        return;
    }
    
    switch (gathering) {
    case PresetTab::Unset:
        break;
    case PresetTab::User:
    case PresetTab::System: {
        if (!gather_start) return;
        auto preset = chem_host->host_preset();
        if (preset && !preset->empty()) {
            get_tab(gathering).list.add(preset);
        }
        return;
    } break;
    }
    if (chem_host) {
        auto preset = chem_host->host_preset();
        if (preset) {
            live_preset = std::make_shared<PresetInfo>();
            live_preset->init(preset);
            live_preset_label->text(live_preset->name);
            live_preset_label->describe(live_preset->meta_text());
            Tab& tab = active_tab();
            auto n = tab.list.index_of_id(live_preset->id);
            if (n >= 0) {
                auto p = tab.list.nth(n);
                p->set_text(live_preset->text);
                tab.list.set_dirty();
                set_current_index(n);
            }
        } else {
            live_preset = nullptr;
            no_preset(live_preset_label);
        }
    } else {
        live_preset = nullptr;
        no_preset(live_preset_label);
    }
    if (live_preset && my_module->track_live) {
        scroll_to_live();
    }
    for (auto pw : preset_grid) {
        if (!pw->valid()) break;
        pw->live = live_preset && live_preset->valid() && (pw->preset_id() == live_preset->id);
        auto current = active_tab().current_index;
        pw->current = (current >= 0) && (current == pw->preset_index);
    }
}

void PresetUi::set_current_index(size_t index)
{
    active_tab().current_index = index;
}

bool PresetUi::host_available()
{
    if (!chem_host) return false;
    if (chem_host->host_busy()) return false;
    if (!chem_host->host_connection(ChemDevice::Haken)) return false;
    auto em = chem_host->host_matrix();
    if (!em || !em->is_ready() || em->busy()) return false;

    return true;
}

void PresetUi::start_spinner()
{
    if (!spinning) {
        startSpinner(this, Vec(170.f, 190.f));
        spinning = true;
    }
}

void PresetUi::stop_spinner()
{
    stopSpinner(this);
    spinning = false;
}

void PresetUi::onConnectHost(IChemHost *host)
{
    if (chem_host) {
        chem_host->host_matrix()->unsubscribeEMEvents(this);
    }
    chem_host = host;
    if (chem_host) {
        chem_host->host_matrix()->subscribeEMEvents(this);
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        onPresetChange();
    } else {
        onConnectionChange(ChemDevice::Haken, nullptr);
        no_preset(live_preset_label);
    }
}

void PresetUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (ChemDevice::Haken == device) {
        user_tab.clear();
        system_tab.clear();
    }
    onConnectionChangeUiImpl(this, device, connection);
}

void PresetUi::set_track_live(bool track)
{
    if (!module) return;
    if (track == my_module->track_live) return;
    my_module->track_live = track;
    if (track) {
        scroll_to_live();
    }
}

void PresetUi::on_search_text_changed()
{
    if (my_module->search_incremental || search_entry->empty()) {
        on_search_text_enter();
    }
}

void PresetUi::on_search_text_enter()
{
    if (!my_module) return;
    Tab& tab{active_tab()};
    auto query = search_entry->getText();
    if (my_module) {
        my_module->search_query = query;
    }
    PresetId current_id = tab.current_id();
    PresetId track_id;
    if (!current_id.valid() && tab.list.count()) {
        track_id = tab.list.nth(tab.scroll_top)->id;
    }
    tab.list.set_search_query(query, my_module->search_name, my_module->search_meta, my_module->search_anchor);
    tab.current_index = tab.list.index_of_id(current_id.valid() ? current_id : track_id);
    scroll_to_page_of_index(tab.current_index);
}

bool PresetUi::filtering()
{
    return !search_entry->empty() || active_tab().list.filtered();
}

void PresetUi::on_filter_change(FilterId filter, uint64_t state)
{
    if (!my_module) return;
    my_module->filters()[filter] = state;

    Tab& tab{active_tab()};
    PresetId current_id = tab.current_id();
    PresetId track_id;
    if (!current_id.valid() && tab.list.count()) {
        track_id = tab.list.nth(tab.scroll_top)->id;
    }
    tab.list.set_filter(filter, state);
    tab.current_index = tab.list.index_of_id(current_id.valid() ? current_id : track_id);
    scroll_to_page_of_index(tab.current_index);
}

void PresetUi::clear_filters()
{
    Tab& tab{active_tab()};
    PresetId current_id = tab.current_id();
    PresetId track_id;
    if (!current_id.valid() && tab.list.count()) {
        track_id = tab.list.nth(tab.scroll_top)->id;
    }

    search_entry->setText("");
    for (auto pf: filter_buttons) {
        pf->close_dialog();
        pf->set_state(0);
    }
    if (my_module) {
        my_module->clear_filters(active_tab_id);
    }
    active_tab().list.no_filter();

    tab.current_index = tab.list.index_of_id(current_id.valid() ? current_id : track_id);
    scroll_to_page_of_index(tab.current_index);
}


void PresetUi::set_tab(PresetTab tab_id, bool fetch)
{
    switch (tab_id) {
    case PresetTab::User:
        user_label->style(current_tab_style);
        system_label->style(tab_style);
        active_tab_id = PresetTab::User;
        break;

    case PresetTab::Unset:
        assert(false);
        goto sys;

    case PresetTab::System:
    sys:
        user_label->style(tab_style);
        system_label->style(current_tab_style);
        active_tab_id = PresetTab::System;
        break;
    }

    if (my_module) {
        my_module->active_tab = tab_id;
        auto filters = my_module->filters();
        for (auto fb: filter_buttons) {
            fb->set_state(*filters++);
        }
   }

    Tab& tab = active_tab();
    if (fetch && (0 == tab.count()) && host_available()) {
        if (load_presets(tab_id)) {

        } else {
            request_presets(tab_id);
        }
    }
    
    auto theme = theme_engine.getTheme(getThemeName());
    user_label->applyTheme(theme_engine, theme);
    system_label->applyTheme(theme_engine, theme);

    scroll_to(tab.scroll_top);
}

void PresetUi::scroll_to(ssize_t index)
{
    Tab& tab = active_tab();
    tab.scroll_top = index < 0 ? 0 : size_t(index);
    size_t ip = tab.scroll_top;
    for (auto pw: preset_grid) {
        if (ip < tab.count()) {
            auto preset = tab.list.nth(ip);
            auto live_id = get_live_id();
            bool live = live_id.valid() && (preset->id == live_id);
            pw->set_preset(ip, ssize_t(ip) == tab.current_index, live, preset);
        } else {
            pw->clear_preset();
        }
        ++ip;
    }
    update_page_controls();
}

void PresetUi::scroll_to_page_of_index(ssize_t index)
{
    scroll_to(page_index(index));
}

ssize_t PresetUi::page_index(ssize_t index)
{
    index = std::max(ssize_t(0), index);
    index = std::min(ssize_t(active_tab().count()), index);
    return PAGE_CAPACITY * (index / PAGE_CAPACITY);
}

void PresetUi::scroll_to_live()
{
    auto live_id = get_live_id();
    if (!live_id.valid()) return;
    ssize_t index = active_tab().list.index_of_id(live_id);
    if (index < 0) index = 0;
    scroll_to(page_index(index));
}

void PresetUi::page_up(bool ctrl, bool shift)
{
    Tab& tab = active_tab();
    if (tab.scroll_top < PAGE_CAPACITY) return;
    if (ctrl) {
        scroll_to(0);
    } else {
        auto index = std::max(ssize_t(0), ssize_t(tab.scroll_top) - PAGE_CAPACITY);
        scroll_to(index);
    }
}

void PresetUi::page_down(bool ctrl, bool shift)
{
    Tab& tab = active_tab();
    auto count = tab.list.count();
    if (count < PAGE_CAPACITY) {
        scroll_to(0);
        return;
    }
    if (ctrl) {
        scroll_to(page_index(count));
    } else {
        size_t pos = std::min(count, tab.scroll_top + PAGE_CAPACITY);
        scroll_to(page_index(pos));
    }
}

void PresetUi::update_page_controls()
{
    Tab& tab = active_tab();
    size_t count = tab.list.count();
    size_t page = 1 + (tab.scroll_top / PAGE_CAPACITY);
    size_t pages = 1 + (count / PAGE_CAPACITY);
    auto info = format_string("%d of %d", page, pages);
    page_label->text(info);

    if (0 == tab.scroll_top) {
        up_button->enable(false);
        down_button->enable(count > PAGE_CAPACITY);
        return;
    }

    up_button->enable(true);
    down_button->enable(true);

    if (count <= PAGE_CAPACITY) {
        up_button->enable(false);
        down_button->enable(false);
        return;
    }
    if (tab.scroll_top < PAGE_CAPACITY) {
        up_button->enable(false);
    }
    if (count - tab.scroll_top <= PAGE_CAPACITY) {
        down_button->enable(false);
    }
}

void PresetUi::send_preset(ssize_t index)
{
    if (index < 0) return;
    Tab& tab = active_tab();
    if (index >= ssize_t(tab.count())) return;
    auto preset = tab.list.nth(index);
    if (preset && host_available()) {
        chem_host->host_haken()->select_preset(ChemId::Preset, preset->id);
    }
}


void PresetUi::next_preset(bool ctrl, bool shift)
{
    Tab& tab = active_tab();
    if (tab.current_index < 0) {
        if (live_preset) {
            tab.current_index = tab.list.index_of_id(live_preset->id);
            if (tab.current_index >= 0) {
                ++tab.current_index;
                if (tab.current_index >= ssize_t(tab.count())) {
                    tab.current_index = 0;
                }
            } else {
                return;
            }
        } else {
            tab.current_index = tab.scroll_top;
        }
    } else {
        ++tab.current_index;
        if (tab.current_index >= ssize_t(tab.count())) {
            tab.current_index = 0;
        }
    }
    send_preset(tab.current_index);
}

void PresetUi::previous_preset(bool ctrl, bool shift)
{
    Tab& tab = active_tab();
    if (tab.current_index < 0) {
        if (live_preset) {
            tab.current_index = tab.list.index_of_id(live_preset->id);
            if (tab.current_index >= 0) {
                --tab.current_index;
                if (tab.current_index < 0) {
                    tab.current_index = tab.count() -1;
                }
            }
        } else {
            tab.current_index = tab.scroll_top;
        }
    } else {
        --tab.current_index;
        if (tab.current_index < 0) {
            tab.current_index = tab.count() - 1;
        }
    }
    send_preset(tab.current_index);
}
