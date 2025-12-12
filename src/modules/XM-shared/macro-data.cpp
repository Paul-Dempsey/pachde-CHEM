#include "macro-data.hpp"
#include "services/json-help.hpp"

namespace pachde {

MacroDescription::MacroDescription(json_t* root, int64_t module_id) {
    from_json(root, module_id);
}

MacroDescription::MacroDescription(int64_t module, ssize_t knob) :
    module_id(module), knob_id(knob)
{}

void MacroDescription::clear() {
    macro_number = INVALID_MACRO;
    name.clear();
    range = MacroRange::Bipolar;
    min = -1.f;
    max = 1.f;
    cv_port = true;
}

void MacroDescription::init (const MacroDescription& source)
{
    module_id = source.module_id;
    knob_id = source.knob_id;
    macro_number = source.macro_number;
    name = source.name;
    range = source.range;
    min = source.min;
    max = source.max;
    cv_port = source.cv_port;
    modulation = source.modulation;
}

void MacroDescription::set_range(MacroRange r) {
    range = r;
    switch (range) {
    case MacroRange::Bipolar: min = -1.f; max = 1.f; break;
    case MacroRange::Unipolar: min = 0.f; max = 1.f; break;
    case MacroRange::Custom: break;
    }
}

bool MacroDescription::valid() {
    return (module_id >= int64_t(0)) && (knob_id >= 0) && (knob_id < 8) && in_range(macro_number, U8(7), U8(90));
}

void MacroDescription::set_em_value(uint16_t em)
{
    em_value = last_em_value = em;
    param_value = static_cast<float>((static_cast<double>(em) - static_cast<double>(Haken::zero14)) * inv_zero14);
    mod_value = xm_modulated_value(param_value, cv, modulation ? mod_amount : 100.f);
}

void MacroDescription::set_mod_amount(float amount)
{
    assert(in_range(amount, -100.f, 100.f));
    mod_amount = amount;
    mod_value = xm_modulated_value(param_value, cv, modulation ? mod_amount : 100.f);
}

void MacroDescription::set_param_value(float value)
{
    param_value = clamp(value, -1.f, 1.f);
    mod_value = xm_modulated_value(param_value, cv, modulation ? mod_amount : 100.f);
    em_value = xm_em_value_any(mod_value);
}


void MacroDescription::from_json(json_t* root, int64_t module_id) {
    module_id = module_id == -1 ? get_json_int64(root, "module", -1) : module_id;
    knob_id = get_json_int(root, "knob", -1);
    macro_number = get_json_int(root, "macro", INVALID_MACRO);
    name = get_json_string(root, "name");
    min = get_json_float(root,"min", -1.f);
    max = get_json_float(root,"max", 1.f);
    if ((min == -1.f) && max == 1.f) {
        range = MacroRange::Bipolar;
    } else if ((min == 0.f) && (max == 1.f)) {
        range = MacroRange::Unipolar;
    } else {
        range = MacroRange::Custom;
    }
    cv_port = get_json_bool(root, "cv", true);
    modulation = get_json_bool(root, "modulated", cv_port);
    mod_amount = get_json_float(root, "mod-amount", 0.f);
}

json_t * MacroDescription::to_json() {
    json_t * root = json_object();
    json_object_set_new(root, "module", json_integer(module_id));
    json_object_set_new(root, "knob", json_integer(knob_id));
    json_object_set_new(root, "macro", json_integer(macro_number));
    json_object_set_new(root, "name", json_string(name.c_str()));
    json_object_set_new(root, "min", json_real(min));
    json_object_set_new(root, "max", json_real(max));
    json_object_set_new(root, "cv", json_boolean(cv_port));
    json_object_set_new(root, "modulated", json_boolean(modulation));
    json_object_set_new(root, "mod-amount", json_real(mod_amount));
    return root;
}

std::vector<std::shared_ptr<pachde::MacroDescription>>::iterator MacroData::find(int64_t module_id, ssize_t knob_id)
{
    assert((knob_id >= 0) && (module_id >= 0));
    auto it = std::find_if(data.begin(), data.end(), [module_id, knob_id](std::shared_ptr<MacroDescription> m) {
        return (module_id == m->module_id) && (knob_id == m->knob_id);
    });
    return it;
}

std::vector<std::shared_ptr<pachde::MacroDescription>>::iterator MacroData::find_macro_number(uint8_t macro_number)
{
    auto it = std::find_if(data.begin(), data.end(), [macro_number](std::shared_ptr<MacroDescription> m) {
        return macro_number == m->macro_number;
    });
    return it;
}

std::vector<std::shared_ptr<pachde::MacroDescription>>::iterator MacroData::find_module(int64_t id)
{
    auto it = std::find_if(data.begin(), data.end(), [id](std::shared_ptr<MacroDescription> m) {
        return id == m->module_id;
    });
    return it;
}

void MacroData::add(std::shared_ptr<MacroDescription> macro)
{
    assert(find(macro->module_id, macro->knob_id) == data.cend());
    data.push_back(macro);
}

void MacroData::add_update(std::shared_ptr<MacroDescription> macro)
{
    auto it = find(macro->module_id, macro->knob_id);
    if (it == data.cend()) {
        data.push_back(macro);
    } else {
        (*it)->init(*macro);
    }
}

void MacroData::remove(int64_t module_id, ssize_t knob_id)
{
    auto it = find(module_id, knob_id);
    if (it != data.end()) {
        data.erase(it);
    }
}

void MacroData::remove(int64_t module_id)
{
    auto it = find_module(module_id);
    while (it != data.end())
    {
        data.erase(it);
        it = find_module(module_id);
    }
}

std::shared_ptr<MacroDescription> MacroData::get_macro(int64_t module_id, ssize_t knob_id)
{
    auto it = find(module_id, knob_id);
    return it == data.end() ? nullptr : *it;
}

void MacroData::from_json(json_t* root, int64_t module_id)
{
    data.clear();
    auto jar = json_object_get(root, "macros");
    if (jar) {
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto macro = std::make_shared<MacroDescription>(jp, module_id);
            data.push_back(macro);
        }
    }
}

void MacroData::to_json(json_t* root)
{
    auto jar = json_array();
    for (auto macro: data) {
        if (macro->valid()) {
            json_array_append_new(jar, macro->to_json());
        }
    }
    json_object_set_new(root, "macros", jar);
}

}