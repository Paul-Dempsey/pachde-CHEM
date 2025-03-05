#pragma once
#include <rack.hpp>
#include "../services/colors.hpp"
#include "../services/svgtheme.hpp"
#include "../services/misc.hpp"
using namespace ::rack;
using namespace svg_theme;

namespace pachde {

namespace track_constant {
    static constexpr const float default_radius{18.f};
    static constexpr const float default_track_width{1.25f};
    static constexpr const float default_dot_radius{2.25f};
    static const NVGcolor default_track_color {nvgRGB(0x99, 0x69, 0x33)};
    static const NVGcolor default_dot_color {nvgRGB(0xf9, 0xa5, 0x4b)};
    static const NVGcolor default_inactive_color {0,0,0,0};
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
    bool active;
    const char * track_key;
    const char * inactive_key;

    NVGcolor track_color;
    NVGcolor dot_color;
    NVGcolor inactive_track_color;

    TrackWidget() :
        value(0.f),
        radius(18.f), 
        track_width(track_constant::default_track_width),
        dot_radius(track_constant::default_dot_radius),
        min_angle(),
        max_angle(),
        active(false),
        track_key(track_constant::default_track_key),
        inactive_key(track_constant::default_inactive_track_key),
        track_color(track_constant::default_track_color),
        dot_color(track_constant::default_dot_color),
        inactive_track_color(track_constant::default_inactive_color) // no track
    {
        box.size = {radius, radius};
    }

    void set_active(bool enable) { active = enable; }
    void set_value(float value) {
        assert(in_range(value, 0.f, 10.f));
        this->value = value;
    }

    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        auto style = theme->getStyle(track_key);
        if (style) {
            if (style->isApplyStroke()) {
                track_color = fromPacked(style->strokeWithOpacity());
                track_width = style->isApplyStrokeWidth() ? style->stroke_width: 1.25f;
                dot_radius = std::max(1 + track_width, dot_radius);
            } else {
                track_color = track_constant::default_track_color;
            }
            if (style->isApplyFill()) {
                dot_color = fromPacked(style->fillWithOpacity());
            } else {
                dot_color = track_constant::default_dot_color;
            }
        } else {
            track_color = track_constant::default_track_color;
            dot_color = track_constant::default_dot_color;
        }

        style = theme->getStyle(inactive_key);
        if (style) {
            if (style->isApplyStroke()) {
                inactive_track_color = fromPacked(style->strokeWithOpacity());
            } else if (style->isApplyFill()) {
                inactive_track_color = fromPacked(style->fillWithOpacity());
            } else {
                inactive_track_color = track_constant::default_inactive_color;
            }
        } else {
            inactive_track_color = track_constant::default_inactive_color;
        }
        return true;
    }

    void drawLayer(const DrawArgs& args, int layer) override
    {
        Base::drawLayer(args, layer);
        if (1 != layer) return;
        if (rack::settings::rackBrightness > .95f) return;
        if (active) {
            KnobTrack(args.vg, radius, radius, min_angle, max_angle, radius, track_width, track_color);

            float angle = M_PI - rescale(value, 0.f, 10.f, min_angle, max_angle);
            float xdot = radius * std::sin(angle);
            float ydot = radius * std::cos(angle);
            Dot(args.vg, radius + xdot, radius + ydot, dot_color, true, dot_radius);
            CircularHalo(args.vg, radius + xdot, radius + ydot, dot_radius +.25f, 3.25 * dot_radius, dot_color);
        }
    }

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);
        if (active) {
            if (rack::settings::rackBrightness <= .95f) return;
            KnobTrack(args.vg, radius, radius, min_angle, max_angle, radius, track_width, track_color);

            float angle = M_PI - rescale(value, 0.f, 10.f, min_angle, max_angle);
            float xdot = radius * std::sin(angle);
            float ydot = radius * std::cos(angle);
            Dot(args.vg, radius + xdot, radius + ydot, dot_color, true, dot_radius);

        } else if (inactive_track_color.a > 0.f) {
            KnobTrack(args.vg, radius, radius, min_angle, max_angle, radius, track_width, inactive_track_color);
        }
    }
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

}