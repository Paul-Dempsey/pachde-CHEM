#pragma once
#include "../../services/haken-midi.hpp"
#include "../../em/EaganMatrix.hpp"
#include "../../chem-id.hpp"
#include "../../chem-core.hpp"

namespace pachde {

enum class PresetListRequestKind { ccTaskUser, ccTaskSystem, UserIdList, SystemIdList, OsmosePage };

struct IEnumeratePresets {
    ChemId chem_id{ChemId::Unknown};
    virtual bool next(HakenMidi* haken) = 0;
};

struct HakenPresetEnumerator: IEnumeratePresets
{
    std::vector<PresetId> ids;
    size_t current{0};

    HakenPresetEnumerator(ChemId source) {
        chem_id = source;
    }
    void add(PresetId id) { ids.push_back(id); }
    bool next(HakenMidi* haken) override;
};

class PresetIdListBuilder : IHandleEmEvents
{
    HakenPresetEnumerator& target;
    IChemHost * chem{nullptr};
    
    PresetIdListBuilder(ChemId client_id, IChemHost* host, HakenPresetEnumerator& e) :
        target(e),
        chem(host)
    {
        IHandleEmEvents::em_event_mask = IHandleEmEvents::PresetChanged;
        IHandleEmEvents::module_id = client_id;
    }
    void onPresetChanged() override {
        auto preset = chem->host_preset();
        if (preset && !preset->empty()) {
            target.add(preset->id);
        }
    }
};


struct OsmosePresetEnumerator: IEnumeratePresets
{
    uint8_t page{0};
    uint8_t last_page{0};
    uint8_t index{0};
    
    OsmosePresetEnumerator(ChemId source, uint8_t page) :
        page(page), last_page(page) { chem_id = source; }

    OsmosePresetEnumerator(ChemId source, uint8_t page, uint8_t last_page) :
        page(page), last_page(last_page) { chem_id = source; }

    bool next(HakenMidi* haken) override;
};

constexpr const float OSMOSE_PRESET_START_TIME {0.75};
constexpr const float OSMOSE_PRESET_RESPONSE_TIME {2.5};
constexpr const float OSMOSE_SETTLE_TIME {2.5};

constexpr const float HAKEN_PRESET_START_TIME {0.25};
constexpr const float HAKEN_PRESET_RESPONSE_TIME {1.5};
constexpr const float HAKEN_SETTLE_TIME {2.0};

struct PresetListBuildCoordinator
{
    enum class Phase { Init, Start, PendBegin, Begin, PendReceive, Receive, Settle, End};
    Phase phase{Phase::Init};
    IEnumeratePresets * iter{nullptr};
    MidiLog * log{nullptr};

    float total;
    float begin;
    float pend;
    float settle;

    float begin_timeout;
    float receive_timeout;
    float settle_timeout;

    Phase get_phase() { return phase; }

    void configure(MidiLog * logger, bool osmose, IEnumeratePresets * iterator);
    void start_building();
    void preset_started();
    void preset_received();

    // true = continue
    // false = check state
    //      Phase::End = done scanning
    //      Phase::PendBegin = did not receive start of preset in timeout
    //      Phase::PendReceive = did not receive preset change in timeout
    //      other = N/A
    bool process(HakenMidi* haken, const rack::Module::ProcessArgs& args);
};

}