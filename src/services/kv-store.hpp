#pragma once
#include <rack.hpp>
#include "../plugin.hpp"


namespace pachde {

struct KVStore {
    std::string path;
    std::map<std::string, std::string> data;
    bool loaded{false};
    bool modified{false};

    KVStore(const std::string & file_path) : path(file_path) {
        assert(!path.empty());
    }
    ~KVStore() {
        save();
    }
    void update(const std::string& key, const std::string& value);
    const std::string lookup(const std::string& key);
    bool load();
    bool save();
};

std::shared_ptr<KVStore> get_plugin_kv_store();

}