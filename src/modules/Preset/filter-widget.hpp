#pragma once
#include <rack.hpp>
#include "../../widgets/theme-button.hpp"
#include "../../widgets/bits-widget.hpp"
#include "../../widgets/TipWidget.hpp"

using namespace ::rack;

namespace pachde {

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