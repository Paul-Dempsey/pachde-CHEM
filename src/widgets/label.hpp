#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/packed-color.hpp"
#include "services/colors.hpp"
#include "services/text.hpp"
#include "services/text-align.hpp"
#include "services/svg-theme.hpp"
#include "tip-widget.hpp"
using namespace pachde;

namespace widgetry {

struct LabelStyle {
    const char * key{"label"};
    PackedColor color{colors::G50};
    bool bold{false};
    float text_height{12.f};
    // float left_margin{0.f};
    // float right_margin{0.f};
    // float top_margin{0.f};
    // float bottom_margin{0.f};
    HAlign halign{HAlign::Center};
    VAlign valign{VAlign::Middle};
    Orientation orientation{Orientation::Normal};
    float baseline{INFINITY};

    LabelStyle() {}
    LabelStyle(const char * key) : key(key) {}

    LabelStyle(const char * key, PackedColor color, float font_size = 12.f, bool bold = false)
        : key(key), color(color), bold(bold), text_height(font_size) {}

    LabelStyle(const char * key, const char * color_spec, float font_size = 12.f, bool bold = false)
        : key(key), color(parseColor(color_spec, colors::G50)), bold(bold), text_height(font_size) {}

    LabelStyle(const char * key, const char * color_spec, HAlign halign, float font_size = 12.f, bool bold = false)
        : key(key), color(parseColor(color_spec, colors::G50)), bold(bold), text_height(font_size), halign(halign) {}

    LabelStyle(const char * key, HAlign halign, float font_size, bool bold = false)
        : key(key), bold(bold), text_height(font_size), halign(halign) {}

    LabelStyle(const char * key, HAlign halign, VAlign valign, float font_size, bool bold = false)
        : key(key), bold(bold), text_height(font_size), halign(halign), valign(valign) {}

    LabelStyle(const char * key, HAlign halign, VAlign valign, PackedColor color, float font_size, bool bold = false)
        : key(key), bold(bold), text_height(font_size), halign(halign), valign(valign) {}

    void applyTheme(std::shared_ptr<svg_theme::SvgTheme> theme) {
        if (!key || !*key) return;
        if (!theme->getFillColor(color, key, true)) {
            color = colors::Red;
        }
    }
};

struct TextLabel : TransparentWidget, svg_theme::IThemed
{
    using Base = TransparentWidget;

    LabelStyle* format{nullptr};
    std::string text;
    bool own_format{false};
    bool bright{false};

    TextLabel() {}
    TextLabel(LabelStyle* s) : format(s) {}

    ~TextLabel() {
        if (own_format) {
            assert(format);
            delete format;
        }
    }

    LabelStyle* create_owned_format() {
        format = new LabelStyle();
        own_format = true;
        return format;
    }

    void set_style(LabelStyle* s, bool owned = false) {
        if (own_format) {
            delete format;
            own_format = owned;
        }
        format = s;
    }

    PackedColor get_color() { return format ? format->color : colors::G50; }
    void set_color(PackedColor color) { if (format) format->color = color; }

    void set_text(const std::string& name) { text = name; }
    const std::string& get_text() { return text; }

    void glowing(bool glow) { bright = glow; }

    void applyTheme(std::shared_ptr<svg_theme::SvgTheme> theme) override {
        if (format) format->applyTheme(theme);
    }

    void draw_text(const DrawArgs& args) {
        auto font = ((format && format->bold)) ? GetPluginFontSemiBold() : GetPluginFontRegular();
        if (!FontOk(font)) return;
        if (format) {
            draw_oriented_text_box(
                args.vg, box.zeroPos(), 0.f, 0.f, 0.f, 0.f,
                text, font, format->text_height, format->color,
                format->halign, format->valign, format->orientation,
                format->baseline
            );
        } else {
            draw_oriented_text_box(
                args.vg, box.zeroPos(), 0.f, 0.f, 0.f, 0.f,
                text, font, 12.f, colors::G50,
                HAlign::Center, VAlign::Baseline, Orientation::Normal
            );
        }
    }

    void drawLayer(const DrawArgs& args, int layer) override {
        Base::drawLayer(args, layer);
        if (1 != layer) return;
        if (!bright) return;
        draw_text(args);
    }

    void draw(const DrawArgs& args) override {
        Base::draw(args);
        if (bright) return;
        draw_text(args);
    }
};

struct TipLabel : TextLabel {
    using Base = TextLabel;

    TipHolder* tip_holder;

    TipLabel() : tip_holder(nullptr) {}
    TipLabel(LabelStyle* s) : Base(s), tip_holder(nullptr) {}

    virtual ~TipLabel() {
        if (tip_holder) delete tip_holder;
        tip_holder = nullptr;
    }

    void ensureTipHolder() {
        if (!tip_holder) {
            tip_holder = new TipHolder();
        }
    }

    std::string description() {
        if (!tip_holder) return "";
        return tip_holder->tip_text;
    }

    void describe(std::string text) {
        ensureTipHolder();
        tip_holder->setText(text);
    }

    void destroyTip() {
        if (tip_holder) { tip_holder->destroyTip(); }
    }

    void createTip() {
        ensureTipHolder();
        tip_holder->createTip();
    }

    void onHover(const HoverEvent& e) override {
        Base::onHover(e);
        e.consume(this);
    }

    void onEnter(const EnterEvent& e) override {
        Base:: onEnter(e);
        createTip();
    }

    void onLeave(const LeaveEvent& e) override {
        Base:: onLeave(e);
        destroyTip();
    }

    void onDragLeave(const DragLeaveEvent& e) override {
        Base::onDragLeave(e);
        destroyTip();
    }

    void onDragEnd(const DragEndEvent& e) override {
        Base::onDragEnd(e);
        destroyTip();
    }

    void onButton(const ButtonEvent& e) override {
        Base::onButton(e);
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
            destroyTip();
            createContextMenu();
            e.consume(this);
        }
    }

    void createContextMenu() {
        ui::Menu* menu = createMenu();
    	appendContextMenu(menu);
    }

    virtual void appendContextMenu(ui::Menu* menu) {}
};

template <typename TLabel = TextLabel>
TLabel* createLabel(Rect bounds, const std::string& text, LabelStyle* format, std::shared_ptr<svg_theme::SvgTheme> theme = nullptr) {
    auto label = new TLabel(format);
    label->box = bounds;
    label->text = text;
    if (theme) {
        label->applyTheme(theme);
    }
    return label;
}

template <typename TLabel = TextLabel>
TLabel* createLabel(Vec(pos), const std::string& text, LabelStyle* format, float width) {
    auto label = new TLabel(format);
    label->box.pos = pos;
    label->box.size.x = width;
    label->box.size.y = format->text_height;
    label->text = text;
    return label;
}

template <typename TLabel = TextLabel>
TLabel* createLabelCentered(Vec(pos), const std::string& text, LabelStyle* format, float width) {
    auto label = new TLabel(format);
    label->box.pos = pos;
    label->box.pos.x -= width * .5f;
    label->box.size.x = width;
    label->box.size.y = format->text_height;
    label->text = text;
    return label;
}
template <typename TLabel = TextLabel>
TLabel* createLabelRight(Vec(pos), const std::string& text, LabelStyle* format, float width) {
    auto label = new TLabel(format);
    label->box.pos = pos;
    label->box.pos.x -= width;
    label->box.size.x = width;
    label->box.size.y = format->text_height;
    label->text = text;
    return label;
}
template <typename TLabel = TextLabel>
TLabel* createLabel(Vec(pos), const std::string& text, float width, float text_height) {
    auto label = new TLabel(new LabelStyle);
    label->own_format = true;
    label->box.pos = pos;
    label->box.size.x = width;
    label->box.size.y = text_height;
    label->format->text_height = text_height;
    label->text = text;
    return label;
}

}
