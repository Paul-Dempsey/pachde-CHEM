
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
    LabelStyle title{"dlg-title", HAlign::Left, VAlign::Top, colors::Black, 14.f, true};
    LabelStyle section{"dlg-section", "hsl(42, .6, .8)", HAlign::Left, 14.f, true};
    LabelStyle head{"dlg-heading", HAlign::Left, 14.f, true};
    LabelStyle left{"dlg-label", HAlign::Left, 12.f, false};
    LabelStyle center;
    LabelStyle right{"dlg-label", HAlign::Right, 12.f, false};
    LabelStyle info{"dlg-info", colors::PortCorn, 12.f, false};
    LabelStyle note{"dlg-note", HAlign::Left, 9.f, false};

    void initStyles(std::shared_ptr<svg_theme::SvgTheme> svg_theme) {
        title.applyTheme(svg_theme);
        section.applyTheme(svg_theme);
        head.applyTheme(svg_theme);
        left.applyTheme(svg_theme);
        center.applyTheme(svg_theme);
        right.applyTheme(svg_theme);
        info.applyTheme(svg_theme);
        note.applyTheme(svg_theme);
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