#pragma once
#include <rack.hpp>
#include "../../widgets/theme-button.hpp"
#include "../../widgets/bits-widget.hpp"
#include "../../widgets/TipWidget.hpp"

using namespace ::rack;

namespace pachde {

// like CheckMenu
struct StateButton : SvgButton, IApplyTheme
{
    using Base = SvgButton;
    const char * up_svg{nullptr};
    const char * down_svg{nullptr};
    TipHolder * tip_holder{nullptr};
    bool button_down{false};
    bool last_button_down{false};

    //std::function<bool()> get_fn{nullptr};
    std::function<void()> click_fn{nullptr};

    StateButton (
        const char * svg_up,
        const char * svg_down, 
        //std::function<bool()> get_state,
        std::function<void()> on_click
    ) :
        up_svg(svg_up),
        down_svg(svg_down),
        //get_fn(get_state),
        click_fn(on_click)
    {
    };
    virtual ~StateButton()
    {
        if (tip_holder) {
            delete tip_holder;
            tip_holder = nullptr;
        }
    }
    void describe(std::string text)
    {
        if (!tip_holder) {
            tip_holder = new TipHolder();
        }
        tip_holder->setText(text);
    }

    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        bool refresh = frames.size() > 0; 
        if (refresh) {
            frames.clear();
            sw->setSvg(nullptr);
        }
    
        addFrame(engine.loadSvg(asset::plugin(pluginInstance, up_svg), theme));
        addFrame(engine.loadSvg(asset::plugin(pluginInstance, down_svg), theme));
    
        if (refresh) {
            sw->setSvg(frames[0]);
            if (fb) {
                fb->setDirty();
            }
        }

        return true;
    }

    void onHover(const HoverEvent& e) override {
        e.consume(this);
    }

    void destroyTip() {
        if (tip_holder) { tip_holder->destroyTip(); }
    }
    void createTip() {
        if (tip_holder) { tip_holder->createTip(); }
    }

    void onEnter(const EnterEvent& e) override {
        Base::onEnter(e);
        createTip();
    }

    void onLeave(const LeaveEvent& e) override {
        destroyTip();
        Base::onLeave(e);
    }

    void onDragStart(const DragStartEvent& e) override
    {
        destroyTip();
        Base::onDragStart(e);
        button_down = true;
    }

    void onDragLeave(const DragLeaveEvent& e) override
    {
        destroyTip();
        Base::onDragLeave(e);
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        Base::onDragEnd(e);
        destroyTip();
        button_down = false;
    }

    void step() override {
        if (button_down == last_button_down) return;
        if (button_down) {
            if (sw->svg->handle != frames[1]->handle) {
                sw->setSvg(frames[1]);
                fb->setDirty();
            }
            describe("Clear all filters");
        } else {
            if (sw->svg->handle != frames[0]->handle) {
                sw->setSvg(frames[0]);
                fb->setDirty();
            }
            describe("Unfiltered");
        }
        last_button_down = button_down;
    }
    void onAction(const ActionEvent& e) override {
        if (click_fn) click_fn();
    }
};

StateButton* makeFilterStateButton(
    Vec pos,
    //std::function<bool()> get_state,
    SvgThemeEngine &engine,
    std::shared_ptr<SvgTheme> theme,
    std::function<void()> on_click
);

struct FilterButton : SvgButton, IApplyTheme
{
    using Base = SvgButton;
    
    const char * up_svg{nullptr};
    const char * down_svg{nullptr};
    TipHolder * tip_holder{nullptr};
    BitsWidget* dialog{nullptr};
    std::function<void(uint64_t item)> change_fn{nullptr};

    FilterButton(const char * svg_up, const char * svg_down, std::function<void(uint64_t item)> on_change);
    
    void init (
        const std::string& name, int rows, float item_width,
        const std::vector<std::string>& items,
        SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme);

    uint64_t get_state() { return dialog->get_state(); }
    void set_state(uint64_t state) {
        dialog->set_state(state);
        describe(dialog->make_summary());
    }

    virtual ~FilterButton()
    {
        if (tip_holder) {
            delete tip_holder;
            tip_holder = nullptr;
        }
    }
    void close_dialog() { dialog->close(); }
    
    void describe(std::string text)
    {
        if (!tip_holder) {
            tip_holder = new TipHolder();
        }
        tip_holder->setText(text);
    }

    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override;

    void onHover(const HoverEvent& e) override {
        e.consume(this);
    }

    void destroyTip() {
        if (tip_holder) { tip_holder->destroyTip(); }
    }
    void createTip() {
        if (tip_holder) { tip_holder->createTip(); }
    }

    void onEnter(const EnterEvent& e) override {
        Base::onEnter(e);
        createTip();
    }

    void onLeave(const LeaveEvent& e) override {
        destroyTip();
        Base::onLeave(e);
    }

    void onDragStart(const DragStartEvent& e) override
    {
        destroyTip();
        Base::onDragStart(e);
    }

    void onDragLeave(const DragLeaveEvent& e) override
    {
        destroyTip();
        Base::onDragLeave(e);
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        Base::onDragEnd(e);
        destroyTip();
    }

    void onAction(const ActionEvent& e) override;
    void step() override;
};

FilterButton* makeCatFilter(Vec pos, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change);
FilterButton* makeTypeFilter(Vec pos, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change);
FilterButton* makeCharacterFilter(Vec pos, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change);
FilterButton* makeMatrixFilter(Vec pos, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change);
FilterButton* makeSettingFilter(Vec pos, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change);

}