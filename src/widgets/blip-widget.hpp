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

    Blip() 
    :   hole(no_light),
        rim(RampGray(G_50)),
        light(GetStockColor(StockColor::Azure)),
        bright(0.f)
    {
        box.size.x = box.size.y = 16.f;
    }
    Blip(NVGcolor hole, NVGcolor rim, NVGcolor light)
    :   hole(hole),
        rim(rim),
        light(light),
        bright(0.f)
    {
        box.size.x = box.size.y = 16.f;
    }

    void setHole(NVGcolor co) { hole = co; }
    void setRim(NVGcolor co) { rim = co; }
    void setLight(NVGcolor co) { light = co; }
    void setBrightness(float brightness) { bright = brightness; }
    void draw(const DrawArgs& args) override
    {
        auto vg = args.vg;
        float cx = box.size.x * .5f;
        float cy = box.size.y * .5f;
        if (isColorVisible(hole)) {
            Circle(vg, cx, cy, 3.f, hole);
        }
        OpenCircle(vg, cx, cy, 3.25f, rim, 0.5f);
        if (isColorVisible(light) && bright > 0.f) {
            CircularHalo(vg, cx, cy, 3.25, cx, nvgTransRGBAf(light, bright));
        }
    }
};

inline Blip* createBlipCentered(
    float x, float y,
    const char * tip = nullptr,
    NVGcolor light = GetStockColor(StockColor::Azure),
    NVGcolor rim = RampGray(G_50),
    NVGcolor hole = nvgRGBA(0, 0, 0, 0)
    )
{
    auto w = new Blip(hole, rim, light);
    w->box.pos.x = x;
    w->box.pos.y = y;
    if (tip) w->describe(tip);
    Center(w);
    return w;
}

}
