#pragma once
#include <rack.hpp>
#include "services/colors.hpp"
#include "services/svgtheme.hpp"
using namespace ::rack;
using namespace svg_theme;

namespace pachde {

namespace track_constant {
    static constexpr const float default_radius{18.f};
    static constexpr const float default_track_width{1.75f};
    static constexpr const float default_dot_radius{2.25f};
    static const NVGcolor default_track_color {nvgRGB(0x99, 0x69, 0x33)};
    static const NVGcolor default_dot_color {nvgRGB(0xf9, 0xa5, 0x4b)};
    static const NVGcolor default_inactive_color {nvgRGBA(0,0,0,0)};
    static const char * default_track_key {"k-track"};
    static const char * default_inactive_track_key {"k-track-na"};
}

struct TrackWidget : TransparentWidget, IApplyTheme
{
    using Base = TransparentWidget;

    float value;
    float radius;
    float track_width;
    float dot_radius;
    float min_angle;
    float max_angle;
    float min_value{0.f};
    float max_value{10.f};
    bool active;
    const char * track_key;
    const char * inactive_key;

    NVGcolor track_color;
    NVGcolor dot_color;
    NVGcolor inactive_track_color;

    TrackWidget();

    void set_active(bool enable);
    void set_value(float value);
    void set_min_max_value(float min, float max);
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override;
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;
};

struct TrackIndicator : TrackWidget {
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;
};

template <typename TTrack = TrackWidget>
TTrack * createTrackWidget(
    Knob * knob,
    SvgThemeEngine &engine,
    std::shared_ptr<SvgTheme> theme,
    const char * track_theme_key = track_constant::default_track_key,
    const char * inactive_theme_key = track_constant::default_inactive_track_key
    )
{
    auto o = new TTrack();
    if (track_theme_key) o->track_key = track_theme_key;
    if (inactive_theme_key) o->inactive_key = inactive_theme_key;
    o->applyTheme(engine, theme);

    o->min_angle = knob->minAngle;
    o->max_angle = knob->maxAngle;
    o->box = knob->box.grow({2.f, 2.f});
    o->radius = o->box.size.x * .5f;
    return o;
}

template <typename TTrack = TrackWidget>
TTrack * createTrackWidget(
    Vec pos,
    float knob_radius,
    float min_angle,
    float max_angle,
    SvgThemeEngine &engine,
    std::shared_ptr<SvgTheme> theme,
    const char * track_theme_key = track_constant::default_track_key,
    const char * inactive_theme_key = track_constant::default_inactive_track_key
    )
{
    auto o = new TTrack();
    if (track_theme_key) o->track_key = track_theme_key;
    if (inactive_theme_key) o->inactive_key = inactive_theme_key;
    o->applyTheme(engine, theme);
    o->box.pos = pos;
    o->box.size = Vec{knob_radius + 2.f, knob_radius + 2.f};
    o->min_angle = min_angle;
    o->max_angle = max_angle;
    o->radius = o->box.size.x * .5f;
    return o;
}


}