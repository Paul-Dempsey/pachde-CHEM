#include "Macro.hpp"
#include "../../services/em-param-quantity.hpp"
using namespace pachde;

MacroModule::MacroModule()
:   attenuator_target(IN_INVALID),
    last_attenuator_target(IN_INVALID),
    glow_knobs(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configInput(IN_M1, "Macro 1 (i)");
    configInput(IN_M2, "Macro 2 (ii)");
    configInput(IN_M3, "Macro 3 (iii)");
    configInput(IN_M4, "Macro 4 (iv)");
    configInput(IN_M5, "Macro 5 (v)");
    configInput(IN_M6, "Macro 6 (vi)");

    configU14ccEmParam(Haken::ch1, Haken::ccI,   this, Params::P_M1, 0.f, 10.f, 0.f, "Macro 1", "v");
    configU14ccEmParam(Haken::ch1, Haken::ccII,  this, Params::P_M2, 0.f, 10.f, 0.f, "Macro 2", "v");
    configU14ccEmParam(Haken::ch1, Haken::ccIII, this, Params::P_M3, 0.f, 10.f, 0.f, "Macro 3", "v");
    configU14ccEmParam(Haken::ch1, Haken::ccIV,  this, Params::P_M4, 0.f, 10.f, 0.f, "Macro 4", "v");
    configU14ccEmParam(Haken::ch1, Haken::ccV,   this, Params::P_M5, 0.f, 10.f, 0.f, "Macro 5", "v");
    configU14ccEmParam(Haken::ch1, Haken::ccVI,  this, Params::P_M6, 0.f, 10.f, 0.f, "Macro 6", "v");
    configParam(P_ATTENUVERT, -100.f, 100.f, 0.f, "Modulation amount", "%")->displayPrecision = 4;

    configLight(L_M1a, "Modulation amount on M1");
    configLight(L_M2a, "Modulation amount on M2");
    configLight(L_M3a, "Modulation amount on M3");
    configLight(L_M4a, "Modulation amount on M4");
    configLight(L_M5a, "Modulation amount on M5");
    configLight(L_M6a, "Modulation amount on M6");
}

void MacroModule::dataFromJson(json_t* root)
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
    if (!device_claim.empty()) {
        auto jar = json_object_get(root, "attenuation");
        if (jar) {
            json_t* jp;
            size_t index;
            json_array_foreach(jar, index, jp) {
                attenuation[index] = json_real_value(jp);
            }
        }
    }
    ModuleBroker::get()->try_bind_client(this);
}

json_t* MacroModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));

    if (!device_claim.empty()) {
        auto jaru = json_array();
        for (int i = 0; i <= IN_M6; ++i) {
            json_array_append_new(jaru, json_real(attenuation[i]));
        }
        json_object_set_new(root, "attenuation", jaru);
    }

    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

// IChemClient
::rack::engine::Module* MacroModule::client_module() { return this; }
std::string MacroModule::client_claim() { return device_claim; }

void MacroModule::onConnectHost(IChemHost* host)
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

void MacroModule::onPresetChange()
{
    if (chem_host) {
        if (chem_host->host_busy()) return;
        auto preset = chem_host->host_preset();
        if (preset) {
            macro_names.fill_macro_names();
            macro_names.parse_text(preset->text);
            update_from_em();
        }
    }
    if (chem_ui) ui()->onPresetChange();
}

void MacroModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void MacroModule::onPortChange(const PortChangeEvent& e)
{
    if (e.type == Port::OUTPUT) return;
    if (e.connecting) {
        attenuator_target = e.portId;
        auto pq = getParamQuantity(P_ATTENUVERT);
        if (pq) {
            pq->setImmediateValue(attenuation[attenuator_target]);
        }
    } else {
        for (int i = IN_M1; i <= IN_M6; ++i) {
            if (getInput(i).isConnected()) {
                attenuator_target = i;
                auto pq = getParamQuantity(P_ATTENUVERT);
                if (pq) {
                    pq->setImmediateValue(attenuation[i]);
                }
                return;
            }
        }
        attenuator_target = Inputs::IN_INVALID;
        auto pq = getParamQuantity(P_ATTENUVERT);
        if (pq) {
            pq->setImmediateValue(0.f);
        }
    }
}

void MacroModule::update_from_em()
{
    if (chem_host && chem_host->host_preset()) {
        auto em = chem_host->host_matrix();
        if (em) {
            for (int param = Params::P_M1; param <= P_M6; ++param) {
                float m = em->get_macro_voltage(param);
                getParam(param).setValue(m);
            }
        }
    }
}

void MacroModule::process_params(const ProcessArgs& args)
{
    if (attenuator_target >= 0) {
        auto pq = getParamQuantity(P_ATTENUVERT);
        if (pq) {
            attenuation[attenuator_target] = pq->getValue();
        }
    }
    update_from_em();
}

void MacroModule::process(const ProcessArgs& args)
{
    if (!chem_host || chem_host->host_busy()) return;
    
    if (((args.frame + id) % 40) == 0) {
        process_params(args);
    }

    for (int i = IN_M1; i <= IN_M6; ++i) {
        if (getInput(i).isConnected()) {
            auto in = getInput(i).getVoltage();
            modulated[i] = modulated_value(getParam(i).getValue(), in, attenuation[i]);
        } else {
            modulated[i] = getParam(i).getValue();
        }
    }

    if (((args.frame + id) % 63) == 0) {
        // attenuator lights
        if (last_attenuator_target != attenuator_target) {
            for (int i = L_M1a; i <= L_M6a; ++i) {
                getLight(i).setSmoothBrightness(i == attenuator_target ? .6f : 0.f, 90);
            }
        }
        last_attenuator_target = attenuator_target;
    }

}

Model *modelMacro = createModel<MacroModule, MacroUi>("chem-macro");

