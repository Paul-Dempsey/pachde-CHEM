#include "menu-widgets.hpp"
#include "services/text.hpp"
#include "services/theme.hpp"

namespace widgetry {

static const NVGcolor co_dot{nvgHSL(200.f/360.f, .5, .5)};
static const NVGcolor co_gray{RampGray(G_50)};

OptionMenuEntry::OptionMenuEntry(rack::ui::MenuItem* item) :
    child_item(item)
{
    child_item->box.pos.x += 9.f;
    addChild(child_item);
}

OptionMenuEntry::OptionMenuEntry(bool selected, rack::ui::MenuItem* item) :
    child_item(item), selected(selected)
{
    child_item->box.pos.x += 9.f;
    addChild(child_item);
}

void OptionMenuEntry::step() {
    MenuEntry::step();
    box.size.x = child_item->box.size.x + 9.f;
    box.size.y = child_item->box.size.y;
}

void OptionMenuEntry::draw(const DrawArgs &args) {
    MenuEntry::draw(args);
    if (selected) {
        Circle(args.vg, 8.f, box.size.y*.5, 3.f, co_dot);
    } else {
        OpenCircle(args.vg, 8.f, box.size.y*.5, 3.f, co_gray, .75f);
    }
}

FancyLabel::FancyLabel() {
    switch (theme::get_rack_ui_theme()) {
    case theme::RackUiTheme::Light:
        co_bg = nvgRGB(0xf0, 0xf0, 0xf0);
        co_text = color::lerp(co_bg, nvgRGB(0x04, 0x04, 0x04), 0.6);
        break;
    case theme::RackUiTheme::Dark:
        co_bg = nvgRGB(0x20, 0x20, 0x20);
        co_text = color::lerp(co_bg, nvgRGB(0xf0, 0xf0, 0xf0), 0.6);
        break;
    case theme::RackUiTheme::HighContrast:
        co_bg = nvgRGB(0,0,0);
        co_text = color::lerp(co_bg, nvgRGB(0xff, 0xff, 0xff), 0.8); ;
        break;
    }
}

constexpr const float RULE_Y = 9.f;

void FancyLabel::draw(const DrawArgs& args) {
    auto vg = args.vg;
    auto font = GetPluginFontSemiBold();
    if (!FontOk(font)) return;
    FillRect(vg, 0, 0, box.size.x, box.size.y - 1.f, co_bg);
    SetTextStyle(vg, font, co_text, 16.f);
    nvgTextAlign(vg, NVG_ALIGN_BASELINE | align);
    float bounds[4];
    float x{0.f};
    switch (align) {
    case NVG_ALIGN_LEFT:   x = 3.5f; break;
    case NVG_ALIGN_CENTER: x = box.size.x*.5; break;
    case NVG_ALIGN_RIGHT:  x = box.size.x - 3.5; break;
    default: assert(false); break;
    }
    nvgText(vg, x, 14.f, text.c_str(), nullptr);
    if (co_rule.a > 0) {
        nvgTextBounds(vg, x, 14.f, text.c_str(), nullptr, bounds);
        if (bounds[0] - 5 > 3.5) {
            Line(vg, 3.5, RULE_Y, bounds[0] - 5, RULE_Y, co_rule, 1.f);
        } if (bounds[2] + 5 < box.size.x - 3.5) {
            Line(vg, bounds[2] + 5, RULE_Y, box.size.x - 3.5, RULE_Y, co_rule, 1.f);
        }
    }
}

// OptionMenuEntry* make_theme_item(ThemeBase* it, const char * name, ThemeSetting setting) {
//     auto entry = new OptionMenuEntry(createMenuItem(name, "", [it, setting](){ it->setThemeSetting(setting); }));
//     entry->selected = (it->getThemeSetting() == setting);
//     return entry;
// }


}