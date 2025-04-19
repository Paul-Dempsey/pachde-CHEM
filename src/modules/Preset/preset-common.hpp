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

// inline void copy_filters(uint64_t* source, uint64_t* destination) {
//     std::memcpy(destination, source, 5 * sizeof(uint64_t));
// }

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