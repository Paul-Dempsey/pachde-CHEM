#include "knob-track-widget.hpp"
#include "../services/misc.hpp"
namespace pachde {

TrackWidget::TrackWidget() :
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

void TrackWidget::set_active(bool enable) { active = enable; }
void TrackWidget::set_value(float value) {
    this->value = clamp(value, min_value, max_value);
}
void TrackWidget::set_min_max_value(float min, float max) {
    assert(min < max);
    min_value = min;
    max_value = max;
}

bool TrackWidget::applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme)
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

void TrackWidget::drawLayer(const DrawArgs& args, int layer)
{
    Base::drawLayer(args, layer);
    if (1 != layer) return;
    if (rack::settings::rackBrightness > .95f) return;
    if (active) {
        //KnobTrack(args.vg, radius, radius, min_angle, max_angle, radius, track_width, track_color);
        float angle = M_PI - rescale(value, min_value, max_value, min_angle, max_angle);
        float x = radius + radius * std::sin(angle);
        float y = radius + radius * std::cos(angle);
        Dot(args.vg, x, y, dot_color, true, dot_radius);
        TrackGliss(args.vg, radius, radius, x, y, min_angle, max_angle, radius, track_width, dot_color);
        CircularHalo(args.vg, x, y, dot_radius +.25f, 3.25 * dot_radius, dot_color);
    }
}

void TrackWidget::draw(const DrawArgs& args)
{
    Base::draw(args);
    if (active) {
        KnobTrack(args.vg, radius, radius, min_angle, max_angle, radius, track_width, track_color);
        if (rack::settings::rackBrightness <= .95f) return;

        float angle = M_PI - rescale(value, min_value, max_value, min_angle, max_angle);
        float x = radius + radius * std::sin(angle);
        float y = radius + radius * std::cos(angle);
        Dot(args.vg, x, y, dot_color, true, dot_radius);
        TrackGliss(args.vg, radius, radius, x, y, min_angle, max_angle, radius, track_width, dot_color);
    } else if (inactive_track_color.a > 0.f) {
        KnobTrack(args.vg, radius, radius, min_angle, max_angle, radius, track_width, inactive_track_color);
    }
}

void TrackIndicator::drawLayer(const DrawArgs &args, int layer)
{
    if (1 != layer) return;
    if (rack::settings::rackBrightness > .95f) return;
    if (active) {
        float angle = M_PI - rescale(value, min_value, max_value, min_angle, max_angle);
        float x = radius + radius * std::sin(angle);
        float y = radius + radius * std::cos(angle);
        CircularHalo(args.vg, x, y, 1, 2 * dot_radius, dot_color);
        //KnobTrack(args.vg, radius, radius, min_angle, max_angle, radius, track_width, track_color);
        Line(args.vg, radius, radius, x, y, dot_color, track_width *.75);
        TrackGliss(args.vg, radius, radius, x, y, min_angle, max_angle, radius, track_width, dot_color);
    }
}

void TrackIndicator::draw(const DrawArgs &args)
{
    if (active) {
        KnobTrack(args.vg, radius, radius, min_angle, max_angle, radius, track_width, track_color);
        if (rack::settings::rackBrightness <= .95f) return;

        float angle = M_PI - rescale(value, min_value, max_value, min_angle, max_angle);
        float x = radius + radius * std::sin(angle);
        float y = radius + radius * std::cos(angle);
        Line(args.vg, radius, radius, x, y, dot_color, track_width *.75);
        TrackGliss(args.vg, radius, radius, x, y, min_angle, max_angle, radius, track_width, dot_color);

    } else if (inactive_track_color.a > 0.f) {
        KnobTrack(args.vg, radius, radius, min_angle, max_angle, radius, track_width, inactive_track_color);
    }
}
}