// Copyright (C) Paul Chase Dempsey
#pragma once
#include "my-plugin.hpp"
#include "services/svg-theme.hpp"
#include "services/colors.hpp"
#include "tip-widget.hpp"
using namespace svg_theme;
using namespace pachde;

namespace widgetry {

// send a change notification to widget
void notifyChange(Widget* widget);

template <typename TLight>
void applyLightTheme(TLight* light, std::shared_ptr<SvgTheme> theme, const char * theme_key = "led")
{
    if (theme) {
        auto style = theme->getStyle(theme_key);
        if (style) {
            if (style->isApplyFill()) {
                light->bgColor = fromPacked(style->fillWithOpacity());
            }
            if (style->isApplyStroke()) {
                light->borderColor = fromPacked(style->strokeWithOpacity());
            }
        }
    }
}

// A Themed screw, based on the standard Rack screw.
struct ThemeScrew : app::SvgScrew
{
    void loadSvg(ILoadSvg* loader) {
        setSvg(loader->loadSvg(asset::plugin(pluginInstance, "res/widgets/Screw.svg")));
    }

    void apply_theme(std::shared_ptr<SvgTheme> theme) {
        applySvgTheme(sw->svg, theme);
        fb->dirty = true;
    }
};

// A themed Port
struct ThemePort : app::SvgPort
{
    void loadSvg(ILoadSvg* loader) {
        setSvg(loader->loadSvg(asset::plugin(pluginInstance, "res/widgets/Port.svg")));
    }

    void apply_theme(std::shared_ptr<SvgTheme> theme) {
        applySvgTheme(sw->svg, theme);
        fb->dirty = true;
    }
};

struct ThemeColorPort : app::SvgPort,  IThemed
{
    NVGcolor ring;
    const char* key;
    float ring_width;

    ThemeColorPort() :
        ring(nvgRGBA(0, 0, 0, 0)),
        key("cop-xxxx"),
        ring_width(2.f)
    {
        this->shadow->hide();
    }

    void ringColor(const char* key, const NVGcolor& color) { this->key = key; ring = color; }

    void draw(const DrawArgs& args) override;

    void loadSvg(ILoadSvg* loader) {
        setSvg(loader->loadSvg(asset::plugin(pluginInstance, "res/widgets/ColorPort.svg")));
    }

    void applyTheme(std::shared_ptr<SvgTheme> theme) override
    {
        // assume the cached Svgs are updated together
        //applySvgTheme(sw->svg, theme);
        fb->dirty = true;

        auto style = theme->getStyle(key);
        if (style) {
            if (style->isApplyStroke()) {
                ring = fromPacked(style->strokeWithOpacity());
            } else if (style->isApplyFill()) {
                ring = fromPacked(style->fillWithOpacity());
            }
            if (style->isApplyStrokeWidth()) {
                ring_width = style->stroke_width;
            }
        }
    }
};

template <class TPortWidget = ThemeColorPort>
TPortWidget* createThemedColorInput(math::Vec pos, ILoadSvg* loader, engine::Module* module, int inputId, const char * key, const NVGcolor& color) {
	TPortWidget* o = new TPortWidget();
    o->loadSvg(loader);
	o->box.pos = pos;
	o->app::PortWidget::module = module;
	o->app::PortWidget::type = engine::Port::INPUT;
	o->app::PortWidget::portId = inputId;
    o->ringColor(key, color);
	return o;
}

template <class TPortWidget = ThemeColorPort>
TPortWidget* createThemedColorOutput(math::Vec pos, ILoadSvg* loader, engine::Module* module, int outputId, const char*  key, const NVGcolor& color) {
	TPortWidget* o = new TPortWidget();
	o->box.pos = pos;
    o->loadSvg(loader);
	o->app::PortWidget::module = module;
	o->app::PortWidget::type = engine::Port::OUTPUT;
	o->app::PortWidget::portId = outputId;
    o->ringColor(key, color);
	return o;
}

// TODO: any steteful SVG-based widgets need to be sent onChange() to how the correct updates state after a theme change
// Need to recurse widgets do this for any known stateful widgets

// themed vertical 2-value switch
struct ThemeSwitchV2 : app::SvgSwitch
{
    ThemeSwitchV2()
    {
        shadow->opacity = 0.f; // hide the default shadow, like the Rack vertical switches do
    }

    void loadSvg(ILoadSvg* loader) {
        if (frames.size() > 0) {
            frames.clear();
        }
        addFrame(loader->loadSvg(asset::plugin(pluginInstance, "res/widgets/vswitch2-0.svg")));
        addFrame(loader->loadSvg(asset::plugin(pluginInstance, "res/widgets/vswitch2-1.svg")));
        fb->dirty = true;
    }

    // For an SvgSwitch, Rack selects the current presentation (this->sw) from one of
    // a series of backing SVGs ("frames"). A simple invalidation (DirtyEvent) of the widget doesn't force
    // selection of the frame. In order to set the correct frame for the current value of the parameter
    // backing the switch, it is required to send a ChangeEvent and the handler calculates the correct frame for us.
    // If we only dirty, the new theme isn't shown until the value of the switch changes.
};

template <class TWidget>
TWidget* createThemedWidget(::rack::math::Vec pos, ILoadSvg* loader) {
	TWidget* o = new TWidget();
	o->loadSvg(loader);
	o->box.pos = pos;
	return o;
}

template <class TPanel = ::rack::app::SvgPanel>
TPanel* createThemedPanel(std::string svgPath, ILoadSvg* loader) {
	TPanel* panel = new TPanel;
	panel->setBackground(loader->loadSvg(svgPath));
	return panel;
}

template <class TParamWidget>
TParamWidget* createThemedParam(::rack::math::Vec pos, ILoadSvg* loader, ::rack::engine::Module* module, int paramId) {
	TParamWidget* o = new TParamWidget();
    o->loadSvg(loader);
	o->app::ParamWidget::module = module;
	o->app::ParamWidget::paramId = paramId;
	o->initParamQuantity();
	o->box.pos = pos;
	return o;
}

template <class TPortWidget>
TPortWidget* createThemedInput(::rack::math::Vec pos, ILoadSvg* loader, ::rack::engine::Module* module, int inputId) {
	TPortWidget* o = new TPortWidget();
    o->loadSvg(loader);
	o->app::PortWidget::module = module;
	o->app::PortWidget::type = ::rack::engine::Port::INPUT;
	o->app::PortWidget::portId = inputId;
	o->box.pos = pos;
	return o;
}


template <class TPortWidget>
TPortWidget* createThemedOutput(::rack::math::Vec pos, ILoadSvg* loader, ::rack::engine::Module* module, int outputId) {
	TPortWidget* o = new TPortWidget();
    o->loadSvg(loader);
	o->app::PortWidget::module = module;
	o->app::PortWidget::type = ::rack::engine::Port::OUTPUT;
	o->app::PortWidget::portId = outputId;
	o->box.pos = pos;
	return o;
}


}