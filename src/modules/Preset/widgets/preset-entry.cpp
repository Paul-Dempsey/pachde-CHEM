#include "preset-entry.hpp"
#include "em/preset-meta.hpp"
#include "services/misc.hpp"
#include "chem.hpp"
#include "widgets/hamburger.hpp"
#include "../Preset.hpp"

namespace pachde {

PresetEntry::PresetEntry(std::vector<PresetEntry*>& peers) :
    preset_index(-1),
    live(false),
    current(false),
    hovered(false),
    peers(peers)
{
    addChild(label = createLabel<TipLabel>(Vec(1,1), "", &grid_style, 158.f));
}

PresetEntry* PresetEntry::create(Vec pos, std::vector<PresetEntry*>& peers, PresetUi* host) {
    PresetEntry* pe = new PresetEntry(peers);
    pe->box.pos = pos;
    pe->box.size.x = pe->label->box.size.x + 2;
    pe->box.size.y = 14.f;
    pe->set_ui(host);
    return pe;
}

void PresetEntry::set_preset(int index, bool is_current, bool is_live, std::shared_ptr<PresetInfo> preset)
{
    this->preset = preset;
    preset_index = index;
    live = is_live;
    current = is_current;
    label->set_text(preset->name);
    label->describe(preset->meta_text());
    notifyChange(this);
}

void PresetEntry::set_current(ssize_t index)
{
    for (auto pw: peers) {
        pw->current = false;
    }
    current = true;
    notifyChange(this);
}

void PresetEntry::clear_preset()
{
    preset = nullptr;
    preset_index = -1;
    live = false;
    current = false;;
    label->set_text("");
    label->describe("");
    notifyChange(this);
}

void PresetEntry::applyTheme(std::shared_ptr<SvgTheme> theme)
{
    preset_element.apply_theme(theme);
    live_element.apply_theme(theme);
    current_element.apply_theme(theme);
    hover_element.apply_theme(theme);
    category_style.apply_theme(theme);
    //label->applyTheme(theme);
}

void PresetEntry::appendContextMenu(ui::Menu *menu)
{
    if (preset) {
        menu->addChild(createMenuLabel<HamburgerTitle>(preset->name));
        menu->addChild(createMenuItem("Send", "", [this](){ send_preset(); }, !preset_id().valid()));
        menu->addChild(createMenuItem("Set current", "", [this](){
            for (auto pit = peers.begin(); pit != peers.end(); pit++) {
                (*pit)->current = false;
            }
            current = true;
            ui->set_current_index(preset_index);
        }));
        menu->addChild(createMenuItem("Copy info", "", [this](){
            auto info = preset->meta_text();
            glfwSetClipboardString(APP->window->win, info.c_str());
        }));
    }
    // else {
    //     menu->addChild(createMenuLabel<HamburgerTitle>("Slot Actions"));
    //     menu->addChild(createMenuItem("Attempt auto-fill", "", [this](){ }));
    // }
}

void PresetEntry::send_preset()
{
    if (!ui) return;
    auto id = preset_id();
    if (!id.valid()) return;
    ui->send_preset(preset_index);
}

void PresetEntry::onButton(const ButtonEvent &e)
{
    for (auto pit = peers.begin(); pit != peers.end(); pit++) {
        (*pit)->current = false;
    }

    auto mod = e.mods & RACK_MOD_MASK;
    switch (e.button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        if (e.action == GLFW_PRESS)
        {
            if (valid()) {
                current = true;
                ui->set_current_index(preset_index);
            } else if (!peers.empty()){
                for (auto pit = peers.rbegin(); pit != peers.rend(); pit++) {
                    PresetEntry* pe = *pit;
                    if (pe->valid()) {
                        pe->current = true;
                        ui->set_current_index(pe->preset_index);
                        break;
                    }
                }
            }
            if (current && !live) {
                send_preset();
            }
        }
        break;

    case GLFW_MOUSE_BUTTON_RIGHT:
        if (0 == mod) {
            e.consume(this);
            switch (e.action) {
                case GLFW_RELEASE: {
                    createContextMenu();
                } break;
            }
        }
        break;
    }
    Base::onButton(e);
}

void PresetEntry::draw(const DrawArgs &args)
{
    auto vg = args.vg;
    if (live) {
        FittedBoxRect(vg, 0, 0, box.size.x + live_element.width(), box.size.y, live_element.nvg_stroke_color(), Fit::Inside, live_element.width());
    }

    if (hovered && valid()) {
        FillRect(vg, 0, 0, box.size.x, box.size.y, hover_element.nvg_color());
        FittedBoxRect(vg, 0, 0, box.size.x + hover_element.width(), box.size.y, hover_element.nvg_stroke_color(), Fit::Inside, hover_element.width());
    }

    Base::draw(args);

    if (current) {
        nvgBeginPath(vg);
        nvgMoveTo(vg, -4.f, box.size.y*.5f - 3.f);
        nvgLineTo(vg,   .5f, box.size.y*.5f);
        nvgLineTo(vg, -4.f, box.size.y*.5f + 3.f);
        nvgClosePath(vg);
        nvgFillColor(vg, current_element.nvg_color());
        nvgFill(vg);
    }
    if (valid()) {
        auto font = GetPluginFontRegular();
        if (!FontOk(font)) return;

        nvgSave(vg);
        SetTextStyle(vg, font, category_style.nvg_color(), 8.f);
        RightAlignText(vg, box.size.x - 1.5, label->box.size.y, preset->category_code().c_str(), nullptr);
        nvgRestore(vg);
    }
}

};