#pragma once
namespace pachde {

enum class PresetTab { Unset = -1, System, User };
enum FilterId {
    Category,
    Type,
    Character,
    Matrix,
    Setting
};

inline bool any_filter(uint64_t* p)
{
    if (*p++) return true;
    if (*p++) return true;
    if (*p++) return true;
    if (*p++) return true;
    if (*p++) return true;
    return false;
}

}