// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>

namespace eaganmatrix {

enum class PresetGroup : uint8_t {
    Category,
    Type,
    Character,
    Matrix,
    Setting,
    Unknown,
};

const char * toString(PresetGroup group);
inline bool is_space(char c) { return ' ' == c || '\n' == c || '\r' == c || '\t' == c; }
inline bool not_space(char c) { return !is_space(c); }
inline bool is_underscore(char c) { return '_' == c; }

std::pair<std::string::const_iterator, std::string::const_iterator>
get_token(std::string::const_iterator start, std::string::const_iterator end, std::function<bool(char)> pred);

union MetaCode {
    uint16_t code;
    char letters[2];

    MetaCode(uint16_t u) : code(u) {}
    operator uint16_t() const { return code; }
    MetaCode(const char * code_token) : letters{*code_token, *(code_token + 1)} {}
    MetaCode(char a, char b) : letters{a, b} {}
    MetaCode(std::string::const_iterator it) : letters{*it, *(1+it)} {}
    std::string to_string() { return std::string{letters[0], letters[1]}; }
};

constexpr const uint16_t ST = 0x5453;
constexpr const uint16_t WI = 0x4957;
constexpr const uint16_t VO = 0x4f56;
constexpr const uint16_t KY = 0x594b;
constexpr const uint16_t CL = 0x4c43;
constexpr const uint16_t OT = 0x544f;
constexpr const uint16_t PE = 0x4550;
constexpr const uint16_t PT = 0x5450;
constexpr const uint16_t PR = 0x5250;
constexpr const uint16_t DO = 0x4f44;
constexpr const uint16_t MD = 0x444d;
constexpr const uint16_t CV = 0x5643;
constexpr const uint16_t UT = 0x5455;
constexpr const uint16_t ZZ = 0x5a5a;

struct PresetMeta {
    uint16_t code;
    PresetGroup group;
    uint8_t index;
    const char * name;
    PresetMeta (const char * code_token, PresetGroup g, uint8_t index, const char * name)
    :   code(MetaCode(code_token)),
        group(g),
        index(index),
        name(name)
    {}
};

class HakenMetaCode
{
    std::vector<std::shared_ptr<PresetMeta>> data;

public:
    HakenMetaCode();

    std::shared_ptr<PresetMeta> find(uint16_t key) const;
    std::vector<std::shared_ptr<PresetMeta>> make_category_list(const std::string& text) const;
    //std::string make_category_json(const std::string& text) const;
    std::string make_category_multiline_text(const std::string& text) const;
    std::string categoryName(uint16_t key) const;
};

void FillMetaCodeList(const std::string& text, std::vector<uint16_t>& vec);
void FillMetaCodeMasks(const std::vector<uint16_t>& meta_codes, uint64_t* masks);
void foreach_code(const std::string&, std::function<bool(uint16_t)> callback);
bool order_codes(const uint16_t &a, const uint16_t &b);

extern const HakenMetaCode hakenMetaCode;

std::string parse_author(const std::string& text);

}
