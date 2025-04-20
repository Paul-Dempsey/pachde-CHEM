#include "filter-widget.hpp"

namespace pachde {

///Creation helpers
          
FilterButton* makeFilter (
    Vec pos,
    int rows,
    float item_width,
    std::vector<std::string>& items,
    const std::string&name,
    const char * up_svg,
    const char * down_svg,
    SvgThemeEngine &engine,
    std::shared_ptr<SvgTheme> theme,
    std::function<void(uint64_t item)> on_change
    )
{
    FilterButton* o = new FilterButton(up_svg, down_svg, on_change);
    o->box.pos = pos;
    o->init(name, rows, item_width, items, engine, theme);
    return o;
}

FilterButton* makeCatFilter(Vec pos, SvgThemeEngine &engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change)
{
    std::vector<std::string> info {
        "Classic",
        "Control Voltage",
        "Drone",
        "Keyboard",
        "Midi",
        "Other",
        "Percussion",
        "Processor",
        "Tuned Percussion",
        "Strings",
        "Utility",
        "Vocal",
        "Winds",
        "(Unknown)"
    };
    return makeFilter(
        pos,
        5, 56.f,
        info,
        "Category",
        "res/widgets/category-button.svg",
        "res/widgets/category-button-down.svg",
        engine, theme, on_change
    );
}

FilterButton* makeTypeFilter(Vec pos, SvgThemeEngine &engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change)
{
    std::vector<std::string> info {
        "Atonal",
        "Bass",
        "Bowed",
        "Brass",
        "Demo Preset",
        "Electric Piano",
        "Flute",
        "Lead",
        "Organ",
        "Pad",
        "Plucked",
        "Double Reed",
        "Single Reed",
        "Struck"
    };
    return makeFilter(
        pos,
        5, 50.f, 
        info,
        "Type",
        "res/widgets/type-button.svg",
        "res/widgets/type-button-down.svg",
        engine, theme, on_change
    );
}

FilterButton* makeCharacterFilter(Vec pos, SvgThemeEngine &engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change)
{
    std::vector<std::string> info {
        "Acoustic",
        "Aggressive",
        "Airy",
        "Analog",
        "Arpeggio",
        "Big",
        "Bright",
        "Chords",
        "Clean",
        "Dark",
        "Digital",
        "Distorted",
        "Dry",
        "Echo",
        "Electric",
        "Ensemble",
        "Evolving",
        "FM",
        "Hybrid",
        "Icy",
        "Intimate",
        "Lo-fi",
        "Looping",
        "Layered",
        "Morphing",
        "Metallic",
        "Nature",
        "Noise",
        "Random",
        "Reverberant",
        "Sound Design",
        "Stereo",
        "Shaking",
        "Simple",
        "Soft",
        "Strumming",
        "Synthetic",
        "Warm",
        "Woody"
    };
    return makeFilter(
        pos,
        10,
        50.f,
        info,
        "Character",
        "res/widgets/character-button.svg", 
        "res/widgets/character-button-down.svg",
        engine, theme, on_change
    );
}

FilterButton* makeMatrixFilter(Vec pos, SvgThemeEngine &engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change)
{
    std::vector<std::string> info {
        "Additive",
        "BiqBank",
        "BiqGraph",
        "BiqMouth",
        "Cutoff Mod",
        "Formula Delay",
        "Micro Delay",
        "Sum Delay",
        "Voice Delay",
        "HarMan",
        "Kinetic",
        "ModMan",
        "Osc Jenny",
        "Osc Phase",
        "Osc DSF",
        "SineBank",
        "SineSpray",
        "WaveBank"
    };
    return makeFilter(
        pos,
        6,
        50.f,
        info,
        "Matrix",
        "res/widgets/matrix-button.svg",
        "res/widgets/matrix-button-down.svg",
        engine, theme, on_change
    );
}

FilterButton* makeSettingFilter(Vec pos, SvgThemeEngine &engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change)
{
    std::vector<std::string> info {
        "Channel 1",
        "External Midi Clock",
        "Mono Interval",
        "Portamento",
        "Rounding",
        "Split Voice",
        "Single Voice",
        "Touch Area"
    };
    return makeFilter(
        pos,
        4,
        64.f,
        info,
        "Setting",
        "res/widgets/gear-button.svg",
        "res/widgets/gear-button-down.svg",
        engine,
        theme,
        on_change
    );
}

// Impl

FilterButton::FilterButton(const char *svg_up, const char *svg_down, std::function<void(uint64_t item)> on_change):
    up_svg(svg_up),
    down_svg(svg_down),
    change_fn(on_change)
{
    this->shadow->hide();
}

void FilterButton::init(const std::string &name, int rows, float item_width, const std::vector<std::string> &items, SvgThemeEngine &engine, std::shared_ptr<SvgTheme> theme)
{
    if (item_width <= 0.f) item_width = 42;
    applyTheme(engine, theme);

    const float DIALOG_SEP = 3.5f;

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

bool FilterButton::applyTheme(SvgThemeEngine &engine, std::shared_ptr<SvgTheme> theme)
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
    if (dialog && !dialog->isVisible()) {
        // dialog is not a child widget when not shown
        ApplyChildrenTheme(dialog, engine, theme);
    }
    return true;
}


void FilterButton::onAction(const ActionEvent &e)
{
    {
        destroyTip();
        if (!dialog->isVisible()) {
            auto parent = getParent();
            if (parent) {
                // selfish
                for (auto child: parent->children) {
                    auto peer = dynamic_cast<FilterButton*>(child);
                    if (peer) peer->close_dialog();
                }
                parent->addChild(dialog);
                dialog->setVisible(true);
                APP->event->setSelectedWidget(dialog);
            }
        }
    }
}

void FilterButton::step()
{
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
    }
}

}