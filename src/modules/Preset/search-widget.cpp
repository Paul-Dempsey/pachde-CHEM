#include "search-widget.hpp"
#include "../../services/misc.hpp"
namespace pachde {

SearchField::SearchField()
{
    multiline = false;
}

bool SearchField::applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme)
{
    bg_style.apply_theme(theme);
    text_style.apply_theme(theme);
    prompt_style.apply_theme(theme);
    selection_style.apply_theme(theme);
    return true;
}

void SearchField::onSelectKey(const SelectKeyEvent &e)
{
    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
        if (e.key == GLFW_KEY_ESCAPE && (0 == e.mods)) {
            setText("");
            //APP->event->setSelectedWidget(nullptr);
            e.consume(this);
        }
    }
    Base::onSelectKey(e);
}

void SearchField::onChange(const ChangeEvent &e)
{
    if (change_handler) change_handler();
}

void SearchField::onAction(const ActionEvent &e)
{
    if (enter_handler) enter_handler();
}

int SearchField::getTextPosition(math::Vec mousePos)
{
    if (text.empty()) return 0.f;

    if (mousePos.x < 1.5f) return 0;

    auto font = GetPluginFontRegular();
    if (!FontOk(font)) return 0;

    NVGglyphPosition glyphs[120];
    int glyph_count;
    auto vg = APP->window->vg;
    const char * txt = text.c_str();

    nvgSave(vg);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 0.f);
    nvgFontSize(vg, 12.f);

    float x = 1.5f;
    glyph_count = nvgTextGlyphPositions(vg, x, 1.f, txt, nullptr, glyphs, 120);
    NVGglyphPosition* pg = glyphs;
    for (int i = 0; i < glyph_count; ++i, ++pg) {
        if (in_range(mousePos.x, pg->minx, pg->maxx)) {
            nvgRestore(vg);
            return pg->str - txt;
        }
    }
    nvgRestore(vg);
    return text.size();
}

void SearchField:: draw(const DrawArgs& args)
{
    auto font = GetPluginFontRegular();
    if (!FontOk(font)) return;
    auto vg = args.vg;

    nvgScissor(vg, RECT_ARGS(args.clipBox));
    nvgSave(vg);
    nvgTextAlign(vg, NVG_ALIGN_TOP|NVG_ALIGN_LEFT);

    FillRect(vg, 0, 0, box.size.x, box.size.y, bg_style.nvg_color());

    bool active = (this == APP->event->selectedWidget);
    if (active || !text.empty()) {
        SetTextStyle(vg, font, text_style.nvg_color(), 12.f);
        const char * txt = text.c_str();

        // draw caret only when active
        if (active) {
            if (text.empty()) {
                FillRect(vg, 1.5f, .5f, 1.f, box.size.y - .5f, selection_style.nvg_color());
            } else {
                NVGglyphPosition glyphs[120];
                int glyph_count;
                float cx_start, cx_end;
                glyph_count = nvgTextGlyphPositions(vg, 1.5f, 1.f, txt, nullptr, glyphs, 120);

                // { //debug
                //     NVGglyphPosition* _pg = glyphs;
                //     auto _co = nvgTransRGBAf(PORT_VIOLET, .5f);
                //     for (int i = 0; i < glyph_count; ++i, ++_pg) {
                //         Line(vg, _pg->x, 0.f, _pg->x, 5.f, nvgRGBAf(0, 1, 1, .25), 1.f);
                //         BoxRect(vg, _pg->minx, .5, _pg->maxx-_pg->minx, box.size.y - .5, _co, .2f);
                //     }
                // } //debug

                NVGglyphPosition* pg = glyphs;
                for (int i = 0; i < glyph_count; ++i, ++pg) {
                    cx_start = pg->x;
                    if (pg->str == txt + cursor) {
                        break;
                    }
                    if (pg->str + 1 == txt + cursor) {
                        cx_start = pg->maxx;
                        break;
                    }
                }
                pg = glyphs;
                for (int i = 0; i < glyph_count; ++i, ++pg) {
                    cx_end = pg->x;
                    if (pg->str == txt + selection) {
                        break;
                    }
                    if (pg->str + 1 == txt + selection) {
                        cx_end = pg->maxx;
                        break;
                    }
                }
                if (cx_end < cx_start) {
                    std::swap(cx_end, cx_start);
                }
                float w = std::max(1.f, cx_end - cx_start);
                float h;
                nvgTextMetrics(vg, nullptr, nullptr, &h);
                FillRect(vg, cx_start, 1.f, w, 1.f + h, selection_style.nvg_color());
            }
        }
        nvgFillColor(vg, text_style.nvg_color());
        nvgText(vg, 1.5f, 1.f, txt, nullptr);
    } else if (!placeholder.empty() && (this == APP->event->hoveredWidget)) {
        SetTextStyle(vg, font, prompt_style.nvg_color(), 10.f);
        nvgText(vg, 1.5f, 1.f, placeholder.c_str(), nullptr);
    }

    nvgRestore(vg);
    nvgResetScissor(vg);
}

}