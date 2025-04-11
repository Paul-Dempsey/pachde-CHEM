#include "preset-entry.hpp"
#include "../../em/preset-meta.hpp"
#include "../../services/misc.hpp"
#include "../../chem.hpp"
namespace pachde {

PresetEntry::PresetEntry(std::vector<PresetEntry*>& peers, std::shared_ptr<SvgTheme> theme) :
    preset_index(-1),
    live(false),
    current(false),
    peers(peers)
{
    layout_hint_color = fromPacked(parseHslaColor("hsla(60, 80%, 60%, .25%)"));
    LabelStyle grid_style{"preset"};
    addChild(label = createLabel<TipLabel>(Vec(1,1), 158.f, "", theme_engine, theme, grid_style));
}

PresetEntry* PresetEntry::create(Vec pos, std::vector<PresetEntry*>& peers, std::shared_ptr<SvgTheme> theme) {
    PresetEntry* pe = new PresetEntry(peers, theme);
    pe->box.pos = pos;
    pe->box.size.x = pe->label->box.size.x + 2;
    pe->box.size.y = 14.f;
    return pe;
}

void PresetEntry::set_preset(int index, bool is_current, bool is_live, std::shared_ptr<PresetDescription> preset)
{
    preset_id = preset->id;
    preset_index = index;
    live = is_live;
    current = is_current;
    label->text(preset->name);
    if (preset->text.empty()) {
        label->describe(preset->summary());
    } else {
        auto meta = hakenCategoryCode.make_category_mulitline_text(preset->text);
        auto text = format_string("%s\n[%d.%d.%d]\n%s", preset->name.c_str(), preset->id.bank_hi(), preset->id.bank_lo(), preset->id.number(), meta.c_str());
        label->describe(text);
    }
}

void PresetEntry::clear_preset()
{
    preset_id.invalidate();
    preset_index = -1;
    live = false;
    current = false;;
    label->text("");
    label->describe("");
}

bool PresetEntry::applyTheme(SvgThemeEngine &theme_engine, std::shared_ptr<SvgTheme> theme)
{
//    wire_style = theme->name == "Wire";
    preset_element.apply_theme(theme);
    live_element.apply_theme(theme);
    current_element.apply_theme(theme);
    label->applyTheme(theme_engine, theme);
    return true;
}

void PresetEntry::onSelectKey(const SelectKeyEvent &e)
{
    //TODO: auto scroll off ends
    if (((e.mods & RACK_MOD_MASK) == 0) && (e.action == GLFW_PRESS)) {
        if (e.key == GLFW_KEY_UP) {
            auto pit = peers.begin();
            while (pit != peers.end()) {
                auto last = pit;
                pit++;
                if (*pit == this) {
                    if ((*last)->preset_id.valid()) {
                        current = false;
                        (*last)->current = true;
                        APP->event->setSelectedWidget(*last);
                    }
                    e.consume(this);
                    return;
                }
            }
            e.consume(this);
        }
        if (e.key == GLFW_KEY_DOWN) {
            auto pit = peers.rbegin();
            while (pit != peers.rend()) {
                auto last = pit;
                pit++;
                if (*pit == this) {
                    if ((*last)->preset_id.valid()) {
                        current = false;
                        (*last)->current = true;
                        APP->event->setSelectedWidget(*last);
                    }
                    e.consume(this);
                    return;
                }
            }
            e.consume(this);
        }
    }
    Base::onSelectKey(e);
}

void PresetEntry::onButton(const ButtonEvent &e)
{
    for (auto pit = peers.begin(); pit != peers.end(); pit++) {
        (*pit)->current = false;
    }
    if (preset_id.valid()) {
        current = true;
    } else if (!peers.empty()){
        for (auto pit = peers.rbegin(); pit != peers.rend(); pit++) {
            if ((*pit)->preset_id.valid()) {
                (*pit)->current = true;
                break;
            }
        }
    }
    if (current && !live) {
        if (auto p = dynamic_cast<ChemModuleWidget*>(getParent())) {
            if (auto m = p->getChemModule()) {
                if (m->chem_host) {
                    m->chem_host->host_haken()->select_preset(ChemId::Preset, preset_id);
                }
            }
        }
    }
    Base::onButton(e);
}

void PresetEntry::draw(const DrawArgs &args)
{
    auto vg = args.vg;
    if (live) {
        FittedBoxRect(vg, 0, 0, box.size.x, box.size.y, live_element.nvg_stroke_color(), Fit::Inside, live_element.width());
    }

    Base::draw(args);

    if (layout_hints) {
        label->layout_hints = false;
        BoxRect(args.vg, 0, 0, box.size.x, box.size.y, layout_hint_color, 0.35f);
    }
    if (current) {
        nvgBeginPath(vg);
        nvgMoveTo(vg, -4.5f, box.size.y*.5f - 3.5f);
        nvgLineTo(vg,  1.5f, box.size.y*.5f);
        nvgLineTo(vg, -4.5f, box.size.y*.5f + 3.5f);
        nvgClosePath(vg);
        nvgFillColor(vg, current_element.nvg_color());
        nvgFill(vg);
    }
}

};