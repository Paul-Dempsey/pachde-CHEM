// Copyright (C) Paul Chase Dempsey
#pragma once
#include "my-plugin.hpp"
#include "services/svg-theme.hpp"
#include "services/colors.hpp"
using namespace svg_theme;

namespace pachde {

constexpr const PackedColor DEFAULT_DISABLED_SCREEEN{0x80303030};

struct DisableWidget : TipWidget, IThemed
{
    Widget* target{nullptr};
    NVGcolor screen{fromPacked(DEFAULT_DISABLED_SCREEEN)};
    bool enabled{true};
    bool circular{false};

    DisableWidget(Widget* target_widget)
    {
        target = target_widget;
        box = target->box;
        describe("(unavailable)");
    }

    static DisableWidget* create(Widget* target_widget, bool enabled = true)
    {
        auto result = new DisableWidget(target_widget);
        result->set_enable(enabled);
        return result;
    }

    static DisableWidget* createCircular(Widget* target_widget, bool enabled = true)
    {
        auto result = new DisableWidget(target_widget);
        result->circular = true;
        result->set_enable(enabled);
        return result;
    }

    void circular_target() { circular = true; }

    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        auto co = theme->getFillColor("disable-screen", true);
        screen = fromPacked(isVisibleColor(co) ? co : DEFAULT_DISABLED_SCREEEN);
        return true;
    }

    void set_enable(bool enable)
    {
        auto parent = target->getParent();
        bool has_me = parent->hasChild(this);
        if (enable) {
            if (has_me) { parent->removeChild(this); }
        } else {
            if (!has_me) { parent->addChildAbove(this, target); }
        }
        enabled = enable;
    }

    void draw(const DrawArgs& args) override
    {
        if (enabled) return;

        auto vg = args.vg;
        if (circular) {
            Circle(args.vg, box.size.x*.5f, box.size.y*.5f, box.size.x*.5f - 1.f, screen);
        } else {
            FillRect(vg, 0, 0, box.size.x, box.size.y, screen);
        }
    }
};


}