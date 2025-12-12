// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/misc.hpp"
#include "widgets/hamburger.hpp"
#include "widgets/menu-widgets.hpp"
using namespace ::widgetry;

namespace pachde {

enum Tuning : uint8_t {
    EqualTuning = 0,
    OneTone = 1,
    FiftyTone = 50,
    JustC = 60,
    JustCs,
    JustD,
    JustEb,
    JustF,
    JustFs,
    JustG,
    JustAb,
    JustA,
    JustBb,
    JustB,
    UserTuning1 = 80,
    UserTuning2,
    UserTuning3,
    UserTuning4,
    UserTuning5,
    UserTuning6,
    UserTuning7,
    UserTuning8,
    UserTuningLast = UserTuning8
};

inline Tuning nTone(int nth) {
    assert(pachde::in_range(nth, 1, 50));
    return static_cast<Tuning>((static_cast<int>(OneTone) + nth - 1));
}


// Packed tuning is for when you need a linear sequence of values,
// such as putting tuning on a knob.
enum PackedTuning: uint8_t {
    ptEqual,
    ptOneTone,
    ptFiftyTone = 50,
    ptJustC,
    ptJustCs,
    ptJustD,
    ptJustEb,
    ptJustF,
    ptJustFs,
    ptJustG,
    ptJustAb,
    ptJustA,
    ptJustBb,
    ptJustB,
    ptUser1,
    ptUser2,
    ptUser3,
    ptUser4,
    ptUser5,
    ptUser6,
    ptUser7,
    ptUser8,
    ptUserLast = ptUser8
};

uint8_t packTuning(Tuning tuning);
Tuning unpackTuning(uint8_t packed);

std::string describeTuning(Tuning grid);

struct TuningParamQuantity : ::rack::engine::ParamQuantity
{
    void setTuning(Tuning tuning) {
        setValue(packTuning(tuning));
    }

    Tuning getTuning() {
        uint8_t packed = getValue();
        return unpackTuning(packed);
    }
	std::string getDisplayValueString() override {
        return describeTuning(getTuning());
    }
};

template <typename TPQ = TuningParamQuantity>
TPQ* configTuningParam(Module* module, int paramId)
{
    assert(paramId >= 0 && static_cast<size_t>(paramId) < module->params.size() && static_cast<size_t>(paramId) < module->paramQuantities.size());
    if (module->paramQuantities[paramId]) {
        delete module->paramQuantities[paramId];
    }

    TPQ* q = new TPQ;
    q->module = module;
    q->paramId = paramId;
    q->minValue = 0.f;
    q->maxValue = static_cast<float>(PackedTuning::ptUserLast);
    q->name = "Tuning";

    module->paramQuantities[paramId] = q;

    Param* p = &module->params[paramId];
    p->value = q->getDefaultValue();

    return q;
}

struct TuningMenu : HamburgerUi<ParamWidget>
{
    Tuning getParamValue() {
        uint8_t packed = getParamQuantity()->getValue();
        return unpackTuning(packed);
    }
    void setParamValue(Tuning tuning) {
        getParamQuantity()->setValue(packTuning(tuning));
    }

    OptionMenuEntry * createTuningMenuItem(Tuning tuningItem);
    void appendContextMenu(Menu* menu) override;
};

}
