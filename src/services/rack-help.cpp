#include "rack-help.hpp"

namespace pachde {

bool lrtb_widget_sort(const Widget* a, const Widget* b) {
    if (a->box.pos.y < b->box.pos.y) return true;
    if (a->box.pos.y > b->box.pos.y) return false;
    return a->box.pos.x < b->box.pos.x;
}



void fill_right_module_positions(std::vector<saveModulePos>& module_positions, ModuleWidget* this_widget, float extra)
{
    module_positions.clear();
    std::vector<ModuleWidget*> module_widgets = APP->scene->rack->getModules();
    std::sort(module_widgets.begin(), module_widgets.end(), lrtb_widget_sort);
    for (auto m: module_widgets) {
        if (m->box.pos.y < this_widget->box.pos.y) continue;
        if (m->box.pos.y > this_widget->box.pos.y) break;
        // module is to the right in the same row
        if (m->box.pos.x > this_widget->box.pos.x) {
            if (module_positions.empty()) { // first module to the right
                if (m->box.pos.x > (this_widget->box.pos.x + this_widget->box.size.x + extra)) {
                    // There's room for the flyout, so nothing to restore
                    return;
                }
            }
            module_positions.push_back(saveModulePos{m, m->box.pos});
        }
    }
}

void restore_right_module_positions(std::vector<saveModulePos>& module_positions, ModuleWidget* this_widget)
{
    if (module_positions.empty()) return;
    std::vector<ModuleWidget*> module_widgets = APP->scene->rack->getModules();
    std::sort(module_widgets.begin(), module_widgets.end(), lrtb_widget_sort);
    for (auto m: module_widgets) {
        auto it = std::find_if(module_positions.cbegin(), module_positions.cend(), [m](const saveModulePos& smp) {
            return smp.module_widget == m;
        });
        if (it != module_positions.cend()) {
            APP->scene->rack->setModulePosForce(m, (*it).saved_pos);
        }
    }
}


}