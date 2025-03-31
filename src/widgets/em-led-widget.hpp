#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../em/wrap-HakenMidi.hpp"
#include "../services/misc.hpp"
using namespace pachde;
namespace pachde {

struct OctaveShiftLeds {
    uint8_t shift{60};

    void set_shift(uint8_t shift_value) {
        shift = shift_value;
    }
    bool is_minus2() { return shift < 48; }
    bool is_minus1() { return in_range(shift, U8(48), U8(59)); }
    bool is_plus1() { return in_range(shift, U8(61), U8(72)); }
    bool is_plus2() { return shift > 72; }

    void update_lights(Module * module, int first_light) {
        if (!module) return;
        bool m2 = is_minus2();
        bool m1 = is_minus1();
        bool p1 = is_plus1();
        bool p2 = is_plus2();
        module->getLight(first_light++).setBrightness(m2);
        module->getLight(first_light++).setBrightness(m1);
        module->getLight(first_light++).setBrightness(m2);
        module->getLight(first_light++).setBrightness(p2);
        module->getLight(first_light++).setBrightness(p1);
        module->getLight(first_light).setBrightness(p2);
    }
};

template <typename TParentWidget>
void create_octave_shift_leds(TParentWidget* parent, float cx, float cy, float spread, Module * module, int first_light_id) {
    float x = cx - 3.5 * spread;
    parent->addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), module, first_light_id++)); x += spread;
    parent->addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), module, first_light_id++)); x += spread;
    parent->addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), module, first_light_id++)); x += spread;
    x += spread;
    parent->addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), module, first_light_id++)); x += spread;
    parent->addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), module, first_light_id++)); x += spread;
    parent->addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), module, first_light_id++));
}

// Rounding
//
// Light ids must be contiguous and in the following order:
//   L_ROUND_Y,
//   L_ROUND_INITIAL,
//   L_ROUND,
//   L_ROUND_RELEASE,
struct RoundingLeds {
    uint8_t rate{0};
    bool initial{false};
    uint8_t mode{Haken::rNormal};

    void set_rate(uint8_t em_round_rate) { rate = em_round_rate; }
    void set_mode(uint8_t em_round_mode) { mode = em_round_mode; }
    void set_initial(bool em_round_initial) { initial = em_round_initial; }
    void update_lights(Module * module, int first_light) {
        if (!module) return;
        bool on_y    = mode >= Haken::rViaY;
        bool release = mode <= Haken::rRelease;
        module->getLight(first_light++).setBrightness(on_y);
        module->getLight(first_light++).setBrightness(initial);
        module->getLight(first_light++).setBrightness(rate / 127.f);
        module->getLight(first_light).setBrightness((rate > 0) && release);
    }
};

template <typename TParentWidget>
void create_rounding_leds(TParentWidget* parent, float cx, float cy, float spread, Module * module, int first_light_id)
{
    float x = cx - 1.5 * spread;
    parent->addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), module, first_light_id++)); x += spread;
    parent->addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), module, first_light_id++)); x += spread;
    parent->addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), module, first_light_id++)); x += spread;
    parent->addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), module, first_light_id));
}

}