#include "preset-macro.hpp"

namespace eaganmatrix {

PresetMacro::PresetMacro()
{
    // max 56 according to the EaganMatrix Programming cookbook
    macro[0].reserve(56);
    macro[1].reserve(56);
    macro[2].reserve(56);
    macro[3].reserve(56);
    macro[4].reserve(56);
    macro[5].reserve(56);
}

void PresetMacro::default_macro_names()
{
    macro[0] = "i";
    macro[1] = "ii";
    macro[2] = "iii";
    macro[3] = "iv";
    macro[4] = "v";
    macro[5] = "vi";
}
void PresetMacro::empty_macro_names()
{
    macro[0].clear();
    macro[1].clear();
    macro[2].clear();
    macro[3].clear();
    macro[4].clear();
    macro[5].clear();
}
void PresetMacro::fill_macro_names(const char* name)
{
    macro[0] = name;
    macro[1] = name;
    macro[2] = name;
    macro[3] = name;
    macro[4] = name;
    macro[5] = name;
}
void PresetMacro::modern_macro_names()
{
    macro[0] = "M1";
    macro[1] = "M2";
    macro[2] = "M3";
    macro[3] = "M4";
    macro[4] = "M5";
    macro[5] = "M6";
}

int index_for_id(const std::string& id)
{
    auto it = id.cbegin();
    switch (id.size()) {
    case 1:
        switch (*it) {
        case 'i': return 0;
        case 'v': return 4;
        }
        break;
    case 2:
        switch (*it++) {
        case 'i':
            if ('i' == *it) return 1;
            if ('v' == *it) return 3;
            break;
        case 'g':
            if ('1' == *it) return 4;
            if ('2' == *it) return 5;
            break;
        case 'v':
            if ('i' == *it) return 5;
            break;
        }
        break;
    case 3:
        if (('i' == *it++) && ('i' == *it++) && ('i' == *it)) return 2;
        break;
    }
    return -1;
}

void PresetMacro::parse_text(const std::string& text)
{
    if (text.empty()) return;

    std::string id;
    id.reserve(3);
    enum ParseState { Macro, Name, Value } state = ParseState::Macro;
    int macro_index = -1;
    for (auto p = text.c_str(); *p; ++p) {
        char ch = *p;
        switch (state) {
        case ParseState::Macro: {
            switch (ch) {
                case ' ': case '\r':case '\n': case '\t':
                    break;
                case '=':
                    macro_index = index_for_id(id);
                    id = "";
                    if (-1 != macro_index) {
                        macro[macro_index].clear();
                    }
                    state = ParseState::Name;
                    break;
                default:
                    id.push_back(ch);
                    break;
            }
        } break;
        
        case ParseState::Name: {
            switch (ch) {
                case ' ': case '\r': case '\n': case '\t':
                    state = ParseState::Macro;
                    break;
                case '_':
                    state = ParseState::Value;
                    break;
                default:
                    if (-1 != macro_index) {
                        macro[macro_index].push_back(ch);
                    }
                    break;
            }
        } break;

        case ParseState::Value: {
            switch (ch) {
                case ' ': case '\r': case '\n': case '\t':
                    state = ParseState::Macro;
                    break;
                default:
                    break;
            }
        } break;
        }
    }
}

const char * name_for_index(int i) {
    switch (i) {
    case 0: return "i";
    case 1: return "ii";
    case 2: return "iii";
    case 3: return "iv";
    case 4: return "v";
    case 5: return "vi";
    default: return "?";
    }
}

std::string make_macro_summary(const std::string &source)
{
    PresetMacro mac;
    mac.parse_text(source);
    bool any{false};
    for (int i = 0; i < 6; ++i) {
        if (!mac.macro[i].empty()) {
            any = true;
            break;
        }
    }
    if (!any) return "";

    std::string text = "Macros: ";
    any = false;
    for (int i = 0; i < 6; ++i) {
        if (!mac.macro[i].empty()) {
            if (any) {
                text.push_back(' ');
            }
            text.append(name_for_index(i));
            text.push_back('=');
            text.append(mac.macro[i]);
            any = true;
        }
    }
    return text;
}
}