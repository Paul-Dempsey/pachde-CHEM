#include "Core.hpp"

Model *modelCore = createModel<CoreModule, CoreModuleWidget>("chem-core");

CoreModule::CoreModule()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configOutput(Outputs::OUT_READY, "Ready");
}

// void CoreModule::dataFromJson(json_t* root) {
//     ChemModule::dataFromJson(root);
// }

// json_t* CoreModule::dataToJson() {
//     json_t* root = ChemModule::dataToJson();
//     return root;
// }

const uint64_t PROCESS_LIGHT_INTERVAL = 120;

void CoreModule::processLights(const ProcessArgs &args)
{
    getLight(L_READY).setBrightnessSmooth(this->ready ? 1.0f : 0.f, args.sampleTime * 20);
}

void CoreModule::process(const ProcessArgs &args)
{
    if (0 == ((args.frame + id) % PROCESS_LIGHT_INTERVAL)) {
        processLights(args);
    }

    if (getOutput(OUT_READY).isConnected()) {
        getOutput(OUT_READY).setVoltage(ready ? 10.0f : 0.0f);
    }
}

