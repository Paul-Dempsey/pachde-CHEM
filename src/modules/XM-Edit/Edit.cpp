#include "Edit.hpp"
using namespace pachde;
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/rack-help.hpp"

XMEditModule::XMEditModule()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    std::vector<std::string> off_on{ "off", "on"};
    configSwitch(P_ADD_REMOVE, 0.f, 1.f, 1.f, "Enable macro", off_on);
    configParam(P_RANGE_MIN, 0, Haken::max14, 0, "Minimum");
    configParam(P_RANGE_MAX, 0, Haken::max14, Haken::max14, "Maximum");
    configSwitch(P_INPUT, 0.f, 1.f, 1.f, "Macro in port", off_on);
    configSwitch(P_MOD, 0.f, 1.f, 1.f, "Modulate macro inputs", off_on);

    configLight(L_XM, "Connected XM");
    configLight(L_CORE, "Connected Core");
    configLight(L_OVERLAY, "Connected Overlay");
}

void XMEditModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_read_string(root, "haken-device", device_claim);
    ModuleBroker::get()->try_bind_client(this);
}

json_t* XMEditModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    return root;
}

void XMEditModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl(this, host);
}

void XMEditModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
//    host_connection = connection ? connection->identified() : false;
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void XMEditModule::onPresetChange()
{
    if (chem_ui) ui()->onPresetChange();
}

Module * XMEditModule::get_xm_module()
{
    Module * mod = getLeftExpander().module;
    if (mod && mod->getModel() == modelXM) {
        return mod;
    }
    return nullptr;
}

IExtendedMacro *XMEditModule::get_xm_client()
{
    return dynamic_cast<IExtendedMacro *>(get_xm_module());
}

void XMEditModule::process_params(const ProcessArgs& args)
{
    if (chem_ui) {
        auto my_ui = ui();
        auto macro = my_ui->current_macro;
        if (macro)
        {
            if (macro->range == MacroRange::Custom) {
                int min = getParamInt(getParam(P_RANGE_MIN));
                int max = getParamInt(getParam(P_RANGE_MAX));
                bool change{false};
                if (min != macro->min) {
                    macro->min = min;
                    change = true;
                }
                if (max != macro->max) {
                    macro->max = max;
                    change = true;
                }

                if (change) {
                    auto client = get_xm_client();
                    if (client) client->on_macro_change(my_ui->knob_index);
                }
            } else {
                getParam(P_RANGE_MIN).setValue(macro->range == MacroRange::Bipolar ? 0 : Haken::zero14);
                getParam(P_RANGE_MAX).setValue(Haken::max14);
            }
        }
    }
}

void XMEditModule::process(const ProcessArgs& args)
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
        getLight(L_XM).setSmoothBrightness(nullptr != get_xm_module(), 90);
        getLight(L_CORE).setSmoothBrightness(!device_claim.empty() && chem_host && !chem_host->host_busy(), 90);
        getLight(L_OVERLAY).setSmoothBrightness(nullptr != overlay, 90);
    }
}

Model *modelXMEdit = createModel<XMEditModule, XMEditUi>("chem-xm-edit");

