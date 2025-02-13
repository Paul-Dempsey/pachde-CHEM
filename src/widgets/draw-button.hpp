#pragma once
#include <rack.hpp>
#include "../services/colors.hpp"
#include "../services/svt_rack.hpp"
#include "TipWidget.hpp"
using namespace ::rack;

namespace pachde {

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

struct DrawButtonBase: OpaqueWidget, IApplyTheme
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
        addColor("ctl-hovered", ColorKind::Fill, GetStockColor(StockColor::Coral));
        addColor("ctl-disabled", ColorKind::Stroke, RampGray(Ramp::G_50));
    }

    virtual ~DrawButtonBase()
    {
        if (tip_holder) {
            delete tip_holder;
            tip_holder = nullptr;
        }
    }

    void enable(bool enabled = true) {
        this->enabled = enabled;
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

    void onDragEnd(const DragEndEvent& e) override
    {
        Base::onDragEnd(e);
        destroyTip();
        button_down = false;
    }

    void onHoverKey(const HoverKeyEvent& e) override
    {
        Base::onHoverKey(e);
        key_ctrl = (e.mods & RACK_MOD_MASK) & RACK_MOD_CTRL;
        key_shift = (e.mods & RACK_MOD_MASK) & GLFW_MOD_SHIFT;
    }

    void onButton(const ButtonEvent& e) override {
        Base::onButton(e);

        if (!enabled) return;
        // Dispatch ActionEvent on left click
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            ActionEvent eAction;
            onAction(eAction);
        }
    }

    void onAction(const ActionEvent& e) override
    {
        destroyTip();
        if (enabled && handler) {
            handler(key_ctrl, key_shift);
        } else {
            Base::onAction(e);
        }
    }

    void addColor(const char * theme_key, ColorKind kind, NVGcolor default_color)
    {
        color_styles.push_back(ColorStyle{theme_key, default_color, kind});
    }

    NVGcolor get_color(const char * name)
    {
        for (auto style : color_styles) {
            if (0 == strcmp(style.key, name)) {
                return style.color;
            }
        }
        return no_light;
    }

    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override
    {
        if (color_styles.empty()) return false;
        for (ColorStyle& style : color_styles) {
            PackedColor pcolor = NoColor;
            switch (style.kind) {
            case ColorKind::Fill: pcolor = theme->getFillColor(style.key, true); break;
            case ColorKind::Stroke: pcolor = theme->getStrokeColor(style.key, true); break;
            default:
                assert(false);
                break;
            }
            style.color = isVisibleColor(pcolor) ? fromPacked(pcolor) : style.default_color;
        }
        return true;
    }

    //virtual void appendContextMenu(ui::Menu* menu) {}

    // void createContextMenu() {
    //     ui::Menu* menu = createMenu();
    // 	appendContextMenu(menu);
    // }
};

struct CtlBase: DrawButtonBase
{
    using Base = DrawButtonBase;

    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override
    {
        return Base::applyTheme(theme_engine, theme);
    }

    NVGcolor glyph_color_for_state(bool enabled, bool down)
    {
        if (!enabled) return color_styles[ColorIndex::Disabled].color;
        if (down) return nvgTransRGBAf(color_styles[ColorIndex::Glyph].color, .5f);
        return color_styles[ColorIndex::Glyph].color;
    }

    CtlBase()
    {
    }
};

struct UpButton: CtlBase
{
    using Base = CtlBase;

    UpButton()
    {
        box.size.x = 15.f;
        box.size.y = 15.f;
    }
    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override
    {
        return Base::applyTheme(theme_engine, theme);
    }

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);
        auto vg = args.vg;

        nvgBeginPath(vg);
        nvgMoveTo(vg, .5f, box.size.y);
        nvgLineTo(vg, .5f, .5f);
        nvgLineTo(vg, box.size.x - 1.f, .5f);
        nvgLineTo(vg, box.size.x - 1.f, box.size.y);

        nvgStrokeColor(vg, color_styles[ColorIndex::Frame].color);
        nvgStrokeWidth(vg, .75f);
        nvgStroke(vg);

        if (hovered && enabled && !button_down) {
            Halo(vg, 7.5f, 7.5f, 0.1f, 9.5f, color_styles[ColorIndex::Hovered].color, HALO_FADE);
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

struct DownButton: CtlBase
{
    using Base = CtlBase;
    DownButton()
    {
        box.size.x = 15.f;
        box.size.y = 15.f;
    }

    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override
    {
        return Base::applyTheme(theme_engine, theme);
    }

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);
        auto vg = args.vg;

        nvgBeginPath(vg);
        nvgMoveTo(vg, .5f, 0.f);
        nvgLineTo(vg, .5f, box.size.y - 1.f);
        nvgLineTo(vg, box.size.x - 1.f, box.size.y - 1.f);
        nvgLineTo(vg, box.size.x - 1.f, 0.f);
        //nvgClosePath(vg);
        nvgStrokeColor(vg, color_styles[ColorIndex::Frame].color);
        nvgStrokeWidth(vg, .75f);
        nvgStroke(vg);

        if (hovered && enabled && !button_down) {
            Halo(vg, 7.5f, 7.5f, 0.1f, 9.5, color_styles[ColorIndex::Hovered].color, HALO_FADE);
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

struct PrevButton: CtlBase
{
    using Base = CtlBase;

    PrevButton()
    {
        box.size.x = 15.f;
        box.size.y = 15.f;
    }

    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override
    {
        return Base::applyTheme(theme_engine, theme);
    }

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);
        auto vg = args.vg;

        BoxRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - .5f, color_styles[ColorIndex::Frame].color, .75f);

        if (hovered && enabled && !button_down) {
            Halo(vg, 7.5f, 7.5f, 0.1f, 9.5f, color_styles[ColorIndex::Hovered].color, HALO_FADE);
        }

        Line(vg, 3.f, 7.5f, 12.f, 7.5f, glyph_color_for_state(enabled, button_down), 1.5f);
    }
};

struct NextButton: CtlBase
{
    using Base = CtlBase;
    NextButton()
    {
        box.size.x = 15.f;
        box.size.y = 15.f;
        //addColor("ctl-glyph-dn", RampGray(Ramp::G_50));
    }

    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override
    {
        return Base::applyTheme(theme_engine, theme);
    }

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);
        auto vg = args.vg;

        BoxRect(vg, .5f, .5f, 14.5f, 14.5f, color_styles[ColorIndex::Frame].color, .75f);
        if (hovered && enabled && !button_down) {
            Halo(vg, 7.5f, 7.5f, 0.1f, 9.5f, color_styles[ColorIndex::Hovered].color, HALO_FADE);
        }

        auto co_glyph = glyph_color_for_state(enabled, button_down);
        Line(vg, 3.f, 7.5f, 12.f, 7.5f, co_glyph, 1.5f);
        Line(vg, 7.5f, 3.f, 7.5f, 12.f, co_glyph, 1.5f);
    }
};

}