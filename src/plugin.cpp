#include "plugin.hpp"

Plugin *pluginInstance(nullptr);

void init(Plugin *p)
{
	pluginInstance = p;
	p->addModel(modelCore);
	p->addModel(modelPlay);
	p->addModel(modelMacro);
	p->addModel(modelPre);
	p->addModel(modelFx);
	p->addModel(modelPost);
	p->addModel(modelConvo);
	p->addModel(modelJack);
	p->addModel(modelSustain);
	p->addModel(modelSostenuto);
	p->addModel(modelSostenuto2);
	p->addModel(modelSettings);
	p->addModel(modelPreset);
	p->addModel(modelOverlay);
	p->addModel(modelXM);
	p->addModel(modelXMEdit);
	//p->addModel(modelProto);
}

bool isChemModule(Module* candidate)
{
    if (!candidate) return false;
    auto model =  candidate->model;
    return ((model == modelCore)
        || (model == modelPlay)
        || (model == modelMacro)
        || (model == modelPre)
        || (model == modelFx)
        || (model == modelPost)
        || (model == modelConvo)
        || (model == modelJack)
        || (model == modelSustain)
        || (model == modelSostenuto)
        || (model == modelSostenuto2)
        || (model == modelSettings)
        || (model == modelPreset)
        || (model == modelOverlay)
        //|| (model == modelProto)
        // add new models here
	);
}

bool isPeerModule(Module* me, Module* candidate)
{
    if (!candidate) return false;
	if (me == candidate) return false;
	return isChemModule(candidate);
}

// Theming

// The svg_theme engine is shared by all modules in the plugin
svg_theme::SvgThemeEngine theme_engine;

// initThemeEngine must be called to load the themes.
//
// $CONSIDER: lazy-load theme json as required.
//
bool initThemeEngine()
{
	if (theme_engine.isLoaded()) return true;

	// For demo and authoring purposes, we log to the Rack log.
	//
	// In a production VCV Rack module in the library, logging to Rack's log is disallowed.
	// The logging is necessary only when authoring your theme and SVG.
	// Once your theme is correctly applying to the SVG, you do not need this logging
	// because it's useless to anyone other than someone modifying the SVG or theme.
	//
	theme_engine.setLog([](svg_theme::Severity severity, svg_theme::ErrorCode code, std::string info)->void {
		if (severity >= svg_theme::Severity::Error) {
			DEBUG("Theme %s (%d): %s", SeverityName(severity), code, info.c_str());
		}
	});

	// load colors first, so that themes can refer to colors
 	bool ok = theme_engine.load(asset::plugin(pluginInstance, "res/themes/colors.json"));
	if (!ok) return false;

	auto entries = ::rack::system::getEntries(asset::plugin(pluginInstance, "res/themes"));
	for (std::string file : entries) {
		auto stem = system::getStem(file);
		if (stem == "colors") continue;

		ok = theme_engine.load(file);
		if (!ok) break;
	} 
	return ok;
}

bool reloadThemes()
{
	theme_engine.clear();
	return initThemeEngine();
}
