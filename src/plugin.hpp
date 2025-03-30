#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/svgtheme.hpp"
#include "services/svt_rack.hpp"
#include "widgets/PanelBorder.hpp"

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file

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
extern Model* modelProto;

// Theming

extern svg_theme::SvgThemeEngine theme_engine;
bool initThemeEngine();
bool reloadThemes();

bool isPeerModule(Module* me, Module* candidate);
