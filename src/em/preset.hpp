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
    std::string meta_text() const;
};

struct PresetInfo : PresetDescription {
    using Base = PresetDescription;

    std::vector<uint16_t> meta;

    PresetInfo() {}

    PresetInfo(PresetId id, std::string name, std::string text) :
        PresetDescription(id, name, text)
    {
        FillMetaCodeList(text, meta);
    }

    PresetInfo(std::shared_ptr<PresetDescription> preset) :
        PresetDescription(preset->id, preset->name, preset->text)
    {
        FillMetaCodeList(preset->text, meta);
    }

    PresetInfo(const PresetDescription* preset) :
        PresetDescription(preset->id, preset->name, preset->text)
    {
        FillMetaCodeList(text, meta);
    }

    void init(const PresetDescription* source) {
        Base::init(source);
        meta.clear();
        FillMetaCodeList(text, meta);
    }
    void ensure_meta() {
        if (meta.empty()) {
            FillMetaCodeList(text, meta);
        }
    }
    void set_text(const std::string & new_text) {
        text = new_text;
        meta.clear();
        FillMetaCodeList(text, meta);
    }

    std::string category_code() {
        if (meta.empty()) return "ZZ";
        return MetaCode(meta[0]).to_string();
    }

};

}