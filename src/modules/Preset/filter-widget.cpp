#include "filter-widget.hpp"

namespace pachde {

template <typename TFilter>
TFilter* makeFilter(
    Vec pos,
    const std::string&name,
    int rows,
    float item_width,
    std::vector<std::string>& items,
    SvgThemeEngine &engine,
    std::shared_ptr<SvgTheme> theme,
    std::function<void(uint64_t item)> on_change
    )
{
    TFilter* o = new TFilter();
    o->box.pos = pos;
    o->init(name, rows, item_width, items, engine, theme, on_change);
    return o;
}

CatFilter* makeCatFilter(Vec pos, SvgThemeEngine &engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change)
{
    std::vector<std::string> info {
        "Strings",
        "Winds",
        "Vocal",
        "Keyboard",
        "Classic",
        "Other",
        "Percussion",
        "Tuned Percussion",
        "Processor",
        "Drone",
        "Midi",
        "Control Voltage",
        "Utility"
    };
    return makeFilter<CatFilter>(pos, "Category", 5, 56.f, info, engine, theme, on_change);
}

TypeFilter* makeTypeFilter(Vec pos, SvgThemeEngine &engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change)
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
    return makeFilter<TypeFilter>(pos, "Type",  5, 50.f, info, engine, theme, on_change);
}

CharacterFilter* makeCharacterFilter(Vec pos, SvgThemeEngine &engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change)
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
    return makeFilter<CharacterFilter>(pos, "Character", 10, 50.f, info, engine, theme, on_change);
}

MatrixFilter* makeMatrixFilter(Vec pos, SvgThemeEngine &engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change)
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
    return makeFilter<MatrixFilter>(pos, "Matrix", 6, 50.f, info, engine, theme, on_change);
}

GearFilter* makeGearFilter(Vec pos, SvgThemeEngine &engine, std::shared_ptr<SvgTheme> theme, std::function<void(uint64_t item)> on_change)
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
    return makeFilter<GearFilter>(pos, "Setting", 4, 60.f, info, engine, theme, on_change);
}

}