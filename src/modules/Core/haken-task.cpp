#include "haken-task.hpp"
#include "../../services/misc.hpp"
#include "../../services/midi-io.hpp"
#include "../../services/ModuleBroker.hpp"
#include "Core.hpp"

namespace pachde {

inline CoreModule* Core(void * ptr) { return reinterpret_cast<CoreModule*>(ptr); }

const char * TaskKey(HakenTask task)
{
    switch (task) {
    case HakenTask::None:          return "task-none";
    case HakenTask::MidiDevice:    return "task-midi-device";
    case HakenTask::HeartBeat:     return "task-heart-beat";
    case HakenTask::Updates:       return "task-updates";
    case HakenTask::PresetInfo:    return "task-preset-info";
    case HakenTask::LastPreset:    return "task-last-preset";
    default: return "task-unknown";
    }

}

const char * TaskName(HakenTask task)
{
    switch (task) {
    case HakenTask::None:          return "--";
    case HakenTask::MidiDevice:    return "connect midi device";
    case HakenTask::HeartBeat:     return "heartbeat";
    case HakenTask::Updates:       return "request updates";
    case HakenTask::PresetInfo:    return "request preset info";
    case HakenTask::LastPreset:    return "select last preset";
    case HakenTask::SyncDevices:   return "synchronize midi devices";
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

void HakenTaskInfo::fromJson(json_t* root) {
    if (!root) return;

    auto j = json_object_get(root, "delay");
    if (j) { post_delay = json_number_value(j); }

    j = json_object_get(root, "budget");
    if (j) { 
        budget = json_number_value(j);
        if (budget < 0.f) budget = 0.f;
    }

    // j = json_object_get(root, "midi-rate");
    // if (j) {
    //     rate = static_cast<HakenMidiRate>(json_integer_value(j));
    //     if (!in_range(static_cast<int>(rate), 0, 2)) {
    //         rate = HakenMidiRate::Third;
    //     }
    // }
}

json_t* HakenTaskInfo::toJson() const {
    auto root = json_object();
    json_object_set_new(root, "delay", json_real(post_delay));
    json_object_set_new(root, "budget", json_integer(static_cast<int>(budget)));
    //json_object_set_new(root, "midi_rate", json_integer(static_cast<int>(rate)));
    return root;
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
        HakenTaskInfo( HakenTask::LastPreset,    TaskState::Unscheduled,    2.0f,  4.0f, false ),
        HakenTaskInfo( HakenTask::SyncDevices,   TaskState::Uninitialized, 30.0f,  0.0f, true ),
    };
}

void HakenTasks::subscribeChange(IHakenTaskEvents* client)
{
    auto cit = std::find(clients.cbegin(), clients.cend(), client);
    if (cit != clients.cend()) {
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
    case LastPreset:
    case SyncDevices:
        return &tasks[task_index(id)];
    default:
        return nullptr;
    }
}
void HakenTasks::toJson(json_t* root)
{
    for (const HakenTaskInfo& task: tasks) {
        json_object_set_new(root, TaskKey(task.id), task.toJson());
    }
}

void HakenTasks::fromJson(json_t* root)
{
    for (HakenTaskInfo& task: tasks) {
        task.fromJson(json_object_get(root, TaskKey(task.id)));
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
    auto chem = Core(core);
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
    auto chem = Core(core);

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
                chem->logMessage("CHEM", format_string("%s retries exhausted: rebooting", TaskName(task->id)));
                chem->reboot();
                notifyChange(task->id);
            } else {
                chem->logMessage("CHEM", format_string("throttling %s", TaskName(task->id)));
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
            chem->logMessage("CHEM", format_string("Task complete: %s", TaskName(task->id)));
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

    case TaskState::Done: {
        notifyChange(task->id);
        current = next_task(current);
        return true;
    } break;

    case TaskState::NotApplicable: {
        current = next_task(current);
        return true;
    } break;

    case TaskState::Broken: {
        chem->logMessage("CHEM", format_string("Task broken: %s", TaskName(task->id)));
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
    auto chem = Core(core);
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
            chem->logMessage("CHEM", "Starting task MidiDevice");
            if (chem->isHakenConnected()) {
                task->done();
                notifyChange(HakenTask::MidiDevice);
                current = next_task(current);
            } else {
                auto broker = MidiDeviceBroker::get();
                broker->sync();
                if (chem->isHakenConnected()) {
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
            if (chem->isHakenConnected()){
                chem->logMessage("CHEM", "Starting task Heartbeat");
                chem->haken_midi.editor_present();
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
            if (chem->isHakenConnected()) {
                chem->logMessage("CHEM", "Starting task Updates");
                chem->haken_midi.request_updates();
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
            if (chem->isHakenConnected()) {
                chem->logMessage("CHEM", "Starting task PresetInfo");
                task->pend();
                chem->haken_midi.request_configuration();
                //chem->haken_midi_out.dispatch(DISPATCH_NOW);
                notifyChange(HakenTask::PresetInfo);
            } else {
                task->not_applicable();
                notifyChange(current);
                current = next_task(current);
            }
            break;

        case HakenTask::LastPreset:
            if (chem->isHakenConnected()) {
                // TODO
                //chem->midilog.logMessage("CHEM", "Starting task LastPreset");
                task->done();
                notifyChange(HakenTask::LastPreset);
                current = next_task(current);
            } else {
                task->not_applicable();
                notifyChange(current);
                current = next_task(current);
            }
            break;

        case HakenTask::SyncDevices: {
            auto broker = ModuleBroker::get();
            if (broker->is_primary(chem)) {
                chem->logMessage("CHEM", "Task SyncDevices");
                MidiDeviceBroker::get()->sync();
                task->done();
                notifyChange(HakenTask::SyncDevices);
            } else {
                task->not_applicable();
                notifyChange(current);
                current = next_task(current);
            }
            current = next_task(current);
       } break;

        case HakenTask::End:
            notifyChange(HakenTask::End);
            break;
    }

}

}