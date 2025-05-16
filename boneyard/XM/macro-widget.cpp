#include "macro-widget.hpp"

namespace pachde {

void MacroUiWidget::set_text(std::string t) {
    text = t;
    if (label) label->text(t);
}

void MacroUiWidget::set_edit_mode(bool editing, Module * module, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    if (editing) {
        if (knob) removeChild(knob);
        if (label) removeChild(label);
        label_edit = new TextInput();
        
    } else {
        if (label_edit) removeChild(label_edit);
        enable(module, engine, theme);
    }
}

MacroUiWidget::MacroUiWidget(ssize_t index) : index(index)
{
    box.size.x = 45.f;
    box.size.y = 32.f;
}

MacroUiWidget* MacroUiWidget::create(Vec pos, ssize_t index, std::string text, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    MacroUiWidget* mw = new MacroUiWidget(index);
    mw->box.pos = pos;
    mw->set_text(text);
    mw->add_label(engine, theme);
    return mw;
}

void MacroUiWidget::add_label(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) 
{
    if (!label) {
        LabelStyle macro_label_style{"ctl-label", TextAlignment::Center, 10.f, false};
        label = createLabel(Vec(muiw::CENTER, muiw::KNOB_CY + muiw::KNOB_LABEL_DY), 45.f, text, theme_engine, theme, macro_label_style);
        addChild(label);
    } 
}

void MacroUiWidget::glowing(bool glow) {
    this->glow = glow;
    if (knob) knob->glowing(glow);
}

void MacroUiWidget::disable() {
    if (label) { label->text(""); }
    if (knob) { removeChild(knob); knob = nullptr; }
}

void MacroUiWidget::enable(Module * module, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    if (!knob) {
        knob = createChemKnob<GrayTrimPot>(Vec(muiw::CENTER, muiw::KNOB_CY), module, index, engine, theme);
        knob->glowing(glow);
        addChild(knob);
    }
    if (!track)  {
        track = createTrackWidget(knob, theme_engine, theme);
        addChild(track);
    }
    add_label(theme_engine, theme);
}

bool MacroUiWidget::applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme)
{
    current_style.apply_theme(theme);
    placeholder_style.apply_theme(theme);
    return true;
}

void MacroUiWidget::draw(const DrawArgs& args)
{
    Base::draw(args);
    auto vg = args.vg;
    if (!knob) {
        auto co = placeholder_style.nvg_stroke_color();
        auto w = placeholder_style.width();
        OpenCircle(vg, muiw::CENTER, muiw::KNOB_CY, 8.f, co, w);
        Line(vg, muiw::CENTER, muiw::KNOB_CY - 7.f, muiw::CENTER, muiw::KNOB_CY - 1.f, co, w);
    }
}

}