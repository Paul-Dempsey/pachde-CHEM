#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "click-region-widget.hpp"
#include "label-widget.hpp"

namespace pachde {


struct BitsWidget : OpaqueWidget, IApplyTheme
{
    using Base = OpaqueWidget;

    uint64_t state{0};
    size_t count;
    int rows;
    int cols;
    std::vector<ClickRegion*> hot_spots;
    TextLabel* none_label{nullptr};
    std::vector<TextLabel*> labels;
    
    std::function<void(int item)> change_fn{nullptr};

    ElementStyle envelope{"options-box", "#282828", "hsl(0, 0%, 65%)", 1.25f};
    ElementStyle control_frame{"ctl-frame", "hsl(0, 0%, 55%)", "hsl(0, 0%, 55%)", .5f};
    ElementStyle control_glyph{"ctl-glyph", "hsl(0, 0%, 65%)", "hsl(0, 0%, 65%)", .25f};
    ElementStyle check_style{"option-check", "hsl(120, 50%, 50%)", "hsl(120, 50%, 50%)", .25f};

    const float ROW_HEIGHT = 10.f;
    const float MARGIN_DX = 3.5f;
    const float MARGIN_DY = 3.5f;
    const float COLUMN_SEP = 1.5f;
    const float LABEL_WIDTH = 42.f;
    const float CHECK_DX = 6.f;

    BitsWidget(
        uint64_t initial_state,
        int rows,
        const std::vector<std::string>& items,
        std::function<void(int item)> on_change,
        SvgThemeEngine& theme_engine,
        std::shared_ptr<svg_theme::SvgTheme> theme
    ) :
        state(initial_state),
        count(items.size()),
        rows(rows),
        cols(1 + items.size() / rows),
        change_fn(on_change)
    {
        box.size.y = (MARGIN_DY * 2) + ROW_HEIGHT + (rows * ROW_HEIGHT);
        box.size.x = (MARGIN_DX * 2) + (cols * (LABEL_WIDTH + CHECK_DX)) + ((cols-1) * COLUMN_SEP);
        float x, y;

        LabelStyle style{"choice", TextAlignment::Center, 8.f, false};
        addChild(none_label = createLabel<TextLabel>(Vec(box.size.x*.5f, MARGIN_DY), 32.f, "[ none ]", theme_engine, theme, style));
        addChild(createHoverClickRegion(RECT_ARGS(none_label->box), 0, [=](int id, int mods) { 
            state = 0;
            if (change_fn) change_fn(-1);
        }, "choice-hover"));
        auto r = exit_box_rect();
        addChild(createClickRegion(RECT_ARGS(r), 0, [this](int, int) { setVisible(false); }));

        style.align = TextAlignment::Left;
        auto item_it = items.cbegin();
        x = MARGIN_DX + CHECK_DX;
        y = MARGIN_DY + ROW_HEIGHT;
        for (size_t i = 0; i < items.size(); ++i, item_it++) {

            auto label = createLabel<TextLabel>(Vec(x,y+1), LABEL_WIDTH, *item_it, theme_engine, theme, style);
            labels.push_back(label);
            addChild(label);

            Rect r = label->box;
            r.pos.x -= CHECK_DX;
            r.size.x += CHECK_DX;
            addChild(createHoverClickRegion(RECT_ARGS(r), i, [=](int id, int mods) { 
                uint64_t bit = uint64_t(1) << id;
                if (state & bit) {
                    state = state & ~bit;
                } else {
                    state |= bit;
                }
                if (change_fn) change_fn(id);
            }, "choice-hover"));

            y += ROW_HEIGHT;
            if (0 == ((i+1) % rows)) {
                x += LABEL_WIDTH + COLUMN_SEP + CHECK_DX;
                y = MARGIN_DY + ROW_HEIGHT;
            }
        }
        applyTheme(theme_engine, theme);
    }

    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override
    {
        envelope.apply_theme(theme);
        control_frame.apply_theme(theme);
        control_glyph.apply_theme(theme);
        check_style.apply_theme(theme);
        return true;
    }

    Vec check_pos(int i) {
        Vec pos{MARGIN_DX, 1.5f + MARGIN_DY + ROW_HEIGHT + ROW_HEIGHT*.5f};
        if (i > 0) {
            int row = i % rows;
            int col = i / rows;
            pos.x += (CHECK_DX + LABEL_WIDTH + COLUMN_SEP) * col;
            pos.y += row * ROW_HEIGHT;
        }
        return pos;
    }

    Rect exit_box_rect() {
        return {box.size.x -  MARGIN_DX - 8.f, MARGIN_DY, 8.f, 8.f};
    }

    void onSelectKey(const SelectKeyEvent& e) override
    {
        if (APP->event->getSelectedWidget() != this) {
            Base::onSelectKey(e);
            return;
        }

        if ((e.action == GLFW_PRESS || e.action == GLFW_REPEAT))
        {
            switch (e.key) {
            case GLFW_KEY_ESCAPE:
            case GLFW_KEY_ENTER:
                if (0 == (e.mods & RACK_MOD_MASK))  {
                    APP->event->setSelectedWidget(parent);
                    e.consume(this);
                    setVisible(false);
                    return;
                }
                break;
            }
        }
    }

    void onShow(const ShowEvent& e) override
    {
        APP->event->setSelectedWidget(this);
        Base::onShow(e);
    }

    void onHiide(const HideEvent& e)
    {
        APP->event->setSelectedWidget(parent);
        Base::onHide(e);
    }

    void draw(const DrawArgs& args) override
    {
        auto vg = args.vg;

        // shadow
        nvgScissor(vg, 0, 0, box.size.x+60, box.size.y +60);
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0.f, 3.5f, box.size.x+6.f, box.size.y+15.f, 12.f);
        auto p = nvgBoxGradient(vg, 12, 12, box.size.x-30.f, box.size.y-24.f, 12.f, 50.f, nvgRGB(0,0,0), nvgRGBAf(0,0,0,0));
        nvgFillPaint(vg, p);
        nvgFill(vg);
        nvgResetScissor(vg);
        
        FillRect(vg, 0.f, 0.f, box.size.x, box.size.y, envelope.nvg_color());
        float y = MARGIN_DY + ROW_HEIGHT -.5f;
        Line(vg, 0, y, box.size.x, y, control_frame.nvg_stroke_color(), .5f);
        FittedBoxRect(vg, 0.f, 0.f, box.size.x, box.size.y, envelope.nvg_stroke_color(), Fit::Inside, envelope.width());
        
        auto r = exit_box_rect();
        BoxRect(vg, RECT_ARGS(r), control_frame.nvg_stroke_color(), .5f);
        r = r.grow({-1.25, -1.25});
        auto co = control_glyph.nvg_stroke_color();
        Line(vg, r.pos.x, r.pos.y, r.pos.x + r.size.x, r.pos.y + r.size.y, co, .65f);
        Line(vg, r.pos.x + r.size.x, r.pos.y, r.pos.x, r.pos.y + r.size.y, co, .65f);

        Base::draw(args);

        auto font = GetPluginFontSemiBold();
        if (!FontOk(font)) return;
    
        uint64_t bit = 1;
        for (size_t i = 0; i < count; ++i, bit <<= 1) {
            if (bit & state) {
                auto pos = check_pos(i);
                SetTextStyle(vg, font, check_style.nvg_color(), 5.f);
                nvgText(vg, pos.x, pos.y, "✔", nullptr);
            }
        }
    }

};

}