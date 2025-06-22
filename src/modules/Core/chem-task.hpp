#pragma once
#include <rack.hpp>

struct CoreModule;

enum ChemTaskId { Heartbeat, HakenDevice, EmInit, PresetInfo, SyncDevices };

struct ChemTask
{
    enum State { Untried, Pending, Complete, Broken };
    static const char *TaskStateName(State state);

    ChemTaskId id;
    State state{Untried};
    float time{0.0};
    float budget;
    float period;

    ChemTask (ChemTaskId id, float budget, float period) :
        id(id), budget(budget), period(period) {}

    bool untried() { return state == Untried; }
    bool pending() { return state == Pending; }
    bool completed() { return state == Complete; }
    bool broken() { return state == Broken; }

    bool ready(float increment);
    void reset();
    void start();
    void complete();
    void fail();
};

struct ChemStartupTasks
{
    std::deque<ChemTask> queue;
    CoreModule* core{nullptr};
    ChemStartupTasks() { reset(); };
    void init(CoreModule* the_core) { core = the_core; }

    void reset();
    bool completed() { return queue.empty(); }
    void process(const rack::Module::ProcessArgs& args);
    void heartbeat_received();
    void configuration_received();
};

struct RecurringChemTasks
{
    bool started{false};
    CoreModule* core{nullptr};
    ChemTask heart{ChemTaskId::Heartbeat, 2.0, 12.0};
    ChemTask sync{ChemTaskId::SyncDevices, 0.0, 30.f};
    
    RecurringChemTasks()
    {
        sync.start();
        sync.complete(); //pretend we've completed to wait for next recurrence
    }
    void init(CoreModule* the_core) { core = the_core; }
    void start();
    void reset();
    void process(const rack::Module::ProcessArgs& args);
    void heartbeat_received();
};
