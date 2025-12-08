#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/svt_rack.hpp"
#include "services/misc.hpp"
using namespace svg_theme;

namespace pachde {

template <typename TSvg>
struct Spinner : OpaqueWidget, IApplyTheme
{
    using Base = OpaqueWidget;
    FramebufferWidget* fb{nullptr};
	SvgWidget* sw{nullptr};
	TransformWidget* tw;
    CircularShadow* shadow;
    float angle{0};
    float theta{.01f};
    WallTimer timer;

    Spinner()
    {
        speed(.01f, .02f);

        fb = new widget::FramebufferWidget;
        addChild(fb);

        shadow = new CircularShadow;
        fb->addChild(shadow);
        shadow->box.size = math::Vec();

        tw = new widget::TransformWidget;
        fb->addChild(tw);

        sw = new widget::SvgWidget;
        tw->addChild(sw);
    }

    void speed (float time_step, float angle_step) {
        timer.set_interval(time_step);
        theta = angle_step;
    }

    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        setSvg(theme_engine.loadSvg(asset::plugin(pluginInstance, TSvg::image()), theme));
        return true;
    }

    // identical to SvgKnob::setSvg (but we don't want to be a paramWidget)
    void setSvg(std::shared_ptr<window::Svg> svg) {
        if (svg == sw->svg)
            return;

        sw->setSvg(svg);
        tw->box.size = sw->box.size;
        fb->box.size = sw->box.size;
        box.size = sw->box.size;

        shadow->box.size = sw->box.size;
        // Move shadow
        shadow->box.pos = math::Vec(sw->box.size.x * .05, sw->box.size.y * 0.15);

        fb->setDirty();
    }
    void step() override {
        if (timer.lap()) {
            angle += theta;
            spin_step();
        }
    }
    void spin_step() {
        tw->identity();
        // Rotate SVG
        math::Vec center = sw->box.getCenter();
        tw->translate(center);
        tw->rotate(angle);
        tw->translate(center.neg());
        fb->setDirty();
    }
    void drawLayer(const DrawArgs& args, int layer) override
    {
        if (layer != 1) return;
        if (rack::settings::rackBrightness > .95f) return;
        fb->draw(args);
    }
};

struct ChemSpinSvg {
    static std::string image() { return "res/widgets/spinner-chem.svg"; }
};

using ChemSpinner = Spinner<ChemSpinSvg>;

template <typename Tparent>
void startSpinner(Tparent* parent, Vec pos) {
    auto spinner = new ChemSpinner();
    spinner->box.pos = pos;
    spinner->applyTheme(theme_engine, theme_engine.getTheme(parent->getThemeName()));
    Center(spinner);
    parent->addChild(spinner);
}

template <typename Tparent>
void stopSpinner(Tparent* parent) {
    std::vector<ChemSpinner*> spinners;
    for (auto child : parent->children) {
        auto spinner = dynamic_cast<ChemSpinner*>(child);
        if (spinner) spinners.push_back(spinner);
    }
    for (auto spinner: spinners) {
        parent->removeChild(spinner);
    }
}



}