// Copyright (C) Paul Chase Dempsey
#pragma once
#include "services/text.hpp"
#include "services/svg-theme.hpp"
#include "layout-help.hpp"
using namespace svg_theme;

namespace widgetry {

enum class TextAlignment { Left, Center, Right };
enum class VAlignment { Top, Middle, Bottom };

inline NVGalign fromTextAlignment(TextAlignment ta)
{
    switch (ta) {
    case TextAlignment::Left: return NVG_ALIGN_LEFT;
    case TextAlignment::Center: return NVG_ALIGN_CENTER;
    case TextAlignment::Right: return NVG_ALIGN_RIGHT;
    }
    return NVG_ALIGN_LEFT;
}
inline NVGalign fromVAlignment(VAlignment va)
{
    switch (va) {
    case VAlignment::Top: return NVG_ALIGN_TOP;
    case VAlignment::Middle: return NVG_ALIGN_MIDDLE;
    case VAlignment::Bottom: return NVG_ALIGN_BOTTOM;
    }
    return NVG_ALIGN_TOP;
}

struct LabelStyle
{
    const char* key;
    TextAlignment align;
    VAlignment valign;
    float height;
    bool bold;

    LabelStyle()
    :   key("label"),
        align(TextAlignment::Left),
        valign(VAlignment::Top),
        height(12.f),
        bold(false)
    {}
    LabelStyle(const char * key, TextAlignment align = TextAlignment::Left, float height = 12.f, bool bold = false)
    :   key(key),
        align(align),
        valign(VAlignment::Top),
        height(height),
        bold(bold)
    {}
    LabelStyle(const char * key, TextAlignment align, VAlignment vert, float height = 12.f, bool bold = false)
    :   key(key),
        align(align),
        valign(vert),
        height(height),
        bold(bold)
    {}
    LabelStyle& operator=(const LabelStyle&other) {
        key = other.key;
        align = other.align;
        valign = other.valign;
        height = other.height;
        bold = other.bold;
        return *this;
    }
    LabelStyle(const LabelStyle& other)
    :   key(other.key),
        align(other.align),
        valign(other.valign),
        height(other.height),
        bold(other.bold)
    {}
    NVGalign nvg_alignment() {
        return static_cast<NVGalign>(fromTextAlignment(align) | fromVAlignment(valign));
    }
    bool left() { return align == TextAlignment::Left; }
    bool right() { return align == TextAlignment::Right; }
    bool center() { return align == TextAlignment::Center; }
};

struct BasicTextLabel: Widget, IThemed, ILayoutHelp
{
    using Base = Widget;

    std::string _text;
    NVGcolor _color;
    LabelStyle _style;
    bool bright;

    BasicTextLabel();

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
    const NVGcolor & get_color() { return _color; }
    void color(const NVGcolor &new_color) { _color = new_color; }
    void glowing(bool glow) {
        bright = glow;
    }

    // IThemed
    void applyTheme(std::shared_ptr<SvgTheme> theme) override;

    void render(const DrawArgs& args);
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;
};

struct TextLabel: Widget, ILayoutHelp
{
    using Base = Widget;

    FramebufferWidget* _fb{nullptr};
    BasicTextLabel* _label{nullptr};

    TextLabel()
    {
        layout_hint_color = Overlay(GetStockColor(StockColor::Green), .5f);
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
    const NVGcolor& get_color() { return _label->get_color(); }
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
    void glowing(bool glow) {
        _label->glowing(glow);
        dirty();
    }

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);
        if (layout_hints) {
            draw_widget_bounds(this, args);
        }
    }
};

template<typename TWidget = TextLabel>
TWidget* createStaticTextLabel(math::Vec pos, float width, std::string text) {
    TWidget* w = createWidget<TWidget>(pos);
    w->text(text);
    w->color(RampGray(G_90));
    if (w->_label->centered()) {
        w->setPos(Vec(w->box.pos.x - width*.5f, w->box.pos.y));
    } else if (w->_label->right()) {
        w->setPos(Vec(w->box.pos.x - width, w->box.pos.y));
    }
    w->setSize(Vec(width, w->_label->text_height()));
    return w;
}

template<typename TWidget = TextLabel>
TWidget* createLabel(math::Vec pos, float width, std::string text, const LabelStyle& style) {
    TWidget* w = createWidget<TWidget>(pos);
    w->text(text);
    w->_label->style(style);
    w->color(RampGray(G_90));
    if (w->_label->centered()) {
        w->setPos(Vec(w->box.pos.x - width*.5f, w->box.pos.y));
    } else if (w->_label->right()) {
        w->setPos(Vec(w->box.pos.x - width, w->box.pos.y));
    }
    w->setSize(Vec(width, w->_label->text_height()));
    return w;
}
template<typename TWidget = TextLabel>
TWidget* createLabel(math::Rect bounds, std::string text, const LabelStyle& style) {
    TWidget* w = new TWidget;
    w->text(text);
    w->_label->style(style);
    w->color(RampGray(G_90));
    w->setPos(bounds.pos);
    w->setSize(bounds.size);
    return w;
}

}
