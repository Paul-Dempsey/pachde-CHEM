#pragma once
#include "services/colors.hpp"
#include "services/svg-theme.hpp"
#include "widgets/click-region-widget.hpp"

namespace widgetry {

struct OptionWidget : ClickRegion {
    using Base = ClickRegion;
    const char * disc_key{"option-bullet"};
    PackedColor co_disc{colors::G80};
    float radius{2.5f};

    bool selected{false};
    std::vector<OptionWidget*>* peers{nullptr};

    OptionWidget() {
        box.size = Vec(12.f, 12.f);
        hoverable = true;
    }

    int get_selected_option() {
        for (auto it = peers->cbegin(); it != peers->cend(); it++) {
            if ((*it)->selected) {
                return (*it)->identifier;
            }
        }
        return -1;
    }

    void init(int id, std::vector<OptionWidget*>* option_set, float radius = 2.5f, const char* color_key = nullptr) {
        identifier = id;
        peers = option_set;
        this->radius = radius;
        if (color_key) disc_key = color_key;
    }

    void applyTheme(std::shared_ptr<SvgTheme> theme) override {
        Base::applyTheme(theme);
        if (!theme->getFillColor(co_disc, disc_key, true)) {
            co_disc = colors::G50;
        }
    }

    void onButton(const ButtonEvent& e) override {
        auto mod = e.mods & RACK_MOD_MASK;
        if ((0 == mod) && (e.button == GLFW_MOUSE_BUTTON_LEFT)) {
            e.consume(this);
            for (auto it = peers->cbegin(); it != peers->cend(); it++) {
                (*it)->selected = (*it == this);
            }
        }
        Base::onButton(e);
    }

    void draw(const DrawArgs& args) override {
        Base::draw(args);
        NVGcolor co = fromPacked(co_disc);
        if (selected || hovered) {
            Circle(args.vg, radius, box.size.y * .5, radius, co);
        }
        OpenCircle(args.vg, radius, box.size.y * .5, radius, co, .75f);
    }
};

}