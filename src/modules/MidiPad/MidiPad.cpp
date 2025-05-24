#include "MidiPad.hpp"
using namespace pachde;
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/rack-help.hpp"

MidiPadModule::MidiPadModule()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configInput( 0, "A1 trigger"); configInput( 1, "A2 trigger"); configInput( 2, "A3 trigger"); configInput( 3, "A4 trigger");
    configInput( 4, "B1 trigger"); configInput( 5, "B2 trigger"); configInput( 6, "B3 trigger"); configInput( 7, "B4 trigger");
    configInput( 8, "C1 trigger"); configInput( 9, "C2 trigger"); configInput(10, "C3 trigger"); configInput(11, "C4 trigger");
    configInput(12, "D1 trigger"); configInput(13, "D2 trigger"); configInput(14, "D3 trigger"); configInput(15, "D4 trigger");
}

void MidiPadModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    device_claim = get_json_string(root, "haken-device");

    auto jar = json_object_get(root, "pads");
    if (jar) {
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto pad = std::make_shared<MidiPad>(jp);
            pad_defs[pad->id] = pad;
        }
    }

    ModuleBroker::get()->try_bind_client(this);
}

json_t* MidiPadModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));

    auto jar = json_array();
    for (int i = 0; i < 16; ++i) {
        auto pad = pad_defs[i];
        if (pad) {
            json_array_append_new(jar, pad->to_json());
        }
    }
    json_object_set_new(root, "pads", jar);

    return root;
}

PackedColor MidiPadModule::get_pad_color(int id)
{
    assert(in_range(id, 0, 15));
    auto pad = pad_defs[id];
    return pad ? pad->color: DEFAULT_PAD_COLOR;
}

void MidiPadModule::set_pad_color(int id, PackedColor color)
{
    assert(in_range(id, 0, 15));
    auto pad = pad_defs[id];
    if (pad) pad->color = color;
}

std::shared_ptr<MidiPad> MidiPadModule::first_pad()
{
    for (int i = 0; i < 16; ++i) {
        if (pad_defs[i]) return pad_defs[i];
    }
    return nullptr;
}

void MidiPadModule::ensure_pad(int id)
{
    if (!pad_defs[id]) {
        pad_defs[id] = std::make_shared<MidiPad>(id);
    }
}

// IChemClient
::rack::engine::Module* MidiPadModule::client_module() { return this; }
std::string MidiPadModule::client_claim() { return device_claim; }

void MidiPadModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl(this, host);
}

void MidiPadModule::onPresetChange()
{
//    auto em = chem_host->host_matrix();

    //if (chem_ui) ui()->onPresetChange();
}

void MidiPadModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void MidiPadModule::process_params(const ProcessArgs& args)
{
}

void MidiPadModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);
    if (!chem_host || chem_host->host_busy()) return;

    if (((args.frame + id) % 41) == 0) {
        process_params(args);
    }

    if (editing) {
        if (ticker.stopped()) {
            getLight(L_EDITING).setSmoothBrightness(1, 80);
            ticker.start(1.f);
        } else {
            if (ticker.lap()) {
                auto b = getLight(L_EDITING).getBrightness();
                getLight(L_EDITING).setSmoothBrightness(b > .8f ? 0.f : 1.f, 80);
            }
        }
    }

    if (0 == ((args.frame + id) % 47)) {
        //modulation.update_mod_lights();
        if (!editing && !ticker.stopped()) {
            getLight(L_EDITING).setSmoothBrightness(0, 80);
            ticker.stop();
        }
    }

    if (!editing) {
        // process inputs
    }
}

Model *modelMidiPad = createModel<MidiPadModule, MidiPadUi>("chem-midi-pad");

