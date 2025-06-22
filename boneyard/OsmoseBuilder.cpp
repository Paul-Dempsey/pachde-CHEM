#include "Preset.hpp"

constexpr const double OSMOSE_PRESET_START_TIME {0.75};
constexpr const double OSMOSE_PRESET_RESPONSE_TIME {2.5};
constexpr const double OSMOSE_SETTLE_TIME {2.0};

PresetId OsmoseBuilder::make_id()
{
    return PresetId{cc0, 0, pc};
}

void OsmoseBuilder::init_system(HakenMidi *haken)
{
    init(haken, PresetTab::System, 30, 34);
}

void OsmoseBuilder::init_user(HakenMidi* haken) {
    init(haken, PresetTab::User, 90, 90);
}

void OsmoseBuilder::init(HakenMidi* haken, PresetTab which, uint8_t base, uint8_t last)
{
    log = haken->log;
    tab = which;
    base_page = cc0 = base;
    last_page = last;
    pc = 0;
    send(haken);
}

void OsmoseBuilder::preset_started()
{
    if (state == Start || state == End) return; // ignore spurious preceived before we've sent a rquest
    assert(state == PendingPresetBegin);
    auto current = system::getUnixTime();
    if (log) log->log_message("OB", format_string("Preset Started: %d %d in %f", cc0, pc, (float)(current - state_time)));
    state = State::PendingPresetReceived;
    state_time = current;
}

void OsmoseBuilder::preset_received()
{
    if (state != State::PendingPresetReceived) return; // ignore spurious preceived before we've sent a rquest
    auto current = system::getUnixTime();
    if (log) log->log_message("OB", format_string("Preset Received: %d %d in %f", cc0, pc, (float)(current - state_time)));
    state = State::Settle;
    state_time = current;
}

OsmoseBuilder::ReadyResponse OsmoseBuilder::ready()
{
    ReadyResponse result = ReadyResponse::Timeout;

    switch (state) {
    case State::Start:
    case State::Ready:
        result = ReadyResponse::Ready;
        break;

    case State::PendingPresetBegin:
        // waiting for preset_started()
        if ((system::getUnixTime() - state_time) > OSMOSE_PRESET_START_TIME) {
            state = State::Ready;
            result = ReadyResponse::Timeout;
        } else {
            result = ReadyResponse::Waiting;
        }
        break;

    case State::PendingPresetReceived:
        // waiting for preset_received()
        if ((system::getUnixTime() - state_time) > OSMOSE_PRESET_RESPONSE_TIME) {
            state = State::End;
            result = ReadyResponse::Fail;
        } else {
            result = ReadyResponse::Waiting;
        }
        break;

    case State::Settle:
        if ((system::getUnixTime() - state_time) > OSMOSE_SETTLE_TIME) {
            state = State::Ready;
            result = ReadyResponse::Ready;
        } else {
            result = ReadyResponse::Waiting;
        }
        break;

    case State::End:
        result = ReadyResponse::End;
        break;
    }
    // if (log) log->log_message("OB", format_string("Ready-result %s", 
    //     result == ReadyResponse::Ready ? "Ready" : result == ReadyResponse::Waiting ? "Waiting" : result == ReadyResponse::Timeout ? "Timeout" : "????"));
    return result;
}

void OsmoseBuilder::send(HakenMidi* haken)
{
    state = PendingPresetBegin;
    state_time = system::getUnixTime();
    haken->select_preset(ChemId::Preset, make_id());
    if (log) log->log_message("OB", format_string("Sending [%d:%d] at %f", cc0, pc, state_time - start_time));
}

bool OsmoseBuilder::next(HakenMidi* haken)
{
    assert(State::Ready == state);
    if (127 == pc) {
        if (cc0 == last_page) {
            state = State::End;
            return false;
        }
        pc = 0;
        ++cc0;
    } else {
        ++pc;
    }
    send(haken);
    return true;
}
