#include "Convo.hpp"
using namespace pachde;

ConvoModule::ConvoModule()
:   glow_knobs(false),
    conv_number(0),
    last_conv(-1),
    extend(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configSwitch(P_TYPE, 0.f, 18.f, 6.f, "Convolution type", {
		"Waterphone 1",
        "Waterphone 2",
        "Autoharp 1",
        "Autoharp 2",
		"Dark Guitar",
        "Finger Snap",
        "Wood",
        "Bright Metal",
		"Fiber",
        "Leather",
        "Damped Metal",
        "Nylon",
		"Cloth",
        "Eowave Gong",
        "LVDL Pyramide",
        "LVDL Gong",
        "LVDL Onde",
        "White",
        "Grey"
    });
    configParam(P_LENGTH, 0.f, 10.f, 10.f, "Length");
    configParam(P_TUNING, 0.f, 10.f,  5.f, "Tuning");
    configParam(P_WIDTH,  0.f, 10.f,  5.f, "Width");
    configParam(P_LEFT,   0.f, 10.f, 10.f, "Left attenuation");
    configParam(P_RIGHT,  0.f, 10.f, 10.f, "Right attenuation");

    configParam(P_PRE_MIX,    0.f, 10.f, 0.f, "Pre Mix");
    configParam(P_PRE_INDEX,  0.f, 10.f, 0.f, "Pre Index");
    configParam(P_POST_MIX,   0.f, 10.f, 0.f, "Post Mix");
    configParam(P_POST_INDEX, 0.f, 10.f, 0.f, "Post Index");

    configParam(P_ATTENUVERT, -100.f, 100.f, 0.f, "Input attenuverter", "%");

    configSwitch(P_SELECT, 0.f, 3.f, 0.f, "Select convolution", { "Convolution #1", "Convolution #2", "Convolution #3", "Convolution #4"});
    configSwitch(P_EXTEND, 0.f, 1.f, 0.f, "Extend computation", {"off", "on"});

    configInput(IN_PRE_MIX,    "Pre mix");
    configInput(IN_PRE_INDEX,  "Pre index");
    configInput(IN_POST_MIX,   "Post mix");
    configInput(IN_POST_INDEX, "Post index");

    configLight(L_IN_PRE_MIX,   "Attenuverter on Pre mix");
    configLight(L_IN_PRE_INDEX, "Attenuverter on Pre index");
    configLight(L_IN_POST_MIX,  "Attenuverter on Post mix");
    configLight(L_IN_POST_INDEX,"Attenuverter on Post index");
    // unconfigured L_EXTEND supresses the normal tooltip, which we don't want for lights under buttons

}

void ConvoModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_t* j = json_object_get(root, "haken-device");
    if (j) {
        device_claim = json_string_value(j);
    }
    j = json_object_get(root, "glow-knobs");
    if (j) {
        glow_knobs = json_boolean_value(j);
    }
    ModuleBroker::get()->try_bind_client(this);
}

json_t* ConvoModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

// IChemClient
::rack::engine::Module* ConvoModule::client_module() { return this; }
std::string ConvoModule::client_claim() { return device_claim; }

void ConvoModule::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (!host) {
        device_claim = "";
        if (chem_ui) ui()->onConnectHost(nullptr);
        return;
    }
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    if (conn) {
        device_claim = conn->info.claim();
    }
    if (chem_ui) ui()->onConnectHost(host);
}

void ConvoModule::onPresetChange()
{
    if (chem_ui) ui()->onPresetChange();
}

void ConvoModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void ConvoModule::process_params(const ProcessArgs& args)
{
    {
        auto v = getParamInt(getParam(Params::P_EXTEND));
        getLight(Lights::L_EXTEND).setBrightnessSmooth(v, 45.f);
    }

    int current = getParamInt(getParam(Params::P_SELECT));
    if (last_conv != current) {
        last_conv = current;
        conv_number = current;
        getParam(Params::P_TYPE).setValue(static_cast<float>(convs[conv_number].type));
        getParam(Params::P_LENGTH).setValue(static_cast<float>(convs[conv_number].length));
        getParam(Params::P_TUNING).setValue(static_cast<float>(convs[conv_number].tuning));
        getParam(Params::P_WIDTH).setValue(static_cast<float>(convs[conv_number].width));
        getParam(Params::P_LEFT).setValue(static_cast<float>(convs[conv_number].left));
        getParam(Params::P_RIGHT).setValue(static_cast<float>(convs[conv_number].right));
        return;
    }

    convs[conv_number].type = getParamInt(getParam(Params::P_TYPE));
    convs[conv_number].length = getParamInt(getParam(Params::P_LENGTH));
    convs[conv_number].tuning = getParamInt(getParam(Params::P_TUNING));
    convs[conv_number].width = getParamInt(getParam(Params::P_WIDTH));
    convs[conv_number].left = getParamInt(getParam(Params::P_LEFT));
    convs[conv_number].right = getParamInt(getParam(Params::P_RIGHT));
}

void ConvoModule::process(const ProcessArgs& args)
{
    if (!chem_host || chem_host->host_busy()) return;

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }

}

Model *modelConvo = createModel<ConvoModule, ConvoUi>("chem-convo");

