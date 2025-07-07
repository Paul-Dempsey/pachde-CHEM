#pragma once
namespace pachde {

enum FilterId {
    Category,
    Type,
    Character,
    Matrix,
    Setting
};

inline bool any_filter(uint64_t* p)
{
    if (*p) return true;
    if (*(++p)) return true;
    if (*(++p)) return true;
    if (*(++p)) return true;
    if (*(++p)) return true;
    return false;
}

}