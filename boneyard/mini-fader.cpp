#include "mini-fader.hpp"
#include "../services/colors.hpp"

namespace pachde {

MiniFader::MiniFader() :
    thumb_key("mf-thumb"),
    cap_color(nvgRGB(0x40, 0x95, 0xbf)),
    stem_color(RampGray(G_55)),
    thumb_color(nvgRGB(0xc9, 0x1d, 0xc7)),
    cap_width(1.f),
    stem_width(1.f),
    thumb_height(3.f)
{
    box.size = Vec(8.f, 66.f);
}

void MiniFader::set_thumb(const char * key, NVGcolor default_color, float height)
{
    thumb_key = key;
    thumb_color = default_color;
    thumb_height = height;
}

void MiniFader::initParamQuantity()
{
    Base::initParamQuantity();
    auto pq = getParamQuantity();
	if (pq) {
        pq->snapEnabled = false;
        minimum = pq->getMinValue();
        maximum = pq->getMaxValue();
    } else {
        minimum = 0.f;
        maximum = 10.f;
    }
}

bool MiniFader::applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme)
{
    PackedColor co;
    auto style = theme->getStyle("mf-cap");
    if (style) {
        co = style->strokeWithOpacity();
        cap_color = isVisibleColor(co) ? fromPacked(co) : nvgRGB(0x40, 0x95, 0xbf); 
        cap_width = style->stroke_width;
    }

    style = theme->getStyle("mf-stem");
    if (style) {
        co = style->strokeWithOpacity();
        stem_color = isVisibleColor(co) ? fromPacked(co) : RampGray(G_55); 
        stem_width = style->stroke_width;
    }

    style = theme->getStyle(thumb_key);
    if (style) {
        co = style->fillWithOpacity();
        thumb_color = isVisibleColor(co) ? fromPacked(co) : nvgRGB(0xc9, 0x1d, 0xc7);
    }
    return false;
}

// void MiniFader::onHover(const HoverEvent &e)
// {
//     Base::onHover(e);
// }

void MiniFader::onDragStart(const DragStartEvent &e)
{
    Base::onDragStart(e);
}

void MiniFader::onDragMove(const DragMoveEvent &e)
{
    Base::onDragMove(e);
}

void MiniFader::onDragEnd(const DragEndEvent &e)
{
    Base::onDragEnd(e);
}

void MiniFader::onButton(const ButtonEvent &e)
{
    //auto mods = (e.mods & RACK_MOD_MASK);
    Base::onButton(e);
}

void MiniFader::draw(const DrawArgs &args)
{
    auto vg = args.vg;

    auto pq = getParamQuantity();
    float track = box.size.y - 2 - thumb_height;
    float pos = pq ? rack::math::rescale(pq->getValue(), minimum, maximum, 0.f, track) : 0.f;
    
    float CENTER = box.size.x*.5f;
    Line(vg, CENTER, .75f, CENTER, box.size.y-.75, stem_color, stem_width);
    Line(vg, 1.f, .5f, box.size.x-1.f, .5, cap_color, cap_width);
    Line(vg, 1.f, box.size.y-1.f, box.size.x-1.f, box.size.y-1.f, cap_color, cap_width);

    FillRect(vg, 2.f, box.size.y-1.f - pos - thumb_height, box.size.x - 4.f, thumb_height, thumb_color);
}

MiniFader * createMiniFaderCentered(Vec pos, Module *module, int param_id, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    auto o = createParamCentered<MiniFader>(pos, module, param_id);
    o->applyTheme(engine, theme);
    return o;
}

}