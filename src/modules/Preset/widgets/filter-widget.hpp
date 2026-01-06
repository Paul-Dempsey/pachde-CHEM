#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "widgets/theme-button.hpp"
#include "widgets/bits-widget.hpp"
#include "widgets/tip-widget.hpp"
using namespace widgetry;

namespace pachde {

struct StateButton : SvgButton
{
    using Base = SvgButton;

    const char * up_svg{nullptr};
    const char * down_svg{nullptr};
    TipHolder * tip_holder{nullptr};
    bool button_down{false};
    bool last_button_down{false};

    std::function<void()> click_fn{nullptr};

    StateButton (
        const char * svg_up,
        const char * svg_down,
        std::function<void()> on_click
    ) :
        up_svg(svg_up),
        down_svg(svg_down),
        click_fn(on_click)
    {
        this->shadow->hide();
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

    void loadSvg(ILoadSvg* loader)
    {
        frames.clear();

        addFrame(loader->loadSvg(asset::plugin(pluginInstance, up_svg)));
        addFrame(loader->loadSvg(asset::plugin(pluginInstance, down_svg)));

        sw->setSvg(frames[0]);
        if (fb) {
            fb->setDirty();
        }
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
    ILoadSvg* loader,
    std::function<void()> on_click
);

struct FilterButton : SvgButton, IThemed
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
        std::vector<const char *>& items,
        ILoadSvg* loader, std::shared_ptr<svg_theme::SvgTheme> theme);

    uint64_t get_state() { return dialog->get_state(); }
    void set_state(uint64_t state) {
        dialog->set_state(state);
        describe(dialog->make_summary());
    }

    void applyTheme(std::shared_ptr<SvgTheme> theme) override {
        if (dialog) {
            applyChildrenTheme(dialog, theme);
        }
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

    void loadSvg(ILoadSvg* loader);

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

    void onDragStart(const DragStartEvent& e) override {
        destroyTip();
        Base::onDragStart(e);
    }

    void onDragLeave(const DragLeaveEvent& e) override {
        destroyTip();
        Base::onDragLeave(e);
    }

    void onDragEnd(const DragEndEvent& e) override {
        Base::onDragEnd(e);
        destroyTip();
    }

    void onAction(const ActionEvent& e) override;
    void step() override;
};

FilterButton* makeCatFilter(Vec pos, ILoadSvg* loader, std::shared_ptr<svg_theme::SvgTheme> theme, std::function<void(uint64_t item)> on_change);
FilterButton* makeTypeFilter(Vec pos, ILoadSvg* loader, std::shared_ptr<svg_theme::SvgTheme> theme, std::function<void(uint64_t item)> on_change);
FilterButton* makeCharacterFilter(Vec pos, ILoadSvg* loader, std::shared_ptr<svg_theme::SvgTheme> theme, std::function<void(uint64_t item)> on_change);
FilterButton* makeMatrixFilter(Vec pos, ILoadSvg* loader, std::shared_ptr<svg_theme::SvgTheme> theme, std::function<void(uint64_t item)> on_change);
FilterButton* makeSettingFilter(Vec pos, ILoadSvg* loader, std::shared_ptr<svg_theme::SvgTheme> theme, std::function<void(uint64_t item)> on_change);

}