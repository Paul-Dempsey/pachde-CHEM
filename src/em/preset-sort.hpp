#pragma once
#include "preset.hpp"

namespace eaganmatrix {

enum class PresetOrder {
    None     = -1,
    Natural  = 0,
    Alpha    = 1,
    Category = 2
};

bool preset_no_order(const std::shared_ptr<PresetInfo>& p1, const std::shared_ptr<PresetInfo>& p2);
bool preset_natural_order(const std::shared_ptr<PresetInfo>& p1, const std::shared_ptr<PresetInfo>& p2);
bool preset_alpha_order(const std::shared_ptr<PresetInfo>& p1, const std::shared_ptr<PresetInfo>& p2);
bool preset_category_order(const std::shared_ptr<PresetInfo>& p1, const std::shared_ptr<PresetInfo>& p2);

std::function<bool (const std::shared_ptr<PresetInfo>&, const std::shared_ptr<PresetInfo>&)> getPresetSort(PresetOrder order);

}