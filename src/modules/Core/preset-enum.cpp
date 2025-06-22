#include "preset-enum.hpp"
#include "../../em/preset-list.hpp"

namespace pachde {

// ---- HakenPresetEnumerator  ----

bool HakenPresetEnumerator::next(HakenMidi* haken)
{
    if (current >= ids.size()) return false;
    PresetId id = ids[current++];
    assert(id.valid());
    haken->select_preset(chem_id, id);
    return true;
}

// ---- OsmosePresetEnumerator  ----

bool OsmosePresetEnumerator::next(HakenMidi* haken)
{
    if (index >= 127) {
        index = 0;
        ++page;
        if (page > last_page) return false;
    }
    if (haken->log) {
        haken->log->log_message("OPE", format_string("Preset [%d.%d]", page, index));
    }
    haken->send_message(Tag(MakeCC(Haken::ch1, Haken::ccBankH, page), chem_id));
    haken->send_message(Tag(MakeProgramChange(Haken::ch1, index), chem_id));
    ++index;
    return true;
}

// ---- PresetListBuildCoordinator  ----

void PresetListBuildCoordinator::configure(MidiLog *logger, bool osmose, IEnumeratePresets *iterator)
{
    log = logger;
    iter = iterator;
    if (osmose) {
        begin_timeout = OSMOSE_PRESET_START_TIME;
        receive_timeout = OSMOSE_PRESET_RESPONSE_TIME;
        settle_timeout = OSMOSE_SETTLE_TIME;
    } else {
        begin_timeout = HAKEN_PRESET_START_TIME;
        receive_timeout = HAKEN_PRESET_RESPONSE_TIME;
        settle_timeout = HAKEN_SETTLE_TIME;
    }
}

void PresetListBuildCoordinator::start_building()
{
    assert(iter);
    phase = Phase::Start;
}

void PresetListBuildCoordinator::preset_started()
{
    assert(phase == Phase::PendBegin);
    phase = Phase::Begin;
}

void PresetListBuildCoordinator::preset_received()
{
    assert(phase == Phase::PendReceive);
    phase = Phase::Receive;
}

// true = continue
// false = check state
//      Phase::End = done scanning
//      Phase::PendBegin = did not receive start of preset in timeout
//      Phase::PendReceive = did not receive preset change in timeout
//      other = N/A
bool PresetListBuildCoordinator::process(HakenMidi* haken, const rack::Module::ProcessArgs& args)
{
    switch (phase) {
        case Phase::Init:
            total = 0.0;
            return true;

        case Phase::Start:
            total += args.sampleTime;
            if (iter->next(haken)) {
                begin = 0.0;
                phase = Phase::PendBegin;
                return true;
            } else {
                phase = Phase::End;
                return false;
            }
            break;

        case Phase::PendBegin:
            total += args.sampleTime;
            begin += args.sampleTime;
            if (begin > begin_timeout) {
                return false;
            }
            return true;

        case Phase::Begin:
            total += args.sampleTime;
            pend = 0.0;
            phase = Phase::PendReceive;
            return true;

        case Phase::PendReceive:
            total += args.sampleTime;
            pend += args.sampleTime;
            if (pend > receive_timeout) {
                return false;
            }
            return true;

        case Phase::Receive:
            total += args.sampleTime;
            pend += args.sampleTime;
            settle = 0.0;
            phase = Phase::Settle;
            return true;

        case Phase::Settle:
            total += args.sampleTime;
            settle += args.sampleTime;
            if (settle > settle_timeout) {
                phase = Phase::Start;
            }
            return true;

        case Phase::End:
            return false;

        default:
            assert(false);
            return false;
    }
}

}