// Copyright (C) Paul Chase Dempsey
#pragma once
#include "my-plugin.hpp"
#include "services/svg-theme.hpp"
#include "services/colors.hpp"
#include "tip-widget.hpp"
using namespace svg_theme;

namespace widgetry {

template<typename TSvg>
struct TButton : SvgButton
{
    using Base = SvgButton;

    bool key_ctrl;
    bool key_shift;
    bool sticky;
    bool latched;
    std::function<void(bool, bool)> handler;
    TipHolder * tip_holder;

    TButton()
    :   key_ctrl(false),
        key_shift(false),
        sticky(false),
        latched(false),
        handler(nullptr),
        tip_holder(nullptr)
    {
        this->shadow->hide();
    }

    virtual ~TButton()     {
        if (tip_holder) {
            delete tip_holder;
            tip_holder = nullptr;
        }
    }

    void describe(std::string text)     {
        if (!tip_holder) {
            tip_holder = new TipHolder();
        }
        tip_holder->setText(text);
    }

    void setHandler(std::function<void(bool,bool)> callback) {
        handler = callback;
    }

    void set_sticky(bool is_sticky) {
        sticky = is_sticky;
    }

    void loadSvg(ILoadSvg* loader) {
        frames.clear();
        addFrame(loader->loadSvg(asset::plugin(pluginInstance, TSvg::up())));
        addFrame(loader->loadSvg(asset::plugin(pluginInstance, TSvg::down())));
        fb->dirty = true;
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
        destroyTip();
        Base::onLeave(e);
    }

    void onDragStart(const DragStartEvent& e) override
    {
        destroyTip();
        if (!sticky) Base::onDragStart(e);
    }

    void onDragLeave(const DragLeaveEvent& e) override {
        destroyTip();
        if (!sticky) Base::onDragLeave(e);
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        if (!sticky) Base::onDragEnd(e);
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
        e.consume(this) ;
        if (handler) {
            handler(key_ctrl, key_shift);
        }
        if (sticky) {
            latched = !latched;
            sw->setSvg(frames[latched ? 1 : 0]);
			fb->setDirty();
        } else {
            Base::onAction(e);
        }
    }

    void sync_frame()
    {
        if (sticky){
            sw->setSvg(frames[latched ? 1 : 0]);
            fb->setDirty();
        }
    }

    void appendContextMenu(ui::Menu* menu) {}

    void createContextMenu() {
        ui::Menu* menu = createMenu();
    	appendContextMenu(menu);
    }

};

template <typename TButton>
TButton * createThemedButton(math::Vec pos, ILoadSvg* loader, const char * tip = nullptr) {
    TButton * o  = new(TButton);
    o->loadSvg(loader);
	o->box.pos = pos;
    if (tip) {
        o->describe(tip);
    }
    return o;
}

template <typename TButton, typename TLight>
TButton * createThemedLightButton(math::Vec pos, ILoadSvg* loader, engine::Module* module, int lightId, const char * tip = nullptr) {
    TButton * o  = new(TButton);
    o->loadSvg(loader);
	o->box.pos = pos;

    if (tip) {
        o->describe(tip);
    }

    auto light = createLight<TLight>(Vec(0,0), module, lightId);
    light->box.pos = o->box.size.div(2).minus(light->box.size.div(2));
    o->addChildBottom(light);
    return o;
}

template<typename TSvg>
struct TParamButton : SvgSwitch
{
    using Base = SvgSwitch;

    TParamButton() {
        this->shadow->hide();
    }

    void loadSvg(ILoadSvg* loader) {
        frames.clear();
        addFrame(loader->loadSvg(asset::plugin(pluginInstance, TSvg::up())));
        addFrame(loader->loadSvg(asset::plugin(pluginInstance, TSvg::down())));
        sw->setSvg(frames[0]);
        fb->dirty = true;
    }
};

template <typename TPButton>
TPButton * createThemedParamButton(math::Vec pos, ILoadSvg* loader, rack::engine::Module*module, int paramId) {
    TPButton * o  = createParam<TPButton>(pos, module, paramId);
    o->loadSvg(loader);
    return o;
}

template <typename TPButton, typename TLight>
TPButton * createThemedParamLightButton(math::Vec pos, ILoadSvg* loader, rack::engine::Module* module, int paramId, int lightId) {
    TPButton * o = createThemedParamButton<TPButton>(pos, loader, module, paramId);

    auto light = createLight<TLight>(Vec(0,0), module, lightId);
    light->box.pos = o->box.size.div(2).minus(light->box.size.div(2));
    light->box.pos.y += .75f; // compensate for down button surface being slightly offset, making a centered (wrt bounding box) LED look off-center
    o->addChildBottom(light);
    return o;
}

struct SmallRoundButtonSvg {
    static std::string up() { return "res/widgets/round-push-up.svg"; }
    static std::string down() { return "res/widgets/round-push-down.svg"; }
};

struct LargeRoundButtonSvg {
    static std::string up() { return "res/widgets/round-push-lg-up.svg"; }
    static std::string down() { return "res/widgets/round-push-lg-down.svg"; }
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

struct DotButtonSvg {
    static std::string up() { return "res/widgets/dot-button.svg"; }
    static std::string down() { return "res/widgets/dot-button-down.svg"; }
};

struct SurfaceDirectionButtonSvg {
    static std::string up() { return "res/widgets/surf-dir-up.svg"; }
    static std::string down() { return "res/widgets/surf-dir-down.svg"; }
};

struct CheckButtonSvg {
    static std::string up() { return "res/widgets/check-button-up.svg"; }
    static std::string down() { return "res/widgets/check-button-down.svg"; }
};

struct ChicletButtonSvg {
    static std::string up() { return "res/widgets/chiclet-up.svg"; }
    static std::string down() { return "res/widgets/chiclet-down.svg"; }
};

struct Palette1ButtonSvg {
    static std::string up() { return "res/widgets/palette-1.svg"; }
    static std::string down() { return "res/widgets/palette-down.svg"; }
};
struct Palette2ButtonSvg {
    static std::string up() { return "res/widgets/palette-2.svg"; }
    static std::string down() { return "res/widgets/palette-down.svg"; }
};
struct Palette3ButtonSvg {
    static std::string up() { return "res/widgets/palette-3.svg"; }
    static std::string down() { return "res/widgets/palette-down.svg"; }
};
struct ECircleSvg {
    static std::string up() { return "res/widgets/edit-circle-up.svg"; }
    static std::string down() { return "res/widgets/edit-circle-down.svg"; }
};

using SmallRoundButton = TButton<SmallRoundButtonSvg>;
using LargeRoundButton = TButton<LargeRoundButtonSvg>;
using SquareButton = TButton<SquareButtonSvg>;
using LinkButton = TButton<LinkButtonSvg>;
using HeartButton = TButton<HeartButtonSvg>;
using CheckButton = TButton<CheckButtonSvg>;
using DotButton = TButton<DotButtonSvg>;
using Palette1Button = TButton<Palette1ButtonSvg>;
using Palette2Button = TButton<Palette2ButtonSvg>;
using Palette3Button = TButton<Palette3ButtonSvg>;
using ChicletButton = TButton<ChicletButtonSvg>;
using EditButton = TButton<ECircleSvg>;

using SmallRoundParamButton = TParamButton<SmallRoundButtonSvg>;
using LargeRoundParamButton = TParamButton<LargeRoundButtonSvg>;
using DotParamButton = TParamButton<DotButtonSvg>;
using SurfaceDirectionParamButton = TParamButton<SurfaceDirectionButtonSvg>;
using CheckParamButton = TParamButton<CheckButtonSvg>;
using ChicletParamButton = TParamButton<ChicletButtonSvg>;

}