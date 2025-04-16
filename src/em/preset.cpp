#include "preset.hpp"
#include "../services/misc.hpp"
#include "preset-macro.hpp"

namespace pachde {

json_t* PresetDescription::toJson(bool include_id, bool include_name, bool include_text)
{
    json_t* root = json_object();
    if (include_id) {
        json_object_set_new(root, "id", json_integer(id.key()));
    }
    if (include_name) {
        json_object_set_new(root, "name", json_stringn(name.c_str(), name.size()));
    }
    if (include_text && text.size()) {
        auto data = collapse_space(text);
        json_object_set_new(root, "text", json_stringn(data.c_str(), data.size()));
    }
    return root;
}

void PresetDescription::fromJson(const json_t* root)
{
    auto j = json_object_get(root, "id");
    if (j) {
        id = {uint32_t(json_integer_value(j))};
    }
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