#pragma once
#include <rack.hpp>
using namespace ::rack;

#include "element-style.hpp"
#include "../services/text.hpp"

namespace pachde {

struct TextInput : TextField, IApplyTheme
{
    using Base = TextField;
    float text_height = 12.f;

    ElementStyle bg_style{"entry_bg", "hsla(42, 20%, 100%, 5%)", "hsl(42, 50%, 40%)", .25f};
    ElementStyle text_style{"entry-text", "hsl(0, 0%, 65%)"};
    ElementStyle prompt_style{"entry-prompt", "hsla(0, 0%, 55%, 75%)"};
    ElementStyle selection_style{"entry_sel", "hsl(200, 50%, 40%)"};
    std::function<void(std::string)> change_handler{nullptr};
    std::function<void(std::string)> enter_handler{nullptr};
    std::function<void(bool ctrl, bool shift)> tab_handler{nullptr};

    void set_on_change(std::function<void(std::string)> handler) { change_handler = handler; }
    void set_on_enter(std::function<void(std::string)> handler) { enter_handler = handler; }
    void set_on_tab(std::function<void(bool ctrl, bool shift)> handler) { tab_handler = handler; }

    TextInput();
    bool empty() { return text.empty(); }
    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override;
    void onSelectKey(const SelectKeyEvent& e) override;
    void onChange(const ChangeEvent& e) override;
    void onAction(const ActionEvent& e) override;
    int getTextPosition(math::Vec mousePos) override;
    void draw(const DrawArgs& args) override;
};


struct TextInputMenu : TextInput
{
    using Base = TextInput;

    TextInputMenu() {
        text_height = 18.f;
    }

    void setText(const std::string& text) {
        this->text = text;
        selectAll();
    }

    void onSelectKey(const event::SelectKey &e) override {
        if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER)) {
            if (enter_handler) {
                enter_handler(text);
            }
            ui::MenuOverlay *overlay = getAncestorOfType<ui::MenuOverlay>();
            overlay->requestDelete();
            e.consume(this);
        }
        if (!e.getTarget())
            Base::onSelectKey(e);
    }

    void onChange(const ChangeEvent& e) override {
        Base::onChange(e);
        if (change_handler) { 
            change_handler(text);
        }
    }

    void step() override {
        // Keep selected
        APP->event->setSelectedWidget(this);
        Base::step();
    }
};

}