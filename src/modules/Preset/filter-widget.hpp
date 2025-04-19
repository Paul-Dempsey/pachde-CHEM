#pragma once
#include <rack.hpp>
#include "../../widgets/theme-button.hpp"
#include "../../widgets/bits-widget.hpp"
#include "../../widgets/TipWidget.hpp"

using namespace ::rack;

namespace pachde {

template<typename TSvg>
struct TFilterButton : SvgButton, IApplyTheme
{
    using Base = SvgButton;
    
    TipHolder * tip_holder{nullptr};
    BitsWidget* dialog{nullptr};
    std::function<void(uint64_t item)> change_fn{nullptr};

    TFilterButton()
    {
        this->shadow->hide();
    }
    
    void init(
        const std::string& name,
        int rows,
        float item_width,
        const std::vector<std::string>& items,
        SvgThemeEngine& engine,
        std::shared_ptr<SvgTheme> theme, 
        std::function<void(uint64_t item)> on_change
    )
    {
        change_fn = on_change;

        if (item_width <= 0.f) item_width = 42;
        applyTheme(engine, theme);

        const float DIALOG_SEP = 1.5f;

        dialog = new BitsWidget(name, rows, item_width, items, engine, theme,
            [=](uint64_t state) {
                describe(dialog->make_summary());
                if (change_fn) {
                    change_fn(state);
                }
            });

        describe(dialog->make_summary());

        dialog->box.pos.x = box.pos.x - box.size.x*.5 - dialog->box.size.x - DIALOG_SEP;
        dialog->box.pos.y = box.pos.y - dialog->box.size.y*.5f;
        dialog->setVisible(false);
    }

    uint64_t get_state() { return dialog->get_state(); }
    void set_state(uint64_t state) { dialog->set_state(state); }

    virtual ~TFilterButton()
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

        addFrame(engine.loadSvg(asset::plugin(pluginInstance, TSvg::up()), theme));
        addFrame(engine.loadSvg(asset::plugin(pluginInstance, TSvg::down()), theme));

        if (refresh) {
            sw->setSvg(frames[0]);
            if (fb) {
                fb->setDirty();
            }
        }
        if (dialog && !dialog->isVisible()) {
            // dialog is not a child widget when not shown
            ApplyChildrenTheme(dialog, engine, theme);
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

    void onAction(const ActionEvent& e) override
    {
        destroyTip();
        if (!dialog->isVisible()) {
            getParent()->addChild(dialog);
            dialog->setVisible(true);
        }
    }

    void step() override {
        if (get_state()) {
            if (sw->svg->handle != frames[1]->handle) {
                sw->setSvg(frames[1]);
                fb->setDirty();
            }
        } else {
            if (sw->svg->handle != frames[0]->handle) {
                sw->setSvg(frames[0]);
                fb->setDirty();
            }
            fb->setDirty();
        }
    }
};

struct CatButtonSvg {
    static std::string up() { return "res/widgets/category-button.svg"; }
    static std::string down() { return "res/widgets/category-button-down.svg"; }
};
struct GearButtonSvg {
    static std::string up() { return "res/widgets/gear-button.svg"; }
    static std::string down() { return "res/widgets/gear-button-down.svg"; }
};
struct TypeButtonSvg {
    static std::string up() { return "res/widgets/type-button.svg"; }
    static std::string down() { return "res/widgets/type-button-down.svg"; }
};
struct CharacterButtonSvg {
    static std::string up() { return "res/widgets/character-button.svg"; }
    static std::string down() { return "res/widgets/character-button-down.svg"; }
};
struct MatrixButtonSvg {
    static std::string up() { return "res/widgets/matrix-button.svg"; }
    static std::string down() { return "res/widgets/matrix-button-down.svg"; }
};

using CatFilter = TFilterButton<CatButtonSvg>;
using TypeFilter = TFilterButton<TypeButtonSvg>;
using CharacterFilter = TFilterButton<CharacterButtonSvg>;
using MatrixFilter = TFilterButton<MatrixButtonSvg>;
using GearFilter = TFilterButton<GearButtonSvg>;

CatFilter* makeCatFilter(Vec pos, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change);
TypeFilter* makeTypeFilter(Vec pos, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change);
CharacterFilter* makeCharacterFilter(Vec pos, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change);
MatrixFilter* makeMatrixFilter(Vec pos, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change);
GearFilter* makeSettingFilter(Vec pos, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change);

}