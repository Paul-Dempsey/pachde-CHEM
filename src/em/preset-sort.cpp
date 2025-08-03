#include "preset-sort.hpp"

namespace eaganmatrix {

bool preset_no_order(const std::shared_ptr<PresetInfo>& p1, const std::shared_ptr<PresetInfo>& p2)
{
    return true;
}

bool preset_natural_order(const std::shared_ptr<PresetInfo>& p1, const std::shared_ptr<PresetInfo>& p2)
{
    return p1->id.key() < p2->id.key();
}

bool preset_category_order(const std::shared_ptr<PresetInfo>& p1, const std::shared_ptr<PresetInfo>& p2)
{
    if (p1->meta.empty()) {
        return p2->meta.empty() ? preset_alpha_order(p1, p2) : false;
    }
    if (p2->meta.empty()) {
        return true;
    }
    auto m1 = hakenMetaCode.find(*p1->meta.cbegin());
    assert(m1 && m1->group == PresetGroup::Category);
    auto m2 = hakenMetaCode.find(*p2->meta.cbegin());
    assert(m2 && m2->group == PresetGroup::Category);
    if (m1->group == m2->group) {
        if (m1->group == PresetGroup::Unknown) {
            return preset_alpha_order(p1, p2);
        } else {
            if (m1->index < m2->index) return true;
            if (m1->index == m2->index) return preset_alpha_order(p1, p2);
            return false;
        }
    }
    return preset_alpha_order(p1, p2);
}

bool preset_alpha_order(const std::shared_ptr<PresetInfo>& preset1, const std::shared_ptr<PresetInfo>& preset2)
{
    assert(!preset1->name.empty());
    assert(!preset2->name.empty());
    auto p1 = preset1->name.cbegin();
    auto p2 = preset2->name.cbegin();
    for (; p1 != preset1->name.cend() && p2 != preset2->name.cend(); ++p1, ++p2) {
        if (*p1 == *p2) continue;
        auto c1 = std::tolower(*p1);
        auto c2 = std::tolower(*p2);
        if (c1 == c2) continue;
        if (c1 < c2) return true;
        return false;
    }
    if (p1 == preset1->name.cend() && p2 != preset2->name.cend()) {
        return true;
    }
    return false;
}

std::function<bool (const std::shared_ptr<PresetInfo>&, const std::shared_ptr<PresetInfo>&)>
getPresetSort(PresetOrder order)
{
    switch (order) {
    case PresetOrder::None: return preset_no_order;
    case PresetOrder::Natural: return preset_natural_order;
    case PresetOrder::Alpha: return preset_alpha_order;
    case PresetOrder::Category: return preset_category_order;
    default:
        assert(false);
        return preset_alpha_order;
    }
}

}