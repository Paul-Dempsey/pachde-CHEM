#include "Preset.hpp"

void PresetUi::onButton(const ButtonEvent &e)
{
   //e.consume(this);
   Base::onButton(e);
}

void PresetUi::onSelectKey(const SelectKeyEvent &e)
{
    if (APP->event->getSelectedWidget() != this) {
        Base::onSelectKey(e);
        return;
    }
    auto mods = e.mods & RACK_MOD_MASK;
    Tab& tab = active_tab();
    if ((e.action == GLFW_PRESS || e.action == GLFW_REPEAT))
    {
        switch (e.key) {
        case GLFW_KEY_TAB:
            APP->event->setSelectedWidget(this->search_entry);
            e.consume(this);
            return;

        case GLFW_KEY_ESCAPE:
            APP->event->setSelectedWidget(nullptr);
            e.consume(this);
            return;

        case GLFW_KEY_ENTER:
            if (0 == mods)  {
                if ((tab.current_index >= 0) && host_available() && (tab.list.count() > 0)) {
                    chem_host->host_haken()->select_preset(ChemId::Preset, tab.list.nth(tab.current_index)->id);
                }
            }
            e.consume(this);
            return;

        case GLFW_KEY_HOME:
            if (0 == mods)  {
                tab.current_index = tab.scroll_top;
                scroll_to_page_of_index(tab.current_index);
            } else if (mods & RACK_MOD_CTRL) {
                tab.current_index = 0;
                scroll_to_page_of_index(0);
            }
            e.consume(this);
            return;

        case GLFW_KEY_END:
            if (0 == mods)  {
                tab.current_index = tab.scroll_top + PAGE_CAPACITY - 1;
                scroll_to_page_of_index(tab.current_index);
            } else if (mods & RACK_MOD_CTRL) {
                tab.current_index = tab.count() - 1;
                scroll_to_page_of_index(tab.current_index);
            }
            e.consume(this);
            return;

        case GLFW_KEY_UP:
            if (0 == mods)  {
                if (tab.current_index < 0) {
                    tab.current_index = 0;
                    scroll_to_page_of_index(0);
                    e.consume(this);
                    return;
                } else if (tab.current_index > 0) {
                    tab.current_index -= 1;
                    scroll_to_page_of_index(tab.current_index);
                }
            }
            e.consume(this);
            return;

        case GLFW_KEY_PAGE_UP:
            if (mods & RACK_MOD_CTRL) {
                page_up(false, (e.mods & GLFW_MOD_SHIFT));
                e.consume(this);
                return;
            }

            if (tab.current_index < 0) {
                tab.current_index = 0;
                scroll_to_page_of_index(0);
            } else if (tab.current_index > 0) {
                tab.current_index -= PAGE_CAPACITY;
                if (tab.current_index < 0) tab.current_index = 0;
                scroll_to_page_of_index(tab.current_index);
            }
            e.consume(this);
            return;

        case GLFW_KEY_DOWN:
            if (0 == mods) {
                if (tab.current_index < 0) {
                    tab.current_index = 0;
                    scroll_to_page_of_index(0);
                } else if (tab.current_index < ssize_t(tab.count())) {
                    tab.current_index += 1;
                    scroll_to_page_of_index(tab.current_index);
                }
            }
            e.consume(this);
            return;
            
        case GLFW_KEY_PAGE_DOWN:
            if (0 == mods) {
                if (tab.current_index < 0) {
                tab.current_index = 0;
                scroll_to_page_of_index(0);
                } else if (tab.current_index < ssize_t(tab.count())) {
                    tab.current_index = std::min(ssize_t(tab.count()) -1, tab.current_index + PAGE_CAPACITY);
                    scroll_to_page_of_index(tab.current_index);
                }
            }
            e.consume(this);
            return;

        case GLFW_KEY_RIGHT:
            if (0 == mods) {
                if (tab.current_index < 0) {
                    tab.current_index = 0;
                    scroll_to_page_of_index(0);
                } else if (tab.current_index - tab.scroll_top < ROWS) {
                    tab.current_index = std::min(ssize_t(tab.count()) -1, tab.current_index + ROWS);
                    scroll_to_page_of_index(tab.current_index);
                }
            }
            e.consume(this);
            return;

        case GLFW_KEY_LEFT:
            if (0 == mods) {
                if (tab.current_index < 0) {
                    tab.current_index = 0;
                    scroll_to_page_of_index(0);
                } else if (tab.current_index - tab.scroll_top > ROWS) {
                    tab.current_index = std::max(ssize_t(0), tab.current_index - ROWS);
                    scroll_to_page_of_index(tab.current_index);
                }
            }
            e.consume(this);
            return;
        }
    }
    Base::onSelectKey(e);
}

void PresetUi::onHoverScroll(const HoverScrollEvent &e)
{
    if (in_range(e.pos.x, 8.f, 334.f) && in_range(e.pos.y, 38.f, 185.f)) {
        int delta = PAGE_CAPACITY * ((e.scrollDelta.y < 0) ? 1 : (e.scrollDelta.y > 0) ? -1 : 0);
        if (0 != delta) {
            Tab& tab = active_tab();
            auto index = tab.scroll_top + delta;
            if (index <= tab.count()) {
                scroll_to_page_of_index(index);
            }
        }
        e.consume(this);
        return;
    }
    Base::onHoverScroll(e);
}

void PresetUi::end_osmose_scan()
{
    #ifndef NDEBUG
    DEBUG("OB: Finished in %.4f", system::getUnixTime() - osmose_builder->start_time);
    #endif

    live_preset_label->text("");
    stop_scan = false;
    stop_button->requestDelete();
    stop_button = nullptr;
    save_presets(osmose_builder->tab);
    set_tab(osmose_builder->tab, false);
    delete osmose_builder;
    osmose_builder = nullptr;
    stop_spinner();
    gather_start = false;
    gathering = PresetTab::Unset;
    scroll_to(0);
}

void PresetUi::step()
{
    Base::step();
    bind_host(my_module);

    if (start_delay.running()) {
        if (start_delay.finished()) {
            start_delay.stop();
        } else {
            return;
        }
    }

    Tab& tab = active_tab();
    if (tab.list.empty() && host_available() && (gathering == PresetTab::Unset)) {
        set_tab(active_tab_id, true);
    }

    if (filtering()) {
        filter_off_button->button_down = true;
    }

    if (osmose_builder) {
        bool done = false;
        switch (osmose_builder->ready())
        {
        case OsmoseBuilder::ReadyResponse::Ready:
        case OsmoseBuilder::ReadyResponse::Timeout:
            if (!osmose_builder->next(chem_host->host_haken())) {
                done = true;
                live_preset_label->text("");
            } else {
                live_preset_label->text(format_string("[%d:%d]", osmose_builder->cc0, osmose_builder->pc));
            }
            break;
        case OsmoseBuilder::ReadyResponse::Waiting:
            break;
        }
        if (done || stop_scan) {
            end_osmose_scan();
        }
    }
}

void PresetUi::draw(const DrawArgs &args)
{
    Base::draw(args);
    #ifdef LAYOUT_HELP
    if (hints) {
        Line(args.vg, RCENTER, 0, RCENTER, 380, nvgTransRGBAf(PORT_VIOLET, .5f), .5f);
    }
    #endif
}

void PresetUi::appendContextMenu(Menu *menu)
{
    // menu->addChild(new MenuSeparator);
    // menu->addChild(createCheckMenuItem("Track live preset", "", 
    //     [this](){ return my_module->track_live; },
    //     [this](){ set_track_live(!my_module->track_live); }
    // ));

    Base::appendContextMenu(menu);
}
