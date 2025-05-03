#include "XM-Edit.hpp"
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

void XMEditModule::do_message(PackedMidiMessage msg)
{

}

void XMEditModule::process_params(const ProcessArgs& args)
{
}

void XMEditModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);
    if (!chem_host || chem_host->host_busy()) return;

    if (((args.frame + id) % 41) == 0) {
        process_params(args);
    }

    if (0 == ((args.frame + id) % 47)) {
        getLight(L_XM).setBrightness(chem_ui && ui()->client);
    }
}

Model *modelXMEdit = createModel<XMEditModule, XMEditUi>("chem-xm-edit");

