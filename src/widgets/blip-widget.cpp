#include "blip-widget.hpp"

namespace pachde {

Blip::Blip() 
:   hole(no_light),
    rim(RampGray(G_50)),
    light(GetStockColor(StockColor::Azure)),
    bright(0.f)
{
    box.size.x = box.size.y = 16.f;
}

Blip::Blip(NVGcolor hole, NVGcolor rim, NVGcolor light)
:   hole(hole),
    rim(rim),
    light(light),
    bright(0.f)
{
    box.size.x = box.size.y = 16.f;
}

void Blip::drawLayer(const DrawArgs& args, int layer)
{
    TipWidget::drawLayer(args, layer);
    if (1 != layer) return;

    auto vg = args.vg;
    float cx = box.size.x * .5f;
    float cy = box.size.y * .5f;
        if (rack::settings::rackBrightness < .95f) {
            CircularHalo(vg, cx, cy, 3.25, cx, nvgTransRGBAf(light, rack::settings::haloBrightness));
        } else {
            Circle(vg, cx, cy, 3.f, nvgTransRGBAf(light, bright));
        }
}

void Blip::draw(const DrawArgs& args)
{
    TipWidget::draw(args);

    auto vg = args.vg;
    float cx = box.size.x * .5f;
    float cy = box.size.y * .5f;
    if (isColorVisible(hole)) {
        Circle(vg, cx, cy, 3.f, hole);
    }
    OpenCircle(vg, cx, cy, 3.25f, rim, 0.5f);

    if (isColorVisible(light) && (bright > 0.1f)) {
        if (rack::settings::rackBrightness >= .95f) {
            Circle(vg, cx, cy, 3.f, nvgTransRGBAf(light, bright));
        } else {
            CircularHalo(vg, cx, cy, 3.25, cx, nvgTransRGBAf(light, rack::settings::haloBrightness));
        }
    }
}

Blip* createBlipCentered(
    float x, float y, const char * tip,
    NVGcolor light, NVGcolor rim, NVGcolor hole
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