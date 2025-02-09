// Copyright (C) Paul Chase Dempsey
#include "label-widget.hpp"

// $TODO: labels need to be cleaned up:
// - reduce the number of label types

namespace pachde {

BasicTextLabel::BasicTextLabel()
:   _color(RampGray(G_90)),
    bright(false)
{
}

// IApplyTheme
bool BasicTextLabel::applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme)
{
    _color = fromPacked(theme_engine.getFillColor(theme->name, this->_style.key, true));
    if (!isColorVisible(_color)) {
        _color = RampGray(G_85);
    }
    return true;
}

void BasicTextLabel::render(const DrawArgs& args)
{
    if (_text.empty()) return;

    auto vg = args.vg;
    auto font = _style.bold ? GetPluginFontSemiBold() : GetPluginFontRegular();
    if (!FontOk(font)) return;

    nvgSave(vg);
    SetTextStyle(vg, font, _color, _style.height);
    switch (_style.align) {
    case TextAlignment::Left:
        nvgTextAlign(vg, NVG_ALIGN_TOP|NVG_ALIGN_LEFT);
        nvgText(vg, 0.f, 0.f, _text.c_str(), nullptr);
        break;
    case TextAlignment::Center:
        nvgTextAlign(vg, NVG_ALIGN_TOP|NVG_ALIGN_CENTER);
        nvgText(vg, box.size.x * .5, 0.f, _text.c_str(), nullptr);
        break;
    case TextAlignment::Right:
        nvgTextAlign(vg, NVG_ALIGN_TOP|NVG_ALIGN_RIGHT);
        nvgText(vg, box.size.x, 0.f, _text.c_str(), nullptr);
        break;
    }
    nvgRestore(vg);
}

void BasicTextLabel::drawLayer(const DrawArgs& args, int layer)
{
    Base::drawLayer(args, layer);

    if (1 != layer) return;
    if (!bright) return;
    render(args);
}

void BasicTextLabel::draw(const DrawArgs& args)
{
    Base::draw(args);

    if (bright) return;
    render(args);
}

}