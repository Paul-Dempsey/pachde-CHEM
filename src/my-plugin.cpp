#include "my-plugin.hpp"
#include "services/svg-theme-load.hpp"

::rack::plugin::Plugin *pluginInstance(nullptr);

void init(::rack::plugin::Plugin *p)
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
	p->addModel(modelMidiPad);
}

bool isChemModule(Module* candidate)
{
    if (!candidate) return false;
    auto model =  candidate->model;
    return ((model == modelCore)
        || (model == modelPreset)
        || (model == modelMacro)
        || (model == modelPre)
        || (model == modelFx)
        || (model == modelPost)
        || (model == modelPlay)
        || (model == modelConvo)
        || (model == modelJack)
        || (model == modelSustain)
        || (model == modelSostenuto)
        || (model == modelSostenuto2)
        || (model == modelSettings)
        || (model == modelOverlay)
		|| (model == modelXM)
		|| (model == modelMidiPad)
	);
}

bool isPeerModule(Module* me, Module* candidate)
{
    if (!candidate) return false;
	if (me == candidate) return false;
	if (modelXM == candidate->model) return false;
	return isChemModule(candidate);
}

// Theming
using namespace svg_theme;

RackSvgCache rack_svg_cache;
RackSvgCache * getRackSvgs() { return &rack_svg_cache;}

SvgNoCache no_svg_cache;
SvgNoCache * getSvgNoCache() { return &no_svg_cache; }

ThemeCache theme_cache;

ThemeCache& getThemeCache() {
    initThemeCache();
    return theme_cache;
}

void reloadThemeCache() {
    theme_cache.clear();
    initThemeCache();
}

static const char * theme_files[] = {
        "res/themes/Light.vgt",
        "res/themes/Dark.vgt",
        "res/themes/High.vgt",
        "res/themes/Gray.vgt",
        "res/themes/Ice.vgt",
        "res/themes/Katy.vgt",
        "res/themes/Mellow.vgt",
        "res/themes/Wire.vgt"
    };
void initThemeCache() {
    if (!theme_cache.themes.empty()) return;
#ifdef DEV_BUILD
            ErrorContext err;
            ErrorContext* error_context = &err;
#else
            ErrorContext* error_context = nullptr;
#endif
    for (size_t i = 0; i < sizeof(theme_files)/sizeof(theme_files[0]); i++) {
#ifdef DEV_BUILD
        DEBUG("Loading %s", theme_files[i]);
#endif
        auto theme = loadSvgThemeFile(asset::plugin(pluginInstance, theme_files[i]), error_context);
#ifdef DEV_BUILD
        if (!theme) {
            auto report = err.makeErrorReport();
            WARN("%s", report.c_str());
            err.clear();
        }
#endif
        if (theme) theme_cache.addTheme(theme);
    }
    //theme_cache.sort();
}
