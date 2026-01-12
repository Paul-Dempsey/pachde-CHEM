#pragma once
#include "tip-widget.hpp"
#include "services/colors.hpp"
using namespace svg_theme;
using namespace pachde;

namespace widgetry {

struct Blip: TipWidget
{
    using Base = TipWidget;
    NVGcolor hole;
    NVGcolor rim;
    NVGcolor light;
    float bright;
    float step_bright;
    float radius;

    Blip();
    Blip(const NVGcolor& hole, const NVGcolor& rim, const NVGcolor& light);

    void step() override;
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;

    void set_hole_color(const NVGcolor& co) { hole = co; }
    void set_rim_color(const NVGcolor& co) { rim = co; }
    void set_light_color(const NVGcolor& co) { light = co; }
    void set_radius(float r) { radius = r; }
    void set_brightness(float brightness) { step_bright = bright; bright = brightness; }
};

Blip* createBlipCentered(
    float x, float y,
    const char * tip = nullptr,
    const NVGcolor& light = GetStockColor(StockColor::Azure),
    const NVGcolor& rim = RampGray(G_50),
    const NVGcolor& hole = nvgRGBA(0, 0, 0, 0)
);

Blip* createBlipCentered(
    Vec pos, const char * tip,
    const NVGcolor& light = GetStockColor(StockColor::Azure),
    const NVGcolor& rim = RampGray(G_50),
    const NVGcolor& hole = nvgRGBA(0, 0, 0, 0)
    );
}
