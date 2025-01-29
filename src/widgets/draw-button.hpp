#include <rack.hpp>
#include "../services/svt_Rack.hpp"
#include "../services/colors.hpp"
#include "../services/svt_rack.hpp"
#include "TipWidget.hpp"
using namespace ::rack;

namespace pachde {

struct DrawButtonBase: OpaqueWidget, IApplyTheme
{
    using Base = OpaqueWidget;

    bool button_down;
    bool key_ctrl;
    bool key_shift;
    std::function<void(bool, bool)> handler;
    TipHolder * tip_holder;
    std::vector<const char *> color_keys;
    std::vector<NVGcolor> colors;
    std::vector<NVGcolor> default_colors;

    DrawButtonBase()
    :   button_down(false),
        key_ctrl(false),
        key_shift(false),
        handler(nullptr),
        tip_holder(nullptr)
    {
    }
    virtual ~DrawButtonBase()
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

    void setHandler(std::function<void(bool,bool)> callback)
    {
        handler = callback;
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
        Base::onLeave(e);
        destroyTip();
    }

    void onDragStart(const DragStartEvent& e) override {
        if (e.button != GLFW_MOUSE_BUTTON_LEFT)
            return;
        button_down = true;
    }

    void onDragLeave(const DragLeaveEvent& e) override {
        Base::onDragLeave(e);
        destroyTip();
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        Base::onDragEnd(e);
        destroyTip();
        button_down = false;
    }

    void onHoverKey(const HoverKeyEvent& e) override
    {
        Base::onHoverKey(e);
        key_ctrl = (e.mods & RACK_MOD_MASK) & RACK_MOD_CTRL;
        key_shift = (e.mods & RACK_MOD_MASK) & GLFW_MOD_SHIFT;
    }

    void onAction(const ActionEvent& e) override
    {
        destroyTip();
        if (handler) {
            handler(key_ctrl, key_shift);
        } else {
            Base::onAction(e);
        }
    }

    void addColor(const char * theme_key, NVGcolor default_color)
    {
        color_keys.push_back(theme_key);
        colors.push_back(default_color);
        default_colors.push_back(default_color);
    }

    bool applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme) override
    {
        if (color_keys.empty()) return false;
        colors.clear();
        int index = 0;
        for (auto key : color_keys) {
            auto pcolor = theme->getFillColor(key);
            colors.push_back((svg_theme::NoColor == pcolor) ? default_colors[index] : fromPacked(pcolor));
        }
        return true;
    }

    //virtual void appendContextMenu(ui::Menu* menu) {}

    // void createContextMenu() {
    //     ui::Menu* menu = createMenu();
    // 	appendContextMenu(menu);
    // }
};

struct CtlBase: DrawButtonBase
{
    CtlBase()
    {
        addColor("ctl-frame", RampGray(Ramp::G_50));
        addColor("ctl-glyph", RampGray(Ramp::G_70));
        addColor("ctl-disabled", RampGray(Ramp::G_50));
    }
};

struct UpButton: CtlBase
{
    UpButton()
    {
        box.size.x = 12.f;
        box.size.y = 12.f;
        //addColor("ctl-glyph-dn", RampGray(Ramp::G_50));
    }

    void draw(const DrawArgs& args) override {
        auto vg = args.vg;
        // d="M 7,1.5 2,11.5 h 10 z"
        nvgBeginPath(vg);
        nvgMoveTo(vg, 7.f, 1.5f);
        nvgLineTo(vg, 2.f, 11.5f);
        nvgLineTo(vg, 2.f + 10.f, 11.5f);
        nvgClosePath(vg);
        nvgFillColor(vg, button_down ? nvgTransRGBAf(colors[1], .5f) : colors[1]);
        nvgFill(vg);
    }
};

struct DownButton: CtlBase
{
    DownButton()
    {
        box.size.x = 12.f;
        box.size.y = 12.f;
        //addColor("ctl-glyph-dn", RampGray(Ramp::G_50));
    }

    void draw(const DrawArgs& args) override {
        auto vg = args.vg;
        // d="M 7,11.5 2,3.5 h 10 z"
        nvgBeginPath(vg);
        nvgMoveTo(vg, 7.f, 11.5f);
        nvgLineTo(vg, 2.f, 3.5f);
        nvgLineTo(vg, 2.f + 10.f, 3.5f);
        nvgClosePath(vg);
        nvgFillColor(vg, button_down ? nvgTransRGBAf(colors[1], .5f) : colors[1]);
        nvgFill(vg);
    }
};

}