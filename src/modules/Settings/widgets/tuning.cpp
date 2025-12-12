// Copyright (C) Paul Chase Dempsey
#include "tuning.hpp"

namespace pachde {
using namespace pachde;

Tuning unpackTuning(uint8_t packed) {
    if (packed <= ptFiftyTone) {
        return static_cast<Tuning>(packed);
    }
    if (in_range<uint8_t>(packed, ptJustC, ptJustB)) {
        return static_cast<Tuning>(Tuning::JustC + (packed - ptJustC));
    }
    if (in_range<uint8_t>(packed, ptUser1, ptUserLast)) {
        return static_cast<Tuning>(Tuning::UserTuning1 +  (packed - ptUser1));
    }
    return Tuning::EqualTuning;
}

uint8_t packTuning(Tuning tuning) {
    if (tuning <= Tuning::FiftyTone) {
        return static_cast<uint8_t>(tuning);
    }
    if (in_range(tuning, Tuning::JustC, Tuning::JustB)) {
        return ptJustC + (tuning - Tuning::JustC);
    }
    if (in_range(tuning, Tuning::UserTuning1, Tuning::UserTuningLast)) {
        return ptUser1 + (tuning - Tuning::UserTuning1);
    }
    return 0;
}

OptionMenuEntry * TuningMenu::createTuningMenuItem(Tuning tuningItem) {
    return new OptionMenuEntry(
        getParamValue() == tuningItem,
        createMenuItem(describeTuning(tuningItem), "", [=](){ setParamValue(tuningItem); }));
}

std::string describeTuning(Tuning grid) {
    switch (grid) {
    case Tuning::EqualTuning:  return "Equal"; break;
    case Tuning::JustC:  return "Just C"; break;
    case Tuning::JustCs: return "Just C#"; break;
    case Tuning::JustD:  return "Just D"; break;
    case Tuning::JustEb: return "Just Eb"; break;
    case Tuning::JustF:  return "Just F"; break;
    case Tuning::JustFs: return "Just F#"; break;
    case Tuning::JustG:  return "Just G"; break;
    case Tuning::JustAb: return "Just Ab"; break;
    case Tuning::JustA:  return "Just A"; break;
    case Tuning::JustBb: return "Just Bb"; break;
    case Tuning::JustB:  return "Just B"; break;
    default:
        if (in_range(grid, Tuning::OneTone, Tuning::FiftyTone)) {
            return format_string("%d-tone equal", 1 + grid - Tuning::OneTone);
        }
        if (in_range(grid, Tuning::UserTuning1, Tuning::UserTuningLast)) {
            return format_string("User %d", 1 + grid - Tuning::UserTuning1);
        }
        return "Unknown";
    }
}

std::string describePackedTuning(PackedTuning grid) {
    return describeTuning(unpackTuning(grid));
}

void TuningMenu::appendContextMenu(Menu *menu) {
    if (!module) return;
    menu->addChild(new MenuSeparator);
    menu->addChild(createTuningMenuItem(Tuning::EqualTuning));
    menu->addChild(createSubmenuItem("n-Tone Equal", "", [=](Menu * menu) {
        menu->addChild(createTuningMenuItem(Tuning::OneTone));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(2)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(3)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(4)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(5)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(6)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(7)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(8)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(9)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(10)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(11)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(17)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(19)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(22)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(24)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(26)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(31)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(43)));
        menu->addChild(createTuningMenuItem(Tuning::FiftyTone));
    }));
    menu->addChild(createSubmenuItem("Just", "", [=](Menu * menu) {
        menu->addChild(createTuningMenuItem(Tuning::JustC));
        menu->addChild(createTuningMenuItem(Tuning::JustCs));
        menu->addChild(createTuningMenuItem(Tuning::JustD));
        menu->addChild(createTuningMenuItem(Tuning::JustEb));
        menu->addChild(createTuningMenuItem(Tuning::JustF));
        menu->addChild(createTuningMenuItem(Tuning::JustFs));
        menu->addChild(createTuningMenuItem(Tuning::JustG));
        menu->addChild(createTuningMenuItem(Tuning::JustAb));
        menu->addChild(createTuningMenuItem(Tuning::JustA));
        menu->addChild(createTuningMenuItem(Tuning::JustBb));
        menu->addChild(createTuningMenuItem(Tuning::JustB));
    }));
    menu->addChild(createSubmenuItem("User-defined", "", [=](Menu * menu) {
        menu->addChild(createTuningMenuItem(Tuning::UserTuning1));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 1)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 2)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 3)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 4)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 5)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 6)));
        menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 7)));
    }));
}

}