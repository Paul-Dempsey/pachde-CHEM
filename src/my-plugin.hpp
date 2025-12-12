#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/svg-theme.hpp"

extern ::rack::plugin::Plugin* pluginInstance;

extern Model* modelCore;
extern Model* modelPlay;
extern Model* modelMacro;
extern Model* modelPre;
extern Model* modelFx;
extern Model* modelPost;
extern Model* modelConvo;
extern Model* modelJack;
extern Model* modelSustain;
extern Model* modelSostenuto;
extern Model* modelSostenuto2;
extern Model* modelSettings;
extern Model* modelPreset;
extern Model* modelPreset;
extern Model* modelOverlay;
extern Model* modelXM;
extern Model* modelMidiPad;

// Theming

void initThemeCache();
void reloadThemeCache();
::svg_theme::ThemeCache& getThemeCache();
::svg_theme::RackSvgCache* getRackSvgs();
::svg_theme::SvgNoCache* getSvgNoCache();

bool isChemModule(Module* candidate);
bool isPeerModule(Module* me, Module* candidate);

// <rack-user>/<plugin>/<asset>
inline std::string user_plugin_asset(const std::string& asset) {
    return system::join(asset::user(pluginInstance->slug.c_str()), asset);
}