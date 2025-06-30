#include "kv-store.hpp"

namespace pachde {

const char* KVStore::bool_text(bool f)
{
    return f ? "true": "false";
}

bool KVStore::bool_value(const std::string& text, bool default_value)
{
    switch (text.size()) {
    case 0:
        return default_value;

    case 1: 
        if ('1' == *text.cbegin()) return true;
        if ('0' == *text.cbegin()) return false;
        break;

    case 4:
        if (0 == text.compare("true")) return true;
        break;

    case 5:
        if (0 == text.compare("false")) return false;
        break;

    default:
        break;
    }
    assert(false);
    return default_value;
}

float KVStore::float_value(const std::string &text, float default_value)
{
    if (text.empty()) return default_value;
    char * stop{nullptr};
    const char *s = text.c_str();
    float r = std::strtof(s, &stop);
    if (stop == s) return default_value;
    return r;
}

inline std::string scan_part(const char* start, const char* limit, const char** next)
{
    const char *scan = start;
    for (; scan < limit; ++scan) {
        auto ch = *scan;
        if ('=' == ch) break;
        if ('\n' == ch) break;
    }
    std::string result(start, scan - start);
    scan++;
    *next = scan <= limit ? scan : nullptr;
    return result;
}

void KVStore::update(const std::string& key, const std::string& value) {
    assert(loaded);
    assert(std::string::npos == key.find('='));
    data[key] = value;
    modified = true;
}

const std::string KVStore::lookup(const std::string& key) {
    auto it = data.find(key);
    return (it == data.cend()) ? "" : it->second;
}

bool KVStore::load() {
    if (loaded) return true;
    assert(!path.empty());
    auto size = system::getFileSize(path);
    if (0 == size) {
        loaded = true;
        return true;
    }
    auto f = std::fopen(path.c_str(), "rb");
    if (!f) return false;
    char* bytes = (char*)malloc(size);
    auto red = std::fread(bytes, 1, size, f);
    std::fclose(f);
    if (red != size) {
        free(bytes);
        return false;
    }
    const char * scan = bytes;
    const char * eob = scan + size;
    while (scan && (scan < eob)) {
        auto key = scan_part(scan, eob, &scan);
        if (key.empty()) break;
        auto value = scan_part(scan, eob, &scan);
        data.insert(std::make_pair(key, value));
    }
    free(bytes);
    loaded = true;
    modified = false;
    return true;
}

bool KVStore::save() {
    assert(!path.empty());
    if (!modified) return true;
    auto f = std::fopen(path.c_str(), "wb");
    if (!f) return false;
    for (auto pair : data) {
        std::string line = pair.first + '=' + pair.second + '\n';
        if (0 == std::fwrite(line.c_str(), 1, line.size(), f)) {
            std::fclose(f);
            return false;
        }
    }
    std::fclose(f);
    modified = false;
    return true;
}

std::shared_ptr<KVStore> the_plugin_store {nullptr};
std::shared_ptr<KVStore> get_plugin_kv_store() {
    if (the_plugin_store) return the_plugin_store;
    return the_plugin_store = std::make_shared<KVStore>(user_plugin_asset("CHEM-kv.txt"));
}

}