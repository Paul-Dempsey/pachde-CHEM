#include "my-plugin.hpp"
#include "../Preset.hpp"
#include "services/colors.hpp"
using namespace packed_color;
#include "services/midi-note.hpp"
#include "services/svg-query.hpp"
#include "services/svg-theme.hpp"
#include "services/text.hpp"
using namespace svg_theme;
#define DIALOG_THEMED
#include "widgets/blip-widget.hpp"
#include "widgets/channel-menu.hpp"
#include "widgets/dialog.hpp"
#include "widgets/dialog-help.hpp"
#include "widgets/info-symbol.hpp"
#include "widgets/label.hpp"
#include "widgets/menu-widgets.hpp"
#include "widgets/option-widget.hpp"
#include "widgets/keys.hpp"
#include "midi-learn.hpp"

namespace widgetry {

struct DialogSvg {
    static std::string background() {
        return asset::plugin(pluginInstance, "res/dialogs/preset-midi-config.svg");
    }
};

static const char * MIDI_CONFIG_HELP_URL = "https://github.com/Paul-Dempsey/pachde-CHEM/blob/main/doc/preset-nav.md#navigating-presets-via-midi";

#ifdef MIDI_CHECK
static const char * CHECK_INFO =
    "Click in the learn box to check a controller for its type.\n"
    "• For buttons, press twice.\n"
    "• For knobs & faders, go through the entire range\n"
    "• For an endless encoder turn through a full revolution.\n"
    "Click the reset button to start a new test."
    ;
#endif

struct ConfigPresetMidi : SvgDialog<DialogSvg> {
    using Base = SvgDialog<DialogSvg>;

    SvgCache my_svgs;
    DialogStyles styles;
    LabelStyle info_right{"dlg-info", HAlign::Right, VAlign::Middle, colors::PortCorn, 12.f, false};
    LabelStyle info_left{"dlg-info", HAlign::Left, VAlign::Middle, colors::PortCorn, 12.f, false};
    PresetMidi* midi_handler{nullptr};
    Blip* learn_blip{nullptr};
    Blip* valid_blip{nullptr};
    CheckButton* log_check{nullptr};

    // KEYBOARD
    TextLabel* channel_info{nullptr};
    KeyboardWidget* keys{nullptr};
    PackedColor co_send  {0xff5dcff8};
    PackedColor co_paging{0xff4ef94e};
    PackedColor co_cursor{0xfff9de48};
    PackedColor co_prev  {0xff8174f7};
    PackedColor co_next  {0xff56faf2};
    PackedColor co_first {0xfff958aa};

    // CONTROLLER
    TextLabel* cc_channel_info{nullptr};
    LearnMidiNote* learn_send{nullptr};
    LearnMidiNote* learn_page{nullptr};
    LearnMidiNote* learn_index{nullptr};
    LearnMidiNote* learn_prev{nullptr};
    LearnMidiNote* learn_next{nullptr};
    LearnMidiNote* learn_start{nullptr};

    LearnMidiCc* learn_cc_send{nullptr};
    LearnMidiCc* learn_cc_toggle{nullptr};
    LearnMidiCc* learn_cc_prev{nullptr};
    LearnMidiCc* learn_cc_next{nullptr};
    LearnMidiCc* learn_cc_page{nullptr};
    LearnMidiCc* learn_cc_item{nullptr};

    TextLabel* cc_send_type{nullptr};

#ifdef MIDI_CHECK
    LearnMidiControlType* learn_cc{nullptr};
    ControllerType last_controller_type{ControllerType::Unknown};
    TextLabel* cctype_info{nullptr};
    TextLabel* cc_min{nullptr};
    TextLabel* cc_max{nullptr};
    TextLabel* cc_time{nullptr};
    bool request_test{false};
#endif

    ConfigPresetMidi(ModuleWidget* source, ILoadSvg* svg_loader) :
        Base(source, svg_loader)
    {}

    void set_midi_handler(PresetMidi* midi) { midi_handler = midi; }

    void init_colors(std::shared_ptr<svg_theme::SvgTheme> svg_theme) {
        info_right.applyTheme(svg_theme);
        info_left.applyTheme(svg_theme);
        if (!svg_theme->getFillColor(co_send,   "co-send",    true)) { co_send   = 0xff5dcff8; }
        if (!svg_theme->getFillColor(co_paging, "co-paging",  true)) { co_paging = 0xff4ef94e; }
        if (!svg_theme->getFillColor(co_cursor, "co-cursor",  true)) { co_cursor = 0xfff9de48; }
        if (!svg_theme->getFillColor(co_prev,   "co-prev",    true)) { co_prev   = 0xff8174f7; }
        if (!svg_theme->getFillColor(co_next,   "co-next",    true)) { co_next   = 0xff56faf2; }
        if (!svg_theme->getFillColor(co_first,  "co-first",   true)) { co_first  = 0xfff958aa; }
    }

    void create_ui(std::shared_ptr<svg_theme::SvgTheme> svg_theme) {
        //InfoSymbol* info{nullptr};

        auto layout = Base::get_svg();
        applySvgTheme(layout, svg_theme);
        styles.note.key = "dlg-label";
        styles.info.halign = HAlign::Right;
        styles.initStyles(svg_theme);
        init_colors(svg_theme);
        ::svg_query::BoundsIndex bounds;
        svg_query::addBounds(layout, "k:", bounds, true);
        //svg_query::addBounds(layout, "p:", bounds, false); // visible placeholders

        add_close_button(this, bounds, "k:close", svg_theme);
        addChild(createLabel(bounds["k:dlg-title"], "Preset | Midi Navigation", &styles.title));

        {
            auto help_btn = Center(createThemedButton<QuestionButton>(bounds["k:help"].getCenter(), &my_svgs, "Show Help"));
            help_btn->set_handler([](bool,bool){
                system::openBrowser(MIDI_CONFIG_HELP_URL);
            });
            addChild(help_btn);
        }

        Rect r = bounds["k:valid"];
        addChild(valid_blip = createWidgetCentered<Blip>(r.getCenter()));
        valid_blip->set_radius(r.size.x * .5);
        addChild(createLabel(bounds["k:valid-label"], "ok", &styles.note));

        r = bounds["k:learn"];
        addChild(learn_blip = createWidgetCentered<Blip>(r.getCenter()));
        learn_blip->set_radius(r.size.x * .5);
        learn_blip->set_light_color(fromPacked(parseColor("hsl(120, .8, .5)", colors::PortGreen)));
        addChild(createLabel(bounds["k:learn-label"], "learn", &styles.note));

        addChild(createLabel(bounds["k:key-label"], "Keyboard", &styles.section));

        auto channel_menu = createWidgetCentered<ChannelMenu>(bounds["k:kchannel"].getCenter());
        channel_menu->poke = &midi_handler->key_channel;
        channel_menu->describe("Channel");
        addChild(channel_menu);
        addChild(channel_info = createLabel(bounds["k:kchannel-info"], "", &info_left));

        addChild(keys = new KeyboardWidget(bounds["k:keys"]));

        addChild(createLabel(bounds["k:ksend-label"],   "Send",    &styles.left));
        addChild(createLabel(bounds["k:kpaging-label"], "Paging",  &styles.left));
        addChild(createLabel(bounds["k:kcursor-label"], "Cursor*", &styles.left));
        addChild(createLabel(bounds["k:kprev-label"],   "Prev",    &styles.left));
        addChild(createLabel(bounds["k:knext-label"],   "Next",    &styles.left));
        addChild(createLabel(bounds["k:kfirst-label"],  "First*",  &styles.left));
        addChild(learn_send  = new LearnMidiNote(bounds["k:ksend-nn"],   KeyAction::KeySend,   midi_handler));
        addChild(learn_page  = new LearnMidiNote(bounds["k:kpaging-nn"], KeyAction::KeyPaging, midi_handler));
        addChild(learn_index = new LearnMidiNote(bounds["k:kcursor-nn"], KeyAction::KeyCursor, midi_handler));
        addChild(learn_prev  = new LearnMidiNote(bounds["k:kprev-nn"],   KeyAction::KeyPrev,   midi_handler));
        addChild(learn_next  = new LearnMidiNote(bounds["k:knext-nn"],   KeyAction::KeyNext,   midi_handler));
        addChild(learn_start = new LearnMidiNote(bounds["k:kfirst-nn"],  KeyAction::KeyFirst,  midi_handler));

        // CONTROLLER

        addChild(createLabel(bounds["k:cc-label"], "Controller", &styles.section));

        channel_menu = createWidgetCentered<ChannelMenu>(bounds["k:cc-channel"].getCenter());
        channel_menu->poke = &midi_handler->cc_channel;
        channel_menu->describe("Channel");
        addChild(channel_menu);
        addChild(cc_channel_info = createLabel(bounds["k:cc-channel-info"], "", &info_left));
        addChild(cc_send_type = createLabel(bounds["k:cc-sendtype"], "", &styles.info));
        addChild(createLabel(bounds["k:buttons-label"],      "buttons", &styles.head));
        addChild(createLabel(bounds["k:knobs-faders-label"], "knobs | faders", &styles.head));
        addChild(createLabel(bounds["k:cc-send-label"],   "Send", &styles.left));
        addChild(createLabel(bounds["k:cc-toggle-label"], "Paging/Cursor", &styles.left));
        addChild(createLabel(bounds["k:cc-prev-label"],   "Prev", &styles.left));
        addChild(createLabel(bounds["k:cc-next-label"],   "Next", &styles.left));
        addChild(createLabel(bounds["k:cc-page-label"],   "Page", &styles.left));
        addChild(createLabel(bounds["k:cc-item-label"],   "Item", &styles.left));
        addChild(learn_cc_send   = new LearnMidiCc(bounds["k:cc-send"],   ccAction::Send,   midi_handler));
        addChild(learn_cc_toggle = new LearnMidiCc(bounds["k:cc-toggle"], ccAction::Toggle, midi_handler));
        addChild(learn_cc_prev   = new LearnMidiCc(bounds["k:cc-prev"],   ccAction::Prev,   midi_handler));
        addChild(learn_cc_next   = new LearnMidiCc(bounds["k:cc-next"],   ccAction::Next,   midi_handler));
        addChild(learn_cc_page   = new LearnMidiCc(bounds["k:cc-page"],   ccAction::Page,   midi_handler));
        addChild(learn_cc_item   = new LearnMidiCc(bounds["k:cc-item"],   ccAction::Item,   midi_handler));

        learn_cc_send->set_controller_type_handler (
            [=](ControllerType ct) {
                cc_send_type->set_text(ct == ControllerType::Unknown ? "" : controller_type_name(ct));
            }
        );

        // restore persisted settings
        for (const CcControl& ctl: midi_handler->cc_control) {
            switch (ctl.role) {
            case ccAction::Unknown: assert(false); break;
            case ccAction::Send:   learn_cc_send->init(ctl); break;
            case ccAction::Page:   learn_cc_page->init(ctl); break;
            case ccAction::Item:   learn_cc_item->init(ctl); break;
            case ccAction::Toggle: learn_cc_toggle->init(ctl); break;
            case ccAction::Prev:   learn_cc_prev->init(ctl); break;
            case ccAction::Next:   learn_cc_next->init(ctl); break;
            }
        }

        // Check
#ifdef MIDI_CHECK
        info = createWidgetCentered<InfoSymbol>(bounds["k:cc-test-info"].getCenter());
        info->describe(CHECK_INFO);
        addChild(info);
        addChild(createLabel(bounds["k:cc-test-label"], "Check", &styles.right));
        addChild(learn_cc = new LearnMidiControlType(bounds["k:cc-test"], midi_handler));
        addChild(cctype_info = createLabel(bounds["k:cc-type--blank"], "", &styles.info));
        addChild(cc_min = createLabel(bounds["p:cc-min--blank"], "", &styles.info));
        addChild(cc_max = createLabel(bounds["p:cc-max--blank"], "", &styles.info));
        addChild(cc_time = createLabel(bounds["k:test-time"], "", &styles.info));
        auto reset_test = createThemedButton<SmallRoundButton>(bounds["k:test-reset"].getCenter(), load_svg, "Start/reset check");
        reset_test->set_handler([=](bool,bool) {
            request_test = true;
        });
        addChild(Center(reset_test));
#endif
        // tab order
        learn_send->next_widget      = learn_page;
        learn_page->next_widget      = learn_index;
        learn_index->next_widget     = learn_prev;
        learn_prev->next_widget      = learn_next;
        learn_next->next_widget      = learn_start;
        learn_start->next_widget     = learn_cc_send;
        learn_cc_send->next_widget   = learn_cc_toggle;
        learn_cc_toggle->next_widget = learn_cc_prev;
        learn_cc_prev->next_widget   = learn_cc_next;
        learn_cc_next->next_widget   = learn_cc_page;
        learn_cc_page->next_widget   = learn_cc_item;
        learn_cc_item->next_widget   = learn_send;

        // shift + tab order
        learn_send->prev_widget      = learn_cc_item;
        learn_page->prev_widget      = learn_send;
        learn_index->prev_widget     = learn_page;
        learn_prev->prev_widget      = learn_index;
        learn_next->prev_widget      = learn_prev;
        learn_start->prev_widget     = learn_next;
        learn_cc_send->prev_widget   = learn_start;
        learn_cc_toggle->prev_widget = learn_cc_send;
        learn_cc_prev->prev_widget   = learn_cc_toggle;
        learn_cc_next->prev_widget   = learn_cc_prev;
        learn_cc_page->prev_widget   = learn_cc_next;
        learn_cc_item->prev_widget   = learn_cc_page;

        // Logging
        log_check = createThemedButton<CheckButton>(bounds["k:log"].pos, &my_svgs);
        log_check->set_sticky(true);
        log_check->latched = midi_handler->is_logging();
        log_check->sync_frame();
        log_check->set_handler([=](bool, bool){
            midi_handler->enable_logging(!midi_handler->is_logging());
        });
        addChild(log_check);

        // Midi Connection
        addChild(createLabel(bounds["k:conn"], midi_handler->connection_name(), &info_right));
        addChild(createLabel(bounds["k:log-label"], "Log MIDI", &styles.left));

        my_svgs.changeTheme(svg_theme);
        applyChildrenTheme(this, svg_theme);
    }

    // helper to set learned key's color in the keys widget
    void set_learn_key(KeyAction action, eNote note, int octave, LearnMidiNote* student) {
        auto nn = MidiNote(note, octave).number();
        midi_handler->key_code[action] = nn;
        student->value = nn;
        student->update_text();
    }

    void step() override {
        Base::step();

        log_check->latched = midi_handler->is_logging();
        log_check->sync_frame();

        channel_info->set_text(0xFF == midi_handler->key_channel ? "any" : format_string("%d", midi_handler->key_channel));
        cc_channel_info->set_text(0xFF == midi_handler->cc_channel ?  "any" : format_string("%d", midi_handler->cc_channel));
        valid_blip->set_light_color(fromPacked(midi_handler->is_valid_key_configuration() ? colors::PortGreen : colors::Red));
        valid_blip->set_brightness(1.f);

        learn_blip->set_brightness(midi_handler->student ? 1.f : 0.f);

#ifdef MIDI_CHECK
        if (request_test) {
            request_test = false;
            learn_cc->reset();
            APP->event->setSelectedWidget(learn_cc);
        }
        cc_min->set_text(learn_cc->min_text);
        cc_max->set_text(learn_cc->max_text);
        if (learn_cc->learning()) {
            cc_time->set_text(format_string("%.1f", learn_cc->remaining()));
        } else {
            cc_time->set_text("");
        }
        if (last_controller_type != learn_cc->controller_type) {
            last_controller_type = learn_cc->controller_type;
            cctype_info->set_text(controller_type_name(last_controller_type));
        }
#endif
        // TODO: some way to not update on step - general change notification?
        keys->reset_colors();
        bool send_only;
        if (UndefinedCode != midi_handler->key_code[KeySend]) {
            keys->set_color(eNoteFromNoteNumber(midi_handler->key_code[KeySend]), co_send);
            send_only = true;
        } else {
            send_only = false;
        }
        if (UndefinedCode != midi_handler->key_code[KeyPaging]) {
            keys->set_color(eNoteFromNoteNumber(midi_handler->key_code[KeyPaging]), co_paging);
            send_only = false;
        }
        if (UndefinedCode != midi_handler->key_code[KeyCursor]) {
            keys->set_color(eNoteFromNoteNumber(midi_handler->key_code[KeyCursor]), co_cursor);
            send_only = false;
        }
        if (UndefinedCode != midi_handler->key_code[KeyPrev]) {
            keys->set_color(eNoteFromNoteNumber(midi_handler->key_code[KeyPrev]), co_prev);
            send_only = false;
        }
        if (UndefinedCode != midi_handler->key_code[KeyNext]) {
            keys->set_color(eNoteFromNoteNumber(midi_handler->key_code[KeyNext]), co_next);
            send_only = false;
        }
        if (UndefinedCode != midi_handler->key_code[KeyFirst]) {
            keys->set_color(eNoteFromNoteNumber(midi_handler->key_code[KeyFirst]), co_first);
            send_only = false;
        }
        if (send_only) {
            auto nn = midi_handler->key_code[KeySend];
            auto octave = MidiNote(nn).octave();
            auto note = eNoteFromNoteNumber(nn);
            switch (note) {
            case eNote::F: {
                set_learn_key(KeyPaging, eNote::Fs, octave, learn_page);
                set_learn_key(KeyCursor, eNote::Ab, octave, learn_index);
                set_learn_key(KeyPrev, eNote::G, octave, learn_prev);
                set_learn_key(KeyNext, eNote::A, octave, learn_next);
                set_learn_key(KeyFirst, eNote::Bb, octave, learn_start);
            } break;
            case eNote::C: {
                set_learn_key(KeyPaging, eNote::D, octave, learn_page);
                set_learn_key(KeyPrev, eNote::Cs, octave, learn_prev);
                set_learn_key(KeyNext, eNote::Eb, octave, learn_next);
                set_learn_key(KeyFirst, eNote::E, octave, learn_start);
            } break;
            default: break;
            }
        }
    }

 #ifdef MIDI_CHECK
    void draw (const DrawArgs& args) override {
        Base::draw(args);
        if (learn_cc->msg_count) {
            auto vg = args.vg;
            auto t = format_string("%d [%c%c%c]", learn_cc->msg_count, learn_cc->ztoa ? '<':'-', learn_cc->atoz ? '>':'-', learn_cc->interior  ? 'i':'-');
            draw_text_box(vg, 218, 250, 60, 14, 0,0,0,0, t, GetPluginFontRegular(), 10, colors::PortYellow, HAlign::Center, VAlign::Middle);
        }
    }
#endif
};

void show_preset_midi_configuration(PresetUi* source, std::shared_ptr<svg_theme::SvgTheme> svg_theme) {
    auto dlg = createDialog<ConfigPresetMidi, ModuleWidget>(source, Vec(source->box.size.x *.5, source->box.size.y *.5 - 15.f), &source->module_svgs, true);
    dlg->set_midi_handler(&source->my_module->preset_midi);
    dlg->create_ui(svg_theme);
}

};