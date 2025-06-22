#include "Core.hpp"
#include "../../services/ModuleBroker.hpp"

const char * ChemTask::TaskStateName(ChemTask::State state)
{
    switch (state) {
    case State::Untried:  return "uninitialized";
    case State::Pending:  return "pending";
    case State::Complete: return "complete";
    case State::Broken:   return "broken";
    default: return "--";
    }
}

bool ChemTask::ready(float increment)
{
    bool result = false;
    time += increment;
    switch (state) {
    case Untried:
        result = true;
        break;

    case Pending:
        if (time > budget) {
            state = Broken;
        }
        break;

    case Complete:
        result = (time >= period);
        break;

    case Broken:
        break;
    }
    return result;
}

void ChemTask::reset() {
    state = Untried;
    time = 0.0;
}

void ChemTask::start() {
    state = Pending;
    time = 0.0;
}

void ChemTask::complete() { state = Complete; }
void ChemTask::fail() { state = Broken; }

// ----  ChemStartupTasks  --------------------------------

void ChemStartupTasks::reset()
{
    if (core) {
        core->log_message("CoreStart", "reset");
        for (size_t i = 0; i < sizeof(core->start_states)/sizeof(core->start_states[0]); ++i) {
            core->start_states[i] = ChemTask::State::Untried;
        }
    }
    queue.clear();
    queue.push_back(ChemTask(ChemTaskId::HakenDevice, 0.0, .5));
    queue.push_back(ChemTask(ChemTaskId::Heartbeat, 0.5, 1.0));
    queue.push_back(ChemTask(ChemTaskId::EmInit, 0.0, 1.0));
    queue.push_back(ChemTask(ChemTaskId::PresetInfo, 2.0, 3.0));
}

void ChemStartupTasks::process(const rack::Module::ProcessArgs& args)
{
    assert(core); // init not called
    if (queue.empty()) return;
    ChemTask& task = queue.front();
    switch (task.id) {
    case ChemTaskId::SyncDevices:
        assert(false);
        queue.pop_front();
        break;

    case ChemTaskId::HakenDevice: {
        if (task.ready(args.sampleTime)) {
            if (core->is_haken_connected()) {
                queue.pop_front();
                core->start_states[ChemTaskId::HakenDevice] = ChemTask::State::Complete;
            } else {
                core->log_message("CoreStart", "HakenDevice");
                auto broker = MidiDeviceBroker::get();
                broker->sync();
                if (!core->is_haken_connected()) {
                    broker->bindAvailableEm(&core->haken_device);
                }
                if (core->is_haken_connected()) {
                    queue.pop_front();
                    core->start_states[ChemTaskId::HakenDevice] = ChemTask::State::Complete;
                } else {
                    task.complete(); // wait for timeout to try again
                    core->start_states[ChemTaskId::HakenDevice] = ChemTask::State::Untried;
                }
            }
        }
    } break;

    case ChemTaskId::Heartbeat: {
        if (task.ready(args.sampleTime)) {
            switch (task.state) {
            case ChemTask::State::Untried: 
                if (core->is_haken_connected()) {
                    core->log_message("CoreStart", "Heartbeat");
                    core->haken_midi.editor_present(ChemId::Core);
                    core->haken_midi_out.dispatch(DISPATCH_NOW);
                    task.start();
                }
                core->start_states[ChemTaskId::Heartbeat] = task.state;
                break;
                
            case ChemTask::Pending:
                core->start_states[task.id] = task.state;
                break;
                
            case ChemTask::Complete:
                core->start_states[ChemTaskId::Heartbeat] = ChemTask::State::Complete;
                queue.pop_front();
                break;
                
            case ChemTask::State::Broken:
                core->log_message("CoreStart", "heartbeat broken");
                reset();
                break;
            }
            break;
        }
    } break;

    case ChemTaskId::EmInit: {
        if (task.ready(args.sampleTime)) {
            if (core->is_haken_connected()) {
                core->log_message("CoreStart", "Request Updates");
                core->haken_midi.request_updates(ChemId::Core);
                core->haken_midi_out.dispatch(DISPATCH_NOW);
                queue.pop_front();
                core->start_states[ChemTaskId::EmInit] = ChemTask::State::Complete;
            }
        } else {
            core->start_states[task.id] = task.state;
        }
    } break;

    case ChemTaskId::PresetInfo: {
        if (task.ready(args.sampleTime)) {
            if (task.completed()) {
                queue.pop_front();
                core->start_states[ChemTaskId::PresetInfo] = ChemTask::State::Complete;
            } else if (core->is_haken_connected()) {
                core->log_message("CoreStart", "Request PresetInfo");
                core->haken_midi.request_configuration(ChemId::Core);
                core->haken_midi_out.dispatch(DISPATCH_NOW);
                task.start();
                core->start_states[ChemTaskId::PresetInfo] = task.state;
            }
        } else if (task.broken()) {
            core->log_message("CoreStart", "PresetInfo broken");
            reset();
        }
    } break;
    }
}

void ChemStartupTasks::heartbeat_received()
{
    ChemTask& task = queue.front();
    if (ChemTaskId::Heartbeat == task.id) {
        core->log_message("CoreStart", format_string("heartbeat in %.4f", task.time));
        core->start_states[ChemTaskId::Heartbeat] = ChemTask::State::Complete;
        queue.pop_front();
    }
}
void ChemStartupTasks::configuration_received()
{
    ChemTask& task = queue.front();
    if (ChemTaskId::PresetInfo == task.id) {
        core->log_message("CoreStart", format_string("PresetInfo in %.4f", task.time));
        core->start_states[ChemTaskId::PresetInfo] = ChemTask::State::Complete;
        task.complete();
    }
}

// ----  RecurringChemTasks  --------------------------------

void RecurringChemTasks::start()
{
    started = true;
}

void RecurringChemTasks::reset()
{
    started = false;
}

void RecurringChemTasks::process(const rack::Module::ProcessArgs& args)
{
    if (!started || core->disconnected) return;
    auto conn = core->host_connection(ChemDevice::Haken);
    if (!conn || !conn->identified()) return;
    if (core->host_busy()) return;

    bool heart_ready = heart.ready(args.sampleTime);
    bool sync_ready = sync.ready(args.sampleTime);
    if (!(heart_ready || sync_ready)) return;

    if (heart_ready) {
        core->log_message("Core", "Task Heartbeat");
        core->haken_midi.editor_present(ChemId::Core);
        heart.start();
    }
    if (sync_ready && !heart.pending() && !heart.broken()) {
        auto broker = ModuleBroker::get();
        sync.start();
        if (broker->is_primary(core)) {
            core->log_message("Core", "Task SyncDevices");
            MidiDeviceBroker::get()->sync();
        }
        sync.complete();
    }
}

void RecurringChemTasks::heartbeat_received()
{
    heart.complete();
}
