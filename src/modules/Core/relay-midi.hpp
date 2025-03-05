#pragma once
#include <rack.hpp>
#include "../../em/EaganMatrix.hpp"
#include "../../em/midi-message.h"

namespace pachde{

struct RelayMidi : IDoMidi
{
    EaganMatrix* em{nullptr};
    std::vector<IDoMidi*> targets;

    void set_em(EaganMatrix* the_em) { em = the_em; }

    void register_target(IDoMidi* target) {
        assert(targets.cend() == std::find(targets.cbegin(), targets.cend(), target));
        targets.push_back(target);
    }

    void unregister_target(IDoMidi* target) {
        auto item = std::find(targets.cbegin(), targets.cend(), target);
        targets.erase(item);
    }

    void doMessage(PackedMidiMessage message) override {
        for (auto target: targets) {
            target->doMessage(message);
        }
        em->onMessage(message);
    }
};

}