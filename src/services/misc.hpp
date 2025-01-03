// Copyright (C) Paul Chase Dempsey
#pragma once

#include <rack.hpp>
#include <string>

using namespace ::rack;
namespace pachde {

#if !defined U8
#define U8(arg) static_cast<uint8_t>(arg)
#endif

std::string format_string(const char *fmt, ...);
const char * printable(const std::string& s);
bool alpha_order(const std::string& a, const std::string& b); 
std::size_t common_prefix_length(const std::string& alpha, const std::string& beta);
std::size_t common_prefix_length_insensitive(const std::string& alpha, const std::string& beta);
inline bool match_insensitive(const std::string& alpha, const std::string& beta) {
    return alpha.size() == beta.size()
        && alpha.size() == common_prefix_length_insensitive(alpha, beta);
}

std::string to_lower_case(const std::string& name);
bool is_safe_file_char(char ch, bool allow_space = true);
std::string to_file_safe(const std::string& str, bool allow_space = true);

int randomZeroTo(int size);
//std::string AbbreviatedName(std::string name);
std::string TempName(const std::string& suffix);

template <typename T>
inline bool in_range(T value, T minimum, T maximum) { return minimum <= value && value <= maximum; }

template <typename T>
inline bool in_range_limit(T value, T minimum, T limit) { return minimum <= value && value < limit; }

bool GetBool(const json_t* root, const char* key, bool default_value);
float GetFloat(const json_t* root, const char* key, float default_value);

enum class InitState : uint8_t {
    Uninitialized,
    Pending,
    Complete,
    Broken
};

const char * InitStateName(InitState state);

// enum Expansion {
//     None  = 0x00,
//     Left  = 0x01,
//     Right = 0x10,
//     Both  = 0x11
// };
// struct ExpanderPresence {
//     Expansion exp;
//     //Expansion operator() () const { return exp; }
//     ExpanderPresence() : exp(Expansion::None) {}
//     ExpanderPresence(Expansion e) : exp(e) {}
//     bool operator == (const ExpanderPresence& rhs) { return exp == rhs.exp; }
//     bool operator == (const Expansion& rhs) { return exp == rhs; }
//     static ExpanderPresence fromRackSide(int rackSide) {
//         return ExpanderPresence(rackSide == 0 ? Expansion::Left : rackSide == 1 ? Expansion:: Right : Expansion::None);
//     }
//     void add(Expansion expansion) { exp = static_cast<Expansion>(exp | expansion); }
//     void remove(Expansion expansion) { exp = static_cast<Expansion>(exp & ~expansion); }
//     void addRight() { add(Expansion::Right); }
//     void addLeft() { add(Expansion::Left); }
//     void removeRight() { remove(Expansion::Right); }
//     void removeLeft() { remove(Expansion::Left); }
//     void clear() { exp = Expansion::None; }
//     bool right() const { return exp & Expansion::Right; }
//     bool left() const { return exp & Expansion::Left; }
//     bool both() const { return exp == Expansion::Both; }
//     bool empty() const { return exp == Expansion::None; }
//     bool any() { return !empty(); }
// };

struct RateTrigger
{
    float rate_ms;
    int steps;
    int step_trigger;

    explicit RateTrigger(float rate = 2.5f)
    {
        configure(rate);
        steps = 0;
    }

    void configure(float rate) {
        rate_ms = rate;
        onSampleRateChanged(Module::SampleRateChangeEvent{
            APP->engine->getSampleRate(),
            APP->engine->getSampleTime()
        });
    }

    // after reset, fires on next step
    void reset() { steps = step_trigger; }
    void start() { steps = 0; }

    void onSampleRateChanged(const Module::SampleRateChangeEvent& e)
    {
        step_trigger = e.sampleRate * (rate_ms / 1000.0f);
    }

    bool process()
    {
        ++steps;
        if (steps >= step_trigger)
        {
            steps = 0;
            return true;
        }
        return false;
    }
};

}
