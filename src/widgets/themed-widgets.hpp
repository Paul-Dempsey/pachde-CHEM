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
    using Base = SvgButton;

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
    
    void onHover(const HoverEvent& e) override {
        e.consume(this);
    }

    void destroyTip() {
        if (tip_holder) { tip_holder->destroyTip(); }
    }
    void createTip() {
        if (tip_holder) { tip_holder->createTip(); }
    }

    void onEnter(const EnterEvent& e) override {
        Base::onEnter(e);
        createTip();
    }
    void onLeave(const LeaveEvent& e) override {
        Base::onLeave(e);
        destroyTip();
    }
    void onDragLeave(const DragLeaveEvent& e) override {
        Base::onDragLeave(e);
        destroyTip();
    }
    void onDragEnd(const DragEndEvent& e) override
    {
        Base::onDragEnd(e);
        destroyTip();
    }

    void onHoverKey(const HoverKeyEvent& e) override
    {
        Base::onHoverKey(e);
        key_ctrl = (e.mods & RACK_MOD_MASK) & RACK_MOD_CTRL;
        key_shift = (e.mods & RACK_MOD_MASK) & GLFW_MOD_SHIFT;
    }

    void onAction(const ActionEvent& e) override
    {
        destroyTip();
        if (handler) {
            handler(key_ctrl, key_shift);
        } else {
            Base::onAction(e);
        }
    }

    virtual void appendContextMenu(ui::Menu* menu) {}

    void createContextMenu() {
        ui::Menu* menu = createMenu();
    	appendContextMenu(menu);
    }

};

template <typename TButton>
TButton * createThemedButton(math::Vec pos, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme, const char * tip = nullptr) {
    TButton * o  = new(TButton);
    o->applyTheme(engine, theme);
	o->box.pos = pos;
    if (tip) {
        o->describe(tip);
    }
    return o;
}

template <typename TButton, typename TLight>
TButton * createThemedLightButton(math::Vec pos, engine::Module* module, int lightId, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme, const char * tip = nullptr) {
    TButton * o  = new(TButton);
    o->applyTheme(engine, theme);
	o->box.pos = pos;
    if (tip) {
        o->describe(tip);
    }
    auto light = createLight<TLight>(Vec(0,0), module, lightId);
    light->box.pos = o->box.size.div(2).minus(light->box.size.div(2));
    o->addChildBottom(light);
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

struct HeartButtonSvg {
    static std::string up() { return "res/widgets/heart-button.svg"; }
    static std::string down() { return "res/widgets/heart-button-down.svg"; }
};

using SmallRoundButton = TButton<SmallRoundButtonSvg>;
using SquareButton = TButton<SquareButtonSvg>;
using LinkButton = TButton<LinkButtonSvg>;
using HeartButton = TButton<HeartButtonSvg>;


struct GlowKnob : rack::RoundKnob {
    using Base = rack::RoundKnob;
    bool enabled{true};
    bool bright{false};
    bool wire{false};

    GlowKnob() {
        box.size.x = 32.f;
        box.size.y = 32.f;
        this->shadow->hide();
    }

    // enabling
    void enable(bool on) { enabled = on; }
    void onButton(const ButtonEvent& e) override{
        if (!enabled) {
            e.stopPropagating();
            return;
        }
        Base::onButton(e);
    }
    void onHoverScroll(const HoverScrollEvent& e) override {
        if (enabled) Base::onHoverScroll(e);
        e.consume(this);
    }
    void onChange(const ChangeEvent& e) override {
        if (enabled) Base::onChange(e);
        e.consume(this);
    }

    // "glowing"
    void glowing(bool glow) { bright = glow; }

    void drawLayer(const DrawArgs& args, int layer) override
    {
        if (layer != 1) return;
        if (!enabled) return;
        if (rack::settings::rackBrightness > .95f) return;
        if (bright) fb->draw(args);
    }

    void draw(const DrawArgs& args) override
    {
        if (enabled && bright && rack::settings::rackBrightness < .95f) return;
        if (enabled) {
            Base::draw(args);
        } else {
            if (wire) {
                float cx = box.size.x*.5f;
                float cy = box.size.y*.5f;
                float r = box.size.x*.5f - 1.25f;
                auto co = RampGray(G_40);
                OpenCircle(args.vg, cx, cy, r, co, .75f);
                Line(args.vg, cx, cy - 1.f, cx, cy - r, co, .75f);
            } else {
                Base::draw(args);
                Circle(args.vg, box.size.x*.5f, box.size.y*.5f, box.size.x*.5f - 1.f, nvgTransRGBAf(RampGray(G_55), .6f));
            }
        }
    }
};

template<typename TSvg>
struct TKnob : GlowKnob, IApplyTheme
{
    using Base = GlowKnob;

    // IApplyTheme
    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        wire = (theme->name == "Wire");
        bg->setSvg(engine.loadSvg(asset::plugin(pluginInstance, TSvg::bg()), theme));
        setSvg(engine.loadSvg(asset::plugin(pluginInstance, TSvg::knob()), theme));
        return true;
    }

    void drawLayer(const DrawArgs& args, int layer) override
    {
        if (layer != 1) return;
        if (rack::settings::rackBrightness > .95f) return;

        bool base_bright = bright;
        if (wire) {
            bright = true;
        }

        Base::drawLayer(args, layer);

        if (wire) {
            bright = base_bright;
        }
    }
};

struct GrayKnobSvg {
    static std::string bg() { return "res/widgets/knob-bg.svg"; }
    static std::string knob() { return "res/widgets/knob.svg"; }
};
struct RedKnobSvg {
    static std::string bg() { return "res/widgets/knob-red-bg.svg"; }
    static std::string knob() { return "res/widgets/knob-red.svg"; }
};
struct GreenKnobSvg {
    static std::string bg() { return "res/widgets/knob-green-bg.svg"; }
    static std::string knob() { return "res/widgets/knob-green.svg"; }
};
struct BlueKnobSvg {
    static std::string bg() { return "res/widgets/knob-blue-bg.svg"; }
    static std::string knob() { return "res/widgets/knob-blue.svg"; }
};
struct YellowKnobSvg {
    static std::string bg() { return "res/widgets/knob-yellow-bg.svg"; }
    static std::string knob() { return "res/widgets/knob-yellow.svg"; }
};
struct VioletKnobSvg {
    static std::string bg() { return "res/widgets/knob-violet-bg.svg"; }
    static std::string knob() { return "res/widgets/knob-violet.svg"; }
};
struct TrimPotSvg {
    static std::string bg() { return "res/widgets/trimpot-bg.svg"; }
    static std::string knob() { return "res/widgets/trimpot.svg"; }
};

using BasicKnob = TKnob<GrayKnobSvg>;

using GrayKnob = TKnob<GrayKnobSvg>;
using BlueKnob = TKnob<BlueKnobSvg>;
using RedKnob = TKnob<RedKnobSvg>;
using GreenKnob = TKnob<GreenKnobSvg>;
using YellowKnob = TKnob<YellowKnobSvg>;
using VioletKnob = TKnob<VioletKnobSvg>;

using TrimPot = TKnob<TrimPotSvg>;

template <typename TKnob>
TKnob* createChemKnob(Vec pos, Module * module, int paramId, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
	auto o = createParam<TKnob>(pos, module, paramId);
    o->applyTheme(engine, theme);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
    return o;
}



}