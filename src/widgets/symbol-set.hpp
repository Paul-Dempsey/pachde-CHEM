// Copyright (C) Paul Chase Dempsey
#pragma once
#include "my-plugin.hpp"
#include "services/svg-theme.hpp"
#include "services/colors.hpp"
//#include "tip-widget.hpp"
using namespace svg_theme;

namespace widgetry {

struct SymbolProvider
{
    std::vector<std::shared_ptr<window::Svg>> frames;
    std::vector<std::string> sources;

    size_t source_count() { return sources.size(); }
    size_t count() { return frames.size(); }
    bool empty() { return frames.empty(); }

    std::shared_ptr<window::Svg> item(size_t index) {
        assert(index < count());
        return frames[index];
    }

    void add_source(std::string source) {
        sources.push_back(source);
    }

    void loadSvg(ILoadSvg* loader) {
        frames.clear();
        for (auto it = sources.cbegin(); it != sources.cend(); it++) {
            frames.push_back(loader->loadSvg(asset::plugin(pluginInstance, *it)));
        }
    }

};

struct SymbolSetWidget : OpaqueWidget
{
    using Base = OpaqueWidget;
    ::rack::widget::FramebufferWidget* fb;
	::rack::widget::SvgWidget* sw;
    SymbolProvider* source{nullptr};

    int index;
    float scale;

    SymbolSetWidget(SymbolProvider* source) : source(source), index(-1), scale(1.0f) {
        box.size.x = 18.f;
        box.size.y = 18.f;

        fb = new ::rack::widget::FramebufferWidget;
        addChild(fb);

        sw = new ::rack::widget::SvgWidget;
        fb->addChild(sw);
    }

    void set_index(int item) {
        assert(item < count());
        if (item != index) {
            index = item;
            if (!source->empty()) {
                setSvg(source->item(index));
            }
        }
    }

    int count() { return source->count(); }

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

}