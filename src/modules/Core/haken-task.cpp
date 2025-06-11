#include "haken-task.hpp"
#include "../../services/misc.hpp"
#include "../../services/midi-io.hpp"
#include "../../services/ModuleBroker.hpp"
#include "Core.hpp"

namespace pachde {

inline CoreModule* CoreMod(void * ptr) { return reinterpret_cast<CoreModule*>(ptr); }

const char * TaskKey(HakenTask task)
{
    switch (task) {
    case HakenTask::None:          return "task-none";
    case HakenTask::MidiDevice:    return "task-midi-device";
    case HakenTask::HeartBeat:     return "task-heart-beat";
    case HakenTask::Updates:       return "task-updates";
    case HakenTask::PresetInfo:    return "task-preset-info";
    default: return "task-unknown";
    }

}

const char * TaskName(HakenTask task)
{
    switch (task) {
    case HakenTask::None:          return "--";
    case HakenTask::MidiDevice:    return "Connect midi device";
    case HakenTask::HeartBeat:     return "Heartbeat";
    case HakenTask::Updates:       return "Request updates";
    case HakenTask::PresetInfo:    return "Request preset info";
    case HakenTask::SyncDevices:   return "Synchronize midi devices";
    default: return "unknown task";
    }
}

const char * TaskStateName(TaskState state)
{
    switch (state) {
    case TaskState::Uninitialized:  return "uninitialized";
    case TaskState::Pending:        return "pending";
    case TaskState::Complete:       return "complete";
    case TaskState::Done:           return "done";
    case TaskState::Waiting:        return "waiting";
    case TaskState::NotApplicable:  return "n/a";
    case TaskState::Broken:         return "broken";
    default: return "--";
    }
}

HakenTasks::HakenTasks()
:   current(HakenTask::None)
{
    tasks = {
        //             id,                       state,                     post  budget periodic
        HakenTaskInfo( HakenTask::MidiDevice,    TaskState::Uninitialized,  2.0f,  0.0f, false ),
        HakenTaskInfo( HakenTask::HeartBeat,     TaskState::Uninitialized, 15.0f,  2.0f, true ),
        HakenTaskInfo( HakenTask::Updates,       TaskState::Uninitialized,  1.0f,  0.0f, false ),
        HakenTaskInfo( HakenTask::PresetInfo,    TaskState::Uninitialized,  2.0f,  4.0f, false ),
        HakenTaskInfo( HakenTask::SyncDevices,   TaskState::Uninitialized, 30.0f,  0.0f, true ),
    };
}

void HakenTasks::subscribeChange(IHakenTaskEvents* client)
{
    if (clients.cend() == std::find(clients.cbegin(), clients.cend(), client)) {
        clients.push_back(client);
    }
}
void HakenTasks::unsubscribeChange(IHakenTaskEvents* client)
{
    auto cit = std::find(clients.cbegin(), clients.cend(), client);
    if (cit != clients.cend()) {
        clients.erase(cit);
    }
}
void HakenTasks::notifyChange(HakenTask task)
{
    for(auto client : clients) {
        client->onHakenTaskChange(task);
    }
}

HakenTaskInfo * HakenTasks::get_task(HakenTask id)
{
    switch (id) {
    case MidiDevice:
    case HeartBeat:
    case Updates:
    case PresetInfo:
    case SyncDevices:
        return &tasks[task_index(id)];
    default:
        return nullptr;
    }
}

void HakenTasks::refresh()
{
    for (HakenTaskInfo& task: tasks) {
        if (task.scheduled()) {
            task.refresh();
        }
    }
    current = HakenTask::MidiDevice;
}

void HakenTasks::back_off_midi_rate()
{
    auto chem = CoreMod(core);
    switch (midi_rate) {
    case HakenMidiRate::Full:
        chem->send_midi_rate(HakenMidiRate::Third);
        chem->haken_midi_out.dispatch(DISPATCH_NOW);
        midi_rate = HakenMidiRate::Third;
        break;
    case HakenMidiRate::Third:
        chem->send_midi_rate(HakenMidiRate::Twentieth);
        chem->haken_midi_out.dispatch(DISPATCH_NOW);
        midi_rate = HakenMidiRate::Twentieth;
        break;
    case HakenMidiRate::Twentieth: break;
    }
}

// phase timing, retry, midi rate
bool HakenTasks::process_task_timing(const rack::Module::ProcessArgs& args, HakenTaskInfo* task)
{
    auto chem = CoreMod(core);

    switch (task->state) {
    case TaskState::Uninitialized: {
        task->time = 0.f;
    } break;

    case TaskState::Unscheduled:
        current = next_task(current);
        return true;

    case TaskState::Pending: {
        task->time += args.sampleTime;
        if ((task->budget > 0.f) && (task->time > task->budget)) {
            ++task->attempt;
            if (task->attempt > 3) {
                chem->log_message("CHEM", format_string("%s retries exhausted: rebooting", TaskName(task->id)));
                chem->reboot();
                notifyChange(task->id);
            } else {
                chem->log_message("CHEM", format_string("throttling %s", TaskName(task->id)));
                back_off_midi_rate();
                task->state = TaskState::Uninitialized;
                return false;
            }
        }
        return true;
    } break;

    case TaskState::Complete: {
        chem->restore_midi_rate();
        if (task->in_post) {
            task->time += args.sampleTime;
            if (task->time > task->post_delay) {
                task->in_post = false;
                task->done();
                notifyChange(task->id);
                current = next_task(current);
            }
        } else {
            chem->log_message("CHEM", format_string("Task complete: %s", TaskName(task->id)));
            if (task->post_delay > 0.f) {
                if (task->periodic) {
                    task->time = 0.f;
                    task->wait();
                    notifyChange(task->id);
                    current = next_task(current);
                } else {
                    task->in_post = true;
                    task->time = 0.f;
                }
            } else {
                task->done();
                notifyChange(task->id);
                current = next_task(current);
            }
        }
        return true;
    } break;

    case TaskState::Waiting: {
        assert(task->periodic);
        task->time += args.sampleTime;
        if (task->time > task->post_delay) {
            task->refresh();
            notifyChange(task->id);
        }
        current = next_task(current);
        return true;
    } break;

    case TaskState::Done:
    case TaskState::NotApplicable: {
        current = next_task(current);
        return true;
    } break;

    case TaskState::Broken: {
        chem->log_message("CHEM", format_string("Task broken: %s", TaskName(task->id)));
        notifyChange(task->id);
        task->refresh();
        return true;
    } break;
    }

    return false;
}

void HakenTasks::complete_task(HakenTask id)
{
    auto task = get_task(id);
    if (task) {
        task->complete();
    }
}

void HakenTasks::process(const rack::Module::ProcessArgs& args)
{
    auto chem = CoreMod(core);
    HakenTaskInfo* task = nullptr;
    if (HakenTask::End == current) {
        for (HakenTaskInfo& a_task: tasks) {
            if (a_task.scheduled() && !a_task.isdone()) {
                current = a_task.id;
                task = &a_task;
                task->attempt = 0;
                break;
            }
        }
        if (!task) return;
    }
    if (!task) {
        task = get_task(current);
    }
    if (!task) {
        current = HakenTask::MidiDevice;
        task = get_task(current);
    }

    if (process_task_timing(args, task)) return;

    switch (current) {
        default:
        case HakenTask::None:
            notifyChange(current);
            assert(false);
            break;

        case HakenTask::MidiDevice:
            chem->log_message("CHEM", "Starting task MidiDevice");
            if (chem->is_haken_connected()) {
                task->done();
                notifyChange(HakenTask::MidiDevice);
                current = next_task(current);
            } else {
                auto broker = MidiDeviceBroker::get();
                broker->sync();
                if (chem->is_haken_connected()) {
                    task->done();
                } else if (broker->bindAvailableEm(&chem->haken_device)) {
                    task->done();
                } else {
                    task->not_applicable();
                }
                notifyChange(HakenTask::MidiDevice);
                current = next_task(current);
            }
            break;

        case HakenTask::HeartBeat:
            if (chem->is_haken_connected()){
                chem->log_message("CHEM", "Starting task Heartbeat");
                chem->haken_midi.editor_present(ChemId::Core);
                chem->haken_midi_out.dispatch(DISPATCH_NOW);
                task->pend();
                notifyChange(HakenTask::HeartBeat);
            } else {
                task->not_applicable();
                notifyChange(current);
                current = next_task(current);
            }
            break;

        case HakenTask::Updates:
            if (chem->is_haken_connected()) {
                chem->log_message("CHEM", "Starting task Updates");
                chem->haken_midi.request_updates(ChemId::Core);
                // chem->haken_midi_out.dispatch(DISPATCH_NOW);
                task->done();
                notifyChange(HakenTask::Updates);
                current = next_task(current);
            } else {
                task->not_applicable();
                notifyChange(current);
                current = next_task(current);
            }
            break;

        case HakenTask::PresetInfo:
            if (chem->is_haken_connected()) {
                chem->log_message("CHEM", "Starting task PresetInfo");
                task->pend();
                chem->haken_midi.request_configuration(ChemId::Core);
                //chem->haken_midi_out.dispatch(DISPATCH_NOW);
                notifyChange(HakenTask::PresetInfo);
            } else {
                task->not_applicable();
                notifyChange(current);
                current = next_task(current);
            }
            break;

        case HakenTask::SyncDevices: {
            auto broker = ModuleBroker::get();
            if (broker->is_primary(chem)) {
                chem->log_message("CHEM", "Task SyncDevices");
                MidiDeviceBroker::get()->sync();
                task->done();
                notifyChange(HakenTask::SyncDevices);
            } else {
                task->not_applicable();
                notifyChange(current);
            }
            current = next_task(current);
       } break;

        case HakenTask::End:
            notifyChange(HakenTask::End);
            break;
    }

}

}