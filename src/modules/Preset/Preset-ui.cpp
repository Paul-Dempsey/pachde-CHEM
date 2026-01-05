#include "Preset.hpp"
#include "em/preset-meta.hpp"
#include "services/colors.hpp"
#include "widgets/click-region-widget.hpp"
#include "widgets/logo-widget.hpp"
#include "widgets/uniform-style.hpp"
#include "widgets/hamburger.hpp"
#include "widgets/spinner.hpp"
#include "./widgets/midi-config-dialog.hpp"

using PM = PresetModule;
using namespace eaganmatrix;

void no_preset(TipLabel* label) {
    label->set_text("");
    label->describe("(none)");
}

void PresetUi::on_list_changed(eaganmatrix::PresetTab which) {
    Tab& tab = get_tab(which);
    std::shared_ptr<PresetList> ppl{nullptr};
    if (chem_host) {
        auto ipl = chem_host->host_ipreset_list();
        if (ipl) {
            switch (which) {
            case PresetTab::Unset: break;
            case PresetTab::System: ppl = ipl->host_system_presets(); break;
            case PresetTab::User: ppl = ipl->host_user_presets(); break;
            }
        }
    }
    tab.set_list(ppl);
    update_help();
    onPresetChange();
    if (!my_module->track_live) scroll_to_live();
}

bool PresetUi::load_presets(PresetTab which) {
    valid_tab(which);
    if (!host_available()) return false;
    auto pl = chem_host->host_ipreset_list();
    if (!pl) return false;
    Tab & tab = get_tab(which);
    tab.list.preset_list = (PresetTab::User == which) ? pl->host_user_presets() : pl->host_system_presets();
    return (nullptr != tab.list.preset_list) && !tab.list.preset_list->empty();
}

void PresetUi::sort_presets(PresetOrder order) {
    if (!my_module) return;

    Tab & tab = active_tab();
    if (!tab.list.preset_list) return;
    if (0 == tab.count()) return;

    PresetId current_id;
    if ((tab.current_index >= 0) && tab.list.count()) {
        current_id = tab.list.nth(tab.current_index)->id;
    }

    tab.list.sort(order);
    tab.list.save();

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

void PresetUi::onSystemBegin() {
    other_system_gather = true;
    help_label->set_text("scanning System presets");
    help_label->setVisible(true);
    live_preset_label->set_text("");
    live_preset_label->describe("");
}

void PresetUi::onSystemComplete() {
    other_system_gather = false;
    system_tab.list.refresh_filter_view();
    if (active_tab_id == PresetTab::System) {
        scroll_to(0);
    }
    help_label->setVisible(false);
    help_label->set_text("");
}

void PresetUi::onUserBegin() {
    other_user_gather = true;
    help_label->set_text("scanning User presets");
    help_label->setVisible(true);
    live_preset_label->set_text("");
    live_preset_label->describe("");
}

void PresetUi::onUserComplete() {
    other_user_gather = false;
    user_tab.list.refresh_filter_view();
    if (active_tab_id == PresetTab::User) {
        scroll_to(0);
    }
    help_label->setVisible(false);
    help_label->set_text("");
}

void PresetUi::onPresetChange() {
    // ignore preset changes while gathering presets
    if (other_user_gather || other_system_gather) return;

    if (chem_host) {
        auto preset = chem_host->host_preset();
        if (preset) {
            live_preset = std::make_shared<PresetInfo>();
            live_preset->init(preset);
            live_preset_label->set_text(preset->name);
            live_preset_label->describe(live_preset->meta_text());
            Tab& tab = active_tab();
            auto n = tab.list.index_of_id(preset->id);
            if (n >= 0) {
                auto p = tab.list.nth(n);
                if (!preset_equal(preset, p.get()))
                {
                    p->init(preset);
                    tab.list.set_dirty();
                }
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
    auto current = active_tab().current_index;
    if (live_preset && my_module->track_live) {
        scroll_to_live();
        set_nav_param(current);
    }
    for (auto pw : preset_grid) {
        if (!pw->valid()) break;
        pw->live = live_preset && live_preset->valid() && (pw->preset_id() == live_preset->id);
        pw->current = (current >= 0) && (current == pw->preset_index);
    }
}

void PresetUi::set_live_current() {
    if (!module) return;
    if (live_preset && live_preset->valid()) {
        Tab& tab = active_tab();
        auto index = tab.list.index_of_id(live_preset->id);
        if (index >= 0) {
            set_nav_param(index);
            set_current_index(index);
            scroll_to_page_of_index(index);
        }
    }
}

void PresetUi::set_current_index(size_t index) {
    Tab& tab = active_tab();
    tab.current_index = clamp(index, 0, tab.count());
}

bool PresetUi::host_available() {
    if (!chem_host) return false;
    if (chem_host->host_busy()) return false;
    if (!chem_host->host_connection(ChemDevice::Haken)) return false;
    return true;
}

void PresetUi::onConnectHost(IChemHost *host) {
    if (chem_host) {
        auto em = chem_host->host_matrix();
        if (em) em->unsubscribeEMEvents(this);
    }
    chem_host = host;
    if (chem_host) {
        auto em = chem_host->host_matrix();
        if (em) em->subscribeEMEvents(this);
        auto ipl = chem_host->host_ipreset_list();
        if (ipl) {
            ipl->register_preset_list_client(this);
        }
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        onPresetChange();
    } else {
        onConnectionChange(ChemDevice::Haken, nullptr);
        no_preset(live_preset_label);
    }
}

void PresetUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) {
    if (ChemDevice::Haken == device) {
        user_tab.clear();
        system_tab.clear();
        set_tab(active_tab_id, true);
    }
    onConnectionChangeUiImpl(this, device, connection);
    start_delay.run();
}

void PresetUi::set_track_live(bool track) {
    if (!module) return;
    if (track == my_module->track_live) return;
    my_module->track_live = track;
    if (track) {
        scroll_to_live();
    }
}

void PresetUi::on_search_text_changed(std::string text) {
    if (my_module->search_incremental || text.empty()) {
        on_search_text_enter();
    }
}

void PresetUi::on_search_text_enter() {
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

bool PresetUi::filtering() {
    return !search_entry->empty() || active_tab().list.filtered();
}

void PresetUi::on_filter_change(FilterId filter, uint64_t state) {
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

void PresetUi::configure_midi() {
    show_preset_midi_configuration(this, getSvgTheme());
}

void PresetUi::clear_filters() {
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

void PresetUi::set_tab(PresetTab tab_id, bool fetch) {
    switch (tab_id) {
    case PresetTab::User:
        user_label->set_style(&current_tab_style);
        system_label->set_style(&tab_style);
        active_tab_id = PresetTab::User;
        break;

    case PresetTab::Unset:
        assert(false);
        goto sys;

    case PresetTab::System:
    sys:
        user_label->set_style(&tab_style);
        system_label->set_style(&current_tab_style);
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
    if (fetch && (0 == tab.count()) && !start_delay.running() && host_available()) {
        load_presets(tab_id);
    }
    auto theme = getSvgTheme();
    applyChildrenTheme(user_label, theme);
    applyChildrenTheme(system_label, theme);

    scroll_to(tab.scroll_top);
    //update_help();
}

void PresetUi::scroll_to(ssize_t index) {
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

void PresetUi::scroll_to_page_of_index(ssize_t index) {
    scroll_to(page_index(index));
}

ssize_t PresetUi::page_index(ssize_t index) {
    index = std::max(ssize_t(0), index);
    index = std::min(ssize_t(active_tab().count()), index);
    return PAGE_CAPACITY * (index / PAGE_CAPACITY);
}

void PresetUi::scroll_to_live() {
    auto live_id = get_live_id();
    if (!live_id.valid()) return;
    ssize_t index = active_tab().list.index_of_id(live_id);
    if (index < 0) index = 0;
    scroll_to(page_index(index));
}

void PresetUi::page_up(bool ctrl, bool shift) {
    Tab& tab = active_tab();
    if (tab.scroll_top < PAGE_CAPACITY) return;
    if (ctrl) {
        scroll_to(0);
    } else {
        auto index = std::max(ssize_t(0), ssize_t(tab.scroll_top) - PAGE_CAPACITY);
        scroll_to(index);
    }
}

void PresetUi::page_down(bool ctrl, bool shift) {
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

void PresetUi::update_page_controls() {
    Tab& tab = active_tab();
    size_t count = tab.list.count();
    size_t page = 1 + (tab.scroll_top / PAGE_CAPACITY);
    size_t pages = 1 + (count / PAGE_CAPACITY);
    auto info = format_string("%d of %d", page, pages);
    page_label->set_text(info);

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

void PresetUi::update_help() {
    Tab& tab = active_tab();
    if (tab.list.empty()) {
        help_label->set_text("scan presets using the Core actions menu");
        help_label->setVisible(true);
    } else {
        help_label->set_text("");
        help_label->setVisible(false);
    }
}

void PresetUi::send_random_preset() {
    Tab& tab = active_tab();
    if (tab.list.empty()) return;
    ssize_t index = std::round(rack::random::uniform() * (tab.count()-1));
    scroll_to(index);
    send_preset(index);
}

void PresetUi::send_preset(ssize_t index) {
    if (index < 0) return;
    Tab& tab = active_tab();
    if (index >= ssize_t(tab.count())) return;
    auto preset = tab.list.nth(index);
    if (preset && host_available()) {
        chem_host->request_preset(ChemId::Preset, preset->id);
    }
}

void PresetUi::next_preset(bool ctrl, bool shift) {
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

void PresetUi::previous_preset(bool ctrl, bool shift) {
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
