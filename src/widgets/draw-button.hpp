#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/colors.hpp"
#include "tip-widget.hpp"
#include "services/svg-theme.hpp"
using namespace svg_theme;
using namespace pachde;

namespace widgetry {

constexpr const float HALO_FADE = .65f;

enum ColorKind : uint8_t {
    Fill, Stroke
};

struct ColorStyle {
    const char * key;
    NVGcolor color;
    NVGcolor default_color;
    ColorKind kind;

    ColorStyle(const char * name, NVGcolor fallback, ColorKind kind)
    : key(name), color(fallback), default_color(fallback), kind(kind)
    {}
};

enum ColorIndex {
    Frame,
    Glyph,
    Hovered,
    Disabled
};

struct DrawButtonBase: OpaqueWidget, IThemed
{
    using Base = OpaqueWidget;

    bool button_down;
    bool enabled;
    bool hovered;
    bool key_ctrl;
    bool key_shift;
    std::function<void(bool, bool)> handler;
    TipHolder * tip_holder;
    std::vector<ColorStyle> color_styles;

    DrawButtonBase()
    :   button_down(false),
        enabled(true),
        hovered(false),
        key_ctrl(false),
        key_shift(false),
        handler(nullptr),
        tip_holder(nullptr)
    {
        addColor("ctl-frame", ColorKind::Stroke, RampGray(Ramp::G_50));
        addColor("ctl-glyph", ColorKind::Stroke, RampGray(Ramp::G_70));
        addColor("ctl-hovered", ColorKind::Fill, GetStockColor(StockColor::pachde_blue_medium));
        addColor("ctl-disabled", ColorKind::Stroke, RampGray(Ramp::G_50));
    }

    virtual ~DrawButtonBase() {
        if (tip_holder) {
            delete tip_holder;
            tip_holder = nullptr;
        }
    }

    void enable(bool enabled = true) {
        this->enabled = enabled;
    }

    void describe(std::string text) {
        if (!tip_holder) {
            tip_holder = new TipHolder();
        }
        tip_holder->setText(text);
    }

    void set_handler(std::function<void(bool,bool)> callback) {
        handler = callback;
    }

    void destroyTip() {
        if (tip_holder) { tip_holder->destroyTip(); }
    }

    void createTip() {
        if (tip_holder) { tip_holder->createTip(); }
    }

    void onHover(const HoverEvent& e) override {
        Base::onHover(e);
        e.consume(this);
    }

    void onEnter(const EnterEvent& e) override {
        Base::onEnter(e);
        createTip();
        hovered = true;
    }

    void onLeave(const LeaveEvent& e) override {
        Base::onLeave(e);
        destroyTip();
        hovered = false;
    }

    void onDragStart(const DragStartEvent& e) override {
        if (e.button != GLFW_MOUSE_BUTTON_LEFT)
            return;
        button_down = true;
    }

    void onDragLeave(const DragLeaveEvent& e) override {
        Base::onDragLeave(e);
        destroyTip();
    }

    void onDragEnd(const DragEndEvent& e) override {
        Base::onDragEnd(e);
        destroyTip();
        button_down = false;
    }

    void onHoverKey(const HoverKeyEvent& e) override {
        Base::onHoverKey(e);
        key_ctrl = e.mods & RACK_MOD_CTRL;
        key_shift = e.mods & GLFW_MOD_SHIFT;
    }

    void onButton(const ButtonEvent& e) override {
        Base::onButton(e);

        if (!enabled) return;
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            key_ctrl = e.mods & RACK_MOD_CTRL;
            key_shift = e.mods & GLFW_MOD_SHIFT;
            ActionEvent eAction;
            onAction(eAction);
        }
    }

    void onAction(const ActionEvent& e) override {
        destroyTip();
        if (enabled && handler) {
            handler(key_ctrl, key_shift);
        } else {
            Base::onAction(e);
        }
    }

    void addColor(const char * theme_key, ColorKind kind, NVGcolor default_color) {
        color_styles.push_back(ColorStyle{theme_key, default_color, kind});
    }

    NVGcolor get_color(const char * name) {
        for (auto style : color_styles) {
            if (0 == strcmp(style.key, name)) {
                return style.color;
            }
        }
        return no_light;
    }

    void applyTheme(std::shared_ptr<svg_theme::SvgTheme> theme) override {
        if (color_styles.empty()) return;
        for (ColorStyle& style : color_styles) {
            PackedColor pcolor = colors::NoColor;
            switch (style.kind) {
            case ColorKind::Fill: theme->getFillColor(pcolor, style.key, true); break;
            case ColorKind::Stroke: theme->getStroke(pcolor, style.key, true, nullptr); break;
            default:
                assert(false);
                break;
            }
            style.color = packed_color::isVisible(pcolor) ? fromPacked(pcolor) : style.default_color;
        }
    }

};

struct DrawButtonCtlBase: DrawButtonBase
{
    using Base = DrawButtonBase;

    NVGcolor glyph_color_for_state(bool enabled, bool down) {
        if (!enabled) return color_styles[ColorIndex::Disabled].color;
        auto co = color_styles[ColorIndex::Glyph].color;
        if (down) return nvgTransRGBAf(co, .5f);
        return co;
    }

};

struct UpButton: DrawButtonCtlBase
{
    using Base = DrawButtonCtlBase;

    UpButton() {
        box.size = Vec{15.f, 15.f};
    }

    void draw(const DrawArgs& args) override {
        Base::draw(args);
        auto vg = args.vg;

        BoxRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - .5f, color_styles[ColorIndex::Frame].color, .75f);

        if (hovered && enabled && !button_down) {
            FillRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - .5f, color_styles[ColorIndex::Hovered].color);
        }

        nvgBeginPath(vg);
        nvgMoveTo(vg, 7.5f, 2.5f);
        nvgLineTo(vg, 2.5f, 11.5f);
        nvgLineTo(vg, 2.5f + 10.f, 11.5f);
        nvgClosePath(vg);

        nvgFillColor(vg, glyph_color_for_state(enabled, button_down));
        nvgFill(vg);
    }
};

struct DownButton: DrawButtonCtlBase
{
    using Base = DrawButtonCtlBase;

    DownButton() {
        box.size.x = 15.f;
        box.size.y = 15.f;
    }

    void draw(const DrawArgs& args) override {
        Base::draw(args);
        auto vg = args.vg;

        BoxRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - .5f, color_styles[ColorIndex::Frame].color, .75f);

        if (hovered && enabled && !button_down) {
            FillRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - .5f, color_styles[ColorIndex::Hovered].color);
        }

        nvgBeginPath(vg);
        nvgMoveTo(vg, 7.5f, 11.5f);
        nvgLineTo(vg, 2.5f, 3.5f);
        nvgLineTo(vg, 2.5f + 10.f, 3.5f);
        nvgClosePath(vg);
        nvgFillColor(vg, glyph_color_for_state(enabled, button_down));
        nvgFill(vg);
    }
};

struct PrevButton: DrawButtonCtlBase
{
    using Base = DrawButtonCtlBase;

    PrevButton() {
        box.size.x = 15.f;
        box.size.y = 15.f;
    }

    void draw(const DrawArgs& args) override {
        Base::draw(args);
        auto vg = args.vg;

        BoxRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - .5f, color_styles[ColorIndex::Frame].color, .75f);

        if (hovered && enabled && !button_down) {
            FillRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - .5f, color_styles[ColorIndex::Hovered].color);
        }

        Line(vg, 3.f, 7.5f, 12.f, 7.5f, glyph_color_for_state(enabled, button_down), 1.5f);
    }
};

struct NextButton: DrawButtonCtlBase
{
    using Base = DrawButtonCtlBase;

    NextButton() {
        box.size.x = 15.f;
        box.size.y = 15.f;
    }

    void draw(const DrawArgs& args) override {
        Base::draw(args);
        auto vg = args.vg;

        BoxRect(vg, .5f, .5f, 14.25f, 14.25f, color_styles[ColorIndex::Frame].color, .75f);
        if (hovered && enabled && !button_down) {
            FillRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - .5f, color_styles[ColorIndex::Hovered].color);
        }

        auto co_glyph = glyph_color_for_state(enabled, button_down);
        Line(vg, 3.f, 7.5f, 12.f, 7.5f, co_glyph, 1.5f);
        Line(vg, 7.5f, 3.f, 7.5f, 12.f, co_glyph, 1.5f);
    }
};

struct FrameButton: DrawButtonCtlBase
{
    using Base = DrawButtonCtlBase;

    FrameButton() {
        box.size.x = 12.f;
        box.size.y = 12.f;
    }

    void draw(const DrawArgs& args) override {
        Base::draw(args);
        auto vg = args.vg;

        if (hovered && enabled && !button_down) {
            FillRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - .5f, color_styles[ColorIndex::Hovered].color);
        }
        FittedBoxRect(vg, 0,0, box.size.x, box.size.y, color_styles[ColorIndex::Frame].color, Fit::Inside, .5f);
    }
};


}