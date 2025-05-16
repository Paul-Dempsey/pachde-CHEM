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

struct MacroDescription
{
    int64_t module_id{-1};
    ssize_t knob_id{-1};
    uint8_t macro_number{INVALID_MACRO};
    std::string name;
    MacroRange range{Bipolar};
    uint16_t min{0};
    uint16_t max{Haken::max14};
    bool modulated{true};

    MacroDescription(json_t* root) {
        from_json(root);
    }

    MacroDescription(int64_t module, ssize_t knob) : module_id(module), knob_id(knob) {
        assert(-1 != module);
        assert(-1 != knob);
    }

    void clear() {
        macro_number = INVALID_MACRO;
        name.clear();
        range = MacroRange::Bipolar;
        min = 0;
        max = Haken::max14;
        modulated = true;
    }

    void init (const MacroDescription& source)
    {
        module_id = source.module_id;
        knob_id = source.knob_id;
        macro_number = source.macro_number;
        name = source.name;
        range = source.range;
        min = source.min;
        max = source.max;
        modulated = source.modulated;
    }

    void set_range(MacroRange r) {
        range = r;
        switch (range) {
        case MacroRange::Bipolar: min = 0; max = Haken::max14; break;
        case MacroRange::Unipolar: min = Haken::zero14; max = Haken::max14; break;
        case MacroRange::Custom: break;
        }
    }

    bool valid() { return (module_id >= int64_t(0)) && (knob_id >= 0) && in_range(macro_number, U8(7), U8(90)); }
    float rack_min() { return rescale(min, 0, Haken::max14, -5.f, 5.f); }
    float rack_max() { return rescale(max, 0, Haken::max14, -5.f, 5.f); }

    void from_json(json_t* root) {
        module_id = get_json_int64(root, "module", -1);
        knob_id = get_json_int(root, "knob", -1);
        macro_number = get_json_int(root, "macro", INVALID_MACRO);
        name = get_json_string(root, "name");
        min = get_json_int(root,"min", 0);
        max = get_json_int(root,"max", Haken::max14);
        if ((min == 0) && max == (Haken::max14)) {
            range = MacroRange::Bipolar;
        } else if ((min == Haken::zero14) && (max == Haken::max14)) {
            range = MacroRange::Unipolar;
        } else {
            range = MacroRange::Custom;
        }
        modulated = get_json_bool(root, "modulated", false);
    }

    json_t * to_json() {
        json_t * root = json_object();
        json_object_set_new(root, "module", json_integer(module_id));
        json_object_set_new(root, "knob", json_integer(knob_id));
        json_object_set_new(root, "macro", json_integer(macro_number));
        json_object_set_new(root, "name", json_string(name.c_str()));
        json_object_set_new(root, "min", json_integer(min));
        json_object_set_new(root, "max", json_integer(max));
        json_object_set_new(root, "modulated", json_boolean(modulated));
        return root;
    }
};

struct MacroData
{
    std::vector<std::shared_ptr<MacroDescription>> data;
    MacroData() {}
    bool empty() { return data.empty(); }
    size_t size() { return data.size(); }

    std::vector<std::shared_ptr<pachde::MacroDescription>>::iterator find(int64_t module_id, ssize_t knob_id)
    {
        assert((knob_id >= 0) && (module_id >= 0));
        auto it = std::find_if(data.begin(), data.end(), [module_id, knob_id](std::shared_ptr<MacroDescription> m) {
            return (module_id == m->module_id) && (knob_id == m->knob_id);
        });
        return it;
    }

    std::vector<std::shared_ptr<pachde::MacroDescription>>::iterator find_macro_number(uint8_t macro_number)
    {
        auto it = std::find_if(data.begin(), data.end(), [macro_number](std::shared_ptr<MacroDescription> m) {
            return macro_number == m->macro_number;
        });
        return it;
    }

    std::vector<std::shared_ptr<pachde::MacroDescription>>::iterator find_module(int64_t id)
    {
        auto it = std::find_if(data.begin(), data.end(), [id](std::shared_ptr<MacroDescription> m) {
            return id == m->module_id;
        });
        return it;
    }

    void add(std::shared_ptr<MacroDescription> macro)
    {
        auto it = find(macro->module_id, macro->knob_id);
        assert(it == data.cend());
        data.push_back(macro);
    }

    void add_update(std::shared_ptr<MacroDescription> macro)
    {
        auto it = find(macro->module_id, macro->knob_id);
        if (it == data.cend()) {
            data.push_back(macro);
        } else {
            (*it)->init(*macro);
        }
    }

    void remove(int64_t module_id, ssize_t knob_id)
    {
        auto it = find(module_id, knob_id);
        if (it != data.end()) {
            data.erase(it);
        }
    }

    void remove(int64_t module_id)
    {
        auto it = find_module(module_id);
        while (it != data.end())
        {
            data.erase(it);
            it = find_module(module_id);
        }
    }

    void prune_leaving(const std::vector<int64_t>& preserve)
    {
        std::vector<int64_t> removals;
        for (auto m: data) {
            auto rit = std::find(preserve.cbegin(), preserve.cend(), m->module_id);
            if (preserve.cend() == rit) {
                removals.push_back(m->module_id);
            }
        }
        for (auto id : removals) {
            remove(id);
        }
    }

    std::shared_ptr<MacroDescription> get_macro(int64_t module_id, ssize_t knob_id)
    {
        auto it = find(module_id, knob_id);
        return it == data.end() ? nullptr : *it;
    }

    void from_json(json_t* root)
    {
        data.clear();
        auto jar = json_object_get(root, "macros");
        if (jar) {
            json_t* jp;
            size_t index;
            json_array_foreach(jar, index, jp) {
                auto macro = std::make_shared<MacroDescription>(jp);
                data.push_back(macro);
            }
        }
    }

    void to_json(json_t* root)
    {
        auto jar = json_array();
        for (auto macro: data) {
            if (macro->valid()) {
                json_array_append_new(jar, macro->to_json());
            }
        }
        json_object_set_new(root, "macros", jar);
    }
};

}