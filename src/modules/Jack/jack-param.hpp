#pragma once
#include <rack.hpp>
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/misc.hpp"
#include "../../widgets/hamburger.hpp"

using namespace ::rack;
namespace pachde {

struct JackQuantity : ParamQuantity
{
    JackQuantity()
    {
        snapEnabled = true;
        randomizeEnabled = false;
        smoothEnabled = false;
        minValue = 1.f;
        maxValue = 127.f;
        defaultValue = Haken::ccSus;
    }

    std::string getDisplayValueString() override {
        int v = static_cast<int>(getValue());
        switch (v) {
        case Haken::ccMod: return "(modulation)";
        case Haken::ccBreath: return "(breath)";
        case Haken::ccUndef: return "(undefined)";
        case Haken::ccFoot: return "(foot pedal)";
        case Haken::ccVol: return "(volume)";
        case Haken::ccOctShift: return "Oct switch";
        case Haken::ccMonoSwitch: return "Mono switch";
        case Haken::ccFineTune: return "Fine tune";
        case Haken::ccExpres: return "(expression)";
        case Haken::ccI: return "Macro i";
        case Haken::ccII: return "Macro ii";
        case Haken::ccIII: return "Macro iii";
        case Haken::ccIV: return "Macro iv";
        case Haken::ccV: return "Macro v";
        case Haken::ccVI: return "Macro vi";
        case Haken::ccPost: return "Post-level";
        case Haken::ccAudIn: return "Audio in";
        case Haken::ccReci1: return "R1";
        case Haken::ccReci2: return "R2";
        case Haken::ccReci3: return "R3";
        case Haken::ccReci4: return "R4";
        case Haken::ccReciMix: return "RMix";
        case Haken::ccRoundRate: return "Round rate";
        case Haken::ccPre: return "Pre-level";
        case Haken::ccRndIni: return "Round initial";
        case Haken::ccAdvance: return "Advance";
        case Haken::ccReci5: return "R5";
        case Haken::ccReci6: return "R6";
        case Haken::ccSus: return "Sustain";
        case Haken::ccStretch: return "Oct stretch";
        case Haken::ccSos: return "Sos 1";
        case Haken::ccSos2: return "Sos 2";
        default: return format_string("cc %d", v);
        }
    }
};

template <typename TPQ = JackQuantity>
TPQ* configJackParam(Module * module, int pedal_number, int paramId, const char * name) {
    assert(paramId >= 0 && static_cast<size_t>(paramId) < module->params.size() && static_cast<size_t>(paramId) < module->paramQuantities.size());
    if (module->paramQuantities[paramId]) {
        delete module->paramQuantities[paramId];
    }
    TPQ* q = new TPQ;
    q->module = module;
    q->paramId = paramId;
    q->name = name;
    switch (pedal_number) {
    case 1: break;
    case 2: q->defaultValue = Haken::ccSos; break;
    default:
        assert(false);
        break;
    }
    module->paramQuantities[paramId] = q;
    Param* p = &module->params[paramId];
    p->value = q->getDefaultValue();
    return q;
}

struct JackMenu : HamburgerUi<ParamWidget> {

    MenuItem* createItem(const char * name, int value) {
        return createCheckMenuItem(name, "", 
            [=]() { return static_cast<int>(getParamQuantity()->getValue()) == value; }, 
            [=](){ getParamQuantity()->setValue(static_cast<float>(value)); });
    }
    void appendContextMenu(Menu* menu) override
    {
        if (!module) return;
        menu->addChild(new MenuSeparator);

        menu->addChild(createItem("Sustain",          Haken::ccSus));
        menu->addChild(createItem("Sos 1",            Haken::ccSos));
        menu->addChild(createItem("Sos 2",            Haken::ccSos2));
        
        menu->addChild(createSubmenuItem("Macros", "", [=](Menu* menu){
            menu->addChild(createItem("Macro i",          Haken::ccI));
            menu->addChild(createItem("Macro ii",         Haken::ccII));
            menu->addChild(createItem("Macro iii",        Haken::ccIII));
            menu->addChild(createItem("Macro iv",         Haken::ccIV));
            menu->addChild(createItem("Macro v",          Haken::ccV));
            menu->addChild(createItem("Macro vi",         Haken::ccVI));
        }));

        menu->addChild(createSubmenuItem("Fx", "", [=](Menu* menu){
            menu->addChild(createItem("R1",   Haken::ccReci1));
            menu->addChild(createItem("R2",   Haken::ccReci2));
            menu->addChild(createItem("R3",   Haken::ccReci3));
            menu->addChild(createItem("R4",   Haken::ccReci4));
            menu->addChild(createItem("R5",   Haken::ccReci5));
            menu->addChild(createItem("R6",   Haken::ccReci6));
            menu->addChild(createItem("RMix", Haken::ccReciMix));
        }));

        menu->addChild(createSubmenuItem("Levels", "", [=](Menu* menu){
            menu->addChild(createItem("Pre-level",        Haken::ccPre));
            menu->addChild(createItem("Post-level",       Haken::ccPost));
            menu->addChild(createItem("Audio in",         Haken::ccAudIn));
        }));

        menu->addChild(createSubmenuItem("Switches", "", [=](Menu* menu){
            menu->addChild(createItem("Oct switch",       Haken::ccOctShift));
            menu->addChild(createItem("Mono switch",      Haken::ccMonoSwitch));
            menu->addChild(createItem("Advance preset",   Haken::ccAdvance));
        }));
        menu->addChild(createSubmenuItem("Misc", "", [=](Menu* menu){
            menu->addChild(createItem("Round rate",       Haken::ccRoundRate));
            menu->addChild(createItem("Round initial",    Haken::ccRndIni));
            menu->addChild(createItem("Oct stretch",      Haken::ccStretch));
            menu->addChild(createItem("Fine tune",        Haken::ccFineTune));
        }));
        menu->addChild(createSubmenuItem("Extras", "", [=](Menu* menu){
            menu->addChild(createItem("cc1 (modulation)", Haken::ccMod));
            menu->addChild(createItem("cc2 (breath)", Haken::ccBreath));
            menu->addChild(createItem("cc3 (undefined)", Haken::ccUndef));
            menu->addChild(createItem("cc4 (foot pedal)", Haken::ccFoot));
            menu->addChild(createItem("cc7 (volume)", Haken::ccVol));
            menu->addChild(createItem("cc11 (expression)", Haken::ccExpres));
        }));

    }
};

}