#pragma once
#include <rack.hpp>
using namespace ::rack;
namespace pachde {

template<typename TSymbol>
struct TSymbolWidget: OpaqueWidget
{
    using Base = OpaqueWidget;

    widget::FramebufferWidget* fb;
	widget::SvgWidget* sw;

    TSymbolWidget() {
        fb = new widget::FramebufferWidget;
        addChild(fb);
        sw = new widget::SvgWidget;
        fb->addChild(sw);
        setSvg(Svg::load(asset::plugin(pluginInstance, TSymbol::symbol()))); 
    }

    void setSvg(std::shared_ptr<window::Svg> svg) {
        if (svg == sw->svg) return;

        sw->setSvg(svg);
        fb->box.size = sw->box.size;
        box.size = sw->box.size;

        fb->setDirty();
    }
    void draw(const DrawArgs& args) override {
        Base::draw(args);
    }
};

struct LogoSvg {
    static std::string symbol() { return "res/CHEM-logo.svg"; }
};

using Logo = TSymbolWidget<LogoSvg>;

}