#include "filter-widget.hpp"

namespace pachde {

// Creation helpers

StateButton* makeFilterStateButton(
    Vec pos,
    ILoadSvg* loader,
    std::function<void()> on_click
)
{
    StateButton* sb = new StateButton(
        "res/widgets/filtered-button-up.svg",
        "res/widgets/filtered-button-down.svg",
        on_click
    );
    sb->box.pos = pos;
    sb->loadSvg(loader);
    return sb;
}

// filter button

FilterButton* makeFilter (
    Vec pos,
    int rows,
    float item_width,
    std::vector<const char *>& items,
    const std::string&name,
    const char * up_svg,
    const char * down_svg,
    ILoadSvg* loader,
    std::shared_ptr<svg_theme::SvgTheme> theme,
    std::function<void(uint64_t item)> on_change
    )
{
    FilterButton* o = new FilterButton(up_svg, down_svg, on_change);
    o->box.pos = pos;
    o->init(name, rows, item_width, items, loader, theme);
    return o;
}

static std::vector<const char *> cat_info {
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
FilterButton* makeCatFilter(Vec pos, ILoadSvg* loader, std::shared_ptr<svg_theme::SvgTheme> theme, std::function<void(uint64_t item)> on_change)
{
    return makeFilter(
        pos, 5, 75, cat_info,
        "Category",
        "res/widgets/category-button.svg",
        "res/widgets/category-button-down.svg",
        loader, theme, on_change
    );
}

static std::vector<const char *> type_info {
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
FilterButton* makeTypeFilter(Vec pos, ILoadSvg* loader, std::shared_ptr<svg_theme::SvgTheme> theme, std::function<void(uint64_t item)> on_change)
{
    return makeFilter(
        pos, 5, 70.f, type_info,
        "Type",
        "res/widgets/type-button.svg",
        "res/widgets/type-button-down.svg",
        loader, theme, on_change
    );
}

static std::vector<const char *> character_info {
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

FilterButton* makeCharacterFilter(Vec pos, ILoadSvg* loader, std::shared_ptr<svg_theme::SvgTheme> theme, std::function<void(uint64_t item)> on_change)
{
    return makeFilter(
        pos, 10, 70.f, character_info,
        "Character",
        "res/widgets/character-button.svg",
        "res/widgets/character-button-down.svg",
        loader, theme, on_change
    );
}
static std::vector<const char *> matrix_info {
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

FilterButton* makeMatrixFilter(Vec pos, ILoadSvg* loader, std::shared_ptr<svg_theme::SvgTheme> theme, std::function<void(uint64_t item)> on_change)
{
    return makeFilter(
        pos, 6, 70.f, matrix_info,
        "Matrix",
        "res/widgets/matrix-button.svg",
        "res/widgets/matrix-button-down.svg",
        loader, theme, on_change
    );
}

static std::vector<const char *> settings_info {
    "Channel 1",
    "External Midi Clock",
    "Mono Interval",
    "Portamento",
    "Rounding",
    "Split Voice",
    "Single Voice",
    "Touch Area"
};

FilterButton* makeSettingFilter(Vec pos, ILoadSvg* loader, std::shared_ptr<svg_theme::SvgTheme> theme, std::function<void(uint64_t item)> on_change)
{
    return makeFilter(
        pos, 4, 90.f, settings_info,
        "Setting",
        "res/widgets/gear-button.svg",
        "res/widgets/gear-button-down.svg",
        loader, theme, on_change
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

void FilterButton::init(const std::string &name, int rows, float item_width, std::vector<const char *>& items, ILoadSvg* loader, std::shared_ptr<svg_theme::SvgTheme> theme)
{
    if (item_width <= 0.f) item_width = 80;
    loadSvg(loader);

    const float DIALOG_SEP = 3.5f;

    dialog = new BitsWidget(name, rows, item_width, items, theme,
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

void FilterButton::loadSvg(ILoadSvg* loader)
{
    frames.clear();
    addFrame(loader->loadSvg(asset::plugin(pluginInstance, up_svg)));
    addFrame(loader->loadSvg(asset::plugin(pluginInstance, down_svg)));
    sw->setSvg(frames[0]);
    fb->dirty = true;
}


void FilterButton::onAction(const ActionEvent &e)
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