#include "blip-widget.hpp"

namespace pachde {

Blip::Blip() 
:   hole(no_light),
    rim(RampGray(G_50)),
    light(GetStockColor(StockColor::Blue)),
    bright(0.f)
{
    box.size.x = box.size.y = 16.f;
}

Blip::Blip(NVGcolor hole, NVGcolor rim, NVGcolor light)
:   hole(hole),
    rim(rim),
    light(light),
    bright(0.f),
    step_bright(0.f),
    radius(3.25f)
{
    box.size.x = box.size.y = 16.f;
}

void Blip::step() {
    const float inc = .001f;

    if (step_bright < bright) {
        step_bright = std::max(bright, step_bright + inc);
    } else if (step_bright > bright) {
        step_bright = std::min(bright, step_bright - inc);
    }
}

void Blip::drawLayer(const DrawArgs& args, int layer)
{
    TipWidget::drawLayer(args, layer);
    if (1 != layer) return;
    if (step_bright < 0.1f) return;
    if (rack::settings::rackBrightness >= .95f) return;

    auto vg = args.vg;
    float cx = box.size.x * .5f;
    float cy = box.size.y * .5f;
    CircularHalo(vg, cx, cy, radius, radius*3.f, nvgTransRGBAf(light, step_bright * rack::settings::haloBrightness));
}

void Blip::draw(const DrawArgs& args)
{
    TipWidget::draw(args);

    auto vg = args.vg;
    float cx = box.size.x * .5f;
    float cy = box.size.y * .5f;

    if (isColorVisible(hole)) {
        Circle(vg, cx, cy, radius - .25f, hole);
    }

    if (isColorVisible(rim)) {
        OpenCircle(vg, cx, cy, radius, rim, 0.5f);
    }

    if (isColorVisible(light) && (step_bright > 0.1f)) {
        if (rack::settings::rackBrightness >= .95f) {
            Circle(vg, cx, cy, radius - .25f, nvgTransRGBAf(light, step_bright));
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