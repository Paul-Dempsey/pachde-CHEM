// Copyright (C) Paul Chase Dempsey
#pragma once
#include "../plugin.hpp"
#include "../services/svgtheme.hpp"
#include "../services/colors.hpp"
#include "TipWidget.hpp"
using namespace svg_theme;

namespace pachde {

// send a change notification to widget
void notifyChange(Widget* widget);

template <typename TLight>
void applyLightTheme( TLight* light, const std::string& theme_name, const std::string& theme_key = "led")
{
    auto theme = theme_engine.getTheme(theme_name);
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
struct ThemeScrew : app::SvgScrew, IApplyTheme
{
    // IApplyTheme
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        setSvg(theme_engine.loadSvg(asset::plugin(pluginInstance, "res/widgets/Screw.svg"), theme));
        return true;
    }
};

// A themed Port
struct ThemePort : app::SvgPort, IApplyTheme
{
    // IApplyTheme
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        setSvg(theme_engine.loadSvg(asset::plugin(pluginInstance, "res/widgets/Port.svg"), theme));
        return true;
    }
};

struct ThemeColorPort : app::SvgPort,  IApplyTheme
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

    // IApplyTheme
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        setSvg(theme_engine.loadSvg(asset::plugin(pluginInstance, "res/widgets/ColorPort.svg"), theme));
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
        return true;
    }
};

template <class TPortWidget = ThemeColorPort>
TPortWidget* createThemedColorInput(math::Vec pos, engine::Module* module, int inputId, const char * key, const NVGcolor& color, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) {
	TPortWidget* o = new TPortWidget();
	o->box.pos = pos;
	o->app::PortWidget::module = module;
	o->app::PortWidget::type = engine::Port::INPUT;
	o->app::PortWidget::portId = inputId;
	o->applyTheme(engine, theme);
    o->ringColor(key, color);
	return o;
}

template <class TPortWidget = ThemeColorPort>
TPortWidget* createThemedColorOutput(math::Vec pos, engine::Module* module, int outputId, const char*  key, const NVGcolor& color, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) {
	TPortWidget* o = new TPortWidget();
	o->box.pos = pos;
	o->app::PortWidget::module = module;
	o->app::PortWidget::type = engine::Port::OUTPUT;
	o->app::PortWidget::portId = outputId;
	o->applyTheme(engine, theme);
    o->ringColor(key, color);
	return o;
}


// themed vertical 2-value switch
struct ThemeSwitchV2 : app::SvgSwitch, IApplyTheme
{
    ThemeSwitchV2()
    {
        shadow->opacity = 0.f; // hide the default shadow, like the Rack vertical switches do
    }

    // IApplyTheme
    //
    // For an SvgSwitch, Rack selects the current presentation (this->sw) from one of
    // a series of backing SVGs ("frames"). A simple invalidation (DirtyEvent) of the widget doesn't force 
    // selection of the frame. In order to set the correct frame for the current value of the parameter
    // backing the switch, we send a ChangeEvent and the handler calculates the correct frame for us.
    // If we only dirty, the new theme isn't shown until the value of the switch changes.
    //
    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        // Check if we're refreshing the widget with a new theme (and thus need to send a change event),
        // or being initialized by the creation helper.
        bool refresh = frames.size() > 0; 
        if (refresh) {
            frames.clear();
        }

        addFrame(engine.loadSvg(asset::plugin(pluginInstance, "res/widgets/vswitch2-0.svg"), theme));
        addFrame(engine.loadSvg(asset::plugin(pluginInstance, "res/widgets/vswitch2-1.svg"), theme));

        if (refresh) {
            // send change event to ensure the switch ui is set to the correct frame
            notifyChange(this);
        }
        return refresh;
    }
};

}