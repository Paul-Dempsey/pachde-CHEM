#include "preset-enum.hpp"
#include "../../em/preset-list.hpp"
#include "../../services/kv-store.hpp"

namespace pachde {

// ---- HakenPresetEnumerator  ----

std::string HakenPresetEnumerator::next_text()
{
    if (current >= ids.size()) return "(complete)";
    auto id = ids[current];
    return format_string("[%d.%d.%d]", id.bank_hi(), id.bank_lo(), id.number());
}

bool HakenPresetEnumerator::next(HakenMidi *haken, EaganMatrix *)
{
    if (current >= ids.size()) return false;
    expected = ids[current++];
    assert(expected.valid());
    if (haken->log) {
        haken->log->log_message("HPE", format_string("Requesting [%d.%d.%d]", expected.bank_hi(), expected.bank_lo(),expected.number()));
    }
    haken->select_preset(chem_id, expected);
    return true;
}

// ---- OsmosePresetEnumerator  ----

bool OsmosePresetEnumerator::next(HakenMidi* haken, EaganMatrix * em)
{
    if (index > 127) {
        index = 0;
        ++page;
        if (page > last_page) return false;
    }
    if (haken->log) {
        haken->log->log_message("OPE", format_string("Requesting [%d.%d]", page, index));
    }
    expected = PresetId(page, 0, index);
    em->set_osmose_id(expected);
    haken->send_message(Tag(MakeCC(Haken::ch1, Haken::ccBankH, page), chem_id));
    haken->send_message(Tag(MakeProgramChange(Haken::ch1, index), chem_id));
    ++index;
    return true;
}

// ---- PresetListBuildCoordinator  ----

namespace plb_constant {
    constexpr const float OSMOSE_PRESET_START_TIME {1.75f};
    constexpr const float OSMOSE_PRESET_RESPONSE_TIME {2.5f};
    constexpr const float OSMOSE_SETTLE_TIME {2.5f};

    constexpr const float HAKEN_PRESET_START_TIME {0.75f};
    constexpr const float HAKEN_PRESET_RESPONSE_TIME {1.0f};
    constexpr const float HAKEN_SETTLE_TIME {1.5f};

    const char * OSMOSE_START_KEY = "preset-start-osmose";
    const char * OSMOSE_RESPOND_KEY = "preset-respond-osmose";
    const char * OSMOSE_SETTLE_KEY = "preset-settle-osmose";
    const char * EM_START_KEY = "preset-start-em";
    const char * EM_RESPOND_KEY = "preset-respond-em";
    const char * EM_SETTLE_KEY = "preset-settle-em";
    const char * FLOAT_FMT = "%.2f";
}

PresetListBuildCoordinator::PresetListBuildCoordinator(MidiLog *logger, bool osmose, IEnumeratePresets *iterator) :
    iter(iterator),
    log(logger)
{
    using namespace plb_constant;
    auto kv = get_plugin_kv_store();
    if (kv && kv->load()) {
        if (osmose) {
            auto value = kv->lookup(OSMOSE_START_KEY);
            bool update = value.empty();
            begin_timeout = KVStore::float_value(value, OSMOSE_PRESET_START_TIME);
            receive_timeout = KVStore::float_value(kv->lookup(OSMOSE_RESPOND_KEY), OSMOSE_PRESET_RESPONSE_TIME);
            settle_timeout = KVStore::float_value(kv->lookup(OSMOSE_SETTLE_KEY), OSMOSE_SETTLE_TIME);
            if (update) {
                kv->update(OSMOSE_START_KEY, format_string(FLOAT_FMT, begin_timeout));
                kv->update(OSMOSE_RESPOND_KEY, format_string(FLOAT_FMT, receive_timeout));
                kv->update(OSMOSE_SETTLE_KEY, format_string(FLOAT_FMT, settle_timeout));
                kv->save();
            }
        } else {
            auto value = kv->lookup(EM_START_KEY);
            bool update = value.empty();
            begin_timeout = KVStore::float_value(value, HAKEN_PRESET_START_TIME);
            receive_timeout = KVStore::float_value(kv->lookup(EM_RESPOND_KEY), HAKEN_PRESET_RESPONSE_TIME);
            settle_timeout = KVStore::float_value(kv->lookup(EM_SETTLE_KEY), HAKEN_SETTLE_TIME);
            if (update) {
                kv->update(EM_START_KEY, format_string(FLOAT_FMT, begin_timeout));
                kv->update(EM_RESPOND_KEY, format_string(FLOAT_FMT, receive_timeout));
                kv->update(EM_SETTLE_KEY, format_string(FLOAT_FMT, settle_timeout));
                kv->save();
            }
        }
    } else {
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
}

void PresetListBuildCoordinator::start_building()
{
    assert(iter);
    total = 0.0;
    phase = Phase::Start;
    first = true;
}

void PresetListBuildCoordinator::preset_started()
{
    if (phase == Phase::PendBegin) {
        first  = false;
        phase = Phase::Begin;
    }
}

void PresetListBuildCoordinator::preset_received()
{
    if (phase == Phase::PendReceive) {
        phase = Phase::Receive;
    } else if (phase == Phase::Settle) {
        // sometimes osmose sends spurious copies of presets after selection
        // in this case we want to wait until it's done whatever recovery
        // this is doing (I assume it's some kind of recovery),
        // so we reset the settle timeout
        if (log) log->log_message("PLB", "extending settle time");
        settle = 0.0f;
    }
}

void PresetListBuildCoordinator::resume()
{
    if (Phase::End == phase) return;
    if (log) log->log_message("PLB", "resume()");
    phase = Phase::Start;
}

// true = continue
// false = check state
//      Phase::End = done scanning
//      Phase::PendBegin = did not receive start of preset in timeout
//      Phase::PendReceive = did not receive preset change in timeout
//      other = N/A
bool PresetListBuildCoordinator::process(HakenMidi* haken, EaganMatrix * em, const rack::Module::ProcessArgs& args)
{
    if (phase != Phase::End) total += args.sampleTime;
    switch (phase) {
        case Phase::Init:
            return true;

        case Phase::Start:
            if (iter->next(haken, em)) {
                begin = 0.0f;
                phase = Phase::PendBegin;
                return true;
            } else {
                phase = Phase::End;
                return false;
            }
            break;

        case Phase::PendBegin:
            begin += args.sampleTime;
            if ((first && (begin > 2.0)) || (!first && (begin > begin_timeout))) {
                if (log) { log->log_message("PLB", format_string("PendBegin timeout %.6f > %.6f ", begin, first ? 2.0f : begin_timeout)); }
                first  = false;
                return false;
            }
            return true;

        case Phase::Begin:
            if (log) {
                begin += args.sampleTime;
                log->log_message("PLB", format_string("preset_started() in %.6f", begin));
            }
            pend = 0.0f;
            phase = Phase::PendReceive;
            return true;

        case Phase::PendReceive:
            first  = false;
            pend += args.sampleTime;
            if (pend > receive_timeout) {
                if (log) { log->log_message("PLB", "PendReceive timeout"); }
                return false;
            }
            return true;

        case Phase::Receive:
            if (log) {
                pend += args.sampleTime;
                log->log_message("PLB", format_string("preset_received() in %.6f", pend));
            }
            settle = 0.0f;
            phase = Phase::Settle;
            return true;

        case Phase::Settle:
            settle += args.sampleTime;
            if (settle > settle_timeout) {
                if (log) log->log_message("PLB", "Settle complete");
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