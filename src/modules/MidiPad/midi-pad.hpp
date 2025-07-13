#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../../em/midi-message.h"
#include "../../services/color-help.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/element-style.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/TipWidget.hpp"
using namespace ::svg_theme;

namespace pachde {

const PackedColor DEFAULT_PAD_COLOR = 0xff8c8c8c;
const PackedColor DEFAULT_PAD_TEXT_COLOR = 0xffe6e6e6;
extern const char * default_pad_name[];

struct MidiPad
{
    int id;
    bool ok{true};
    PackedColor color;
    PackedColor text_color;
    std::string name;    
    std::string def;
    std::vector<PackedMidiMessage> midi;
    std::string error_message;
    int error_pos;

    MidiPad(int id);
    MidiPad(json_t* j);

    bool compile();
    bool defined() { return ok && !midi.empty(); }
    bool empty() {
        if (-1 == id) return true;
        if (def.empty()) {
            return name.empty() || (0 == name.compare(default_pad_name[id]));
        }
        return false;
    }
    void clear() {
        midi.clear();
        def.clear();
        error_message.clear();
        name = id >= 0 ? default_pad_name[id]: "";
        error_pos = 0;
    }
    json_t * to_json();
    void from_json(json_t* root);
};

struct PadWidget : TipWidget, IApplyTheme
{
    using Base = TipWidget;

    int id{-1};
    bool selected{false};
    bool button_down{false};
    bool wire{false};
    std::shared_ptr<MidiPad> pad{nullptr};
    std::function<void(int)> on_click{nullptr};
    TinyLight<WhiteLight>* light{nullptr};
    TextLabel* label{nullptr};

    ElementStyle pad_style{"pad", DEFAULT_PAD_COLOR, DEFAULT_PAD_COLOR, .35f};
    ElementStyle pad_sel_style{"pad-sel", 0xffefef20, 0xffefef20, 1.f};
    ElementStyle pad_down_style{"pad-down", 0xff802020, 0xff802020, 1.f};

    PadWidget();

    std::string extract_description();

    void init(
        int identifier, 
        std::shared_ptr<MidiPad> the_pad,
        Module* module,
        SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme,
        std::function<void(int)> callback);

    void set_pad(std::shared_ptr<MidiPad> the_pad);
    void on_pad_change(bool name, bool description);

    void onHover(const HoverEvent& e) override;
    void onLeave(const LeaveEvent& e) override;
    void onButton(const ButtonEvent& e) override;
    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override;
    void step() override;
    void draw(const DrawArgs& args) override;

};

}