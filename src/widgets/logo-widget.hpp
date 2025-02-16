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
    float scale;

    TSymbolWidget() : scale(1.0f) {
        fb = new widget::FramebufferWidget;
        addChild(fb);
        sw = new widget::SvgWidget;
        fb->addChild(sw);
        setSvg(Svg::load(asset::plugin(pluginInstance, TSymbol::symbol()))); 
    }

    TSymbolWidget(float scale) : scale(scale) {
        fb = new widget::FramebufferWidget;
        addChild(fb);
        sw = new widget::SvgWidget;
        fb->addChild(sw);
        setSvg(Svg::load(asset::plugin(pluginInstance, TSymbol::symbol()))); 
    }

    void set_scale(float scale) {
        if (scale == this->scale)
            return;
        this->scale = scale;

        // Dispatch Dirty event
        widget::EventContext cDirty;
        DirtyEvent eDirty;
        eDirty.context = &cDirty;
        Widget::onDirty(eDirty);        
    }

    void setSvg(std::shared_ptr<window::Svg> svg) {
        if (svg == sw->svg) return;

        sw->setSvg(svg);
        fb->box.size = sw->box.size * scale;
        box.size = sw->box.size * scale;
        fb->setDirty();
    }

    void drawLayer(const DrawArgs& args, int layer) override {
        DrawArgs args2 = args;
        args2.clipBox.pos = args2.clipBox.pos.div(scale);
        args2.clipBox.size = args2.clipBox.size.div(scale);
        nvgScale(args.vg, scale, scale);
        Base::drawLayer(args2, layer);
    }

    void draw(const DrawArgs& args) override {
        DrawArgs args2 = args;
        args2.clipBox.pos = args2.clipBox.pos.div(scale);
        args2.clipBox.size = args2.clipBox.size.div(scale);
        nvgScale(args.vg, scale, scale);
        Base::draw(args2);
    }
};

struct LogoSvg {
    static std::string symbol() { return "res/CHEM-logo.svg"; }
};
struct LogoWatermarkSvg {
    static std::string symbol() { return "res/CHEM-logo-watermark.svg"; }
};

using Logo = TSymbolWidget<LogoSvg>;
using WatermarkLogo = TSymbolWidget<LogoWatermarkSvg>;

}