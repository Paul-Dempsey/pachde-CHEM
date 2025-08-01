#include "XM.hpp"
using namespace pachde;
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/rack-help.hpp"

XMModule::XMModule() :
//    modulation(this, ChemId::Unknown),
    title_bg(GetPackedStockColor(StockColor::pachde_blue_medium)),
    title_fg(GetPackedStockColor(StockColor::Gray_65p)),
    has_mod_knob(false),
    glow_knobs(false),
    in_mat_poke(false)
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
    // if (chem_host) {
    //     chem_host->unregister_chem_client(this);
    // }
}

void XMModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    device_claim = get_json_string(root, "haken-device");
    if (!device_claim.empty()) {
        //modulation.mod_from_json(root);
    }
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
    has_mod_knob = get_json_bool(root, "modulation", false);
    my_macros.from_json(root);
    update_param_info();

    try_bind_overlay();
    ModuleBroker::get()->try_bind_client(this);
}

json_t* XMModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    json_object_set_new(root, "module-title", json_string(title.c_str()));
    json_object_set_new(root, "title-bg", json_string(hex_string(title_bg).c_str()));
    json_object_set_new(root, "title-fg", json_string(hex_string(title_fg).c_str()));
    json_object_set_new(root, "modulation", json_boolean(has_mod_knob));
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
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
        overlay->overlay_register_client(this);
        onConnectHost(overlay->get_host());
    } else  {
        onConnectHost(nullptr);
    }
}

void XMModule::onRemove(const RemoveEvent &e)
{
    if (overlay) {
        overlay->overlay_unregister_client(this);
        overlay = nullptr;
    }
    if (chem_host) {
        chem_host->unregister_chem_client(this);
        chem_host = nullptr;
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
    for (auto m : my_macros.data) {
        auto pq = getParamQuantity(m->knob_id);
        pq->name = m->name;
        pq->minValue = m->min;
        pq->maxValue = m->max;
        pq->setValue(pq->getValue()); // keep value in range
        getInputInfo(m->knob_id)->name = m->name;
    }
}

void XMModule::on_overlay_change(IOverlay *ovr)
{
    if (overlay) {
        overlay->overlay_unregister_client(this);
    }
    overlay = ovr;
    if (overlay) {
        overlay->overlay_register_client(this);
        chem_host = overlay->get_host();
        if (chem_host) {
            chem_host->register_chem_client(this);
        }
    }
}

// IExtendedMacro
void XMModule::xm_clear() {
    title= "";
    title_bg = GetPackedStockColor(StockColor::pachde_blue_medium);
    title_fg = GetPackedStockColor(StockColor::Gray_65p);

    if (chem_ui) { ui()->xm_clear(); }
}

// IChemClient
::rack::engine::Module* XMModule::client_module() { return this; }
std::string XMModule::client_claim() { return device_claim; }

void XMModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl(this, host);
}

// void XMModule::onPresetChange()
// {
//     if (chem_ui) ui()->onPresetChange();
// }

void XMModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void XMModule::onPortChange(const PortChangeEvent& e)
{
    if (e.connecting) {
        mod_target = e.portId;
    } else {
        if (mod_target == e.portId) {
            mod_target = -1;
            for (int i = 0; i < 8; ++i) {
                if ((i != e.portId) && getInput(i).isConnected()) {
                    mod_target = i;
                    break;
                }
            }
        }
    }
    if (mod_target >= 0) {
        auto macro = my_macros.get_macro(getId(), mod_target);
        getParam(P_MODULATION).setValue(macro ? macro->mod_amount : 0.f);
    }
}

void XMModule::process_params(const ProcessArgs& args)
{
    if (has_mod_knob && (mod_target >= 0)) {
        auto macro = my_macros.get_macro(getId(), mod_target);
        macro->mod_amount = getParam(P_MODULATION).getValue();
    }
}

void XMModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);
    auto jitter_frame = args.frame + id;

    if (!overlay && (0 == (jitter_frame % 120))) {
        try_bind_overlay();
    }
    if (overlay && !chem_host && (0 == (jitter_frame % 97))) {
        onConnectHost(overlay->get_host());
        return;
    }

    bool free_host = chem_host && !chem_host->host_busy();

    if (0 == (jitter_frame % 47)) {
        if (last_mod_target != mod_target) {
            for (int i = 0; i < 8; ++i) {
                getLight(i).setSmoothBrightness((i == mod_target) ? 1.f : 0.f, 90);
            }
            last_mod_target = mod_target;
        }
        getLight(L_OVERLAY).setSmoothBrightness((overlay ? 1.0f : 0.f), 90);
        getLight(L_CORE).setSmoothBrightness(free_host, 90);
    }

    if (!free_host) return;
    if ((jitter_frame % 41) == 0) {
        process_params(args);
    }
    for (auto macro: my_macros.data) {
        macro->cv = getInput(macro->knob_id).getVoltage();
        macro->param_value = getParam(macro->knob_id).getValue();
        float v = xm_modulated_value(macro->param_value, macro->cv, macro->mod_amount);
        macro->mod_value = v;
    }

}

Model *modelXM = createModel<XMModule, XMUi>("chem-xm");

