#include "Fx.hpp"
using namespace pachde;
#include "../../services/em-param-quantity.hpp"
#include "../../em/wrap-HakenMidi.hpp"

FxModule::FxModule()
:   chem_host(nullptr),
    ui(nullptr),
    last_disable(-1),
    glow_knobs(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configU7EmParam(Haken::ch1, Haken::ccReci1,   this, Params::P_R1,  0.f, 10.f, 5.f, "R1");
    configU7EmParam(Haken::ch1, Haken::ccReci2,   this, Params::P_R2,  0.f, 10.f, 5.f, "R2");
    configU7EmParam(Haken::ch1, Haken::ccReci3,   this, Params::P_R3,  0.f, 10.f, 5.f, "R3");
    configU7EmParam(Haken::ch1, Haken::ccReci4,   this, Params::P_R4,  0.f, 10.f, 5.f, "R4");
    configU7EmParam(Haken::ch1, Haken::ccReci5,   this, Params::P_R5,  0.f, 10.f, 5.f, "R5");
    configU7EmParam(Haken::ch1, Haken::ccReci6,   this, Params::P_R6,  0.f, 10.f, 5.f, "R6");
    configU7EmParam(Haken::ch1, Haken::ccReciMix, this, Params::P_MIX, 0.f, 10.f, 5.f, "Mix");
    configParam(P_ATTENUVERT, -100.f, 100.f, 0.f, "Input attenuverter", "%")->displayPrecision = 4;

    configSwitch(P_DISABLE, 0.f, 1.f, 0.f, "Fx", {"on", "off"});
    configSwitch(P_EFFECT, 0.f, 6.f, 0.f, "Effect", {"Short reverb", "Mod delay", "Swept delay", "Analog echo", "Delay with LPF", "Delay with HPF", "Long reverb"});

    configInput(IN_R1, "R1");
    configInput(IN_R2, "R2");
    configInput(IN_R3, "R3");
    configInput(IN_R4, "R4");
    configInput(IN_R5, "R5");
    configInput(IN_R6, "R6");
    configInput(IN_MIX, "Recirculator Mix");
    //configInput(IN_ENABLE, "Enable Recirculator");

    // unconfigured L_DISABLE supresses the normal tooltip, which we don't want for lights under buttons
    configLight(L_MIX, "Fx Mix");
}

void FxModule::dataFromJson(json_t* root)
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

json_t* FxModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

// IChemClient
::rack::engine::Module* FxModule::client_module() { return this; }
std::string FxModule::client_claim() { return device_claim; }

void FxModule::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (!host) {
        device_claim = "";
        if (ui) ui->onConnectHost(nullptr);
        return;
    }
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    if (conn) {
        device_claim = conn->info.claim();
    }
    if (ui) ui->onConnectHost(host);
}

void FxModule::onPresetChange()
{
    auto em = chem_host->host_matrix();
    auto disable = em->is_disable_recirculator();
    getParam(Params::P_DISABLE).setValue(disable);
    getLight(Lights::L_DISABLE).setBrightnessSmooth(disable, 45.f);

    if (ui) ui->onPresetChange();
}

void FxModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (ui) ui->onConnectionChange(device, connection);
}

void FxModule::process(const ProcessArgs& args)
{
    if (!chem_host || chem_host->host_busy()) return;

    if (0 == ((args.frame + id) % 47)) {
        int disable = getParamInt(getParam(Params::P_DISABLE));
        if (last_disable == -1) {
            last_disable = disable;
        } else if (last_disable != disable) {
            last_disable = disable;
            chem_host->host_haken()->disable_recirculator(disable);
        }
        getLight(Lights::L_DISABLE).setBrightnessSmooth(disable, 45.f);

        if (disable) {
            getLight(Lights::L_MIX).setBrightnessSmooth(0, 45.f);
        } else {
            float v = getParam(Params::P_MIX).getValue()* .1f;
            getLight(Lights::L_MIX).setBrightnessSmooth(v, 45.f);
        }

    }
}

Model *modelFx = createModel<FxModule, FxUi>("chem-fx");

