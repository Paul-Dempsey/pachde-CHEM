#pragma once
#include <rack.hpp>
#include "../chem.hpp"

using namespace ::rack;

namespace pachde {

constexpr const uint16_t UNSET_16 = 0xffff;
constexpr const uint8_t UNSET_8 = 0xff;
constexpr const uint8_t INVALID_STREAM = UNSET_8;
enum PortKind : uint8_t { NoSend, CC, Stream, StreamIndex };

struct EmccPortConfig
{
    PortKind kind;
    uint8_t channel_stream;
    uint8_t cc_id;
    bool low_resolution;
    int param_id;
    int input_id;
    int light_id;

    EmccPortConfig(PortKind kind, uint8_t channel_stream, uint8_t cc_id, bool low_res = false, int param = -1, int input = -1, int light = -1) :
        kind(kind),
        channel_stream(channel_stream),
        cc_id(cc_id),
        low_resolution(low_res),
        param_id(param),
        input_id(input),
        light_id(light)
    {
        assert(in_range((int)kind, (int)PortKind::NoSend, (int) PortKind::StreamIndex));
    }

    static EmccPortConfig no_send(int param_id, int input_id, int light_id, uint8_t d1, uint8_t d2, bool low_res = false)
    {
        return EmccPortConfig{PortKind::NoSend, d1, d2, low_res, param_id, input_id, light_id};
    }

    static EmccPortConfig cc(int param_id, int input_id, int light_id, uint8_t chan, uint8_t cc, bool low_res = false)
    {
        assert(in_range((int)chan, (int)Haken::ch1, (int)Haken::ch16));
        return EmccPortConfig{PortKind::CC, chan, cc, low_res, param_id, input_id, light_id};
    }

    static EmccPortConfig stream_poke(int param_id, int input_id, int light_id, uint8_t stream_id, uint8_t poke_id)
    {
        return EmccPortConfig{PortKind::Stream, stream_id, poke_id, true, param_id, input_id, light_id};
    }

    static EmccPortConfig stream_index_poke(int param_id, int input_id, int light_id, uint8_t stream_id, uint8_t poke_id)
    {
        return EmccPortConfig{PortKind::StreamIndex, stream_id, poke_id, true, param_id, input_id, light_id};
    }
};

struct EmControlPort
{
    float param_value{0.f};
    float cv{0.f};
    float mod_amount{0.f};
    float mod_value{0.f};

    PortKind kind;
    uint16_t em_value{UNSET_16};
    uint16_t last_em_value{UNSET_16};
    uint8_t channel_stream{UNSET_8};
    uint8_t cc_id{UNSET_8};
    bool low_resolution{false};
    int param_id{-1};
    int input_id{-1};
    int light_id{-1};
    int index{-1};

    EmControlPort() {}

    EmControlPort(int index, const EmccPortConfig* conf) :
        kind(conf->kind),
        channel_stream(conf->channel_stream),
        cc_id(conf->cc_id),
        low_resolution(conf->low_resolution),
        param_id(conf->param_id),
        input_id(conf->input_id),
        light_id(conf->light_id),
        index(index)
    {}

    void init(int index, const EmccPortConfig& conf) {
        this->index = index;
        kind = conf.kind;
        channel_stream = conf.channel_stream;
        cc_id = conf.cc_id;
        low_resolution = conf.low_resolution;
        param_id = conf.param_id;
        input_id = conf.input_id;
        light_id = conf.light_id;
    }
    bool has_input() { return input_id >= 0; }
    bool has_light() { return light_id >= 0; }
    bool has_param() { return param_id >= 0; }

    uint8_t data_a() { assert(no_send()); return channel_stream; }
    uint8_t data_b() { assert(no_send()); return cc_id; }

    bool no_send() { return PortKind::NoSend == kind; }
    bool is_stream_poke() { return PortKind::Stream == kind || PortKind::StreamIndex == kind; }
    bool is_cc() { return PortKind::CC == kind; }

    bool pending() { return (em_value != last_em_value) && (UNSET_16 != last_em_value); }
    void un_pend() { last_em_value = em_value; }

    uint16_t em() { return em_value; }
    uint8_t em_low() { return em_value >> 7; }
    float parameter() { return param_value; }
    float modulated() { return mod_value; }
    float modulation_amount() { return mod_amount; }

    void pull_param_cv(Module* module);
    void set_mod_amount(float amount);
    void set_param_and_em(float value);
    void set_em_and_param(uint16_t u14);
    void set_em(uint16_t u14) { em_value = u14; }
    void set_em_low(uint8_t u7) { set_em(u7 << 7); }
    void set_em_and_param_low(uint8_t u7) { set_em_and_param(u7 << 7); }
    float modulate();
    float modulated_em_value(uint16_t u14) const {
        return modulated_value(unipolar_14_to_rack(u14), cv, mod_amount);
    }

    void force_send_at_next_opportunity() { last_em_value = UNSET_16; }
    void send(IChemHost* chem, ChemId tag, bool force = false);
};

const float MOD_MIDI_RATE = 0.05f;

struct Modulation
{
    ChemModule* module;

    int mod_target;
    int last_mod_target;
    int mod_param;
    int count;
    bool have_stream;
    ChemId client_tag;

    rack::dsp::Timer midi_timer;

    bool sync_params_ready(const rack::engine::Module::ProcessArgs& args, float rate = MOD_MIDI_RATE);

    std::vector<EmControlPort> ports;
    EmControlPort& get_port(int index) {
        return ports[index];
    }
    EmControlPort* get_param_port(int param_id) {
        auto it = std::find_if(ports.begin(), ports.end(), [=](EmControlPort& port){ return port.param_id == param_id; });
        return (it == ports.end()) ? nullptr : &*it;
    }
    EmControlPort* get_input_port(int input_id) {
        auto it = std::find_if(ports.begin(), ports.end(), [=](EmControlPort& port){ return port.input_id == input_id; });
        return (it == ports.end()) ? nullptr : &*it;
    }
    EmControlPort* get_light_port(int light_id) {
        auto it = std::find_if(ports.begin(), ports.end(), [=](EmControlPort& port){ return port.light_id == light_id; });
        return (it == ports.end()) ? nullptr : &*it;
    }

    Modulation(ChemModule* module, ChemId client_tag);

    void configure(int mod_param_id, int data_length, const EmccPortConfig* data);
    bool has_target() { return mod_target >= 0; }
    void mod_to_json(json_t* root);
    void mod_from_json(json_t* root);

    void set_em_and_param(int index, uint16_t em_value, bool with_module);
    void set_em_and_param_low(int index, uint8_t em_value, bool with_module);

    // auto-track active modulation target
    void onPortChange(const ::rack::engine::Module::PortChangeEvent &e);
    // manually set modulation target
    void set_modulation_target(int target);
    void sync_send();
    void pull_mod_amount();
    // on/off lights for active modulation target
    void update_mod_lights();

    void zero_modulation();
};

}