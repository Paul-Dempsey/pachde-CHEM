#pragma once
#include "../XM-shared/xm-edit-common.hpp"
using namespace pachde;

struct MacroData
{
    std::vector<std::shared_ptr<MacroDescription>> data;
    MacroData() {}
    bool empty() { return data.empty(); }
    size_t size() { return data.size(); }

    void init(ssize_t count) {
        data.clear();
        for (ssize_t i = 0; i < count; ++i) {
            auto p = std::make_shared<MacroDescription>();
            p->index = i;
            add(p);
        }
    }
    void add(std::shared_ptr<MacroDescription> macro) {
        if (-1 == macro->index) {
            macro->index = ssize_t(data.size());
        }
        data.push_back(macro);
    }

    std::shared_ptr<MacroDescription> get_macro(ssize_t index) {
        if (index < 0) return nullptr;
        if (index >= ssize_t(data.size())) return nullptr;
        return data[index];
    }

    void from_json(json_t* root) {
        data.clear();
        auto jar = json_object_get(root, "macros");
        if (jar) {
            json_t* jp;
            size_t index;
            json_array_foreach(jar, index, jp) {
                auto macro = std::make_shared<MacroDescription>();
                macro->from_json(jp);
                data.push_back(macro);
            }
        }
    }

    void to_json(json_t* root) {
        auto jar = json_array();
        for (auto macro: data) {
            json_array_append_new(jar, macro->to_json());
        }
        json_object_set_new(root, "macros", jar);
    }
};
