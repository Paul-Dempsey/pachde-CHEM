#include "preset-entry.hpp"
#include "../../em/preset-meta.hpp"
#include "../../services/misc.hpp"
#include "../../chem.hpp"
#include "Preset.hpp"

namespace pachde {

PresetEntry::PresetEntry(std::vector<PresetEntry*>& peers, std::shared_ptr<SvgTheme> theme) :
    preset_index(-1),
    live(false),
    current(false),
    hovered(false),
    peers(peers)
{
    layout_hint_color = fromPacked(parseHslaColor("hsla(60, 80%, 60%, .25%)"));
    LabelStyle grid_style{"preset"};
    addChild(label = createLabel<TipLabel>(Vec(1,1), 158.f, "", theme_engine, theme, grid_style));
}

PresetEntry* PresetEntry::create(Vec pos, std::vector<PresetEntry*>& peers, PresetUi* host, std::shared_ptr<SvgTheme> theme) {
    PresetEntry* pe = new PresetEntry(peers, theme);
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
    label->text(preset->name);
    label->describe(preset->meta_text());
    notifyChange(this);
}

void PresetEntry::clear_preset()
{
    preset = nullptr;
    preset_index = -1;
    live = false;
    current = false;;
    label->text("");
    label->describe("");
    notifyChange(this);
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

void PresetEntry::onButton(const ButtonEvent &e)
{
    for (auto pit = peers.begin(); pit != peers.end(); pit++) {
        (*pit)->current = false;
    }
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
        if (auto p = dynamic_cast<ChemModuleWidget*>(getParent())) {
            if (auto m = p->getChemModule()) {
                if (m->chem_host) {
                    m->chem_host->host_haken()->select_preset(ChemId::Preset, preset_id());
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

    if (hovered && valid()) {
        FillRect(vg, 0, 0, box.size.x, box.size.y, hover_element.nvg_color());
        FittedBoxRect(vg, 0, 0, box.size.x, box.size.y, hover_element.nvg_stroke_color(), Fit::Inside, hover_element.width());
    }

    Base::draw(args);

    if (layout_hints) {
        label->layout_hints = false;
        BoxRect(args.vg, 0, 0, box.size.x, box.size.y, layout_hint_color, 0.35f);
    }
    if (current) {
        nvgBeginPath(vg);
        nvgMoveTo(vg, -3.5f, box.size.y*.5f - 2.5f);
        nvgLineTo(vg,   .5f, box.size.y*.5f);
        nvgLineTo(vg, -3.5f, box.size.y*.5f + 2.5f);
        nvgClosePath(vg);
        nvgFillColor(vg, current_element.nvg_color());
        nvgFill(vg);
    }
    if (valid()) {
        auto font = GetPluginFontRegular();
        if (!FontOk(font)) return;
    
        nvgSave(vg);
        SetTextStyle(vg, font, category_style.nvg_color(), 8.f);
        RightAlignText(vg, box.size.x - 1.5, label->box.size.y, preset->category_code().c_str(), nullptr, BaselineCorrection::Baseline);
        nvgRestore(vg);
    }
}

};