#pragma once
#include "TipWidget.hpp"
#include "../services/svt_rack.hpp"
#include "../services/colors.hpp"
using namespace svg_theme;

namespace pachde {

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
    Blip(NVGcolor hole, NVGcolor rim, NVGcolor light);

    void step() override;
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;

    void set_hole_color(NVGcolor co) { hole = co; }
    void set_rim_color(NVGcolor co) { rim = co; }
    void set_light_color(NVGcolor co) { light = co; }
    void set_radius( float r) { radius = r; }
    void set_brightness(float brightness) { step_bright = bright; bright = brightness; }
};

Blip* createBlipCentered(
    float x, float y,
    const char * tip = nullptr,
    NVGcolor light = GetStockColor(StockColor::Azure),
    NVGcolor rim = RampGray(G_50),
    NVGcolor hole = nvgRGBA(0, 0, 0, 0)
);

}
