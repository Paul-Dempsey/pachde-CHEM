#pragma once
#include "TipWidget.hpp"
#include "../services/svt_rack.hpp"
#include "../services/colors.hpp"
using namespace svg_theme;

namespace pachde {

struct Blip: TipWidget
{
    NVGcolor hole;
    NVGcolor rim;
    NVGcolor light;
    float bright;
    Blip();
    Blip(NVGcolor hole, NVGcolor rim, NVGcolor light);

    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;

    void setHole(NVGcolor co) { hole = co; }
    void setRim(NVGcolor co) { rim = co; }
    void setLight(NVGcolor co) { light = co; }
    void setBrightness(float brightness) { bright = brightness; }

};

Blip* createBlipCentered(
    float x, float y,
    const char * tip = nullptr,
    NVGcolor light = GetStockColor(StockColor::Azure),
    NVGcolor rim = RampGray(G_50),
    NVGcolor hole = nvgRGBA(0, 0, 0, 0)
    );


}
