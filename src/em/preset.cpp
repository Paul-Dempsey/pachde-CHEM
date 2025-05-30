#include "preset.hpp"
#include "../services/misc.hpp"
#include "preset-macro.hpp"

namespace pachde {

std::string id_spec_to_string(PresetId id) {
    return format_string("%d.%d.%d", id.bank_hi(), id.bank_lo(), id.number());
}

PresetId parse_id_spec(std::string spec)
{
    if (spec.empty()) return PresetId();

    uint8_t hi = 0, lo = 0, n = 0;
    auto it = spec.cbegin();
    auto end = spec.cend();
    while (it != end && ('.' != *it)) {
        assert(isdigit(*it));
        hi = (hi*10) + (*it - '0');
        it++;
    }
    it++;
    while (it !=end && ('.' != *it)) {
        assert(isdigit(*it));
        lo = (lo*10) + (*it - '0');
        it++;
    }
    it++;
    while (it != end) {
        assert(isdigit(*it));
        n = (n*10) + (*it - '0');
        it++;
    }
    return PresetId(hi,lo,n);
}

json_t* PresetDescription::toJson(bool include_id, bool include_name, bool include_text)
{
    json_t* root = json_object();
    if (include_id) {
        json_object_set_new(root, "id", json_string(id_spec_to_string(id).c_str()));
    }
    if (include_name) {
        json_object_set_new(root, "name", json_string(name.c_str()));
    }
    if (include_text && text.size()) {
        auto data = collapse_space(text);
        json_object_set_new(root, "text", json_string(data.c_str()));
    }
    return root;
}

void PresetDescription::fromJson(const json_t* root)
{
    id = parse_id_spec(get_json_string(root, "id"));
    name = get_json_string(root, "name");
    text = get_json_string(root, "text");
}

std::string PresetDescription::summary() const
{
    return format_string("[%d.%d.%d] %s", id.bank_hi(), id.bank_lo(), id.number(), name.c_str());
}

std::string PresetDescription::meta_text() const
{
    if (text.empty()) return summary();

    auto info = format_string("%s\n[%d.%d.%d]", name.c_str(), id.bank_hi(), id.bank_lo(), id.number());

    auto macs = make_macro_summary(text);
    if (!macs.empty()) {
        info.push_back('\n');
        info.append(macs);
    }
 
    auto cats = hakenMetaCode.make_category_multiline_text(text);
    if (!cats.empty()) {
        info.push_back('\n');
        info.append(cats);
    }

    auto author = parse_author(text);
    if (!author.empty()) {
        info.append("\nAuthor: ");
        info.append(author);
    }
    return info;
}
}