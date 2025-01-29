// implementation of svg_theme
#include "svgtheme.hpp"
#include "svt_rack.hpp"

namespace svg_theme {

const char * SeverityName(Severity sev) {
    switch (sev) {
        case Severity::Info: return "Info";
        case Severity::Warn: return "Warn";
        case Severity::Error: return "Error";
        case Severity::Critical: return "Critical";
    }
    return "[unknown]";
}

std::string GetTag(NSVGshape* shape)
{
    if (!shape) return "";
    if (!shape->id[0]) return "";
    auto id = std::string(shape->id);
    auto dashes = id.rfind("--");
    if (dashes != std::string::npos) {
        id = id.substr(dashes + 2);
    }
    return id;
}

inline int hex_value(unsigned char ch) {
    if (ch > 'f' || ch < '0') { return -1; }
    if (ch <= '9') { return ch & 0xF; }
    if (ch < 'A') { return -1; }
    if (ch < 'G') { return 10 + ch - 'A'; }
    if (ch < 'a') { return -1; }
    return 10 + ch - 'a';
}

inline PackedColor PackRGB(unsigned int r, unsigned int g, unsigned int b) {
    return r | (g << 8) | (b << 16) | (255 << 24);
}
inline PackedColor PackRGBA(unsigned int r, unsigned int g, unsigned int b, unsigned int a) {
    return r | (g << 8) | (b << 16) | (a << 24);
}

bool isValidHexColor(std::string hex) {
    switch (hex.size()) {
        case 1 + 3: 
        case 1 + 4:
        case 1 + 6: 
        case 1 + 8: break;
        default: return false;
    }
    if (*hex.begin() != '#') return false;
    return std::string::npos == hex.find_first_not_of("0123456789ABCDEFabcdef", 1);
}

std::vector<unsigned char> ParseHex(std::string hex) {
    std::vector<unsigned char> result;

    // Color representations:
    //  short: #rgb, #rgba
    //  long: #rrggbb, #rrggbbaa
    bool long_hex = true;
    switch (hex.size()) {
        case 1 + 3: 
        case 1 + 4: long_hex = false; break;
        case 1 + 6: 
        case 1 + 8: long_hex = true; break;
        default: return result;
    }

    enum State { Hex = -1, R1, R2, G1, G2, B1, B2, A1, A2, End };
    int state = State::Hex;
    int value = 0;
    for (unsigned char ch: hex) {
        if (state == State::Hex) {
            if (ch == '#') {
                ++state;
            } else {
                return result;
            }
        } else {
            auto nibble = hex_value(ch);
            if (-1 == nibble) {
                result.clear();
                return result;
            }
            if (state & 1) { // odd
                value |= nibble;
                result.push_back(value);
                value = 0;
                ++state;
            } else { // even
                value = nibble << 4;
                if (long_hex) {
                    ++state;
                } else {
                    result.push_back(value);
                    value = 0;
                    state += 2;
                }
            }
        }
        if (state >= State::End) {
            break;
        }
    }
    return result;
}

std::string format_string(const char *fmt, ...)
{
    const int len = 256;
    std::string s(len, '\0');
    va_list args;
    va_start(args, fmt);
    auto r = std::vsnprintf(&(*s.begin()), len + 1, fmt, args);
    return r < 0 ? "??" : s;
}

const PackedColor OPAQUE_BLACK = 255 << 24;

void SvgThemeEngine::clear()
{
    themes.clear();
    colors.clear();
    svgs.clear();
}

std::shared_ptr<SvgTheme> SvgThemeEngine::getTheme(const std::string& name)
{
    auto r = std::find_if(themes.begin(), themes.end(), [=](const std::shared_ptr<SvgTheme> theme) {
        return 0 == theme->name.compare(name);
    });
    return r == themes.end() ? nullptr : *r;
}

bool SvgThemeEngine::requireValidHexColor(std::string hex, const char * name)
{
    if (isValidHexColor(hex)) return true;
    logError(ErrorCode::InvalidHexColor, format_string("'%s': invalid hex color: '%s'", name, hex.c_str()));
    return false;
}
bool SvgThemeEngine::requireArray(json_t* j, const char * name)
{
    if (json_is_array(j)) return true;
    logError(ErrorCode::ArrayExpected, format_string("'%s': array expected", name));
    return false;
}
bool SvgThemeEngine::requireObject(json_t* j, const char * name)
{
    if (json_is_object(j)) return true;
    logError(ErrorCode::ObjectExpected, format_string("'%s': object expected", name));
    return false;
}
bool SvgThemeEngine::requireObjectOrString(json_t* j, const char * name)
{
    if (json_is_object(j) || json_is_string(j)) return true;
    logError(ErrorCode::ObjectOrStringExpected, format_string("'%s': Object or string expected", name));
    return false;
}
bool SvgThemeEngine::requireString(json_t* j, const char * name)
{
    if (json_is_string(j)) return true;
    logError(ErrorCode::StringExpected, format_string("'%s': String expected", name));
    return false;
}
bool SvgThemeEngine::requireNumber(json_t* j, const char * name)
{
    if (json_is_number(j)) return true;
    logError(ErrorCode::NumberExpected, format_string("'%s': Number expected", name));
    return false;
}
bool SvgThemeEngine::requireInteger(json_t* j, const char * name)
{
    if (json_is_integer(j)) return true;
    logError(ErrorCode::IntegerExpected, format_string("'%s': Integer expected", name));
    return false;
}

PackedColor parseHexColor(const char * text)
{
    auto parts = ParseHex(text);
    if (parts.size() == 3) {
        return PackRGB(parts[0], parts[1], parts[2]);
    }
    if (parts.size() == 4) {
        return PackRGBA(parts[0], parts[1], parts[2], parts[3]);
    }
    // user error coding the color
    return OPAQUE_BLACK;
}

float getNumber(json_t * j)
{
    if (json_is_real(j)) return json_real_value(j);
    if (json_is_number(j)) return json_number_value(j);
    if (json_is_integer(j)) return static_cast<float>(json_integer_value(j));
    return 1.f;
}

bool SvgThemeEngine::parseColor(const char * spec, const char *name, PackedColor* result)
{
    if (isValidHexColor(spec)) {
        *result = parseHexColor(spec);
        return true;
    }

    auto item = colors.find(spec);
    if (item != colors.end()) {
        *result = item->second;
        return true;
    }

    *result = OPAQUE_BLACK;
    return requireValidHexColor(spec, name);
}

bool SvgThemeEngine::parseOpacity(json_t * root, std::shared_ptr<Style> style)
{
    auto oopacity = json_object_get(root, "opacity");
    if (oopacity) {
        if (!requireNumber(oopacity, "opacity")) return false;
        style->setOpacity(std::max(0.f, std::min(1.f, getNumber(oopacity))));
    }
    return true;
}

bool SvgThemeEngine::parseGradient(json_t* ogradient, Gradient& gradient)
{
    bool ok = true;
    gradient.nstops = 0;
    if (ogradient) {
        if (!requireArray(ogradient, "gradient")) return false;

        int index = 0;
        PackedColor color = OPAQUE_BLACK;
        float offset = 0.f;

        json_t * item;
        size_t n;
        json_array_foreach(ogradient, n, item) {
            if (n > 1) {
                logError(ErrorCode::TwoGradientStopsMax, "A maximum of two gradient stops is allowed");
                return false;
            }
            auto oindex = json_object_get(item, "index");
            if (oindex) {
                if (requireInteger(oindex, "index")) {
                    index = json_integer_value(oindex);
                    if (!(0 == index || 1 == index)) {
                        logError(ErrorCode::GradientStopIndexZeroOrOne, "Gradient stop index must be 0 or 1");
                        ok = false;
                    } 
                } else {
                    index = 0;
                    ok = false;
                }
            }
            
            auto ocolor = json_object_get(item, "color");
            if (ocolor) {
                if (requireString(ocolor, "color")) {
                    auto hex = json_string_value(ocolor);
                    if (!parseColor(hex, "color", &color)) {
                        ok = false;
                    }
                } else {
                    color = 0;
                    ok = false;
                }
            }
            
            auto ooffset = json_object_get(item, "offset");
            if (ooffset) {
                if (requireNumber(ooffset, "offset")) {
                    offset = getNumber(ooffset);
                } else {
                    offset = 0.f;
                    ok = false;
                }
            }

            if (ok) {
                gradient.stops[index] = GradientStop(index, offset, color);
            }
        }
        if (ok) {
            int count = 0;
            if (gradient.stops[0].index >= 0) ++count;
            if (gradient.stops[1].index >= 0) ++count;
            gradient.nstops = count;
        }
    }
    return ok;
}

bool SvgThemeEngine::parseFill(json_t* root, std::shared_ptr<Style> style)
{
    PackedColor color = OPAQUE_BLACK;

    auto ofill = json_object_get(root, "fill");
    if (!ofill) return true;
    if (!requireObjectOrString(ofill, "fill")) return false;
    if (json_is_string(ofill)) {
        auto value = json_string_value(ofill);
        if (0 == strcmp(value, "none")) {
            style->fill.setNone();
        } else {
            if (!parseColor(value, "fill", &color)) return false;
            style->fill.setColor(color);
        }
    } else {
        auto ocolor = json_object_get(ofill, "color");
        if (ocolor) {
            if (!requireString(ocolor, "color")) return false;
            auto hex = json_string_value(ocolor);
            if (!parseColor(hex, "color", &color)) return false;
            style->fill.setColor(color);
        }
        auto ogradient = json_object_get(ofill, "gradient");
        if (ogradient) {
            if (ocolor) {
                logError(ErrorCode::OneOfColorOrGradient, "'fill': Only one of 'color' or 'gradient' allowed");
                return false;
            }
            Gradient gradient;
            if (parseGradient(ogradient, gradient) && gradient.nstops > 0) {
                style->fill.setGradient(gradient);
            }
        }
    }
    return true;
}

bool SvgThemeEngine::parseStroke(json_t* root, std::shared_ptr<Style> style)
{
    PackedColor color = OPAQUE_BLACK;

    auto ostroke = json_object_get(root, "stroke");
    if (ostroke) {
        if (!requireObjectOrString(ostroke, "stroke")) return false;

        if (json_is_string(ostroke)) {
            auto value = json_string_value(ostroke);
            if (0 == strcmp(value, "none")) {
                style->stroke.setNone();
            } else {
            if (!parseColor(value, "stroke", &color)) return false;
                style->stroke.setColor(color);
            }
        } else {
            auto owidth = json_object_get(ostroke, "width");
            if (owidth) {
                if (!requireNumber(owidth, "width")) return false;
                style->setStrokeWidth(getNumber(owidth));
            }

            auto ocolor = json_object_get(ostroke, "color");
            if (ocolor) {
                if (!requireString(ocolor, "color")) return false;
                auto hex = json_string_value(ocolor);
                if (!parseColor(hex, "color", &color)) return false;
                style->stroke.setColor(color);
            }

            auto ogradient = json_object_get(ostroke, "gradient");
            if (ogradient) {
                if (ocolor) {
                    logError(ErrorCode::OneOfColorOrGradient, "'stroke': Only one of 'color' or 'gradient' allowed");
                    return false;
                }
                Gradient gradient;
                if (parseGradient(ogradient, gradient) && gradient.nstops > 0) {
                    style->stroke.setGradient(gradient);
                }
            }
        }
    }
    return true;
}

bool SvgThemeEngine::parseStyle(const char * name, json_t* root, std::shared_ptr<SvgTheme> theme)
{
    logInfo(format_string("Parsing '%s'", name));
    auto style = std::make_shared<Style>();
    if (!parseFill(root, style)) return false;
    if (!parseStroke(root, style)) return false;
    if (!parseOpacity(root, style)) return false;
    theme->styles[name] = style;
    return true;
}

bool SvgThemeEngine::parseTheme(json_t* root, std::shared_ptr<SvgTheme> theme)
{
    void * n = nullptr;
    const char* key = nullptr;
    json_t* j = nullptr;

    json_object_foreach_safe(root, n, key, j) {
        if (json_is_object(j)) {
            if (!parseStyle(key, j, theme)) return false;
        } else {
            logError(ErrorCode::ObjectExpected, format_string("Theme '%s': Each style must be an object", theme->name.c_str()));
            return false;
        }
    }
    return true;
}

bool SvgThemeEngine::parseColors(json_t* root) {
    void * n = nullptr;
    const char* key = nullptr;
    json_t* j = nullptr;

    json_object_foreach_safe(root, n, key, j) {
        if (json_is_string(j)) {
            auto value = json_string_value(j);
            if (!requireValidHexColor(value, key)) return false;
            this->colors.insert(std::make_pair(std::string(key), parseHexColor(value)));
        } else {
            logError(ErrorCode::StringExpected, "Expected a named color, e.g. \"Aqua\" : \"#00ffff\",");
        }
    }
    return true;
}

bool SvgThemeEngine::load(const std::string& filename)
{
    bool ok = true;
	FILE* file = std::fopen(filename.c_str(), "r");
	if (!file) {
        log(Severity::Critical, ErrorCode::CannotOpenJsonFile, filename.c_str());
        return false;
    }

	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root)
    {
        logError(ErrorCode::JsonParseFailed, format_string("Parse error - %s %d:%d %s",
            error.source, error.line, error.column, error.text));
        std::fclose(file);
        return false;
    }

    if (json_is_array(root)) {
        json_t * item; size_t n;
        json_array_foreach(root, n, item) {
            if (json_is_object(item)) {
                json_t* j;

                j = json_object_get(item, "colors");
                if (j) {
                    if (!parseColors(j)) {
                        ok = false;
                        break;
                    }
                    continue;
                }

                j = json_object_get(item, "name");
                const char * name = nullptr;
                if (j && json_is_string(j)) {
                    name = json_string_value(j);
                }
                if (name && *name) {
                    j = json_object_get(item, "theme");
                    if (j && json_is_object(j)) {
                        logInfo(format_string("Parsing theme '%s'", name));
                        auto theme = std::make_shared<SvgTheme>();
                        theme->name = name;
                        theme->file = filename;
                        if (parseTheme(j, theme)) {
                            themes.push_back(theme);
                        } else {
                            ok = false;
                            break;
                        }
                    } else {
                        logError(ErrorCode::ThemeExpected, "Expected a 'theme' object");
                        ok = false;
                        break;
                    }
                } else {
                    logError(ErrorCode::NameExpected, "Each theme must have a non-empty name");
                    ok = false;
                    break;
                }
            } else {
                logError(ErrorCode::ObjectExpected, "Expected a named 'theme' object");
                ok = false;
                break;
            }
        }
    } else {
        logError(ErrorCode::ArrayExpected, "The top level element must be an array");
        ok = false;
    }

	json_decref(root);
    std::fclose(file);
    if (!ok) {
        themes.clear();
    }
    return ok;
}

bool SvgThemeEngine::applyPaint(std::string tag, NSVGpaint & target, Paint& source)
{
    if (!source.isApplicable()) return false;

    switch (source.Kind()) {
        case PaintKind::None:
            if (target.type != NSVG_PAINT_NONE) {
                if ((target.type == NSVG_PAINT_RADIAL_GRADIENT)
                    || (target.type == NSVG_PAINT_LINEAR_GRADIENT)) {
                    logWarning(ErrorCode::RemovingGradientNotSupported, 
                        format_string("'%s': Removing gradient not supported (leaks memory)", tag.c_str()));
                    return false;
                }
                target.type = NSVG_PAINT_NONE;
                return true;
            }
            break;

        case PaintKind::Color: {
                auto source_color = source.getColor();
                if ((target.type != NSVG_PAINT_COLOR) || (target.color != source_color)) {
                    if ((target.type == NSVG_PAINT_RADIAL_GRADIENT)
                        || (target.type == NSVG_PAINT_LINEAR_GRADIENT)) {
                        logWarning(ErrorCode::RemovingGradientNotSupported, 
                             format_string("'%s': Removing gradient not supported (leaks memory)", tag.c_str()));
                        return false;
                    }
                    target.type = NSVG_PAINT_COLOR;
                    target.color = source_color;
                    return true;
                }
            }
            break;

        case PaintKind::Gradient: {
                auto gradient = source.getGradient();
                if (!gradient) return false; // unexpected - defensive

                if (!((target.type == NSVG_PAINT_RADIAL_GRADIENT)
                    || (target.type == NSVG_PAINT_LINEAR_GRADIENT))) {
                    logWarning(ErrorCode::GradientNotPresent, 
                        format_string("'%s': Skipping SVG element without a gradient", tag.c_str()));
                    return false;
                }

                bool changed = false;
                for (auto n = 0; n < gradient->nstops; ++n) {
                    const GradientStop& stop = gradient->stops[n];
                    if (stop.index > target.gradient->nstops) {
                        logWarning(ErrorCode::GradientStopNotPresent, 
                            format_string("'%s': Gradient stop %d not present in SVG", tag.c_str()));
                    } else {
                        NSVGgradientStop& target_stop = target.gradient->stops[stop.index];
                        if (target_stop.offset != stop.offset) {
                            target_stop.offset = stop.offset;
                            changed = true;
                        }
                        if (target_stop.color != stop.color) {
                            target_stop.color = stop.color;
                            changed = true;
                        }
                    }
                }
                return changed;
            }

        default:
            return false;
    }
    return false;
}


bool SvgThemeEngine::applyFill(std::string tag, NSVGshape* shape, std::shared_ptr<Style> style)
{
    return style->isApplyFill() ? applyPaint(tag, shape->fill, style->fill) : false;
}

bool SvgThemeEngine::applyStroke(std::string tag, NSVGshape* shape, std::shared_ptr<Style> style)
{
    return style->isApplyStroke() ? applyPaint(tag, shape->stroke, style->stroke) : false;
}

bool SvgThemeEngine::applyTheme(std::shared_ptr<SvgTheme> theme, NSVGimage* svg)
{
    if (!theme || !svg || !svg->shapes) return false;
    bool modified = false;
    for (NSVGshape* shape = svg->shapes; nullptr != shape; shape = shape->next) {
        std::string tag = GetTag(shape);
        if (tag.empty()) continue;
        auto style = theme->getStyle(tag);
        if (style) {
            if (style->isApplyOpacity() && (shape->opacity != style->opacity)) {
                shape->opacity = style->opacity;
                modified = true;
            }
            if (style->isApplyStrokeWidth() && (shape->strokeWidth != style->stroke_width)) {
                shape->strokeWidth = style->stroke_width;
                modified = true;
            }
            if (applyFill(tag, shape, style)) {
                modified = true;
            }
            if (applyStroke(tag, shape, style)) {
                modified = true;
            }
        }
    }
    return modified;
}

std::shared_ptr<::rack::window::Svg> SvgThemeEngine::loadSvg(const std::string& filename, const std::shared_ptr<SvgTheme> theme)
{
    auto key = filename;
    key.append( ":" + theme->name); 
	const auto& pair = svgs.find(key);
	if (pair != svgs.end())
		return pair->second;

	// Load svg
	std::shared_ptr<::rack::window::Svg> svg;
	try {
		svg = std::make_shared<::rack::window::Svg>();
		svg->loadFile(filename);
        applyTheme(theme, svg->handle);
	}
	catch (rack::Exception& e) {
		WARN("%s", e.what());
		svg = NULL;
	}
	svgs[key] = svg;
	return svg;
}


bool SvgThemeEngine::applyTheme(std::shared_ptr<SvgTheme> theme, std::string filename, std::shared_ptr<rack::window::Svg>& svg) {
	auto newSvg = this->loadSvg(filename, theme);
 
	if (newSvg && (newSvg != svg)) {
		svg = newSvg;
		return true;
	}
	return false;
}


void SvgThemeEngine::showCache() {
	unsigned int n = 0;
	for (auto entry : svgs) {
		DEBUG("%u %s %p", ++n, entry.first.c_str(), (entry.second).get());
	}
}


PackedColor SvgThemeEngine::getStockColor(const char *name)
{
    auto it = this->colors.find(name);
    if (it == colors.end()) return NoColor;
    return it->second;
}

// -----------------------------------------------------------------------
// svt_rack
//

void sendDirty(Widget* widget)
{
	EventContext cDirty;
	Widget::DirtyEvent eDirty;
	eDirty.context = &cDirty;
	widget->onDirty(eDirty);
}

bool ApplyChildrenTheme(Widget * widget, SvgThemeEngine& themes, std::shared_ptr<SvgTheme> theme, bool top)
{
    bool modified = false;

    for (Widget* child : widget->children) {
        if (ApplyChildrenTheme(child, themes, theme, false)) {
            modified = true;
        }
    }

    auto themed = dynamic_cast<IApplyTheme*>(widget);
    if (themed && themed->applyTheme(themes, theme)) {
        modified = true;
    }

    if (top && modified) {
        sendDirty(widget);
    }

    return modified;
}

void AppendThemeMenu(Menu* menu, IThemeHolder* holder, SvgThemeEngine& themes)
{
    auto theme_names = themes.getThemeNames();
    if (theme_names.empty()) return; // no themes

    std::vector<rack::ui::MenuItem*> menus;
    for (auto theme : theme_names) {
        menus.push_back(createCheckMenuItem(
            theme, "", [=]() { return 0 == theme.compare(holder->getThemeName()); },
            [=]() { holder->setThemeName(theme); }
        ));
    }
    for (auto child: menus) {
        menu->addChild(child);
    }    
}

}