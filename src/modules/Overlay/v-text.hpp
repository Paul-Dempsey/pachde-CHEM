#pragma once
#include <rack.hpp>
#include "../../services/colors.hpp"
#include "../../services/text.hpp"
using namespace ::rack;
namespace pachde {

struct VerticalLabel: OpaqueWidget {
    std::string text{"hello"};
    PackedColor color{PackRGB(uint32_t(240), uint32_t(240), uint32_t(240))};
    bool bold{true};
    float height{28.f};

    void draw(const DrawArgs& args) override
    {
        auto vg = args.vg;
        auto font = bold ? GetPluginFontSemiBold() : GetPluginFontRegular();
        if (!FontOk(font)) return;
        nvgRotate(vg, M_PI/-2.f);
        nvgTranslate(vg, -box.size.y*.5, 0);
        SetTextStyle(vg, font, fromPacked(color), height);
        nvgTextAlign(vg, NVG_ALIGN_TOP|NVG_ALIGN_CENTER);
        nvgText(vg, box.size.x*.4, 0, text.c_str(), nullptr);
    }
};

struct VText : OpaqueWidget {
    FramebufferWidget* fb{nullptr};
    VerticalLabel* label;

    void modify(bool modified = true) {
        if (fb) fb->setDirty(modified);
    }
    void dirty() { if (fb) fb->setDirty(); }

    VText() {
        fb = new widget::FramebufferWidget;
        label = new VerticalLabel();
        fb->addChild(label);
        set_size(Vec(10.f,10.f));
        addChild(fb);
    }
    void set_size(Vec size) {
        box.size = size;
        label->box.size = size;
        fb->box.size = size;
        dirty();
    }
    void set_text_color(PackedColor color) {
        label->color = color;
        dirty();
    }
    void set_text_height(float height) {
        label->height = height;
        dirty();
    }
    void set_text(std::string t) {
        label->text = t;
        dirty();
    }
};

}