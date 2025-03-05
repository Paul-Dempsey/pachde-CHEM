#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../services/svgtheme.hpp"
using namespace svg_theme;

namespace pachde {

struct MiniFader: ParamWidget, IApplyTheme
{
    using Base = ParamWidget;

    const char * thumb_key;
    NVGcolor cap_color;
    NVGcolor stem_color;
    NVGcolor thumb_color;
    float cap_width;
    float stem_width;
    float thumb_height;
    float minimum;
    float maximum;
    float drag_pos;

    MiniFader();
    void set_thumb(const char * key, NVGcolor default_color, float height = 3.f);

    // ParamWidget
    void initParamQuantity() override;

    // IApplyTheme
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override;

    // Widget
    //void onHover(const HoverEvent& e) override;
    void onDragStart(const DragStartEvent& e) override;
    void onDragMove(const DragMoveEvent& e) override;
    void onDragEnd(const DragEndEvent& e) override;
    void onButton(const ButtonEvent& e) override;
    void draw(const DrawArgs& args) override;
};

MiniFader * createMiniFaderCentered(Vec pos, Module *module, int param_id, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme);

}