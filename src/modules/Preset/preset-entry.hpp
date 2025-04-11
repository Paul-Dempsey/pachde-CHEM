#pragma once
#include <rack.hpp>
#include "../../widgets/tip-label-widget.hpp"
#include "../../widgets/element-style.hpp"
#include "../../em/preset.hpp"

namespace pachde {

struct PresetEntry : OpaqueWidget, IApplyTheme, ILayoutHelp
{
    using Base = OpaqueWidget;
    TipLabel* label{nullptr};
    PresetId preset_id;
    ssize_t preset_index;
    bool live;
    bool current;
    std::vector<PresetEntry*>& peers;
    ElementStyle preset_element{"preset", "hsl(0, 0%, 55%)"};
    ElementStyle live_element{"preset-live", "hsl(42, 50%, 50%)", "hsl(42, 50%, 50%)", .35f };
    ElementStyle current_element{"preset-current", "hsl(60, 90%, 50%)", "hsl(60, 90%, 50%)", .25f};

    PresetEntry(std::vector<PresetEntry*>& peers, std::shared_ptr<SvgTheme> theme);
    static PresetEntry* create(Vec pos, std::vector<PresetEntry*>& peers, std::shared_ptr<SvgTheme> theme);

    void set_preset(int index, bool is_current, bool is_live, std::shared_ptr<PresetDescription> preset);
    void clear_preset();

    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override;
    void onSelectKey(const SelectKeyEvent &e) override;
    void onButton(const ButtonEvent&e) override;
    void draw(const DrawArgs& args) override;

};

}