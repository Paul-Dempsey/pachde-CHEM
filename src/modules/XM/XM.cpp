#include "XM.hpp"
using namespace pachde;
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/rack-help.hpp"

XMModule::XMModule() :
//    modulation(this, ChemId::Unknown),
    glow_knobs(false),
    in_mat_poke(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configParam(P_1, -5.f, 5.f, 0.f, "#1");
    configParam(P_2, -5.f, 5.f, 0.f, "#2");
    configParam(P_3, -5.f, 5.f, 0.f, "#3");
    configParam(P_4, -5.f, 5.f, 0.f, "#4");
    configParam(P_5, -5.f, 5.f, 0.f, "#5");
    configParam(P_6, -5.f, 5.f, 0.f, "#6");
    configParam(P_7, -5.f, 5.f, 0.f, "#7");
    configParam(P_8, -5.f, 5.f, 0.f, "#8");
    configParam(P_MODULATION, -5.f, 5.f, 0.f, "Mod amount");

    configInput(IN_1, "in-1");
    configInput(IN_2, "in-2");
    configInput(IN_3, "in-3");
    configInput(IN_4, "in-4");
    configInput(IN_5, "in-5");
    configInput(IN_6, "in-6");
    configInput(IN_7, "in-7");
    configInput(IN_8, "in-8");

    // configLight(L_IN_1, "Mod knob on Macro a");
    // configLight(L_IN_2, "Mod knob on Macro b");
    // configLight(L_IN_3, "Mod knob on Macro c");
    // configLight(L_IN_4, "Mod knob on Macro d");
    // configLight(L_IN_5, "Mod knob on Macro e");
    // configLight(L_IN_6, "Mod knob on Macro f");
    // configLight(L_IN_7, "Mod knob on Macro g");
    // configLight(L_IN_8, "Mod knob on Macro h");
//    dp4(configParam(P_MOD_AMOUNT, -100.f, 100.f, 0.f, "Modulation amount", "%"));

    // EmccPortConfig cfg[] =         {
    // };
    // modulation.configure(Params::P_MOD_AMOUNT, NUM_MOD_PARAMS, cfg);

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
        title_fg = GetPackedStockColor(StockColor::Gray_65p);
    }
    macros.from_json(root);

    for (int i = 0; i < 8; ++i) {
        auto md = macros.get_macro(i);
        auto pq = getParamQuantity(i);
        if (pq) {
            pq->name = md->name;
        }
        auto ininfo = getInputInfo(i);
        if (ininfo) {
            ininfo->name = md->name;
        }
    }

    ModuleBroker::get()->try_bind_client(this);
}

json_t* XMModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    if (!device_claim.empty()) {
        //modulation.mod_to_json(root);
    }
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    json_object_set_new(root, "module-title", json_string(title.c_str()));
    json_object_set_new(root, "title-bg", json_string(hex_string(title_bg).c_str()));
    json_object_set_new(root, "title-fg", json_string(hex_string(title_fg).c_str()));
    macros.to_json(root);
    return root;
}

// IExtendedMacro
//virtual bool has_modulation() = 0;
std::string XMModule::get_title() { return title; }
PackedColor XMModule::get_header_color() { return title_bg; }
PackedColor XMModule::get_header_text_color() { return title_fg; }
void XMModule::set_header_color(PackedColor color) {
    title_bg = color;
    if (chem_ui) { ui()->set_header_color(color); }
}
void XMModule::set_header_text_color(PackedColor color) {
    title_fg = color;
    if (chem_ui) { ui()->set_header_text_color(color); }
}
void XMModule::set_header_text(std::string title) {
    this->title = title;
    if (chem_ui) { ui()->set_header_text(title); }
}
void XMModule::set_macro_edit(int index) {
    if (chem_ui) { ui()->set_macro_edit(index); }
}
std::shared_ptr<MacroDescription> XMModule::get_macro(int index) {
    return macros.get_macro(index);
}
void XMModule::add_macro(int index) {
    if (chem_ui) { ui()->add_macro(index); }
}
void XMModule::remove_macro(int index) {
    if (chem_ui) { ui()->remove_macro(index); }
}
void XMModule::on_macro_change(int index) {
    if (chem_ui) { ui()->on_macro_change(index); }
}

// IChemClient
::rack::engine::Module* XMModule::client_module() { return this; }
std::string XMModule::client_claim() { return device_claim; }

void XMModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl(this, host);
}

void XMModule::onPresetChange()
{
//    auto em = chem_host->host_matrix();

    // modulation.set_em_and_param_low(P_R1, em->get_r1(), true);
    // getParam(P_EFFECT).setValue(em->get_recirculator_type());

    //if (chem_ui) ui()->onPresetChange();
}

void XMModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void XMModule::do_message(PackedMidiMessage message)
{
    if (as_u8(ChemId::Unknown) == midi_tag(message)) return;

    switch (message.bytes.status_byte) {
    case Haken::ccStat1: {
        in_mat_poke = false;

        int param = -1;
        switch (midi_cc(message)) {
//        case Haken::ccReci1: param = P_R1; break;

        default: return;
        };
        assert(param != -1);
//        modulation.set_em_and_param_low(param, midi_cc_value(message), true);
    } break;

    case Haken::sData: //(ch16 key pressure)
        if (in_mat_poke) {
            switch (message.bytes.data1) {

            } return;
        }
        break;

    case Haken::ccStat16:
        if (Haken::ccStream == midi_cc(message)) {
            in_mat_poke = Haken::s_Mat_Poke == midi_cc_value(message);
        }
        break;
    default:
        break;
    }

}

// void XMModule::onExpanderChange(const ExpanderChangeEvent& e)
// {
//     if (e.side) return; // only care about left expanders

// }

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
}

void XMModule::process_params(const ProcessArgs& args)
{
    //modulation.pull_mod_amount();
}

void XMModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);
    if (!chem_host || chem_host->host_busy()) return;

    if (((args.frame + id) % 41) == 0) {
        process_params(args);
    }

    if (!overlay && (0 == ((args.frame + id) % 120))) {
        overlay = find_adjacent_overlay(this);
        if (!overlay) {
            overlay = find_an_overlay(this, device_claim, "");
        }
    }

    if (0 == ((args.frame + id) % 47)) {
        if (last_mod_target != mod_target) {
            for (int i = 0; i < 8; ++i) {
                getLight(i).setSmoothBrightness((i == mod_target) ? 1.f : 0.f, 90);
            }
            last_mod_target = mod_target;
        }
    }
}

Model *modelXM = createModel<XMModule, XMUi>("chem-x-macro");

