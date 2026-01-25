#include "info-symbol.hpp"
using namespace pachde;

namespace widgetry {

void InfoSymbol::applyTheme(std::shared_ptr<SvgTheme> theme) {
    if (!theme->getFillColor(color, "info", true)) {
        color = colors::G85;
    }
    if (!theme->getFillColor(co_hover, "info-hover", true)) {
        co_hover = 0xff06809a;
    }
}

void InfoSymbol::onButton(const ButtonEvent &e) {
    destroyTip();
    if (this->click_handler
        && (e.action == GLFW_PRESS)
        && (e.button == GLFW_MOUSE_BUTTON_LEFT)
        && ((e.mods & RACK_MOD_MASK) == 0)
    ) {
        click_handler();
    }
    Base::onButton(e);
}

void InfoSymbol::draw(const DrawArgs &args) {
    NVGcolor hover_color = fromPacked(co_hover);
    NVGcolor plain_color = fromPacked(color);

    auto vg = args.vg;
    // hover:filled
    if (hovered) {
        nvgBeginPath(vg);
        nvgCircle(vg, 7.5, 7.5, 6.45);
        nvgFillColor(vg, hover_color);
        nvgFill(vg);
    }
    // medallion outline
    nvgBeginPath(vg);
    nvgCircle(vg, 7.5, 7.5, 6.45);
    nvgStrokeColor(vg, plain_color);
    nvgStrokeWidth(vg, 1.);
    nvgStroke(vg);

    // glyph
    const NVGcolor& co = hovered ? RampGray(G_95) : plain_color;

    Circle(vg, 7.5, 4.5, 1., co);

    nvgBeginPath(vg);
    nvgMoveTo(vg, 5.9, 7.3);
    nvgLineTo(vg, 5.9, 6.14);
    nvgLineTo(vg, 8.28, 6.14);
    nvgLineTo(vg, 8.28, 10.02);
    nvgLineTo(vg, 9.03, 10.02);
    nvgLineTo(vg, 9.03, 11.18);
    nvgLineTo(vg, 5.9, 11.18);
    nvgLineTo(vg, 5.9, 10.);
    nvgLineTo(vg, 6.7, 10.);
    nvgLineTo(vg, 6.7, 7.2);
    nvgClosePath(vg);
    nvgFillColor(vg, co);
    nvgFill(vg);
}

}