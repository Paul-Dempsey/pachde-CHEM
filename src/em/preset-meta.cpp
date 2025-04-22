// Copyright (C) Paul Chase Dempsey
#include "preset-meta.hpp"

namespace pachde {

std::pair<std::string::const_iterator, std::string::const_iterator>
get_token(std::string::const_iterator start, std::string::const_iterator end, std::function<bool(char)> pred)
{
    auto it = std::find_if(start, end, [pred](char c) { return !pred(c); });
    if (it == end) return std::make_pair(end, end);
    auto token_start = it;
    it = std::find_if(it, end, pred);
    return std::make_pair(token_start, it);
}

const char * toString(PresetGroup group)
{
    switch (group) {
        case PresetGroup::Category: return "Category";
        case PresetGroup::Type: return "Type";
        case PresetGroup::Character: return "Character";
        case PresetGroup::Matrix: return "Matrix";
        case PresetGroup::Setting: return "Setting";
        default:
        case PresetGroup::Unknown: return "(Unknown)";
    }
}

const uint16_t CATEGORY_TAG = MetaCode('C', '=');
const HakenMetaCode hakenMetaCode {};

HakenMetaCode::HakenMetaCode()
{
    data.reserve(92);
    data.push_back(std::make_shared<PresetMeta>("CL", PresetGroup::Category, 0, "Classic"));
    data.push_back(std::make_shared<PresetMeta>("CV", PresetGroup::Category, 1, "Control Voltage"));
    data.push_back(std::make_shared<PresetMeta>("DO", PresetGroup::Category, 2, "Drone"));
    data.push_back(std::make_shared<PresetMeta>("KY", PresetGroup::Category, 3, "Keyboard"));
    data.push_back(std::make_shared<PresetMeta>("MD", PresetGroup::Category, 4, "Midi"));
    data.push_back(std::make_shared<PresetMeta>("OT", PresetGroup::Category, 5, "Other"));
    data.push_back(std::make_shared<PresetMeta>("PE", PresetGroup::Category, 6, "Percussion"));
    data.push_back(std::make_shared<PresetMeta>("PR", PresetGroup::Category, 7, "Processor"));
    data.push_back(std::make_shared<PresetMeta>("PT", PresetGroup::Category, 8, "Tuned Percussion"));
    data.push_back(std::make_shared<PresetMeta>("ST", PresetGroup::Category, 9, "Strings"));
    data.push_back(std::make_shared<PresetMeta>("UT", PresetGroup::Category, 10, "Utility"));
    data.push_back(std::make_shared<PresetMeta>("VO", PresetGroup::Category, 11, "Vocal"));
    data.push_back(std::make_shared<PresetMeta>("WI", PresetGroup::Category, 12, "Winds"));
    data.push_back(std::make_shared<PresetMeta>("ZZ", PresetGroup::Category, 13, "(Unkown)"));
    data.push_back(std::make_shared<PresetMeta>("AT", PresetGroup::Type, 0, "Atonal"));
    data.push_back(std::make_shared<PresetMeta>("BA", PresetGroup::Type, 1, "Bass"));
    data.push_back(std::make_shared<PresetMeta>("BO", PresetGroup::Type, 2, "Bowed"));
    data.push_back(std::make_shared<PresetMeta>("BR", PresetGroup::Type, 3, "Brass"));
    data.push_back(std::make_shared<PresetMeta>("DP", PresetGroup::Type, 4, "Demo Preset"));
    data.push_back(std::make_shared<PresetMeta>("EP", PresetGroup::Type, 5, "Electric Piano"));
    data.push_back(std::make_shared<PresetMeta>("FL", PresetGroup::Type, 6, "Flute"));
    data.push_back(std::make_shared<PresetMeta>("LE", PresetGroup::Type, 7, "Lead"));
    data.push_back(std::make_shared<PresetMeta>("OR", PresetGroup::Type, 8, "Organ"));
    data.push_back(std::make_shared<PresetMeta>("PA", PresetGroup::Type, 9, "Pad"));
    data.push_back(std::make_shared<PresetMeta>("PL", PresetGroup::Type, 10, "Plucked"));
    data.push_back(std::make_shared<PresetMeta>("RD", PresetGroup::Type, 11, "Double Reed"));
    data.push_back(std::make_shared<PresetMeta>("RS", PresetGroup::Type, 12, "Single Reed"));
    data.push_back(std::make_shared<PresetMeta>("SU", PresetGroup::Type, 13, "Struck"));
    data.push_back(std::make_shared<PresetMeta>("AC", PresetGroup::Character, 0, "Acoustic"));
    data.push_back(std::make_shared<PresetMeta>("AG", PresetGroup::Character, 1, "Aggressive"));
    data.push_back(std::make_shared<PresetMeta>("AI", PresetGroup::Character, 2, "Airy"));
    data.push_back(std::make_shared<PresetMeta>("AN", PresetGroup::Character, 3, "Analog"));
    data.push_back(std::make_shared<PresetMeta>("AR", PresetGroup::Character, 4, "Arpeggio"));
    data.push_back(std::make_shared<PresetMeta>("BG", PresetGroup::Character, 5, "Big"));
    data.push_back(std::make_shared<PresetMeta>("BI", PresetGroup::Character, 6, "Bright"));
    data.push_back(std::make_shared<PresetMeta>("CH", PresetGroup::Character, 7, "Chords"));
    data.push_back(std::make_shared<PresetMeta>("CN", PresetGroup::Character, 8, "Clean"));
    data.push_back(std::make_shared<PresetMeta>("DA", PresetGroup::Character, 9, "Dark"));
    data.push_back(std::make_shared<PresetMeta>("DI", PresetGroup::Character, 10, "Digital"));
    data.push_back(std::make_shared<PresetMeta>("DT", PresetGroup::Character, 11, "Distorted"));
    data.push_back(std::make_shared<PresetMeta>("DY", PresetGroup::Character, 12, "Dry"));
    data.push_back(std::make_shared<PresetMeta>("EC", PresetGroup::Character, 13, "Echo"));
    data.push_back(std::make_shared<PresetMeta>("EL", PresetGroup::Character, 14, "Electric"));
    data.push_back(std::make_shared<PresetMeta>("EN", PresetGroup::Character, 15, "Ensemble"));
    data.push_back(std::make_shared<PresetMeta>("EV", PresetGroup::Character, 16, "Evolving"));
    data.push_back(std::make_shared<PresetMeta>("FM", PresetGroup::Character, 17, "FM"));
    data.push_back(std::make_shared<PresetMeta>("HY", PresetGroup::Character, 18, "Hybrid"));
    data.push_back(std::make_shared<PresetMeta>("IC", PresetGroup::Character, 19, "Icy"));
    data.push_back(std::make_shared<PresetMeta>("IN", PresetGroup::Character, 20, "Intimate"));
    data.push_back(std::make_shared<PresetMeta>("LF", PresetGroup::Character, 21, "Lo-fi"));
    data.push_back(std::make_shared<PresetMeta>("LP", PresetGroup::Character, 22, "Looping"));
    data.push_back(std::make_shared<PresetMeta>("LY", PresetGroup::Character, 23, "Layered"));
    data.push_back(std::make_shared<PresetMeta>("MO", PresetGroup::Character, 24, "Morphing"));
    data.push_back(std::make_shared<PresetMeta>("MT", PresetGroup::Character, 25, "Metallic"));
    data.push_back(std::make_shared<PresetMeta>("NA", PresetGroup::Character, 26, "Nature"));
    data.push_back(std::make_shared<PresetMeta>("NO", PresetGroup::Character, 27, "Noise"));
    data.push_back(std::make_shared<PresetMeta>("RN", PresetGroup::Character, 28, "Random"));
    data.push_back(std::make_shared<PresetMeta>("RV", PresetGroup::Character, 29, "Reverberant"));
    data.push_back(std::make_shared<PresetMeta>("SD", PresetGroup::Character, 30, "Sound Design"));
    data.push_back(std::make_shared<PresetMeta>("SE", PresetGroup::Character, 31, "Stereo"));
    data.push_back(std::make_shared<PresetMeta>("SH", PresetGroup::Character, 32, "Shaking"));
    data.push_back(std::make_shared<PresetMeta>("SI", PresetGroup::Character, 33, "Simple"));
    data.push_back(std::make_shared<PresetMeta>("SO", PresetGroup::Character, 34, "Soft"));
    data.push_back(std::make_shared<PresetMeta>("SR", PresetGroup::Character, 35, "Strumming"));
    data.push_back(std::make_shared<PresetMeta>("SY", PresetGroup::Character, 36, "Synthetic"));
    data.push_back(std::make_shared<PresetMeta>("WA", PresetGroup::Character, 37, "Warm"));
    data.push_back(std::make_shared<PresetMeta>("WO", PresetGroup::Character, 38, "Woody"));
    data.push_back(std::make_shared<PresetMeta>("AD", PresetGroup::Matrix, 0, "Additive"));
    data.push_back(std::make_shared<PresetMeta>("BB", PresetGroup::Matrix, 1, "BiqBank"));
    data.push_back(std::make_shared<PresetMeta>("BH", PresetGroup::Matrix, 2, "BiqGraph"));
    data.push_back(std::make_shared<PresetMeta>("BM", PresetGroup::Matrix, 3, "BiqMouth"));
    data.push_back(std::make_shared<PresetMeta>("CM", PresetGroup::Matrix, 4, "Cutoff Mod"));
    data.push_back(std::make_shared<PresetMeta>("DF", PresetGroup::Matrix, 5, "Formula Delay"));
    data.push_back(std::make_shared<PresetMeta>("DM", PresetGroup::Matrix, 6, "Micro Delay"));
    data.push_back(std::make_shared<PresetMeta>("DS", PresetGroup::Matrix, 7, "Sum Delay"));
    data.push_back(std::make_shared<PresetMeta>("DV", PresetGroup::Matrix, 8, "Voice Delay"));
    data.push_back(std::make_shared<PresetMeta>("HM", PresetGroup::Matrix, 9, "HarMan"));
    data.push_back(std::make_shared<PresetMeta>("KI", PresetGroup::Matrix, 10, "Kinetic"));
    data.push_back(std::make_shared<PresetMeta>("MM", PresetGroup::Matrix, 11, "ModMan"));
    data.push_back(std::make_shared<PresetMeta>("OJ", PresetGroup::Matrix, 12, "Osc Jenny"));
    data.push_back(std::make_shared<PresetMeta>("OP", PresetGroup::Matrix, 13, "Osc Phase"));
    data.push_back(std::make_shared<PresetMeta>("OS", PresetGroup::Matrix, 14, "Osc DSF"));
    data.push_back(std::make_shared<PresetMeta>("SB", PresetGroup::Matrix, 15, "SineBank"));
    data.push_back(std::make_shared<PresetMeta>("SS", PresetGroup::Matrix, 16, "SineSpray"));
    data.push_back(std::make_shared<PresetMeta>("WB", PresetGroup::Matrix, 17, "WaveBank"));
    data.push_back(std::make_shared<PresetMeta>("C1", PresetGroup::Setting, 0, "Channel 1"));
    data.push_back(std::make_shared<PresetMeta>("EM", PresetGroup::Setting, 1, "External Midi Clock"));
    data.push_back(std::make_shared<PresetMeta>("MI", PresetGroup::Setting, 2, "Mono Interval"));
    data.push_back(std::make_shared<PresetMeta>("PO", PresetGroup::Setting, 3, "Portamento"));
    data.push_back(std::make_shared<PresetMeta>("RO", PresetGroup::Setting, 4, "Rounding"));
    data.push_back(std::make_shared<PresetMeta>("SP", PresetGroup::Setting, 5, "Split Voice"));
    data.push_back(std::make_shared<PresetMeta>("SV", PresetGroup::Setting, 6, "Single Voice"));
    data.push_back(std::make_shared<PresetMeta>("TA", PresetGroup::Setting, 7, "Touch Area"));
    //DEBUG("HCMetaCode size: %lld", data.size());
    std::sort(data.begin(), data.end(), [](const std::shared_ptr<PresetMeta>& a, const std::shared_ptr<PresetMeta>& b) { return a->code < b->code; });
}

std::string HakenMetaCode::categoryName(uint16_t key) const
{
    auto cat = find(key);
    return cat ? cat->name : "Unknown";
}

std::shared_ptr<PresetMeta> HakenMetaCode::find(uint16_t key) const
{
    auto item = std::lower_bound(data.cbegin(), data.cend(), key, [](const std::shared_ptr<PresetMeta> &p, uint16_t key){ return p->code < key; });
    return data.cend() != item ? *item : nullptr;
}

void foreach_code(const std::string& text, std::function<bool(uint16_t)> callback)
{
    if (text.empty()) return;
    auto token = std::make_pair(text.cbegin(), text.cend());
    //if (token.first == token.second) return;
    while (true) {
        token = get_token(token.first, text.cend(), is_space);
        if (token.first == token.second) break;
        if (CATEGORY_TAG == MetaCode(token.first)) {
            auto it = token.first + 2;
            auto end = token.second;
            while (true) {
                token = get_token(it, end, is_underscore);
                if (token.first == token.second) break;
                if (!callback(MetaCode(token.first))) return;
                it = token.second;
            }
            break;
        }
        token.first = token.second;
    }
}

bool order_codes(const uint16_t &a, const uint16_t &b)
{
    auto p1 = hakenMetaCode.find(a);
    auto p2 = hakenMetaCode.find(b);
    if (!p1 && !p2) return a < b;
    if (p1 && !p2) return true;
    if (!p1) return false; //p2 is non-null from above two
    if (p1->group < p2->group) return true;
    if (p1->group == p2->group) return p1->index < p2->index;
    return false;
}

bool order_meta(const std::shared_ptr<PresetMeta> a, const std::shared_ptr<PresetMeta> b)
{
    return order_codes(a->code, b->code);
}

void FillMetaCodeList(const std::string& text, std::vector<uint16_t>& vec)
{
    vec.clear();
    if (text.empty()) return;
    foreach_code(text, [&vec](uint16_t code) mutable {
        vec.push_back(code);
        return true;
        });
    std::sort(vec.begin(), vec.end(), order_codes);
    auto default_code = MetaCode("ZZ").code; // default to Unknown if category missing
    if (vec.empty()) {
        vec.push_back(default_code);
    } else {
        auto first_code_meta = hakenMetaCode.find(*vec.cbegin());
        if (!first_code_meta || first_code_meta->group != PresetGroup::Category) {
            vec.insert(vec.begin(), default_code);
        }
    }
}

void FillMetaCodeMasks(const std::vector<uint16_t>& meta_codes, uint64_t* masks)
{
    std::memset(masks, 0, 5 * sizeof(*masks));
    for (MetaCode code : meta_codes) {
        auto meta = hakenMetaCode.find(code);
        if (meta) {
            size_t g = size_t(meta->group);
            uint64_t bit = (uint64_t(1) << meta->index);
            masks[g] |= bit;
        }
    }
}

std::vector<std::shared_ptr<PresetMeta>> HakenMetaCode::make_category_list(const std::string& text) const
{
    std::vector<std::shared_ptr<PresetMeta>> result;
    if (text.empty()) return result;
    foreach_code(text, [this, &result](uint16_t code) mutable { 
        auto item = find(code);
        if (item) {
            result.push_back(item);
        }
        return true;
    });
    std::sort(result.begin(), result.end(), order_meta);
    return result;
}

std::string HakenMetaCode::make_category_multiline_text(const std::string& text) const
{
    if (text.empty()) return "";
    std::string result;
    auto cats = make_category_list(text);
    auto group = PresetGroup::Unknown;
    bool first = true;
    for (auto pm: cats) {
        if (group != pm->group) {
            if (group != PresetGroup::Unknown) {
                result.push_back('\n');
            }
            first = true;
            result.append(toString(pm->group));
            result.append({':', ' ', ' '});
            group = pm->group;
        }
        if (first) {
            first = false;
        } else {
            result.append({',', ' '});
        }
        //result.push_back('"');
        result.append(pm->name);
        //result.push_back('"');
    }
    return result;
}

// std::string HCMetaCode::make_category_json(const std::string& text) const
// {
//     if (text.empty()) return "";
//     std::string result;
//     auto cats = make_category_list(text);

//     result.push_back('{');
//     auto group = PresetGroup::Unknown;
//     bool first = true;
//     bool open_array = false;
//     for (auto pm: cats) {
//         if (group != pm->group) {
//             switch (group) {
//             case PresetGroup::Category: result.append({',', ' '}); break;
//             case PresetGroup::Unknown: break;
//             default:
//                 result.append({']', ',', ' '});
//                 open_array = false;
//                 break;
//             }
//             first = true;
//             result.append(toString(pm->group));
//             if (pm->group == PresetGroup::Category) {
//                 result.push_back(':');
//             } else {
//                 result.append({':', '['});
//                 open_array = true;
//             }
//             group = pm->group;
//         }
//         if (first) {
//             first = false;
//         } else {
//             result.append({',', ' '});
//         }
//         result.push_back('"');
//         result.append(pm->name);
//         result.push_back('"');
//     }
//     if (open_array) { result.push_back(']'); }
//     result.push_back('}');
//     return result;
// }

std::string make_result(const char * start, const char * end) {
    std::string result;
    while (start < end) {
        result.push_back('_' == *start ? ' ' : *start);
        ++start;
    }
    return result;
}

std::string parse_author(const std::string &text)
{
    if (text.empty()) return text;

    const char * end{text.c_str() + text.size()};
    const char * scan{end - 1};
    const char * data{scan};
    const char * eq{nullptr};
    const char * id{nullptr};

    enum State { Data, Id };
    State state = Data;
    const char * lim = end - text.size();
    while (scan >= lim) {
        switch (state) {
        case Data:
            if ('=' == *scan) {
                eq = scan;
                state = Id;
            } else {
                data = scan;
            }
            break;
        case Id:
            if (std::isspace(*scan)) {
                if (!id) return "";
                if ((1 == (eq - id)) && 'A' == *id) {
                    return make_result(data, end);
                } else {
                    data = scan;
                    end = scan + 1;
                    state = Data;
                }
            } else {
                id = scan;
            }
            break;
        }
        --scan;
    }
    if (id && eq && (1 == (eq - id)) && 'A' == *id) {
        return make_result(data, end);
    }

    // Didn't find an Author section (A=). So,
    // handle observed pattern where the author is simply appended
    // with a whitespace separator and without the A= section tag
    auto pos = text.rfind('=');
    if (std::string::npos != pos) {
        auto scan = text.cbegin() + pos;
        while (scan != text.cend() && *scan != ' ') scan++;
        while (scan != text.cend() && *scan == ' ') scan++;
        if (scan != text.cend()) {
            std::string result(scan, text.cend());
            std::replace(result.begin(), result.end(), '_', ' ');
            return result;
        }
    }
    return "";
}

}