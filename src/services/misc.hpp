// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
#include <string>

using namespace ::rack;
namespace pachde {

#define U8(arg) static_cast<uint8_t>(arg)

std::string format_string(const char *fmt, ...);
size_t format_buffer(char * buffer, size_t length, const char* fmt, ...);
std::string hsl_string(float hue, float saturation, float lightness, float alpha);
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
std::string ellipse_string(const std::string& source, size_t trim_point);
int randomZeroTo(int size);
//std::string AbbreviatedName(std::string name);
std::string TempName(const std::string& suffix);

template <typename T>
inline bool in_range(T value, T minimum, T maximum) { return minimum <= value && value <= maximum; }
inline bool in_rangef(float value, float minimum, float maximum){ return (minimum <= value) && (value <= maximum); }

template <typename T>
inline bool in_range_limit(T value, T minimum, T limit) { return minimum <= value && value < limit; }

class WallTimer
{
    double start_time{0.0};
    double interval{0.0};

public:
    WallTimer() {}
    WallTimer(double timeout) : interval(timeout) { }

    void stop() { start_time = 0.0; }
    bool stopped() { return start_time <= 0.0; }
    bool running() { return !stopped(); }

    void set_interval(double interval) {
        assert(interval > 0.0);
        this->interval = interval;
    }

    // 0..1
    float progress() {
        if (interval <= 0.0) return 0.0f;
        return static_cast<float>((rack::system::getTime() - start_time) / interval);
    }

    void run() {
        assert(interval > 0.0);
        start_time = rack::system::getTime();
    }

    void start(double interval) {
        assert(interval > 0.0);
        this->interval = interval;
        run();
    }

    // For one-shots: return true when interval has elapsed.
    // Call run() or start() to begin a new interval
    bool finished() { return (rack::system::getTime() - start_time) >= interval; }

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
