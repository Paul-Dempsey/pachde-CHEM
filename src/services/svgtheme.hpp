#pragma once
#ifndef SVG_THEME_H
#define SVG_THEME_H
#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <nanosvg.h>
#include <jansson.h>
#include <rack.hpp>

namespace svg_theme {

// nanosvg colors are 8-bit (0-255) abgr packed into an unsigned int.
typedef unsigned int PackedColor;
const PackedColor NoColor = 0;

enum Severity { Info, Warn, Error, Critical };
const char * SeverityName(Severity sev);

enum ErrorCode {
    Unspecified                  = 0,
    NoError                      = 1,
    CannotOpenJsonFile           = 2,
    JsonParseFailed              = 3,
    ArrayExpected                = 4,
    ObjectExpected               = 5,
    ObjectOrStringExpected       = 6,
    StringExpected               = 7,
    NumberExpected               = 8,
    IntegerExpected              = 9,
    NameExpected                 = 10,
    ThemeExpected                = 11,
    InvalidHexColor              = 12,
    OneOfColorOrGradient         = 13,
    TwoGradientStopsMax          = 14,
    GradientStopIndexZeroOrOne   = 15,
    GradientStopNotPresent       = 16,
    RemovingGradientNotSupported = 17,
    GradientNotPresent           = 18,
};

// optional logging callback function you provide.
typedef std::function<void(Severity severity, ErrorCode code, std::string info)> LogCallback;

struct GradientStop {
    int index = -1;
    float offset = 0.f;
    PackedColor color = 0;
 
    GradientStop() {}
    GradientStop(int i, float off, PackedColor co)  : index(i), offset(off), color(co) {}
};

struct Gradient {
    int nstops = 0;
    GradientStop stops[2];
};

enum class PaintKind { Unset, Color, Gradient, None };
class Paint {
    PaintKind kind = PaintKind::Unset;
    union {
        PackedColor color;
        Gradient gradient;
    };

public:
    Paint() {}
    Paint(PackedColor color) { setColor(color); }
    Paint(const Gradient& gradient) { setGradient(gradient); }

    PaintKind Kind() { return kind; }
    void setColor(PackedColor new_color) {
        kind = PaintKind::Color;
        color = new_color;
    }
    void setGradient(const Gradient& g) {
        kind = PaintKind::Gradient;
        gradient = g;
    }
    void setNone() {
        kind = PaintKind::None;
    }
    bool isColor() { return kind == PaintKind::Color; }
    bool isGradient() { return kind == PaintKind::Gradient; }
    bool isNone()  { return kind == PaintKind::None; }
    PackedColor getColor() { return isColor() ? color : 0; }
    const Gradient* getGradient() { return isGradient() ? &gradient : nullptr; }
    bool isApplicable() { return kind != PaintKind::Unset; }
};

struct Style {
    Paint fill;
    Paint stroke;
	float opacity = 1.f;
	float stroke_width = 1.f;
    bool apply_stroke_width = false;
    bool apply_opacity = false;

    void setFill(Paint paint) { fill = paint; }
    void setStroke(Paint paint) { stroke = paint; }
    void setOpacity(float alpha) {
        opacity = alpha;
        apply_opacity = true;
    }
    void setStrokeWidth(float width) {
        stroke_width = width;
        apply_stroke_width = true;
    }
    bool isApplyFill() { return fill.isApplicable(); }
    bool isApplyStroke() { return stroke.isApplicable(); }
    bool isApplyOpacity() { return apply_opacity; }
    bool isApplyStrokeWidth() { return apply_stroke_width; }
};

struct SvgTheme {
    std::string name;
    std::string file;
    std::unordered_map<std::string, std::shared_ptr<Style>> styles;

    std::shared_ptr<Style> getStyle(const std::string &name) {
        auto found = styles.find(name);
        if (found != styles.end()) {
            return found->second;
        }
        return nullptr;
    }

    PackedColor getFillColor(const char *name)
    {
        auto sty = getStyle(name);
        if (sty && sty->fill.isApplicable()) {
            return sty->fill.getColor();
        }
        return NoColor;
    }
};


class SvgThemeEngine
{
public:

    // Set a logging callback to receive more detailed information, warnings,
    // and errors when working with svg themes.
    void setLog(LogCallback log) { this->log = log; }

    // load themes from the specified file.
    bool load(const std::string& filename);

    // true if any themes are available after calling load.
    bool isLoaded() { return !themes.empty(); }

    // Get a theme by name
    std::shared_ptr<SvgTheme> getTheme(const std::string& name);

    // load a themed Svg and cache it
    std::shared_ptr<::rack::window::Svg> loadSvg(const std::string& filename, const std::shared_ptr<SvgTheme> theme);

    // Loads an SVG file and applies the theme. If a cached version is available, it is returned directly.
    // This uses an alternative SVG cache, keyed by SVG filename and theme.
    // Returns true if the SVG was newly loaded.
    bool applyTheme(std::shared_ptr<SvgTheme> theme, std::string svgFile, std::shared_ptr<rack::window::Svg>& svg);

    // Apply the theme to an NSVGImage
    // Most client code should not call this directly, to avoid contaminating a cached SVG 
    // (either the themed cache, or the global Rack cache).
    // Returns true if the SVG was modified.
    bool applyTheme(std::shared_ptr<SvgTheme> theme, NSVGimage* svg);

    // Get a list of themes defined in the style sheet
    std::vector<std::string> getThemeNames()
    {
        std::vector<std::string> result;
        for (auto theme: themes) {
            result.push_back(theme->name);
        }
        return result;
    }

    PackedColor getStockColor(const char *name);
    PackedColor getFillColor(const std::string& theme_name, const char *name)
    {
        auto theme = getTheme(theme_name);
        if (theme) {
            auto co = theme->getFillColor(name);
            if (co) return co;
        }
        return getStockColor(name);
    }

    void showCache();

private:

    static void LogNothing(Severity severity, ErrorCode code, std::string info) {}

    std::vector<std::shared_ptr<SvgTheme>> themes;
    std::map<std::string, PackedColor> colors;
    std::map<std::string, std::shared_ptr<::rack::window::Svg>> svgs;
    LogCallback log = LogNothing;

    void logInfo(std::string info) {
        log(Severity::Info, ErrorCode::NoError, info);
    }
    void logError(ErrorCode code, std::string info) {
        log(Severity::Error, code, info);
    }
    void logWarning(ErrorCode code, std::string info) {
        log(Severity::Warn, code, info);
    }
    bool requireValidHexColor(std::string hex, const char * name);
    bool requireArray(json_t* j, const char * name);
    bool requireObject(json_t* j, const char * name);
    bool requireObjectOrString(json_t* j, const char * name);
    bool requireString(json_t* j, const char * name);
    bool requireNumber(json_t* j, const char * name);
    bool requireInteger(json_t* j, const char * name);

    bool parseFill(json_t* root, std::shared_ptr<Style>);
    bool parseStroke(json_t* root, std::shared_ptr<Style>);
    bool parseOpacity(json_t* root, std::shared_ptr<Style>);
    bool parseStyle(const char * name, json_t* root, std::shared_ptr<SvgTheme> theme);
    bool parseTheme(json_t* root, std::shared_ptr<SvgTheme> theme);
    bool parseGradient(json_t* root, Gradient& gradient);
    bool parseColors(json_t* root);
    bool parseColor(const char * spec, const char *name, PackedColor* result);

    bool applyPaint(std::string tag, NSVGpaint & target, Paint& source);
    bool applyStroke(std::string tag, NSVGshape* shape, std::shared_ptr<Style> style);
    bool applyFill(std::string tag, NSVGshape* shape, std::shared_ptr<Style> style);

};

// Widgets that support theming should implement IApplyTheme.
//
// IApplyTheme is what enables the VCV Rack helper ApplyChildrenTheme to update 
// all your UI to a new theme in one line of code.
//
// The modified flag returned by theme_engine.applyTheme(theme, ...) should be 
// propagated back to the caller, so it can can initiate the appropriate
// Dirty event as needed.
//
// Here's the complete example implementation of a themable SVG screw (not 
// including the svg and the theme). 
// See the Demo to see all the bits and pieces come together.
//
// ```cpp
// struct ThemeScrew : app::SvgScrew, svg_theme::IApplyTheme {
//     ThemeScrew() {
//         setSvg(Svg::load(asset::plugin(pluginInstance, "res/Screw.svg")));
//     }
//     // implement IApplyTheme
//     bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override
//     {
//         return theme_engine.applyTheme(theme, sw->svg->handle);
//     }
// };
// ```
//
struct IApplyTheme
{
    virtual bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) = 0;
};

// Implement IThemeHolder to enable the AppendThemeMenu helper from the
// VCV Rack helpers in `svt_rack.h`.
// This is usually most conveniently implemented by your module widget,
// as it is in the Demo.
struct IThemeHolder
{
    virtual std::string getThemeName() = 0;
    virtual void setThemeName(const std::string& theme_name) = 0;
};

} // namespace svg_theme
#endif //SVG_THEME_H

/* Copyright (C) 2023 Paul Chase Dempsey pcdempsey@live.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * This software depends on and extends nanosvg.
 * nanosvg, Copyright (c) 2013-14 Mikko Mononen memon@inside.org
 * See nanosvg.h for license information.
 * 
 * This software depends on Jansson for JSON deserialization
 * Jansson, Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org> licensed under the MIT license.
 *
 */
