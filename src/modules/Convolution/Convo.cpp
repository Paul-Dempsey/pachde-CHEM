#include "Convo.hpp"
#include "../../services/rack-help.hpp"

using namespace pachde;

WidgetInfo widget_info[]
{
    /*0*/{ "Pre mix",    CM::P_PRE_MIX,    CM::IN_PRE_MIX,    CM::L_PRE_MIX },
    /*1*/{ "Pre index",  CM::P_PRE_INDEX,  CM::IN_PRE_INDEX,  CM:: L_PRE_INDEX },
    /*2*/{ "Post mix",   CM::P_POST_MIX,   CM::IN_POST_MIX,   CM:: L_POST_MIX },
    /*3*/{ "Post index", CM::P_POST_INDEX, CM::IN_POST_INDEX, CM:: L_POST_INDEX },

    /*4*/{ "IR 1 Length", CM::P_1_LENGTH, CM::IN_1_LENGTH, CM::L_1_LENGTH },
    /*5*/{ "IR 1 Tuning", CM::P_1_TUNING, CM::IN_1_TUNING, CM::L_1_TUNING },
    /*6*/{ "IR 1 Width",  CM::P_1_WIDTH,  CM::IN_1_WIDTH,  CM::L_1_WIDTH },
    /*7*/{ "IR 1 Left",   CM::P_1_LEFT,   CM::IN_1_LEFT,   CM::L_1_LEFT },
    /*8*/{ "IR 1 Right",  CM::P_1_RIGHT,  CM::IN_1_RIGHT,  CM::L_1_RIGHT },
    /*9*/{ "Convolution IR 1", CM::P_1_TYPE, -1, -1 },

    /*10*/{ "IR 2 Length", CM::P_2_LENGTH, CM::IN_2_LENGTH, CM::L_2_LENGTH },
    /*11*/{ "IR 2 Tuning", CM::P_2_TUNING, CM::IN_2_TUNING, CM::L_2_TUNING },
    /*12*/{ "IR 2 Width",  CM::P_2_WIDTH,  CM::IN_2_WIDTH,  CM::L_2_WIDTH },
    /*13*/{ "IR 2 Left",   CM::P_2_LEFT,   CM::IN_2_LEFT,   CM::L_2_LEFT },
    /*14*/{ "IR 2 Right",  CM::P_2_RIGHT,  CM::IN_2_RIGHT,  CM::L_2_RIGHT },
    /*15*/{ "Convolution IR 2", CM::P_2_TYPE, -1, -1 },

    /*16*/{ "IR 3 Length", CM::P_3_LENGTH, CM::IN_3_LENGTH, CM::L_3_LENGTH },
    /*17*/{ "IR 3 Tuning", CM::P_3_TUNING, CM::IN_3_TUNING, CM::L_3_TUNING },
    /*18*/{ "IR 3 Width",  CM::P_3_WIDTH,  CM::IN_3_WIDTH,  CM::L_3_WIDTH },
    /*19*/{ "IR 3 Left",   CM::P_3_LEFT,   CM::IN_3_LEFT,   CM::L_3_LEFT },
    /*20*/{ "IR 3 Right",  CM::P_3_RIGHT,  CM::IN_3_RIGHT,  CM::L_3_RIGHT },
    /*21*/{ "Convolution IR 3", CM::P_3_TYPE, -1, -1 },

    /*22*/{ "IR 4 Length", CM::P_4_LENGTH, CM::IN_4_LENGTH, CM::L_4_LENGTH },
    /*23*/{ "IR 4 Tuning", CM::P_4_TUNING, CM::IN_4_TUNING, CM::L_4_TUNING },
    /*24*/{ "IR 4 Width",  CM::P_4_WIDTH,  CM::IN_4_WIDTH,  CM::L_4_WIDTH },
    /*25*/{ "IR 4 Left",   CM::P_4_LEFT,   CM::IN_4_LEFT,   CM::L_4_LEFT },
    /*26*/{ "IR 4 Right",  CM::P_4_RIGHT,  CM::IN_4_RIGHT,  CM::L_4_RIGHT },
    /*27*/{ "Convolution IR 4", CM::P_4_TYPE, -1, -1 },

    /*28*/{ "Extend", CM::P_EXTEND, -1, CM::L_EXTEND },
    /*29*/{ "Modulation amount", CM::P_MOD_AMOUNT, -1, -1 }
};
int info_index[] {
    0, 1, 2, 3, // P_PRE_MIX, P_PRE_INDEX, P_POST_MIX, P_POST_INDEX
    9, 15, 21, 27,  // P_1_TYPE, P_2_TYPE, P_3_TYPE, P_4_TYPE,
    4, 10, 16, 22, // P_1_LENGTH, P_2_LENGTH, P_3_LENGTH, P_4_LENGTH,
    5, 11, 17, 23, // P_1_TUNING, P_2_TUNING, P_3_TUNING, P_4_TUNING,
    6, 12, 18, 24, // P_1_WIDTH,  P_2_WIDTH,  P_3_WIDTH,  P_4_WIDTH,
    7, 13, 19, 25, // P_1_LEFT,   P_2_LEFT,   P_3_LEFT,   P_4_LEFT,
    8, 14, 20, 26, // P_1_RIGHT,  P_2_RIGHT,  P_3_RIGHT,  P_4_RIGHT,
    29, // P_MOD_AMOUNT,
    28, // P_EXTEND,
};
const WidgetInfo& param_info(int param_id) { return widget_info[info_index[param_id]]; }

ConvoModule::ConvoModule() :
    modulation(this, ChemId::Convo),
    init_from_em(false),
    glow_knobs(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    for (int i = P_PRE_MIX; i <= P_POST_INDEX; ++i) {
        dp4(configParam(i, 0.f, 10.f, 0.f, widget_info[i].name));
    }

    std::vector<std::string> types {
        "Waterphone 1", "Waterphone 2", "Autoharp 1", "Autoharp 2",
        "Dark Guitar", "Finger Snap", "Wood", "Bright Metal",
        "Fiber", "Leather", "Damped Metal", "Nylon", "Cloth",
        "Eowave Gong", "LVDL Pyramide", "LVDL Gong", "LVDL Onde",
        "White", "Grey"
    };
    for (int i = P_1_TYPE; i <= P_4_TYPE; ++i) {
        configSwitch(i, 0.f, 18.f, 6.f, param_info(i).name, types);
    }

    WidgetInfo* info = widget_info + 4;
    for (int i = 0; i < 4; ++i) {
        dp4(configParam(info->param, 0.f, 10.f, 10.f, info->name)); ++info;
        dp4(configParam(info->param, 0.f, 10.f,  5.f, info->name)); ++info;
        dp4(configParam(info->param, 0.f, 10.f,  5.f, info->name)); ++info;
        dp4(configParam(info->param, 0.f, 10.f, 10.f, info->name)); ++info;
        dp4(configParam(info->param, 0.f, 10.f, 10.f, info->name)); ++info;
        ++info; // skip type
    }
    dp4(configParam(P_MOD_AMOUNT, -100.f, 100.f, 0.f, param_info(P_MOD_AMOUNT).name, "%"));
    configSwitch(P_EXTEND, 0.f, 1.f, 0.f, param_info(P_EXTEND).name, {"off", "on"});

    for (int i = 0; i < NUM_PARAMS; ++i) {
        auto wi = widget_info[i];
        if (wi.input >= 0) {
            configInput(wi.input, wi.name);
        }
        if (wi.light >= 0 && CM::L_EXTEND != wi.light) {
            std::string name{"Modulation amount on "};
            name.append(wi.name);
            configLight(wi.light, name);
        }
    }

    // unconfigured L_EXTEND supresses the normal tooltip, which we don't want for button lights

    EmccPortConfig cfg[] = {
        EmccPortConfig::stream_poke(P_PRE_MIX, IN_PRE_MIX, L_PRE_MIX, Haken::s_Conv_Poke, Haken::id_c_mix1),
        EmccPortConfig::stream_poke(P_PRE_INDEX, IN_PRE_INDEX, L_PRE_INDEX, Haken::s_Conv_Poke, Haken::id_c_idx1),
        EmccPortConfig::stream_poke(P_POST_MIX, IN_POST_MIX, L_POST_MIX, Haken::s_Conv_Poke, Haken::id_c_mix2),
        EmccPortConfig::stream_poke(P_POST_INDEX, IN_POST_INDEX, L_POST_INDEX, Haken::s_Conv_Poke, Haken::id_c_idx2),
        EmccPortConfig::stream_poke(P_1_TYPE, -1, -1, Haken::s_Conv_Poke, Haken::id_c_dat0),
        EmccPortConfig::stream_poke(P_2_TYPE, -1, -1, Haken::s_Conv_Poke, Haken::id_c_dat1),
        EmccPortConfig::stream_poke(P_3_TYPE, -1, -1, Haken::s_Conv_Poke, Haken::id_c_dat2),
        EmccPortConfig::stream_poke(P_4_TYPE, -1, -1, Haken::s_Conv_Poke, Haken::id_c_dat3),
        EmccPortConfig::stream_poke(P_1_LENGTH, IN_1_LENGTH, L_1_LENGTH, Haken::s_Conv_Poke, Haken::id_c_lth0),
        EmccPortConfig::stream_poke(P_2_LENGTH, IN_2_LENGTH, L_2_LENGTH, Haken::s_Conv_Poke, Haken::id_c_lth1),
        EmccPortConfig::stream_poke(P_3_LENGTH, IN_3_LENGTH, L_3_LENGTH, Haken::s_Conv_Poke, Haken::id_c_lth2),
        EmccPortConfig::stream_poke(P_4_LENGTH, IN_4_LENGTH, L_4_LENGTH, Haken::s_Conv_Poke, Haken::id_c_lth3),
        EmccPortConfig::stream_poke(P_1_TUNING, IN_1_TUNING, L_1_TUNING, Haken::s_Conv_Poke, Haken::id_c_shf0),
        EmccPortConfig::stream_poke(P_2_TUNING, IN_2_TUNING, L_2_TUNING, Haken::s_Conv_Poke, Haken::id_c_shf1),
        EmccPortConfig::stream_poke(P_3_TUNING, IN_3_TUNING, L_3_TUNING, Haken::s_Conv_Poke, Haken::id_c_shf2),
        EmccPortConfig::stream_poke(P_4_TUNING, IN_4_TUNING, L_4_TUNING, Haken::s_Conv_Poke, Haken::id_c_shf3),
        EmccPortConfig::stream_poke(P_1_WIDTH, IN_1_WIDTH, L_1_WIDTH, Haken::s_Conv_Poke, Haken::id_c_wid0),
        EmccPortConfig::stream_poke(P_2_WIDTH, IN_2_WIDTH, L_2_WIDTH, Haken::s_Conv_Poke, Haken::id_c_wid1),
        EmccPortConfig::stream_poke(P_3_WIDTH, IN_3_WIDTH, L_3_WIDTH, Haken::s_Conv_Poke, Haken::id_c_wid2),
        EmccPortConfig::stream_poke(P_4_WIDTH, IN_4_WIDTH, L_4_WIDTH, Haken::s_Conv_Poke, Haken::id_c_wid3),
        EmccPortConfig::stream_poke(P_1_LEFT, IN_1_LEFT, L_1_LEFT, Haken::s_Conv_Poke, Haken::id_c_atL0),
        EmccPortConfig::stream_poke(P_2_LEFT, IN_2_LEFT, L_2_LEFT, Haken::s_Conv_Poke, Haken::id_c_atL1),
        EmccPortConfig::stream_poke(P_3_LEFT, IN_3_LEFT, L_3_LEFT, Haken::s_Conv_Poke, Haken::id_c_atL2),
        EmccPortConfig::stream_poke(P_4_LEFT, IN_4_LEFT, L_4_LEFT, Haken::s_Conv_Poke, Haken::id_c_atL3),
        EmccPortConfig::stream_poke(P_1_RIGHT, IN_1_RIGHT, L_1_RIGHT, Haken::s_Conv_Poke, Haken::id_c_atR0),
        EmccPortConfig::stream_poke(P_2_RIGHT, IN_2_RIGHT, L_2_RIGHT, Haken::s_Conv_Poke, Haken::id_c_atR1),
        EmccPortConfig::stream_poke(P_3_RIGHT, IN_3_RIGHT, L_3_RIGHT, Haken::s_Conv_Poke, Haken::id_c_atR2),
        EmccPortConfig::stream_poke(P_4_RIGHT, IN_4_RIGHT, L_4_RIGHT, Haken::s_Conv_Poke, Haken::id_c_atR3),
        EmccPortConfig::stream_poke(P_EXTEND, -1, -1, Haken::s_Conv_Poke, Haken::id_c_phc)
    };
    modulation.configure(Params::P_MOD_AMOUNT, Params::NUM_MODS, cfg);
}

void ConvoModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_read_string(root, "haken-device", device_claim);
    json_read_bool(root, "glow-knobs", glow_knobs);

    if (!device_claim.empty()) {
        modulation.mod_from_json(root);
    }
    ModuleBroker::get()->try_bind_client(this);
}

json_t* ConvoModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    if (!device_claim.empty()) {
        modulation.mod_to_json(root);
    }
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

void ConvoModule::params_from_internal()
{
    for (auto pit = modulation.ports.begin(); pit != modulation.ports.end(); pit++) {
        pit->set_em_and_param_low(conv.data[pit->cc_id]);
    }
}

void ConvoModule::update_from_em()
{
    if (chem_host && chem_host->host_preset()) {
        auto em = chem_host->host_matrix();
        if (em) {
            memcpy(conv.data, em->conv, 30);
            params_from_internal();
            init_from_em = true;
            return;
        }
    }
    conv.set_default();
    params_from_internal();
    init_from_em = false;
}

void ConvoModule::do_message(PackedMidiMessage message)
{
    if (!chem_host || chem_host->host_busy()) return;
    if (as_u8(ChemId::Convo) == message.bytes.tag) return;
    if (init_from_em) {
        conv.do_message(message);
    }
}

// IChemClient
::rack::engine::Module* ConvoModule::client_module() { return this; }
std::string ConvoModule::client_claim() { return device_claim; }

void ConvoModule::onConnectHost(IChemHost* host)
{
    init_from_em = false;
    onConnectHostModuleImpl(this, host);
    if (!chem_host) {
        conv.set_default();
    }
}

void ConvoModule::onPresetChange()
{
    if (!chem_host || chem_host->host_busy()) return;
    update_from_em();
    //if (chem_ui) ui()->onPresetChange();
}

void ConvoModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (ChemDevice::Haken == device) {
        init_from_em = false;
        onPresetChange();
    }
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

uint8_t get_u7_param(::rack::engine::Module* module, int param_id) {
    return unipolar_rack_to_unipolar_7(module->getParam(param_id).getValue());
}

void ConvoModule::process_params(const ProcessArgs& args)
{
    modulation.pull_mod_amount();
    {
        auto v = getParamInt(getParam(Params::P_EXTEND));
        getLight(Lights::L_EXTEND).setBrightnessSmooth(v, 45.f);
    }
}

void ConvoModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);
    if (!host_connected(chem_host) || chem_host->host_busy()) return;

    if (init_from_em && modulation.sync_params_ready(args)) {
        modulation.sync_send();
    }
    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
        modulation.update_mod_lights();
    }
}

Model *modelConvo = createModel<ConvoModule, ConvoUi>("chem-convo");

