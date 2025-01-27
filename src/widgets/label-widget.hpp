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

struct BasicTextLabel: Widget, IApplyTheme
{
    std::string _text;
    NVGcolor _color;
    TextAlignment _align;
    float _text_height;
    bool _bold;
    const char * _text_key;

    BasicTextLabel()
    :   _color(RampGray(G_90)),
        _align(TextAlignment::Left),
        _text_height(12.f),
        _bold(true),
        _text_key("ctl-label")
    {
    }
    std::string getText() { return _text; }
    void setPos(Vec pos) { box.pos = pos; }
    void setSize(Vec size) { box.size = size; }
    void text(std::string text) { _text = text; }
    void text_height(float height) { _text_height = height; box.size.y = height; }
    void style(float height, bool bold = true, TextAlignment alignment = TextAlignment::Left)
    {
        _text_height = height;
        _bold = bold;
        _align = alignment;
    }
    void color(const NVGcolor &new_color) { _color = new_color; }
    void render(const DrawArgs& args)
    {
        if (_text.empty()) return;

        auto vg = args.vg;
        auto font = _bold ? GetPluginFontSemiBold() : GetPluginFontRegular();
        if (!FontOk(font)) return;

        nvgSave(vg);
        SetTextStyle(vg, font, _color, _text_height);
        switch (_align) {
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
        _color = fromPacked(theme_engine.getFillColor(theme->name, this->_text_key));
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

struct DynamicTextLabel : BasicTextLabel
{
    bool _bright = false;
    bool _lazy = false;
    bool _dirt = false;

    DynamicTextLabel()
    {
        _text_key = "dytext";
    }

    void bright(bool lit = true) { _bright = lit; }

    std::function<std::string ()> _getText;
    void setFetch(std::function<std::string ()> get)
    {
        _getText = get;
    }
    void setLazy() {
        _dirt = _lazy = true;
    }
    void modified(bool changed = true)
    {
        _dirt = changed;
    }
    void setText(std::string text)
    {
        BasicTextLabel::text(text);
        _dirt = true;
    }
    void refresh()
    {
        if (_getText) {
            if (!_lazy || _dirt) {
                BasicTextLabel::text(_getText());
                _dirt = false;
            }
        }
    }
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        auto result = BasicTextLabel::applyTheme(theme_engine, theme);
        _dirt = true;
        return result;
    }
    void drawLayer(const DrawArgs& args, int layer) override
    {
        Widget::drawLayer(args, layer);
        if (1 != layer || !_bright) return;
        refresh();
        BasicTextLabel::render(args);
    }
    void draw(const DrawArgs& args) override
    {
        Widget::draw(args);
        if (_bright) return;
        refresh();
        BasicTextLabel::render(args);
    }
};

inline DynamicTextLabel* createLazyDynamicTextLabel(
    Vec pos,
    Vec size, 
    std::function<std::string ()> get,
    float text_height = 12.f,
    bool bold = true,
    TextAlignment alignment = TextAlignment::Center,
    const NVGcolor &color = RampGray(G_90),
    bool bright = false)
{
    auto w = new DynamicTextLabel();
    w->setSize(size);
    w->setPos(pos);
    w->setLazy();
    w->setFetch(get);
    w->style(text_height, bold, alignment);
    if (alignment == TextAlignment::Center) {
        w->box.pos.x -= size.x * .5f;
    }
    w->color(color);
    if (bright) {
        w->bright();
    }
    return w;
}

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
    void style(float size, bool bold = true, TextAlignment alignment = TextAlignment::Left) {

        _label->style(size, bold, alignment);
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
    std::shared_ptr<SvgTheme> theme,
    const char* textKey,
    TextAlignment alignment = TextAlignment::Center,
    float text_height = 12.f,
    bool bold = true
    )
{
    TWidget* w = createWidget<TWidget>(pos);
    w->setSize(Vec(width, text_height));
    if (alignment == TextAlignment::Center) {
        w->setPos(Vec(w->box.pos.x - width*.5f, w->box.pos.y));
    }
    w->text(text);
    w->style(text_height, bold, alignment);
    if (textKey && theme) {
        w->_label->_text_key = textKey;
    } else {
        w->color(RampGray(G_90));
    }
    w->applyTheme(engine, theme);
    return w;
}

template<typename TWidget = DynamicTextLabel>
TWidget* createDynamicLabel (
    math::Vec pos,
    float width,
    std::function<std::string ()> get,
    TextAlignment alignment = TextAlignment::Center,
    float text_height = 12.f,
    bool bold = true,
    const NVGcolor& text_color = RampGray(G_85)
    )
{
    TWidget* w = createWidget<TWidget>(pos);
    w->setSize(Vec(width, text_height));
    if (alignment == TextAlignment::Center) {
        w->setPos(Vec(w->box.pos.x -= width *.5f, w->box.pos.y));
    }
    w->style(text_height, bold, alignment);
    w->setFetch(get);
    w->color(text_color);
    return w;
}
}
