#pragma once
#include "services/haken-midi.hpp"
#include "em/EaganMatrix.hpp"
#include "chem-id.hpp"
#include "chem-core.hpp"

namespace pachde {

struct IEnumeratePresets
{
    ChemId chem_id{ChemId::Unknown};
    virtual ~IEnumeratePresets() {}
    virtual std::string next_text() = 0;
    virtual PresetId expected_id() = 0;
    virtual bool next(HakenMidi* haken, EaganMatrix * em) = 0;
};

struct HakenPresetEnumerator: IEnumeratePresets
{
    HakenPresetEnumerator(const HakenPresetEnumerator&) = delete;

    std::vector<PresetId> ids;
    PresetId expected;
    size_t current{0};
    virtual ~HakenPresetEnumerator() {}
    HakenPresetEnumerator(ChemId source) {
        chem_id = source;
    }
    void add(PresetId id) { ids.push_back(id); }
    std::string next_text() override;
    PresetId expected_id() override { return expected; }
    bool next(HakenMidi* haken, EaganMatrix * em) override;
};

struct PresetIdListBuilder : IHandleEmEvents
{
    PresetIdListBuilder(const PresetIdListBuilder &) = delete;

    HakenPresetEnumerator * target{nullptr};
    IChemHost * chem{nullptr};
    bool end_received{false};
    bool complete{false};

    virtual ~PresetIdListBuilder() {}
    PresetIdListBuilder(ChemId client_id, IChemHost* host, HakenPresetEnumerator* e) :
        target(e),
        chem(host)
    {
        IHandleEmEvents::em_event_mask =
            IHandleEmEvents::PresetChanged
            + IHandleEmEvents::UserComplete
            + IHandleEmEvents::SystemComplete
            + IHandleEmEvents::AllowNested
            ;
        IHandleEmEvents::module_id = client_id;
    }
    bool finished() { return complete; }
    void onUserComplete() override { end_received = true; }
    void onSystemComplete() override { end_received = true; }
    void onPresetChanged() override {
        auto preset = chem->host_preset();
        if (preset && !preset->empty()) {
            if (end_received) {
                complete = true;
            } else {
                target->add(preset->id);
            }
        }
    }
};


struct OsmosePresetEnumerator: IEnumeratePresets
{
    OsmosePresetEnumerator(const OsmosePresetEnumerator&) = delete;

    uint8_t page{0};
    uint8_t last_page{0};
    uint8_t index{0};
    PresetId expected;
    virtual ~OsmosePresetEnumerator() {}
    OsmosePresetEnumerator(ChemId source, uint8_t page) :
    page(page), last_page(page) { chem_id = source; }

    OsmosePresetEnumerator(ChemId source, uint8_t page, uint8_t last_page) :
    page(page), last_page(last_page) { chem_id = source; }

    std::string next_text() override { return format_string("[%d.%d]", page, index); }
    PresetId expected_id() override { return expected; }
    bool next(HakenMidi* haken, EaganMatrix * em) override;
};

struct PresetListBuildCoordinator
{
    PresetListBuildCoordinator(const PresetListBuildCoordinator&) = delete;

    enum class Phase { Init, Start, PendBegin, Begin, PendReceive, Receive, Settle, End};
    Phase phase{Phase::Init};
    IEnumeratePresets * iter{nullptr};
    MidiLog * log{nullptr};
    bool first;

    double total{0.0};
    float begin{0.0};
    float pend{0.0};
    float settle{0.0};

    float begin_timeout{INFINITY};
    float receive_timeout{INFINITY};
    float settle_timeout{INFINITY};

    Phase get_phase() { return phase; }
    ~PresetListBuildCoordinator() {
        if (iter) delete iter;
    }
    // takes ownership of iterator
    PresetListBuildCoordinator(MidiLog * logger, bool osmose, IEnumeratePresets * iterator);
    void start_building();
    void preset_started();
    void preset_received();
    void resume();

    // true = continue
    // false = check state
    //      Phase::End = done scanning
    //      Phase::PendBegin = did not receive start of preset in timeout
    //      Phase::PendReceive = did not receive preset change in timeout
    //      other = N/A
    bool process(HakenMidi* haken, EaganMatrix * em, const rack::Module::ProcessArgs& args);
};

}