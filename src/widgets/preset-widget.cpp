#include "preset-widget.hpp"
#include "../services/colors.hpp"

namespace pachde {

PresetWidget::PresetWidget()
:   preset_name(nullptr),
    grip_color(nvgRGBAf(0x9d, 0x91, 0x31, 0.2f)),
    dot_color(nvgRGB(0x46, 0x40, 0x60)),
    hover_grip(false),
    preset_list(nullptr),
    preset_index(-1),
    live(false),
    selected(false)
{
    box.size.x = 150.f;
    box.size.y = 21.f;
}

bool PresetWidget::applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme)
{
    if (!preset_name) {
        LabelStyle style{"preset", TextAlignment::Left, 14.f};
        preset_name = createStaticTextLabel<TipLabel>(Vec(2.5f, 2.5f), 145.f, "", theme_engine, theme, style);
        addChild(preset_name);
    }
    grip_color = fromPacked(theme->getFillColor("preset-grip", true));
    dot_color = fromPacked(theme->getFillColor("cur-dot", true));
    return true;
}

void PresetWidget::set_preset(int index, std::shared_ptr<PresetDescription> source)
{
    preset_index = index;
    if (index >= 0  && static_cast<size_t>(index) < preset_list->size()) {
        preset_id = source->id;
        preset_name->text(source->name);
        preset_name->describe(source->summary());
    } else {
        preset_id.invalidate();
        preset_name->text("");
        preset_name->describe("");
    }
}

std::shared_ptr<PresetDescription> PresetWidget::get_preset()
{
    if (preset_index < 0) return nullptr;
    return (*preset_list)[preset_index];
}

void PresetWidget::draw(const DrawArgs& args)
{
    auto vg = args.vg;

    Base::draw(args);
    // TODO: theme colors, review design for live/selected/gripped states
    if (live) {
        FillRect(vg, -6.f, box.size.y*.5f - 2.f, 4.f, 4.f, dot_color);
    }
    if (selected) {
        BoxRect(vg, .25f, .25f, box.size.x - .5f, box.size.y - .5f, nvgTransRGBAf(RampGray(G_65), 0.4), .5f);
    }
    if (hover_grip) {
        nvgBeginPath(vg);
        nvgMoveTo(vg, box.size.x - 10.f, 0.f);
        nvgLineTo(vg, box.size.x, 0.f);
        nvgLineTo(vg, box.size.x, box.size.y);
        nvgLineTo(vg, box.size.x - 10.f, box.size.y);
        nvgLineTo(vg, box.size.x - 14.f, box.size.y*.5f);
        nvgClosePath(vg);
        nvgFillColor(vg, grip_color);
        nvgFill(vg);
//        FillRect(vg, box.size.x - 10.f, 0.f, 10.f, box.size.y, grip_color);
    }
}

}