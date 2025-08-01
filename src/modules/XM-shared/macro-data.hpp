#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/colors.hpp"
#include "../../services/misc.hpp"
#include "../../services/svt_rack.hpp"

namespace pachde {

enum MacroRange{ Bipolar, Unipolar, Custom };
const uint8_t INVALID_MACRO{0};

// modulated value for knob track
// param_value = bipolar knob value, -1v to 1v
// cv          = bipolar control voltage, -5v to 5v
// amount      = bipolar amount in percent, -100% to 100%
inline float xm_modulated_value(float param_value, float cv, float amount) {
    if (0.f == amount) return param_value;
    return clamp(param_value + ((cv * (1.f/5.f)) * (amount * .01f)), -1.f, 1.f);
}

struct MacroDescription
{
    float param_value{0.f};
    float cv{0.f};
    float mod_amount{0.f};
    float mod_value{0.f};

    bool modulated{true};
    int64_t module_id{-1};
    ssize_t knob_id{-1};
    uint8_t macro_number{INVALID_MACRO};
    MacroRange range{Bipolar};
    float min{-1.f};
    float max{1.f};
    std::string name;

    MacroDescription(json_t * root);
    MacroDescription(int64_t module, ssize_t knob);
    void clear();
    void init (const MacroDescription& source);
    void set_range(MacroRange r);
    bool valid();
    void from_json(json_t * root);
    json_t * to_json();
};

struct MacroData
{
    std::vector<std::shared_ptr<MacroDescription>> data;
    MacroData() {}
    bool empty() { return data.empty(); }
    size_t size() { return data.size(); }
    std::vector<std::shared_ptr<MacroDescription>>::iterator find(int64_t module_id, ssize_t knob_id);
    std::vector<std::shared_ptr<MacroDescription>>::iterator find_macro_number(uint8_t macro_number);
    std::vector<std::shared_ptr<MacroDescription>>::iterator find_module(int64_t id);
    void add(std::shared_ptr<MacroDescription> macro);
    void add_update(std::shared_ptr<MacroDescription> macro);
    void remove(int64_t module_id, ssize_t knob_id);
    void remove(int64_t module_id);
    void prune_leaving(const std::vector<int64_t>& preserve);
    std::shared_ptr<MacroDescription> get_macro(int64_t module_id, ssize_t knob_id);
    void from_json(json_t* root);
    void to_json(json_t* root);
};

}