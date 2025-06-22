#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../services/misc.hpp"

namespace eaganmatrix {

struct PresetMacro
{
    std::string macro[6];

    PresetMacro();

    void default_macro_names(); // i, ii, iii, iv, v, vi
    void empty_macro_names();
    void fill_macro_names(const char* name = "—"); // default: em-dashes
    void modern_macro_names(); // M1-M6
    void parse_text(const std::string& text);
};

std::string make_macro_summary(const std::string& text);

}