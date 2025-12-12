#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "packed-color.hpp"
using namespace packed_color;

namespace pachde {

#define nvgGRAYf(L) nvgRGBf((L),(L),(L))
#define BLACK nvgGRAYf(0.0f)
#define WHITE nvgGRAYf(1.0f)
#define COLOR_NONE nvgRGBAf(0.0f,0.0f,0.0f,0.0f)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// HSL grays per InkScape
#define GRAY05 nvgRGB(0x0d, 0x0d, 0x0d) // #0d0d0d
#define GRAY08 nvgRGB(0x10, 0x10, 0x10) // #101010
#define GRAY10 nvgRGB(0x1a, 0x1a, 0x1a) // #1a1a1a
#define GRAY15 nvgRGB(0x26, 0x26, 0x26) // #262626
#define GRAY18 nvgRGB(0X30, 0X30, 0X30) // #303030
#define GRAY20 nvgRGB(0x33, 0x33, 0x33) // #333333
#define GRAY25 nvgRGB(0x40, 0x40, 0x40) // #404040
#define GRAY30 nvgRGB(0x4d, 0x4d, 0x4d) // #4d4d4d
#define GRAY35 nvgRGB(0x59, 0x59, 0x59) // #595959
#define GRAY40 nvgRGB(0x66, 0x66, 0x66) // #666666
#define GRAY45 nvgRGB(0x73, 0x73, 0x73) // #737373
#define GRAY50 nvgRGB(0x80, 0x80, 0x80) // #808080
#define GRAY55 nvgRGB(0x8c, 0x8c, 0x8c) // #8c8c8c
#define GRAY60 nvgRGB(0x99, 0x99, 0x99) // #999999
#define GRAY65 nvgRGB(0xa6, 0xa6, 0xa6) // #a6a6a6
#define GRAY70 nvgRGB(0xb2, 0xb2, 0xb2) // #b2b2b2
#define GRAY75 nvgRGB(0xbf, 0xbf, 0xbf) // #bfbfbf
#define GRAY80 nvgRGB(0xcc, 0xcc, 0xcc) // #cccccc
#define GRAY85 nvgRGB(0xd9, 0xd9, 0xd9) // #d9d9d9
#define GRAY90 nvgRGB(0xe5, 0xe5, 0xe5) // #e5e5e5
#define GRAY95 nvgRGB(0xf2, 0xf2, 0xf2) // #f2f2f2

enum Ramp {
    G_0, G_BLACK = G_0,
    G_05, G_08, G_10, G_15, G_18, G_20,
    G_25, G_30, G_35, G_40,
    G_45, G_50, G_55, G_60,
    G_65, G_70, G_75, G_80,
    G_85, G_90, G_95, G_100,
    G_WHITE = G_100
};
NVGcolor RampGray(Ramp g);

#define COLOR_BRAND    nvgRGB(0x45,0x7a,0xa6)  // #457aa6
#define COLOR_BRAND_MD nvgRGB(0x4e,0x8b,0xbf)  // #4e8dbf
#define COLOR_BRAND_HI nvgRGB(0xbd,0xd6,0xfc)  // #bdd6fc
#define COLOR_BRAND_LO nvgRGB(0x40,0x5a,0x80)  // #405a80
#define COLOR_MAGENTA  nvgRGB(0xbf,0x60,0xbe)  // #bf60be
#define COLOR_GREEN    nvgRGB(0x54,0xa7,0x54)  // #54a754
#define COLOR_GREEN_LO nvgRGB(0x39,0x73,0x3a)  // #39733a
#define COLOR_GREEN_HI nvgRGB(0xbd,0xfc,0xbd)  // #bdfcbd

// PORT_* colors are HSL 30-degree hue increments
#define PORT_RED     nvgHSL(0.f, 0.6f, 0.5f)           // #cc3233
#define PORT_ORANGE  nvgHSL(30.f/360.f, 0.80f, 0.5f)   // #e57f19
#define PORT_YELLOW  nvgHSL(60.f/360.f, 0.65f, 0.5f)   // #d2d22c
#define PORT_LIME    nvgHSL(90.f/360.f, 0.60f, 0.5f)   // #7fcc32
#define PORT_GREEN   nvgHSL(120.f/360.f, 0.5f, 0.5f)   // #3fbf3f
#define PORT_GRASS   nvgHSL(150.f/360.f, 0.5f, 0.5f)   // #3fbf7f
#define PORT_CYAN    nvgHSL(180.f/360.f, 0.5f, 0.5f)   // #3fbfbf
#define PORT_CORN    nvgHSL(210.f/360.f, 0.5f, 0.55f)  // #528cc5
#define PORT_BLUE    nvgHSL(240.f/360.f, 0.5f, 0.55f)  // #5252c5
#define PORT_VIOLET  nvgHSL(270.f/360.f, 0.5f, 0.5f)   // #7f3fbf
#define PORT_MAGENTA nvgHSL(300.f/360.f, 0.5f, 0.5f)   // #bf3fbf
#define PORT_PINK    nvgHSL(330.f/360.f, 0.65f, 0.65f) // #df6ba5
#define PORT_DEFAULT nvgHSL(210.f/360.f, 0.5f, 0.65f)  // #79a5d2

#define PORT_LIGHT_ORANGE  nvgHSL(30.f/360.f, 0.80f, 0.75f)  // #f2bf8c
#define PORT_LIGHT_LIME    nvgHSL(90.f/360.f, 0.75f, 0.75f)  // #bfef8f
#define PORT_LIGHT_VIOLET  nvgHSL(270.f/360.f, 0.75f, 0.75f) // #bf8fef

extern const NVGcolor panel_border_color;
extern const NVGcolor blue_light;
extern const NVGcolor green_light;
extern const NVGcolor bright_green_light;
extern const NVGcolor blue_green_light;
extern const NVGcolor orange_light;
extern const NVGcolor yellow_light;
extern const NVGcolor red_light;
extern const NVGcolor white_light;
extern const NVGcolor purple_light;
extern const NVGcolor gray_light;
extern const NVGcolor no_light;

#define IS_SAME_COLOR(p,q) (((p).r == (q).r) && ((p).g == (q).g) && ((p).b == (q).b) && ((p).a == (q).a))
inline NVGcolor Overlay(const NVGcolor &color, float transparency = 0.2f) { return nvgTransRGBAf(color, transparency); }
inline NVGcolor Gray(float L) {
    NVGcolor color;
    color.r = color.b = color.g = L;
    color.a = 1.0f;
    return color;
}

inline NVGcolor nvgHSLAf(float h, float s, float l, float a)
{
    NVGcolor color = nvgHSL(h,s,l);
    color.a = a;
    return color;
}

inline NVGcolor fromPacked(PackedColor co)
{
    return nvgRGBA(co & 0xff, (co >> 8) & 0xff, (co >> 16) & 0xff, (co >> 24) & 0xff);
}

inline PackedColor toPacked(const NVGcolor& co) {
    return packRgba(
        static_cast<uint32_t>(co.r * 255),
        static_cast<uint32_t>(co.g * 255),
        static_cast<uint32_t>(co.b * 255),
        static_cast<uint32_t>(co.a * 255));
}

struct NamedColor {
    const char * name;
    PackedColor color;
};
extern NamedColor stock_colors[];
enum class StockColor {
    pachde_blue,
    pachde_blue_dark,
    pachde_blue_medium,
    pachde_blue_light,
    pachde_default_port,
    None,
    Black,
    Gray_05p,
    Gray_10p,
    Gray_15p,
    Gray_20p,
    Gray_25p,
    Gray_30p,
    Gray_35p,
    Gray_40p,
    Gray_45p,
    Gray_50p,
    Gray_55p,
    Gray_60p,
    Gray_65p,
    Gray_70p,
    Gray_75p,
    Gray_80p,
    Gray_85p,
    Gray_90p,
    Gray_95p,
    White,
    VCV_Rack_white,
    Red,
    Green,
    Blue,
    Yellow,
    Magenta,
    Cyan,
    Alice_blue,
    Antique_white,
    Aqua,
    Aquamarine_Medium,
    Aquamarine,
    Azure,
    Beige,
    Bisque,
    Blanched_almond,
    Blue_Dark,
    Blue_Light,
    Blue_Medium,
    Blue_violet,
    Brown,
    Burlywood,
    Cadet_blue,
    Chartreuse,
    Chocolate,
    Coral_Light,
    Coral,
    Cornflower_blue,
    Cornsilk,
    Crimson,
    Cyan_Dark,
    Cyan_Light,
    Deep_pink,
    Deep_sky_blue,
    Dim_gray,
    Dodger_blue,
    Firebrick,
    Floral_white,
    Forest_green,
    Fuchsia,
    Gainsboro,
    Ghost_white,
    Gold,
    Goldenrod_Dark,
    Goldenrod_Light,
    Goldenrod_Pale,
    Goldenrod,
    Gray_Dark,
    Gray_Light,
    Green_Dark,
    Green_Light,
    Green_Pale,
    Green_yellow,
    Honeydew,
    Hot_pink,
    Indian_red,
    Indigo,
    Ivory,
    Khaki_Dark,
    Khaki,
    Lavender_blush,
    Lavender,
    Lawngreen,
    Lemon_chiffon,
    Lime_green,
    Lime,
    Linen,
    Magenta_Dark,
    Maroon,
    Midnight_blue,
    Mint_cream,
    Misty_rose,
    Moccasin,
    Navajo_white,
    Navy,
    Old_lace,
    Olive_drab,
    Olive_green_Dark,
    Olive,
    Orange_Dark,
    Orange_red,
    Orange,
    Orchid_Dark,
    Orchid_Medium,
    Orchid,
    Papaya_whip,
    Peach_puff,
    Peru,
    Pink_Light,
    Pink,
    Plum,
    Powder_blue,
    Purple_Medium,
    Purple,
    Red_Dark,
    Rosy_brown,
    Royal_blue,
    Saddle_brown,
    Salmon_Dark,
    Salmon_Light,
    Salmon,
    Sandy_brown,
    Sea_green_Dark,
    Sea_green_Light,
    Sea_green_Medium,
    Sea_green,
    Seashell,
    Sienna,
    Silver,
    Sky_blue_Light,
    Sky_blue,
    Slate_blue_Dark,
    Slate_blue_Medium,
    Slate_blue,
    Slate_gray_Dark,
    Slate_gray_Light,
    Slate_gray,
    Snow,
    Spring_green_Medium,
    Spring_green,
    Steel_blue_Light,
    Steel_blue,
    Tan,
    Teal,
    Thistle,
    Tomato,
    Turquoise_Dark,
    Turquoise_Medium,
    Turquoise_Pale,
    Turquoise,
    Violet_Dark,
    Violet_red_Medium,
    Violet_red_Pale,
    Violet,
    Wheat,
    White_smoke,
    Yellow_Light,
    Yellow_green,
    STOCK_COLOR_COUNT
};
inline NVGcolor GetStockColor(StockColor id) {
    if (static_cast<int>(id) < 0 || id > StockColor::STOCK_COLOR_COUNT) return COLOR_NONE;
	return fromPacked(stock_colors[static_cast<int>(id)].color);
}
inline PackedColor GetPackedStockColor(StockColor id) {
    if (static_cast<int>(id) < 0 || id > StockColor::STOCK_COLOR_COUNT) return 0;
	return stock_colors[static_cast<int>(id)].color;
}

// https://en.wikipedia.org/wiki/HSL_and_HSV
// https://stackoverflow.com/questions/596216/formula-to-determine-perceived-brightness-of-rgb-color

// inline float sRGBToLinear(float c) {
//     //rack::simd::pow
//     return c <= 0.04045 ? c / 12.92 : std::pow(((c + 0.055f)/1.055f), 2.4f);
// }

// inline float Luminance (NVGcolor color) {
//     return 0.2126f * sRGBToLinear(color.r) + 0.7152f * sRGBToLinear(color.g) + 0.0722f * sRGBToLinear(color.b);
// }

constexpr const float PI = 3.14159265358979323846;
constexpr const float TWO_PI = 2.0f * PI;
constexpr const float SQRT3 = 1.732050807568877f;

// stb linearizes on load so we don't have to gamma/correct
inline float LuminanceLinear(const NVGcolor& color) {
    return 0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b;
}

inline float Lightness(const NVGcolor& color) {
    return (std::max(std::max(color.r, color.g), color.b) + std::min(std::min(color.r, color.g), color.b))/2.f;
}

// BUGBUG: these functions don't generate values that round trip with nvgHSL/nvgHSLA,
// needed for Copper

inline float Saturation(const NVGcolor& color) {
    return std::max(std::max(color.r, color.g), color.b) - std::min(std::min(color.r, color.g), color.b);
}

inline float Chroma(const NVGcolor& color) {
    return std::max(std::max(color.r, color.g), color.b) - std::min(std::min(color.r, color.g), color.b);
}

inline float Saturation1(const NVGcolor& color) {
    auto L = Lightness(color);
    if (0.f == L || 1.0f == L) return 0;
    return Chroma(color)/(1.f - (2.f * L - 1.f));
}

float Hue1(const NVGcolor& color);

inline float Hue(const NVGcolor& color) {
    return (rack::simd::atan2(SQRT3 * (color.g - color.b), 2 * color.r - color.g - color.b) + PI) / TWO_PI;
}

inline bool isColorTransparent(const NVGcolor& color) { return color.a < 0.001f; }
inline bool isColorVisible(const NVGcolor& color) { return color.a > 0.001f; }

inline NVGcolor RandomColor() { return nvgRGBAf(random::uniform(), random::uniform(), random::uniform(), random::uniform()); }
inline NVGcolor RandomOpaqueColor() { return nvgRGBAf(random::uniform(), random::uniform(), random::uniform(), 1.0f); }
void FillRect(NVGcontext *vg, float x, float y, float width, float height, NVGcolor color);
void GradientRect(NVGcontext * vg, float x, float y, float width, float height, const NVGcolor& top, const NVGcolor& bottom, float y1, float y2);
void RoundRect(NVGcontext *vg, float x, float y, float width, float height, NVGcolor color, float radius);
enum class Fit { Inside, Outside, None };
void FittedBoxRect(NVGcontext *vg, float x, float y, float width, float height, const NVGcolor& color, Fit fit, float strokeWidth = 1.0);
void BoxRect(NVGcontext *vg, float x, float y, float width, float height, NVGcolor color, float strokeWidth = 1.0);
void RoundBoxRect(NVGcontext *vg, float x, float y, float width, float height, NVGcolor color, float radius, float strokeWidth = 1.0);
void Line(NVGcontext * vg, float x1, float y1, float x2, float y2, NVGcolor color, float strokeWidth = 1.0);
void CircleGradient(NVGcontext * vg, float cx, float cy, float r, NVGcolor top, NVGcolor bottom);
void Circle(NVGcontext * vg, float cx, float cy, float r, NVGcolor fill);
void OpenCircle(NVGcontext * vg, float cx, float cy, float r, NVGcolor stroke, float stroke_width = 1.f);
void Dot(NVGcontext*vg, float x, float y, const NVGcolor& co, bool filled = true, float radius = 2.25f, float stroke_width = .5f);
void CircularHalo(NVGcontext* vg, float cx, float cy, float inner_radius, float halo_radius, const NVGcolor& haloColor);
void Halo(NVGcontext* vg, float cx, float cy, float inner_radius, float halo_radius, const NVGcolor& haloColor, float fade = 1.0f);
void KnobTrack(NVGcontext* vg, float cx, float cy, float minAngle, float maxAngle, float track_radius, float track_width, const NVGcolor& color);
void TrackGliss(NVGcontext* vg, float cx, float cy, float xg, float yg, float minAngle, float maxAngle, float track_radius, float track_width, const NVGcolor& color);

template <class TMenuItem = rack::ui::MenuEntry>
rack::ui::MenuEntry* createColorMenuItem(
    PackedColor previewColor,
    std::string text,
    std::string rightText,
    std::function<bool()> checked,
    std::function<void()> action,
    bool disabled = false,
    bool alwaysConsume = false
    )
{
    struct ColorItem : rack::ui::MenuEntry {
        NVGcolor preview;
        rack::ui::MenuItem* check_menu;

        void step() override {
            MenuEntry::step();
            box.size.x = check_menu->box.size.x + 12;
            box.size.y = check_menu->box.size.y;
        }

        void draw(const rack::widget::Widget::DrawArgs& args) override
        {
            MenuEntry::draw(args);
            FillRect(args.vg, 0.f, 1.f, 12.f, box.size.y - 2.f, preview);
        }

    };

	ColorItem* item = new ColorItem;
    item->preview = fromPacked(previewColor);

    rack::ui::MenuItem* child = rack::createCheckMenuItem(text, rightText, checked, action, disabled, alwaysConsume);
    child->box.pos.x = 12.f;
    item->addChild(child);
    item->check_menu = child;

	return item;
}


}
