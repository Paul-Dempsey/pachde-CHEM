#pragma once
#include <rack.hpp>
namespace pachde {

struct PresetFileInfo
{
    std::string connection;
    std::string file;
    uint8_t hardware{0};

    PresetFileInfo() {}

    PresetFileInfo(uint8_t hardware, std::string connection, std::string file) :
        connection(connection),
        file(file),
        hardware(hardware)
    {}

    bool empty() { return (0 == hardware) || file.empty() || connection.empty(); }

    void fromJson(const json_t* root);
    json_t* toJson();

    static PresetFileInfo* create(const json_t* root);
};

void load_pfis(std::string path, std::vector<std::shared_ptr<PresetFileInfo>>& data);
bool save_pfis(std::string path, std::vector<std::shared_ptr<PresetFileInfo>>& data);

std::string pfis_filename();

}