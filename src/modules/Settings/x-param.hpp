#pragma once
#include <rack.hpp>
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/misc.hpp"
#include "../../widgets/hamburger.hpp"

using namespace ::rack;
namespace pachde {

struct BendParamQuantity : ParamQuantity
{
    BendParamQuantity()
    {
        snapEnabled = true;
        randomizeEnabled = false;
        smoothEnabled = false;
        minValue = 1.f;
        maxValue = 127.f;
        defaultValue = 96.f;
        unit = " semitones";
    }

    std::string getDisplayValueString() override {
        int v = static_cast<int>(getValue());
        if (v > 96) {
            int v1 = std::max(1, v - 97);
            return format_string("96|%d", v1);
        } else {
            return format_string("%d", v);
        }
    }
};

template <typename TBPQ = BendParamQuantity>
TBPQ* configBendParam(Module * module, int paramId, const char * name) {
    assert(paramId >= 0 && static_cast<size_t>(paramId) < module->params.size() && static_cast<size_t>(paramId) < module->paramQuantities.size());
    if (module->paramQuantities[paramId]) {
        delete module->paramQuantities[paramId];
    }
    TBPQ*q = new TBPQ;
    q->module = module;
    q->paramId = paramId;
    q->name = name;
    module->paramQuantities[paramId] = q;
    Param* p = &module->params[paramId];
    p->value = q->getDefaultValue();

    return q;    
}

struct BendMenu : HamburgerUi<ParamWidget> {

    MenuItem* createBendItem(const char * name, int value) {
        return createCheckMenuItem(name, "", 
            [=]() { return static_cast<int>(getParamQuantity()->getValue()) == value; }, 
            [=](){ getParamQuantity()->setValue(static_cast<float>(value)); });
    }
    void appendContextMenu(Menu* menu) override
    {
        if (!module) return;
        menu->addChild(new MenuSeparator);
        menu->addChild(createBendItem("1", 1));
        menu->addChild(createBendItem("2", 2));
        menu->addChild(createBendItem("3", 3));
        menu->addChild(createBendItem("4", 4));
        menu->addChild(createBendItem("5", 5));
        menu->addChild(createBendItem("6", 6));
        menu->addChild(createBendItem("7", 7));
        menu->addChild(createBendItem("12", 12));
        menu->addChild(createBendItem("24", 24));
        menu->addChild(createBendItem("36", 36));
        menu->addChild(createBendItem("48", 48));
        menu->addChild(createBendItem("96", 96));
        menu->addChild(createMenuLabel("— MPE+ : Channel 1 —"));
        menu->addChild(createBendItem("96:2", 97));
        menu->addChild(createBendItem("96:5", 100));
        menu->addChild(createBendItem("96:7", 102));
        menu->addChild(createBendItem("96:12", 107));
    }
};

}