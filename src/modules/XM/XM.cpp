#include "XM.hpp"
using namespace pachde;
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/rack-help.hpp"

XMModule::XMModule() :
    title_bg(GetPackedStockColor(StockColor::pachde_blue_medium)),
    title_fg(GetPackedStockColor(StockColor::Gray_65p)),
    glow_knobs(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    dp4(configParam(P_1, -1.f, 1.f, 0.f, "#1"));
    dp4(configParam(P_2, -1.f, 1.f, 0.f, "#2"));
    dp4(configParam(P_3, -1.f, 1.f, 0.f, "#3"));
    dp4(configParam(P_4, -1.f, 1.f, 0.f, "#4"));
    dp4(configParam(P_5, -1.f, 1.f, 0.f, "#5"));
    dp4(configParam(P_6, -1.f, 1.f, 0.f, "#6"));
    dp4(configParam(P_7, -1.f, 1.f, 0.f, "#7"));
    dp4(configParam(P_8, -1.f, 1.f, 0.f, "#8"));
    dp4(configParam(P_MODULATION, -100.f, 100.f, 0.f, "Modulation amount", "%"));

    dp4(configParam(P_RANGE_MIN, -1.f, 1.f, -1.f, "Minimum"));
    dp4(configParam(P_RANGE_MAX, -1.f, 1.f, 1.f, "Maximum"));

    configInput(IN_1, "in-1");
    configInput(IN_2, "in-2");
    configInput(IN_3, "in-3");
    configInput(IN_4, "in-4");
    configInput(IN_5, "in-5");
    configInput(IN_6, "in-6");
    configInput(IN_7, "in-7");
    configInput(IN_8, "in-8");

    configLight(L_OVERLAY, "Overlay module connected");
    configLight(L_CORE, "Core module connected");
}

XMModule::~XMModule()
{
    if (overlay) {
        overlay->overlay_unregister_client(this);
    }
    if (chem_host) {
        chem_host->unregister_chem_client(this);
    }
}

void XMModule::dataFromJson(json_t* root)
{
    if (overlay) {
        overlay->overlay_unregister_client(this);
        overlay = nullptr;
    }
    if (chem_host) {
        chem_host->unregister_chem_client(this);
        chem_host = nullptr;
    }
    if (chem_ui) {
        if (ui()->editing) {
            ui()->set_edit_mode(false);
        }
    }
    ChemModule::dataFromJson(root);
    device_claim = get_json_string(root, "haken-device");
    glow_knobs = get_json_bool(root, "glow-knobs", false);
    title = get_json_string(root, "module-title");
    title_bg = parse_color(get_json_string(root, "title-bg"), RARE_COLOR);
    if (title_bg == RARE_COLOR) {
        title_bg = GetPackedStockColor(StockColor::pachde_blue_dark);
    }
    title_fg = parse_color(get_json_string(root, "title-fg"), RARE_COLOR);
    if (title_fg == RARE_COLOR) {
        title_fg = GetPackedStockColor(StockColor::Gray_75p);
    }
    my_macros.clear();
    my_macros.from_json(root);
    update_param_info();
    if (chem_ui) {
        ui()->update_main_ui(theme_engine.getTheme(getThemeName()));
    }
}

json_t* XMModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    json_object_set_new(root, "module-title", json_string(title.c_str()));
    json_object_set_new(root, "title-bg", json_string(hex_string(title_bg).c_str()));
    json_object_set_new(root, "title-fg", json_string(hex_string(title_fg).c_str()));
    my_macros.to_json(root);
    return root;
}

void XMModule::try_bind_overlay()
{
    overlay = find_adjacent_overlay(this);
    if (!overlay) {
        overlay = find_an_overlay(this, device_claim, "");
    }
    if (overlay) {
        overlay->overlay_register_client(this);
        update_overlay_macros();
        try_bind_overlay_host();
    }
}

void XMModule::try_bind_overlay_host()
{
    if (!overlay) return;
    if (chem_host) {
        assert(!overlay->get_host() || (overlay->get_host() == chem_host));
    } else {
        chem_host = overlay->get_host();
        if (chem_host) {
            chem_host->register_chem_client(this);
            update_from_em();
        }
    }
}

void XMModule::onReset(const ResetEvent &e)
{
    if (chem_ui) {
        ui()->set_edit_mode(false);
    }
    if (overlay) {
        remove_overlay_macros();
        overlay->overlay_unregister_client(this);
        overlay = nullptr;
    }
    my_macros.clear();
    update_param_info();
    if (chem_ui) {
        ui()->update_main_ui(theme_engine.getTheme(ui()->getThemeName()));
    }
}

void XMModule::onRemove(const RemoveEvent &e)
{
    if (overlay) {
        remove_overlay_macros();
        overlay->overlay_unregister_client(this);
        overlay = nullptr;
    }
    if (chem_host) {
        chem_host->unregister_chem_client(this);
        chem_host = nullptr;
    }
}

bool XMModule::has_mod_knob()
{
    return std::any_of(my_macros.data.cbegin(), my_macros.data.cend(),
        [](std::shared_ptr<pachde::MacroDescription> md){
            return md->modulation;
        });
}

void XMModule::set_modulation_target(int id)
{
    if (getInput(id).isConnected()) {
        auto macro = my_macros.get_macro(getId(), id);
        if (macro && macro->modulation) {
            getParamQuantity(P_MODULATION)->setValue(macro->mod_amount);
        }
        mod_target = id;
    }
}

void XMModule::center_knobs()
{
    for (int i = 0; i < 8; ++i) {
        getParamQuantity(i)->setValue(0.f);
        // macro data and em will be updated in next process()
    }
}

void XMModule::update_param_info()
{
    for (int i = 0; i < 8; ++i) {
        auto m = my_macros.get_macro(getId(), i);
        auto pq = getParamQuantity(i);
        if (pq) {
            if (m) {
                pq->name = m->name;
                pq->minValue = m->min;
                pq->maxValue = m->max;
                pq->setValue(pq->getValue()); // keep value in range
                getInputInfo(i)->name = m->name;
            } else {
                pq->name = format_string("#%d", 1 + i);
                pq->minValue = -1.f;
                pq->maxValue = 1.f;
                pq->setValue(0);
                getInputInfo(i)->name = format_string("in-%d", 1 + i);
            }
        }
    }
}

void XMModule::update_overlay_macros()
{
    if (!overlay) return;
    for (auto macro: my_macros.data) {
        if (macro->valid()) {
            overlay->overlay_add_update_macro(macro);
        }
    }
}

void XMModule::remove_overlay_macros()
{
    if (!overlay) return;
    for (auto macro: my_macros.data) {
        overlay->overlay_remove_macro(getId(), macro->knob_id);
    }
}

void XMModule::update_from_em()
{
    if (chem_host && overlay && overlay->overlay_preset_connected()) {
        auto em = chem_host->host_matrix();
        if (!em) return;
        for (auto macro: my_macros.data) {
            auto em_value = em->get_macro_value(macro->macro_number);
            macro->set_em_value(em_value);
        }
    }
}

void XMModule::do_message(PackedMidiMessage message)
{
    if (my_macros.empty()) { return; }
    if (!chem_host) { return; }
    if (!overlay) { return; }
    if (!overlay->overlay_preset_connected()) { return; }
    if (Haken::ccStat1 != message.bytes.status_byte) { return; }
    auto em = chem_host->host_matrix();
    if (!em) { return; }

    auto tag = midi_tag(message);
    if (as_u8(ChemId::Unknown) == tag) { return; }
    if (as_u8(ChemId::Overlay) == tag) { return; }
    if (as_u8(ChemId::XM) == tag) { return; }

    auto cc = midi_cc(message);
    if (cc < Haken::ccM7 || cc > Haken::ccM90) { return; }

    uint8_t number = 0;
    if (cc <= Haken::ccM30) {
        number = (cc - Haken::ccM7);
    } else if (cc < Haken::ccM31) {
        return;
    } else if (cc <= Haken::ccM48) {
        number = (cc - Haken::ccM31);
    }
    if (!number) return;
    if (em->frac_hi) number += 48;

    for (auto macro: my_macros.data) {
        if (number == macro->macro_number) {
            macro->set_em_value(em->get_macro_value(number));
            getParamQuantity(macro->knob_id)->setValue(macro->param_value);
            break;
        }
    }
}

void XMModule::on_overlay_change(IOverlay *new_overlay)
{
    if (overlay == new_overlay) { return; }
    if (overlay) {
        remove_overlay_macros();
        overlay->overlay_unregister_client(this);
    }
    overlay = new_overlay;
    if (chem_host) {
        chem_host->unregister_chem_client(this);
        chem_host = nullptr;
    }
    if (overlay) {
        overlay->overlay_register_client(this);
        update_overlay_macros();
        try_bind_overlay_host();
    }
}

void XMModule::on_connect_preset()
{
    update_from_em();
}

// IExtendedMacro
void XMModule::xm_clear() {
    title= "";
    title_bg = GetPackedStockColor(StockColor::pachde_blue_medium);
    title_fg = GetPackedStockColor(StockColor::Gray_65p);
    if (chem_ui) { ui()->xm_clear(); }
}

// IChemClient
void XMModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl_no_ui(this, host);
}

void XMModule::onPortChange(const PortChangeEvent& e)
{
    if (e.connecting) {
        auto macro = my_macros.get_macro(getId(), e.portId);
        mod_target = e.portId;

        if (macro && macro->modulation) {
            auto pq = getParamQuantity(P_MODULATION);
            if (pq) pq->setValue(macro->mod_amount);
        }
    } else {
        if (mod_target == e.portId) {
            mod_target = -1;
            for (int i = 0; i < 8; ++i) {
                if ((i != e.portId) && getInput(i).isConnected()) {
                    auto macro = my_macros.get_macro(getId(), i);
                    if (macro && macro->modulation) {
                        mod_target = i;
                        auto pq = getParamQuantity(P_MODULATION);
                        if (pq) pq->setValue(macro->mod_amount);
                        break;
                    }
                }
            }
        }
    }
}

void XMModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);
    auto jitter_frame = args.frame + id;

    if (!overlay && (0 == (jitter_frame % 120))) {
        try_bind_overlay();
    }
    if (overlay && !chem_host && (0 == (jitter_frame % 90))) {
        try_bind_overlay_host();
    }

    if (0 == (jitter_frame % 47)) {
        if (last_mod_target != mod_target) {
            for (int i = 0; i < 8; ++i) {
                getLight(i).setBrightness(0.f);
            }
            if (mod_target >= 0) {
                getLight(mod_target).setBrightness(1.f);
            }
            last_mod_target = mod_target;
        }
        getLight(L_OVERLAY).setSmoothBrightness((overlay ? 1.0f : 0.f), 90);
        getLight(L_CORE).setSmoothBrightness(overlay && chem_host, 90);

        auto edit_macro = chem_ui ? ui()->get_edit_macro() : nullptr;
        if (edit_macro && (MacroRange::Custom == edit_macro->range)) {
            float rmin = getParam(Params::P_RANGE_MIN).getValue();
            float rmax = getParam(Params::P_RANGE_MAX).getValue();
            float min = std::min(rmin, rmax);
            float max = std::max(rmin, rmax);
            edit_macro->min = min;
            edit_macro->max = max;
        }
    }

    if (!overlay) { return; }
    if (!host_connected(chem_host) || chem_host->host_busy()) return;
    if (!overlay->overlay_preset_connected()) { return; }
    if (!chem_ui) { return; }
    if (ui()->editing) { return; }

    for (auto macro: my_macros.data) {
        auto knob = macro->knob_id;
        if ((mod_target == knob) && macro->modulation) {
            macro->set_mod_amount(getParam(P_MODULATION).getValue());
        }
        if (macro->cv_port && getInput(knob).isConnected()) {
            macro->cv = macro->modulation
                ? getInput(knob).getVoltage()
                : rescale(getInput(knob).getVoltage(), -5.f, 5.f, macro->min * 5.f, macro->max * 5.f);
        }
        if (0 == (jitter_frame % 31)) {
            macro->set_param_value(getParam(macro->knob_id).getValue());
        }
    }
}

Model *modelXM = createModel<XMModule, XMUi>("chem-xm");

