#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../../widgets/widgets.hpp"
#include "../XM-shared/xm-edit-common.hpp"

namespace S = pachde::style;

std::string macro_number_to_string(uint8_t mn);
uint8_t macro_number_from_string(std::string s);

namespace pachde {

namespace me_constants {
    constexpr const float WIDTH{148.f};
    constexpr const float HEIGHT{200.f};
    constexpr const float CENTER{75.f};
    constexpr const float TOP{18.f};
    constexpr const float LEFT_AXIS{45.f};
    constexpr const float LABEL_OFFSET_DX{5.f};
    constexpr const float ROW_DY{18.f};
    constexpr const float SMALL_ROW_DY{16.f};
    constexpr const float LABEL_DY = 1.5f;
    constexpr const float OPT_CX{53.5f};
    constexpr const float MINMAX_DX{13.f};

    // must track module param ids
    //constexpr const int P_MODULATION{8};
    constexpr const int P_RANGE_MIN{9};
    constexpr const int P_RANGE_MAX{10};
    constexpr const int P_PORT{11};
};

struct MacroEdit : OpaqueWidget, IApplyTheme
{
    using Base = OpaqueWidget;

    MacroDescription macro{0,0};

    TextInput* name_entry{nullptr};
    TextInput* macro_entry{nullptr};
    CheckButton* input_port{nullptr};
    std::vector<StateIndicatorWidget*> range_options;

    ElementStyle envelope{"options-box", "#181818", "hsl(0, 0%, 75%)", 1.25f};

    MacroEdit() 
    {
        using namespace me_constants;
        box.size = Vec{WIDTH, HEIGHT};
    }

    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override 
    {
        envelope.apply_theme(theme);
        return false;
    }

    void set_from_macro(std::shared_ptr<MacroDescription> mac) {
        macro.init(mac);
        name_entry->setText (macro.name);
        macro_entry->setText(macro_number_to_string(macro.macro_number));
    }

    void set_range(MacroRange range) {
        macro.set_range(range);
        int i = 0;
        for (auto r: range_options) {
            r->set_state(i == int(range));
            ++i;
        }
    }

    void create_ui(Module * module, int knob_index, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
    {
        using namespace me_constants;

        macro.module_id = module->id;
        macro.knob_id = knob_index;
        LabelStyle r_label{"ctl-label",  TextAlignment::Right, 14.f, false};

        TextButton* close = createWidgetCentered<TextButton>(Vec(CENTER, 16.f));
        close->set_text("x");
        close->setHandler([=](bool, bool){ on_close(); });
        addChild(close);

        float x, y;
        x = LEFT_AXIS;
        y = TOP;
        addChild(createLabel(Vec(x - LABEL_OFFSET_DX, y), 100.f, "Name", engine, theme, r_label));
        name_entry = createThemedTextInput(x, y, 90, 14,
            engine, theme,
            "",
            [=](std::string s){ macro.name = s; },
            nullptr,
            "<knob label>");
        addChild(name_entry);

        y += ROW_DY;
        addChild(createLabel(Vec(x - LABEL_OFFSET_DX, y), 60.f, "Macro", engine, theme, r_label));
        addChild(macro_entry = createThemedTextInput(x, y, 65, 14,
            engine, theme,
            "",
            [=](std::string s) { macro.macro_number = macro_number_from_string(s); }, 
            nullptr,
            "<macro number 7-90>"
            ));

        y += ROW_DY;
        addChild(createThemedParamButton<CheckParamButton>(Vec(x,y), module, P_PORT, theme_engine, theme));
        addChild(createLabel(Vec(x + 14.f, y), 60.f, "Input port", engine, theme, S::control_label_left));

        y += ROW_DY;
        addChild(createLabel(Vec(x - LABEL_OFFSET_DX, y), 60.f, "Range", engine, theme, r_label));

        StateIndicatorWidget* indicator{nullptr};
        x = OPT_CX;
        //MacroRange range = current_macro ? current_macro->range : MacroRange::Unipolar;
        MacroRange range{MacroRange::Unipolar};

        indicator = createIndicatorCentered(x, y + 7.f, range == MacroRange::Bipolar, "Bipolar");
        indicator->applyTheme(theme_engine, theme);
        addChild(indicator);
        range_options.push_back(indicator);
        addChild(createLabel(Vec(x + 6.f, y), 50, "Bipolar", engine, theme, S::control_label_left));
        addChild(createClickRegion(x -6.f, y - 1.f, 60, 14, int(MacroRange::Bipolar), [=](int id, int) { 
            set_range(MacroRange(id)); 
        }));

        y += SMALL_ROW_DY;
        indicator = createIndicatorCentered(x, y + 7.f, range == MacroRange::Unipolar, "Unipolar");
        indicator->applyTheme(theme_engine, theme);
        addChild(indicator);
        range_options.push_back(indicator);
        addChild(createLabel(Vec(x + 6.f, y), 50, "Unipolar", engine, theme, S::control_label_left));
        addChild(createClickRegion(x -6.f, y - 1.f, 60, 14, int(MacroRange::Unipolar), [=](int id, int) { 
            set_range(MacroRange(id)); 
        }));

        y += SMALL_ROW_DY;
        indicator = createIndicatorCentered(x, y + 7.f, range == MacroRange::Custom, "Custom");
        indicator->applyTheme(theme_engine, theme);
        addChild(indicator);
        range_options.push_back(indicator);
        addChild(createLabel(Vec(x + 6.f, y), 50, "Custom", engine, theme, S::control_label_left));
        addChild(createClickRegion(x -6.f, y - 1.f, 60, 14, int(MacroRange::Custom), [=](int id, int) { 
            set_range(MacroRange(id)); 
        }));

        y += ROW_DY + 8.f;
        auto mini_label_style = LabelStyle{"ctl-label", TextAlignment::Center, 10.f, true};
        addChild(createChemKnob<GreenTrimPot>(Vec(CENTER - MINMAX_DX, y), module, P_RANGE_MIN, theme_engine, theme));
        addChild(createLabel(Vec(CENTER - MINMAX_DX, y + 9.5f), 25, "min", theme_engine, theme, mini_label_style));
        addChild(createChemKnob<GreenTrimPot>(Vec(CENTER + MINMAX_DX, y), module, P_RANGE_MAX, theme_engine, theme));
        addChild(createLabel(Vec(CENTER + MINMAX_DX, y + 9.5f), 25, "max", theme_engine, theme, mini_label_style));

        y += ROW_DY + 8.f;
        x = LEFT_AXIS;
        //addChild(createThemedParamButton<CheckParamButton>(Vec(x,y), module, P_PORT, theme_engine, theme));
        auto resetButton = createThemedButton<DotButton>(Vec(x,y+ 7.f),engine, theme, "Reset");
        resetButton->setHandler([=](bool,bool) {
            // on_reset
        });
        addChild(resetButton);
        addChild(createLabel(Vec(x + 14.f, y), 60.f, "Reset", theme_engine, theme, S::control_label_left));

        // tab order
        name_entry->nextField = macro_entry;
        macro_entry->nextField = this;

        name_entry->prevField = this;
        macro_entry->prevField = name_entry;

    };

    static MacroEdit* createMacroEdit(Vec pos, Module * module, int knob_index, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
    {
        MacroEdit* w = createWidget<MacroEdit>(pos);
        w->create_ui(module, knob_index, engine, theme);
        return w;
    }

};

}