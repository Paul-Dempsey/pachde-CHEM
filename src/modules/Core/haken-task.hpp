#pragma once
#include <rack.hpp>
#include "../../services/haken-midi.hpp"

namespace pachde {

enum HakenTask : uint8_t {
    None = 0xff,
    First = 0,
    MidiDevice = First,
    HeartBeat,
    Updates,
    PresetInfo,
    LastPreset,
    SyncDevices,
    End
};

inline size_t task_index(HakenTask id) { return static_cast<size_t>(id); }

inline HakenTask next_task(HakenTask id)
{
    switch (id) {
        case HakenTask::None: return HakenTask::First;
        case HakenTask::End: return id;
        default: return static_cast<HakenTask>(task_index(id) + 1);
    }
}

enum class TaskState : uint8_t {
    Uninitialized,
    Unscheduled,
    Pending,
    Complete,
    Done,
    Waiting,
    NotApplicable,
    Broken
};

const char * TaskKey(HakenTask task);
const char * TaskName(HakenTask task);
const char * TaskStateName(TaskState state);

struct IHakenTaskEvents
{
    virtual void onHakenTaskChange(HakenTask task) = 0;
};

struct HakenTaskInfo
{
    HakenTask id;
    bool periodic;
    float post_delay;
    float budget;
    TaskState state;
    float time;
    bool in_post;
    int attempt;

    HakenTaskInfo(HakenTask identifier, TaskState state, float post, float budget, bool periodic)
    :   id(identifier),
        periodic(periodic),
        post_delay(post),
        budget(budget),
        state(state),
        time(0.f),
        attempt(0)
    {}

    bool fresh() { return TaskState::Uninitialized == state; }
    bool scheduled() { return TaskState::Unscheduled != state; }
    bool pending() { return TaskState::Pending == state; }
    bool completed() { return TaskState::Complete == state; }
    bool isdone() { return TaskState::Done == state; }
    bool waiting() { return TaskState::Waiting == state; }
    bool na() { return TaskState::NotApplicable == state; }
    bool broken() { return TaskState::Broken == state; }

    void refresh() {
        if (state != TaskState::Unscheduled) {
            state = TaskState::Uninitialized;
        }
        time = 0.f;
        in_post = false;
        attempt = 0;
    }
    void schedule() { refresh(); }
    void unschedule() { state = TaskState::Unscheduled; }
    void pend()  { state = TaskState::Pending; }
    void complete() { state = TaskState::Complete; }
    void done() { state = TaskState::Done; }
    void wait() { state = TaskState::Waiting; }
    void not_applicable() { state = TaskState::NotApplicable;}
    void fail() { state = TaskState::Broken; }

    //void fromJson(json_t* root);
    //json_t* toJson() const;
};

struct HakenTasks
{
    HakenTask current;
    HakenMidiRate midi_rate;
    void* core;
    std::vector<HakenTaskInfo> tasks;
    std::vector<IHakenTaskEvents*> clients;

    HakenTasks();
    void subscribeChange(IHakenTaskEvents* client);
    void unsubscribeChange(IHakenTaskEvents* client);
    void notifyChange(HakenTask id);
    void setCoreModule(void* client) { core = client; }
    HakenTaskInfo * get_task(HakenTask id);
    void back_off_midi_rate();
    void refresh();
    bool process_task_timing(const rack::Module::ProcessArgs& args, HakenTaskInfo* task);

    void complete_task(HakenTask id);
    //void toJson(json_t* root);
    //void fromJson(json_t* root);
    void process(const rack::Module::ProcessArgs& args);
};

}