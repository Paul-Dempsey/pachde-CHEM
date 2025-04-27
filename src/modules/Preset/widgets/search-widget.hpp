#pragma once
#include <rack.hpp>
using namespace ::rack;

#include "../../../widgets/element-style.hpp"
#include "../../../services/text.hpp"

namespace pachde {

struct SearchField : TextField, IApplyTheme
{
    using Base = TextField;

    ElementStyle bg_style{"entry_bg", "hsla(42, 20%, 100%, 5%)", "hsl(42, 50%, 40%)", .25f};
    ElementStyle text_style{"entry-text", "hsl(0, 0%, 65%)"};
    ElementStyle prompt_style{"entry-prompt", "hsla(0, 0%, 55%, 75%)"};
    ElementStyle selection_style{"entry_sel", "hsl(200, 50%, 40%)"};
    std::function<void(void)> change_handler{nullptr};
    std::function<void(void)> enter_handler{nullptr};

    SearchField();
    bool empty() { return text.empty(); }
    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override;
    void onSelectKey(const SelectKeyEvent& e) override;
    void onChange(const ChangeEvent& e) override;
    void onAction(const ActionEvent& e) override;
    int getTextPosition(math::Vec mousePos) override;
    void draw(const DrawArgs& args) override;
};

}