#pragma once
#include <rack.hpp>
#include "../../../widgets/tip-label-widget.hpp"
#include "../../../widgets/element-style.hpp"
#include "../../../em/preset.hpp"

struct PresetUi;

namespace pachde {

struct PresetEntry : OpaqueWidget, IApplyTheme
{
    using Base = OpaqueWidget;
    TipLabel* label{nullptr};
    std::shared_ptr<PresetInfo> preset{nullptr};
    ssize_t preset_index;
    bool live;
    bool current;
    bool hovered;
    std::vector<PresetEntry*>& peers;
    ElementStyle preset_element{"preset", "hsl(0, 0%, 55%)"};
    ElementStyle live_element{"preset-live", "hsl(42, 50%, 50%)", "hsl(42, 50%, 50%)", .35f };
    ElementStyle current_element{"preset-current", "hsl(60, 90%, 50%)", "hsl(60, 90%, 50%)", .25f};
    ElementStyle hover_element{"preset-hover", "hsla(0, 0%, 100%, 5%)", "hsl(120, 50%, 30%)", .5f};
    ElementStyle category_style{"preset-cat", "hsl(200, 50%, 50%)"};
    PresetUi* ui{nullptr};

    void set_ui(PresetUi* host) { ui = host; }

    PresetEntry(std::vector<PresetEntry*>& peers, std::shared_ptr<SvgTheme> theme);
    static PresetEntry* create(Vec pos, std::vector<PresetEntry*>& peers, PresetUi* host, std::shared_ptr<SvgTheme> theme);

    void set_preset(int index, bool is_current, bool is_live, std::shared_ptr<PresetInfo> preset);
    void set_current(ssize_t index);
    void clear_preset();
    bool valid() const { return preset && preset->valid(); }
    PresetId preset_id() const { PresetId dead; return preset ? preset->id : dead; }
    void send_preset();

    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override;
    void appendContextMenu(ui::Menu* menu);
    void createContextMenu()
    {
        ui::Menu* menu = createMenu();
    	appendContextMenu(menu);
    }

    void onHover(const HoverEvent& e) override {
        Base::onHover(e);
        e.consume(this);
    }
    void onEnter(const EnterEvent& e) override {
        Base::onEnter(e);
        if (valid()) {
            label->createTip();
        }
        hovered = true;
    }

    void onLeave(const LeaveEvent& e) override {
        Base::onLeave(e);
        label->destroyTip();
        hovered = false;
    }
    void onButton(const ButtonEvent&e) override;
    void draw(const DrawArgs& args) override;

};

}