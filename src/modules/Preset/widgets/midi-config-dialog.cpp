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
#include "widgets/channel-menu.hpp"
#include "widgets/dialog.hpp"
#include "widgets/dialog-help.hpp"
#include "widgets/info-symbol.hpp"
#include "widgets/label.hpp"
#include "widgets/menu-widgets.hpp"
#include "widgets/option-widget.hpp"
#include "midi-learn.hpp"

namespace widgetry {

struct DialogSvg {
    static std::string background() {
        return asset::plugin(pluginInstance, "res/dialogs/preset-midi-config.svg");
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