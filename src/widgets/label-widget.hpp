// Copyright (C) Paul Chase Dempsey
#pragma once
#include "../services/text.hpp"
#include "../services/svgtheme.hpp"
using namespace svg_theme;

// $TODO: labels need to be cleaned up:
// - reduce the number of label types
// - use a text style struct for alignment, height, bold, color, theme key?

namespace pachde {

enum class TextAlignment { Left, Center, Right };
struct LabelStyle
{
    const char* key;
    TextAlignment align;
    float height;
    bool bold;

    LabelStyle()
    :   key("label"),
        align(TextAlignment::Left),
        height(12.f),
        bold(false)
    {}
    LabelStyle(const char * key, TextAlignment align = TextAlignment::Left, float height = 12.f, bool bold = false)
    :   key(key),
        align(align),
        height(height),
        bold(bold)
    {}
    LabelStyle& operator=(const LabelStyle&other) {
        key = other.key;
        align = other.align;
        height = other.height;
        bold = other.bold;
        return *this;
    }
    LabelStyle(const LabelStyle& other)
    :   key(other.key),
        align(other.align),
        height(other.height),
        bold(other.bold)
    {}

    bool left() { return align == TextAlignment::Left; }
    bool right() { return align == TextAlignment::Right; }
    bool center() { return align == TextAlignment::Center; }
};

struct BasicTextLabel: Widget, IApplyTheme
{
    std::string _text;
    NVGcolor _color;
    LabelStyle _style;

    BasicTextLabel()
    :   _color(RampGray(G_90))
    {
    }
    std::string getText() { return _text; }
    bool left() { return _style.left(); }
    bool right() { return _style.right(); }
    bool centered() { return _style.center(); }
    float text_height() { return _style.height; }
    
    void setPos(Vec pos) { box.pos = pos; }
    void setSize(Vec size) { box.size = size; }
    void text(std::string text) { _text = text; }
    void text_height(float height) { _style.height = height; box.size.y = height; }
    void style(const LabelStyle &other) {
        _style = other;
        box.size.y = _style.height;
    }
    void color(const NVGcolor &new_color) { _color = new_color; }
    void render(const DrawArgs& args)
    {
        if (_text.empty()) return;

        auto vg = args.vg;
        auto font = _style.bold ? GetPluginFontSemiBold() : GetPluginFontRegular();
        if (!FontOk(font)) return;

        nvgSave(vg);
        SetTextStyle(vg, font, _color, _style.height);
        switch (_style.align) {
        case TextAlignment::Left:
            nvgTextAlign(vg, NVG_ALIGN_TOP|NVG_ALIGN_LEFT);
            nvgText(vg, 0.f, 0.f, _text.c_str(), nullptr);
            break;
        case TextAlignment::Center:
            nvgTextAlign(vg, NVG_ALIGN_TOP|NVG_ALIGN_CENTER);
            nvgText(vg, box.size.x * .5, 0.f, _text.c_str(), nullptr);
            break;
        case TextAlignment::Right:
            nvgTextAlign(vg, NVG_ALIGN_TOP|NVG_ALIGN_RIGHT);
            nvgText(vg, box.size.x, 0.f, _text.c_str(), nullptr);
            break;
        }
        nvgRestore(vg);
    }

    // IApplyTheme
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        _color = fromPacked(theme_engine.getFillColor(theme->name, this->_style.key));
        if (!isColorVisible(_color)) {
            _color = RampGray(G_85);
        }
        return true;
    }

    void draw(const DrawArgs& args) override
    {
        Widget::draw(args);
        render(args);
    }
};

struct StaticTextLabel: Widget, IApplyTheme
{
    FramebufferWidget* _fb = nullptr;
    BasicTextLabel* _label = nullptr;

    StaticTextLabel()
    {
        _label = new BasicTextLabel();
        _fb = new widget::FramebufferWidget;
        _fb->addChild(_label);
	    addChild(_fb);
        dirty();
    }
    std::string getText() {
        return _label ? _label->_text : "";
    }
    void setPos(Vec pos) {
        box.pos = pos;
    }
    void setSize(Vec size) {
        box.size = size;
        _fb->box.size = size;
        _label->box.size = size;
    }
    void modified(bool modified = true) {
        _fb->setDirty(modified);
    }
    void dirty() { _fb->setDirty(); }
    void text(const std::string & text) {
        _label->text(text);
        dirty();
    }
    void color(const NVGcolor &new_color) {
        _label->color(new_color);
        dirty();
    }
    void text_height(float height) {
        _label->text_height(height);
        dirty();
    }
    void style(const LabelStyle &other) {
        _label->_style = other;
        box.size.y = other.height;
        dirty();
    }
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        return _label->applyTheme(theme_engine, theme);
    }
    void draw(const DrawArgs& args) override
    {
        Widget::draw(args);
#if defined VISIBLE_STATICTEXTLABEL_BOUNDS
        FillRect(args.vg, _fb->box.pos.x, _fb->box.pos.y, _fb->box.size.x, _fb->box.size.y, Overlay(GetStockColor(StockColor::Yellow), .20f));
#endif
    }
};

struct LazyDynamicLabel : StaticTextLabel
{
    std::function<std::string ()> _getText;
    
    LazyDynamicLabel(){}

    void modified(bool modified = true) {
        if (modified && _getText) {
            text(_getText());
        } else {
            StaticTextLabel::modified(modified);
        }
    }

    void setFetch(std::function<std::string ()> get, bool refreshable = false)
    {
        _getText = get;
    }
};

template<typename TWidget = StaticTextLabel>
TWidget* createStaticTextLabel(
    math::Vec pos,
    float width,
    std::string text,
    SvgThemeEngine& engine, 
    std::shared_ptr<SvgTheme> theme
    )
{
    TWidget* w = createWidget<TWidget>(pos);
    w->text(text);
    w->color(RampGray(G_90));
    if (w->_label->centered()) {
        w->setPos(Vec(w->box.pos.x - width*.5f, w->box.pos.y));
    }
    w->setSize(Vec(width, w->_label->text_height()));
    w->applyTheme(engine, theme);
    return w;
}

template<typename TWidget = StaticTextLabel>
TWidget* createStaticTextLabel(
    math::Vec pos,
    float width,
    std::string text,
    SvgThemeEngine& engine, 
    std::shared_ptr<SvgTheme> theme,
    const LabelStyle& style
    )
{
    TWidget* w = createWidget<TWidget>(pos);
    w->text(text);
    w->_label->style(style);
    w->color(RampGray(G_90));
    if (w->_label->centered()) {
        w->setPos(Vec(w->box.pos.x - width*.5f, w->box.pos.y));
    }
    w->setSize(Vec(width, w->_label->text_height()));
    w->applyTheme(engine, theme);
    return w;
}

}
