// Copyright (C) Paul Chase Dempsey
#pragma once
#include "../plugin.hpp"
#include "../services/svgtheme.hpp"
#include "TipWidget.hpp"
using namespace svg_theme;

namespace pachde {

// send a change notification to widget
void notifyChange(Widget* widget);

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
    NVGcolor ring = nvgRGBA(0, 0, 0, 0);

    ThemeColorPort()
    {
        this->shadow->hide();
    }

    void ringColor(const NVGcolor& color) { ring = color; }

    void draw(const DrawArgs& args) override;

    // IApplyTheme
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        setSvg(theme_engine.loadSvg(asset::plugin(pluginInstance, "res/widgets/ColorPort.svg"), theme));
        return true;
    }
};

template <class TPortWidget = ThemeColorPort>
TPortWidget* createThemedColorInput(math::Vec pos, engine::Module* module, int inputId, const NVGcolor& color, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) {
	TPortWidget* o = new TPortWidget();
	o->box.pos = pos;
	o->app::PortWidget::module = module;
	o->app::PortWidget::type = engine::Port::INPUT;
	o->app::PortWidget::portId = inputId;
	o->applyTheme(engine, theme);
    o->ringColor(color);
	return o;
}

template <class TPortWidget = ThemeColorPort>
TPortWidget* createThemedColorOutput(math::Vec pos, engine::Module* module, int outputId, const NVGcolor& color, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) {
	TPortWidget* o = new TPortWidget();
	o->box.pos = pos;
	o->app::PortWidget::module = module;
	o->app::PortWidget::type = engine::Port::OUTPUT;
	o->app::PortWidget::portId = outputId;
	o->applyTheme(engine, theme);
    o->ringColor(color);
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

template<typename TSvg>
struct TButton : SvgButton, IApplyTheme
{
    bool key_ctrl;
    bool key_shift;
    std::function<void(bool, bool)> handler;
    TipHolder * tip_holder;

    TButton() 
    :   key_ctrl(false),
        key_shift(false),
        handler(nullptr),
        tip_holder(nullptr)
    {
        this->shadow->hide();
    }

    virtual ~TButton()
    {
        if (tip_holder) {
            delete tip_holder;
            tip_holder = nullptr;
        }
    }

    void describe(std::string text)
    {
        if (!tip_holder) {
            tip_holder = new TipHolder();
        }
        tip_holder->setText(text);
    }

    void setHandler(std::function<void(bool,bool)> callback)
    {
        handler = callback;
    }

    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        bool refresh = frames.size() > 0; 
        if (refresh) {
            frames.clear();
        }

        addFrame(engine.loadSvg(asset::plugin(pluginInstance, TSvg::up()), theme));
        addFrame(engine.loadSvg(asset::plugin(pluginInstance, TSvg::down()), theme));

        if (refresh) {
    		sw->setSvg(frames[0]);
            if (fb) {
                fb->setDirty();
            }
        }
        return refresh;
    }
    
    void destroyTip() {
        if (tip_holder) { tip_holder->destroyTip(); }
    }
    void createTip() {
        if (tip_holder) { tip_holder->createTip(); }
    }

    void onEnter(const EnterEvent& e) override {
        SvgButton::onEnter(e);
        createTip();
    }
    void onLeave(const LeaveEvent& e) override {
        SvgButton::onLeave(e);
        destroyTip();
    }
    void onDragLeave(const DragLeaveEvent& e) override {
        SvgButton::onDragLeave(e);
        destroyTip();
    }
    void onDragEnd(const DragEndEvent& e) override
    {
        SvgButton::onDragEnd(e);
        destroyTip();
    }

    void onHoverKey(const HoverKeyEvent& e) override
    {
        SvgButton::onHoverKey(e);
        key_ctrl = (e.mods & RACK_MOD_MASK) & RACK_MOD_CTRL;
        key_shift = (e.mods & RACK_MOD_MASK) & GLFW_MOD_SHIFT;
    }

    void onAction(const ActionEvent& e) override
    {
        destroyTip();
        if (handler) {
            handler(key_ctrl, key_shift);
        } else {
            SvgButton::onAction(e);
        }
    }

    virtual void appendContextMenu(ui::Menu* menu) {}

    void createContextMenu() {
        ui::Menu* menu = createMenu();
    	appendContextMenu(menu);
    }
};

template <class TButton>
TButton * createThemedButton(math::Vec pos, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme, const char * tip = nullptr) {
    TButton * o  = new(TButton);
    o->applyTheme(engine, theme);
	o->box.pos = pos;
    if (tip) {
        o->describe(tip);
    }
    return o;
}

struct SmallRoundButtonSvg {
    static std::string up() { return "res/widgets/round-push-up.svg"; }
    static std::string down() { return "res/widgets/round-push-down.svg"; }
};

struct SquareButtonSvg {
    static std::string up() { return "res/widgets/square-push-up.svg"; }
    static std::string down() { return "res/widgets/square-push-down.svg"; }
};

struct LinkButtonSvg {
    static std::string up() { return "res/widgets/link-button-up.svg"; }
    static std::string down() { return "res/widgets/link-button-down.svg"; }
};

// struct SmallSquareButtonSvg {
//     static std::string up() { return "res/widgets/square-push-sm-up.svg"; }
//     static std::string down() { return "res/widgets/square-push-sm-down.svg"; }
// };


using SmallRoundButton = TButton<SmallRoundButtonSvg>;
using SquareButton = TButton<SquareButtonSvg>;
using LinkButton = TButton<LinkButtonSvg>;

struct ThemeKnob : rack::RoundKnob, IApplyTheme
{
    // IApplyTheme
    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        this->bg->setSvg(engine.loadSvg(asset::plugin(pluginInstance, "res/widgets/Knob-bg.svg"), theme));
        setSvg(engine.loadSvg(asset::plugin(pluginInstance, "res/widgets/Knob.svg"), theme));
        return true;
    }
};

}