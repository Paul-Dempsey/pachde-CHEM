
#pragma once
#include "services/svg-query.hpp"
using namespace ::svg_query;
#define DIALOG_THEMED
#include "dialog.hpp"
#include "close-button.hpp"
#include "theme-button.hpp"
#include "label.hpp"

namespace widgetry {

struct DialogStyles {
    LabelStyle* title_style{nullptr};
    LabelStyle* center_label_style{nullptr};
    LabelStyle* info_label_style{nullptr};

    void createStyles(std::shared_ptr<svg_theme::SvgTheme> svg_theme) {
        title_style = new LabelStyle("dlg-title", colors::Black, 14.f, true);
        title_style->valign = VAlign::Top;
        title_style->halign = HAlign::Left;

        center_label_style = new LabelStyle("dlg-label");
        center_label_style->halign = HAlign::Center;

        info_label_style = new LabelStyle("dlg-info", colors::PortCorn, 12.f);

        title_style->applyTheme(svg_theme);
        center_label_style->applyTheme(svg_theme);
        info_label_style->applyTheme(svg_theme);
    }
};

template <typename TParent>
void add_close_button(TParent* host, ::svg_query::BoundsIndex bounds, const char* key, std::shared_ptr<SvgTheme> svg_theme) {
    auto close = createWidgetCentered<CloseButton>(bounds[key].getCenter());
    close->set_handler([=](){ host->close(); } );
    close->applyTheme(svg_theme);
    host->addChild(close);
}

template <typename TParent, typename TKnob = RoundBlackKnob>
void add_knob(TParent* host, ::svg_query::BoundsIndex &bounds, const char *key, Module* module, int param) {
    host->addChild(createParamCentered<TKnob>(bounds[key].getCenter(), module, param));
}

template <typename TParent>
void add_check(TParent* host, ::svg_query::BoundsIndex &bounds, const char *key,
    Module* module, int param, std::shared_ptr<svg_theme::SvgTheme> svg_theme = nullptr
) {
    auto check = Center(createThemedParamButton<CheckButton>(
        bounds[key].getCenter(),
        &host->my_svgs,
        module,
        param
    ));
    if (svg_theme) {
        check->applyTheme(svg_theme);
    }
    host->addChild(check);
}

template <typename TParent>
TextLabel* add_label(
    TParent* host,
    ::svg_query::BoundsIndex &bounds,
    const char *key,
    const char *text,
    LabelStyle* style,
    std::shared_ptr<svg_theme::SvgTheme> svg_theme
) {
    auto label = createLabel(bounds[key], text, style, svg_theme);
    host->addChild(label);
    return label;
}

}