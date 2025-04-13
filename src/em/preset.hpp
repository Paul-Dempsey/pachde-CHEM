#pragma once
#include <rack.hpp>
#include "PresetId.hpp"
#include "preset-meta.hpp"
#include "../services/misc.hpp"

using namespace ::rack;

namespace pachde {

struct PresetDescription
{
    PresetId id;
    std::string name;
    std::string text;

    PresetDescription() {
        id.invalidate();
        name.reserve(32);
        text.reserve(256);
    }

    PresetDescription(PresetId id, std::string name, std::string text)
    : id(id), name(name), text(text)
    {}

    void init(const PresetDescription* source) {
        if (source) {
            id = source->id;
            name = source->name;
            text = source->text;
        } else {
            clear();
        }
    }

    bool valid() const { return id.valid(); }
    bool empty() const {
        return !valid() || name.empty() || name == "-";
    }

    void clear() {
        id.clear();
        name.clear();
        text.clear();
    }

    json_t* toJson(bool include_id, bool include_name, bool include_text);
    void fromJson(const json_t* root);
    std::string summary() const;

};

struct PresetInfo : PresetDescription {
    using Base = PresetDescription;

    std::vector<uint16_t> meta;

    PresetInfo(PresetId id, std::string name, std::string text) :
        PresetDescription(id, name, text)
    {
        FillCategoryCodeList(text, meta);
    }

    PresetInfo(std::shared_ptr<PresetDescription> preset)
    {
        Base::id = preset->id;
        Base::name = preset->name;
        Base::text = preset->text;

        FillCategoryCodeList(preset->text, meta);
    }

    void init(const PresetDescription* source) {
        Base::init(source);
        meta.clear();
        FillCategoryCodeList(text, meta);
    }

    std::string category_code() {
        if (meta.empty()) return "OT";
        return CategoryCode(meta[0]).to_string();
    }

    std::string meta_text() {
        auto info = hakenCategoryCode.make_category_multiline_text(text);
        return format_string("%s\n[%d.%d.%d]\n%s", name.c_str(), id.bank_hi(), id.bank_lo(), id.number(), info.c_str());
    }
};

}