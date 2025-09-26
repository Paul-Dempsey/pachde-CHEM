#pragma once
#include <rack.hpp>
using namespace ::rack;

namespace pachde
{

// set displayPrecision = 4
inline ParamQuantity* dp4(ParamQuantity* p) {
    p->displayPrecision = 4;
    return p;
}

// set displayPrecision = 2
inline ParamQuantity* dp2(ParamQuantity* p) {
    p->displayPrecision = 2;
    return p;
}

// disable randomization
inline ParamQuantity* no_randomize(ParamQuantity* p) {
    p->randomizeEnabled = false;
    return p;
}

// enable snap (integer rounding)
inline ParamQuantity* snap(ParamQuantity* p) {
    p->snapEnabled = true;
    return p;
}

// sort fn for widgets in left-to-right, top-to-bottom order.
bool lrtb_widget_sort(const Widget* a, const Widget* b);

struct saveModulePos {
    ModuleWidget* module_widget;
    Vec saved_pos;
};

// collect the positions of all mopdules to the right of modwidget.
// if there is more than 'extra' space to the immediate right of modwidget,
// module_positiosn will be empty
void fill_right_module_positions(std::vector<saveModulePos>& module_positions, ModuleWidget* this_widget, float extra);

void restore_right_module_positions(std::vector<saveModulePos>& module_positions, ModuleWidget* this_widget);

}