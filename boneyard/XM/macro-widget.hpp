#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../../widgets/widgets.hpp"

namespace pachde {

namespace muiw {
    constexpr const float CENTER = 22.5f;
    constexpr const float KNOB_CY = 10.f;
    constexpr const float KNOB_LABEL_DY = 9.f;
};

struct MacroUiWidget : OpaqueWidget, IApplyTheme
{
    using Base = OpaqueWidget;
    ssize_t index{-1};
    bool editing{false};
    
    bool glow{false};
    std::string text;
    ElementStyle current_style{"xm-current", "hsl(60,80%,75%)", "hsl(60,80%,75%)", .85f};
    ElementStyle placeholder_style{"xm-placeholder", "hsl(0,0%,60%)", "hsl(0,0%,60%)", .5f};
    GlowKnob* knob{nullptr};
    TrackWidget* track{nullptr};
    TextLabel* label{nullptr};
    TextInput* label_edit{nullptr};
    void set_edit_mode(bool editing, Module * module, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme);

    MacroUiWidget(ssize_t index);
    static MacroUiWidget * create(Vec pos, ssize_t index, std::string text, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme);
    void set_text(std::string t);
    void add_label(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) ;
    void glowing(bool glow);
    void disable();
    void enable(Module * module, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme);
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override;
    void draw(const DrawArgs& args) override;
};

}