#include "my-plugin.hpp"
#include "../Preset.hpp"
#include "services/colors.hpp"
#include "services/midi-note.hpp"
#include "services/svg-query.hpp"
#include "services/svg-theme.hpp"
#include "services/text.hpp"
using namespace svg_theme;
#define DIALOG_THEMED
#include "widgets/blip-widget.hpp"
#include "widgets/dialog.hpp"
#include "widgets/dialog-help.hpp"
#include "widgets/info-symbol.hpp"
#include "widgets/label.hpp"
#include "widgets/menu-widgets.hpp"
#include "widgets/option-widget.hpp"

namespace widgetry {

struct DialogSvg {
    static std::string background() {
        return asset::plugin(pluginInstance, "res/dialogs/preset-midi-config.svg");
    }
};

struct ChannelMenu : Hamburger {
    using Base = Hamburger;
    uint8_t* poke{nullptr};

    void appendContextMenu(ui::Menu* menu) override {
        menu->addChild(createMenuLabel<HamburgerTitle>("Channel"));
        menu->addChild(new OptionMenuEntry( *poke == 0xFF, createMenuItem("[ any ]", "", [=](){ *poke = 0xFF; })));
        menu->addChild(new OptionMenuEntry( *poke == 1, createMenuItem("1", "", [=](){ *poke = 1; })));
        menu->addChild(new OptionMenuEntry( *poke == 2, createMenuItem("2", "", [=](){ *poke = 2; })));
        menu->addChild(new OptionMenuEntry( *poke == 3, createMenuItem("3", "", [=](){ *poke = 3; })));
        menu->addChild(new OptionMenuEntry( *poke == 4, createMenuItem("4", "", [=](){ *poke = 4; })));
        menu->addChild(new OptionMenuEntry( *poke == 5, createMenuItem("5", "", [=](){ *poke = 5; })));
        menu->addChild(new OptionMenuEntry( *poke == 6, createMenuItem("6", "", [=](){ *poke = 6; })));
        menu->addChild(new OptionMenuEntry( *poke == 7, createMenuItem("7", "", [=](){ *poke = 7; })));
        menu->addChild(new OptionMenuEntry( *poke == 8, createMenuItem("8", "", [=](){ *poke = 8; })));
        menu->addChild(new OptionMenuEntry( *poke == 9, createMenuItem("9", "", [=](){ *poke = 9; })));
        menu->addChild(new OptionMenuEntry( *poke == 10, createMenuItem("10", "", [=](){ *poke = 10; })));
        menu->addChild(new OptionMenuEntry( *poke == 11, createMenuItem("11", "", [=](){ *poke = 11; })));
        menu->addChild(new OptionMenuEntry( *poke == 12, createMenuItem("12", "", [=](){ *poke = 12; })));
        menu->addChild(new OptionMenuEntry( *poke == 13, createMenuItem("13", "", [=](){ *poke = 13; })));
        menu->addChild(new OptionMenuEntry( *poke == 14, createMenuItem("14", "", [=](){ *poke = 14; })));
        menu->addChild(new OptionMenuEntry( *poke == 15, createMenuItem("15", "", [=](){ *poke = 15; })));
        menu->addChild(new OptionMenuEntry( *poke == 16, createMenuItem("16", "", [=](){ *poke = 16; })));
    }
};

struct MidiLearner : OpaqueWidget, ILearner, IThemed {
    using Base = OpaqueWidget;
    bool active{false};
    uint8_t value{0xff};
    std::string value_text;
    LearnMode mode{LearnMode::Off};
    PresetAction role{PresetAction::KeySelect};
    PresetMidi* midi_handler{nullptr};
    PackedColor co_normal{colors::G85};
    PackedColor co_active{parseColor("hsl(120, .8, .5)", colors::White)};

    Widget* prev_widget{nullptr};
    Widget* next_widget{nullptr};

    MidiLearner(Rect bounds, LearnMode mode, PresetAction role, PresetMidi* pm) :
        mode(mode), role(role), midi_handler(pm)
    {
        box = bounds;
        value = midi_handler->code[role];
        update_text();
    }
    void update_text() { value_text = (0xFF == value) ? "" : noteFullName(value); }
    void applyTheme(std::shared_ptr<SvgTheme> theme) override {
        if (!theme->getFillColor(co_normal, "learn", true)) {
            co_normal = colors::G65;
        }
        if (!theme->getFillColor(co_active, "learn", true)) {
            co_active = parseColor("hsl(42, .6, .8)", colors::White);
        }
    }
    void learn_value(LearnMode mode, uint8_t new_value) override {
        midi_handler->code[role] = value = new_value;
        update_text();
    }
    void onButton( const ButtonEvent& e) override {
        e.consume(this);
        Base::onButton(e);
    }
    void onSelect(const SelectEvent& e) override {
        active = true;
        midi_handler->set_student(this);
        midi_handler->start_learning(mode);
        Base::onSelect(e);
    }
    void onSelectKey(const SelectKeyEvent& e) override {
        auto mods = e.mods & RACK_MOD_MASK;
        if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
            switch (e.key) {
            case GLFW_KEY_ESCAPE:
                midi_handler->code[role] = value = UndefinedCode;
                value_text = "";
                e.consume(this);
                break;
            case GLFW_KEY_TAB:
                if (0 != (mods & RACK_MOD_SHIFT)) {
                    if (prev_widget) {
                        APP->event->setSelectedWidget(prev_widget);
                        e.consume(this);
                    }
                } else {
                    if (next_widget) {
                        APP->event->setSelectedWidget(next_widget);
                        e.consume(this);
                    }
                }
                break;
            }
        }
    }
    void onDeselect(const DeselectEvent& e) override {
        active = false;
        midi_handler->stop_learning();
        midi_handler->set_student(nullptr);
        Base::onDeselect(e);
    }

    void draw(const DrawArgs& args) override {
        auto vg = args.vg;
        if (active) {
            if (!value_text.empty()) {
                draw_text_box(vg, 0, 0, box.size.x, box.size.y,
                    0.f,0.f,0.f,0.f, // no margins
                    value_text, GetPluginFontSemiBold(), 10.f,
                    active ? co_active : co_normal, HAlign::Center, VAlign::Middle
                );
            }
        } else {
            draw_text_box(vg, 0, 0, box.size.x, box.size.y,
                0.f,0.f,0.f,0.f, // no margins
                value_text.empty() ? "unset" : value_text, GetPluginFontSemiBold(), 10.f,
                active ? co_active : co_normal, HAlign::Center, VAlign::Middle
            );
        }

        if (active) {
            nvgBeginPath(vg);
            nvgMoveTo(vg, 1.125f, 0);
            nvgLineTo(vg, 1.125f, box.size.y);
            nvgMoveTo(vg, box.size.x-1.5f, 0);
            nvgLineTo(vg, box.size.x-1.5f, box.size.y);
            nvgStrokeColor(vg, fromPacked(co_active));
            nvgStrokeWidth(vg, 2.25);
            nvgStroke(vg);
        }
        nvgBeginPath(vg);
        nvgMoveTo(vg, 4.f, 0);
        nvgLineTo(vg, 0, 0);
        nvgLineTo(vg, 0, box.size.y);
        nvgLineTo(vg, 4.f, box.size.y);

        nvgMoveTo(vg, box.size.x - 4.f, 0);
        nvgLineTo(vg, box.size.x, 0);
        nvgLineTo(vg, box.size.x, box.size.y);
        nvgLineTo(vg, box.size.x - 4.f, box.size.y);

        nvgStrokeColor(vg, fromPacked(co_normal));
        nvgStrokeWidth(vg, .75);
        nvgStroke(vg);
    }
};

struct ConfigPresetMidi : SvgDialog<DialogSvg> {
    using Base = SvgDialog<DialogSvg>;

    SvgCache my_svgs;
    DialogStyles styles;
    PresetMidi* midi_handler{nullptr};
    Blip* learn_blip{nullptr};
    Blip* valid_blip{nullptr};
    //std::vector<OptionWidget*> key_options;
    CheckButton* log_check{nullptr};
    TextLabel* channel_info{nullptr};
    ConfigPresetMidi(ModuleWidget* source, ILoadSvg* svg_loader) :
        Base(source, svg_loader)
    {}

    void set_midi_handler(PresetMidi* midi) { midi_handler = midi; }

    void create_ui(std::shared_ptr<svg_theme::SvgTheme> svg_theme) {
        auto layout = Base::get_svg();
        applySvgTheme(layout, svg_theme);
        styles.note.key = "dlg-label";
        styles.info.halign = HAlign::Right;
        styles.initStyles(svg_theme);
        ::svg_query::BoundsIndex bounds;
        svg_query::addBounds(layout, "k:", bounds, true);

        add_close_button(this, bounds, "k:close", svg_theme);
        addChild(createLabel(bounds["k:dlg-title"], "Preset | Midi Navigation", &styles.title));

        addChild(createLabel(bounds["k:conn"], midi_handler->connection_name(), &styles.info));

        Rect r = bounds["k:valid"];
        addChild(valid_blip = createWidgetCentered<Blip>(r.getCenter()));
        valid_blip->set_radius(r.size.x * .5);
        addChild(createLabel(bounds["k:valid-label"], "ok", &styles.note));

        r = bounds["k:learn"];
        addChild(learn_blip = createWidgetCentered<Blip>(r.getCenter()));
        learn_blip->set_radius(r.size.x * .5);
        learn_blip->set_light_color(fromPacked(parseColor("hsl(120, .8, .5)", colors::PortGreen)));
        addChild(createLabel(bounds["k:learn-label"], "learn", &styles.note));

        addChild(createLabel(bounds["k:key-label"], "Note", &styles.section));

        auto channel_menu = createWidgetCentered<ChannelMenu>(bounds["k:kchannel"].getCenter());
        channel_menu->poke = &midi_handler->channel;
        channel_menu->describe("Channel");
        addChild(channel_menu);
        addChild(channel_info = createLabel(bounds["k:kchannel-info"], 0xFF == midi_handler->channel ? "[any]" : format_string("%d", midi_handler->channel), &styles.info));

        addChild(createLabel(bounds["k:kselect-label"], "Select", &styles.right));
        MidiLearner* learn_select = new MidiLearner(bounds["k:kselect-nn"], LearnMode::Note, PresetAction::KeySelect, midi_handler);
        addChild(learn_select);

        addChild(createLabel(bounds["k:kpage-label"], "Page", &styles.right));
        MidiLearner* learn_page = new MidiLearner(bounds["k:kpage-nn"], LearnMode::Note, PresetAction::KeyPage, midi_handler);
        addChild(learn_page);

        addChild(createLabel(bounds["k:kindex-label"], "Index", &styles.right));
        MidiLearner* learn_index = new MidiLearner(bounds["k:kindex-nn"], LearnMode::Note, PresetAction::KeyIndex, midi_handler);
        addChild(learn_index);

        addChild(createLabel(bounds["k:kcursor-label"], "Cursor", &styles.head));

        addChild(createLabel(bounds["k:kprev-label"], "Prev", &styles.right));
        MidiLearner* learn_prev = new MidiLearner(bounds["k:kprev-nn"], LearnMode::Note, PresetAction::KeyPrev, midi_handler);
        addChild(learn_prev);

        addChild(createLabel(bounds["k:knext-label"], "Next", &styles.right));
        MidiLearner* learn_next = new MidiLearner(bounds["k:knext-nn"], LearnMode::Note, PresetAction::KeyNext, midi_handler);
        addChild(learn_next);

        addChild(createLabel(bounds["k:krange-label"], "Range", &styles.head));
        addChild(createLabel(bounds["k:kstart-label"], "First", &styles.right));
        MidiLearner* learn_start = new MidiLearner(bounds["k:kstart-nn"], LearnMode::Note, PresetAction::KeyFirst, midi_handler);
        addChild(learn_start);

        // tab order
        learn_select->next_widget = learn_page;
        learn_page->next_widget   = learn_index;
        learn_index->next_widget  = learn_prev;
        learn_prev->next_widget   = learn_next;
        learn_next->next_widget   = learn_start;
        learn_start->next_widget  = learn_select;
        // shift + tab order
        learn_select->prev_widget = learn_start;
        learn_page->prev_widget   = learn_select;
        learn_index->prev_widget  = learn_page;
        learn_prev->prev_widget   = learn_index;
        learn_next->prev_widget   = learn_prev;
        learn_start->prev_widget  = learn_next;

        log_check = createThemedButton<CheckButton>(bounds["k:log"].pos, &my_svgs);
        log_check->set_sticky(true);
        log_check->latched = midi_handler->is_logging();
        log_check->setHandler([=](bool, bool){
            midi_handler->enable_logging(!midi_handler->is_logging());
        });
        addChild(log_check);
        my_svgs.changeTheme(svg_theme);
        addChild(createLabel(bounds["k:log-label"], "Log MIDI", &styles.left));
    }

    void step() override {
        Base::step();
        log_check->latched = midi_handler->is_logging();
        channel_info->set_text(0xFF == midi_handler->channel ? "[any]" : format_string("%d", midi_handler->channel));
        valid_blip->set_light_color(fromPacked(midi_handler->is_valid_configuration() ? colors::PortGreen : colors::Red));
        valid_blip->set_brightness(1.f);
        learn_blip->set_brightness(midi_handler->student ? 1.f : 0.f);
    }
};

void show_preset_midi_configuration(PresetUi* source, std::shared_ptr<svg_theme::SvgTheme> svg_theme) {
    SvgNoCache nocache;
    auto dlg = createDialog<ConfigPresetMidi, ModuleWidget>(source, Vec(source->box.size.x *.5, source->box.size.y *.5 - 15.f), &nocache, true);
    dlg->set_midi_handler(&source->my_module->preset_midi);
    dlg->create_ui(svg_theme);
}

};