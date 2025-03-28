#pragma once
#include <rack.hpp>
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/misc.hpp"
#include "../../widgets/hamburger.hpp"

using namespace ::rack;
namespace pachde {

struct YParamQuantity : ParamQuantity
{
    YParamQuantity()
    {
        snapEnabled = true;
        randomizeEnabled = false;
        smoothEnabled = false;
        minValue = 0.f;
        maxValue = 127.f;
        defaultValue = Haken::defaultYcc;
    }

    std::string getDisplayValueString() override {
        int v = static_cast<int>(getValue());
        switch (v) {
        case 0: return "No Y";
        case Haken::ccMod:      return "cc 1 (mod wheel)";
        case Haken::ccBreath:   return "cc 2 (breath)";
        case Haken::ccUndef:    return "cc 3";
        case Haken::ccFoot:     return "cc 4 (foot)";
        case Haken::ccVol:      return "cc 7 (vol)";
        case Haken::ccExpres:   return "cc 11 (expression)";
        case Haken::defaultYcc: return "cc 74 (MPE/MPE+)";
        case Haken::xmitNoShelf: return "cc 74 no Shelf";
        default: return format_string("cc %d (?)", v);
        }
    }
};

template <typename TPQ = YParamQuantity>
TPQ* configYParam(Module * module, int paramId, const char * name) {
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

struct YMenu : HamburgerUi<ParamWidget> {

    MenuItem* createItem(const char * name, int value) {
        return createCheckMenuItem(name, "", 
            [=]() { return static_cast<int>(getParamQuantity()->getValue()) == value; }, 
            [=](){ getParamQuantity()->setValue(static_cast<float>(value)); });
    }
    void appendContextMenu(Menu* menu) override
    {
        if (!module) return;
        menu->addChild(new MenuSeparator);
        menu->addChild(createItem("Off (no Y)", 0));
        menu->addChild(new MenuSeparator);
        menu->addChild(createItem("cc 1 (mod wheel)", 1));
        menu->addChild(createItem("cc 2 (breath)", 2));
        menu->addChild(createItem("cc 3", 3));
        menu->addChild(createItem("cc 4 (foot)", 4));
        menu->addChild(createItem("cc 7 (vol)", 7));
        menu->addChild(createItem("cc 11 (expression)", 11));
        menu->addChild(createItem("cc 74 (MPE/MPE+)", Haken::defaultYcc));
        menu->addChild(new MenuSeparator);
        menu->addChild(createItem("cc74 no Shelf", Haken::xmitNoShelf));
    }
};

}