#pragma once
#include <rack.hpp>
#include "em/EaganMatrix.hpp"
#include "em/midi-message.h"

namespace pachde{

struct RelayMidi : IDoMidi
{
    eaganmatrix::EaganMatrix* em{nullptr};
    std::vector<IDoMidi*> targets;

    void set_em(eaganmatrix::EaganMatrix* the_em) { em = the_em; }

    void register_target(IDoMidi* target) {
        assert(targets.cend() == std::find(targets.cbegin(), targets.cend(), target));
        targets.push_back(target);
    }

    void unregister_target(IDoMidi* target) {
        auto item = std::find(targets.cbegin(), targets.cend(), target);
        targets.erase(item);
    }

    void do_message(PackedMidiMessage message) override {
        // em first, so targets can use the em's handling of complex processing like hi-res values
        em->onMessage(message);

        for (auto target: targets) {
            target->do_message(message);
        }
    }
};

}