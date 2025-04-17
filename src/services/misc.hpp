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
size_t format_buffer(char * buffer, size_t length, const char* fmt, ...);
inline std::string u64_to_string(uint64_t value) { return format_string("%0.8llx", value); }
uint64_t parse_hex_u64(const std::string& str);
const char * printable(const std::string& s); // returns a printable string, even if str is empty
char printable(char ch);
bool alpha_order(const std::string& a, const std::string& b); 
std::size_t common_prefix_length(const std::string& alpha, const std::string& beta);
std::size_t common_prefix_length_insensitive(const std::string& alpha, const std::string& beta);
inline bool match_insensitive(const std::string& alpha, const std::string& beta) {
    return alpha.size() == beta.size()
        && alpha.size() == common_prefix_length_insensitive(alpha, beta);
}
std::string spaceless(const std::string& str);
std::string collapse_space(const std::string& str);
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

bool get_json_bool(const json_t* root, const char* key, bool default_value);
float get_json_float(const json_t* root, const char* key, float default_value);
int get_json_int(const json_t* root, const char* key, int default_value);

std::string get_json_string(const json_t* root, const char* key, const std::string& default_value);
std::string get_json_string(const json_t* root, const char* key);

inline void json_read_string(json_t* root, const char* name, std::string& var) {
    if (json_t* j = json_object_get(root, name)) {
        var = json_string_value(j);
    }
}

inline void json_read_bool(json_t* root, const char* name, bool& var) {
    if (json_t* j = json_object_get(root, name)) {
        var = json_boolean_value(j);
    }
}

inline void json_read_float(json_t* root, const char * name, float& var){
    if (json_t* j = json_object_get(root, name)) {
        var = json_real_value(j);
    }
}



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

class WallTimer
{
    double start_time;
    double interval;

public:
    WallTimer() : interval(0.0) { }
    WallTimer(double timeout) : interval(timeout) { }

    void set_interval(double interval) { 
        assert(interval > 0.0);
        this->interval = interval;
    }

    void run() {
        assert(interval > 0.0);
        start_time = rack::system::getTime();
    }

    void start(double interval) {
        this->interval = interval;
        run();
    }

    // For one-shots: return true when interval has elapsed.
    // Call start() to begin a new interval
    bool finished() { return rack::system::getTime() - start_time >= interval; }

    // For periodic intervals: returns true once when time passes the interval and resets.
    bool lap()
    {
        auto current = rack::system::getTime();
        bool lapped = current - start_time >= interval;
        if (lapped) {
            start_time = current;
        }
        return lapped;
    }
};

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
