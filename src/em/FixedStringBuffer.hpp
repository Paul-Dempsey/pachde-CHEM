// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>

using namespace ::rack;
namespace eaganmatrix {

template<const size_t size>
class FixedStringBuffer
{
    char data[size];
    char * end;
    const char * const lim = data + size;

public:
    FixedStringBuffer() { clear(); }

    bool empty() const { return end == data; }
    const char *  str() const { return data; }

    void clear() {
        *data = 0;
        end = data;
    }

    void build(char a) {
        if (end < lim-1) {
            *(end + 1) = 0;
            *end++ = a;
        }
    }
};

}