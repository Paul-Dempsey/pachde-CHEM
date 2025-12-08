#pragma once
#include <rack.hpp>
#include "em/wrap-HakenMidi.hpp"
#include "services/misc.hpp"
#include "widgets/hamburger.hpp"

using namespace ::rack;
namespace pachde {

struct ShiftQuantity : ParamQuantity
{
    ShiftQuantity()
    {
        snapEnabled = true;
        randomizeEnabled = false;
        smoothEnabled = false;
        minValue = 0;
        maxValue = 127;
        defaultValue = 72.f;
    }

    std::string getDisplayValueString() override {
        int v = static_cast<int>(getValue());
        if (0 == (v % 12)) {
            v = (v - 60) / 12;
            return format_string(std::abs(v) > 1 ? "%+d octaves" : "%+d octave", v);
        } else {
            v -= 60;
            return format_string(std::abs(v) > 1 ? "%+d semitones" : "%+d semitone", v);
        }
    }
};

template <typename TPQ = ShiftQuantity>
TPQ* configShiftParam(Module * module, int paramId, const char * name) {
    assert(paramId >= 0 && static_cast<size_t>(paramId) < module->params.size() && static_cast<size_t>(paramId) < module->paramQuantities.size());
    if (module->paramQuantities[paramId]) {
        delete module->paramQuantities[paramId];
    }
    TPQ* q = new TPQ;
    q->module = module;
    q->paramId = paramId;
    q->name = name;
    module->paramQuantities[paramId] = q;
    Param* p = &module->params[paramId];
    p->value = q->getDefaultValue();
    return q;
}

struct ShiftMenu : HamburgerUi<ParamWidget> {

    MenuItem* createItem(const char * name, int value) {
        return createCheckMenuItem(name, "",
            [=]() { return static_cast<int>(getParamQuantity()->getValue()) == value; },
            [=](){ getParamQuantity()->setValue(static_cast<float>(value)); });
    }
    void appendContextMenu(Menu* menu) override
    {
        if (!module) return;
        menu->addChild(new MenuSeparator);
        menu->addChild(createItem("-4 octaves", 12));
        menu->addChild(createItem("-3 octaves", 24));
        menu->addChild(createItem("-2 octaves", 36));
        menu->addChild(createItem("-1 octave", 48));
        menu->addChild(createSubmenuItem("- semitones", "", [=](Menu* menu){
            menu->addChild(createItem("-11", 49));
            menu->addChild(createItem("-10", 50));
            menu->addChild(createItem("-9", 51));
            menu->addChild(createItem("-8", 52));
            menu->addChild(createItem("-7", 53));
            menu->addChild(createItem("-6", 54));
            menu->addChild(createItem("-5", 55));
            menu->addChild(createItem("-4", 56));
            menu->addChild(createItem("-3", 57));
            menu->addChild(createItem("-2", 58));
            menu->addChild(createItem("-1", 59));
        }));
        menu->addChild(createSubmenuItem("+ semitones", "", [=](Menu* menu){
            menu->addChild(createItem("+1", 61));
            menu->addChild(createItem("+2", 62));
            menu->addChild(createItem("+3", 63));
            menu->addChild(createItem("+4", 64));
            menu->addChild(createItem("+5", 65));
            menu->addChild(createItem("+6", 66));
            menu->addChild(createItem("+7", 67));
            menu->addChild(createItem("+8", 68));
            menu->addChild(createItem("+9", 69));
            menu->addChild(createItem("+10", 70));
            menu->addChild(createItem("+11", 71));
        }));
        menu->addChild(createItem("+1 octave", 72));
        menu->addChild(createItem("+2 octaves", 84));
        menu->addChild(createItem("+3 octaves", 106));
        menu->addChild(createItem("+4 octaves", 118));
    }
};

}