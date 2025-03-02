// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../chem-core.hpp"

namespace pachde {

// --  U7EmParamQuantity  --------------------
// Simple 7-bit quantity

struct U7EmParamQuantity : ::rack::engine::ParamQuantity
{
    using Base = ::rack::engine::ParamQuantity;

    uint8_t em_value;
    uint8_t cc;
    uint8_t channel;

    U7EmParamQuantity();

    void send_midi();

    // Set value from em (does not send midi)
    void set_em_value(uint8_t value);

    // If changing greater than em granularity, send midi
    void setValue(float value) override;
};

inline U7EmParamQuantity* get_u7_em_param_quantity(rack::engine::Module* module, int param_id)
{
    if (!module) return nullptr;
    return dynamic_cast<U7EmParamQuantity*>(module->getParamQuantity(param_id));
}

template <typename TSEPQ = U7EmParamQuantity>
TSEPQ* configU7EmParam(uint8_t channel, uint8_t cc, ::rack::engine::Module* module, int paramId, float minValue, float maxValue, float defaultValue, std::string name = "", std::string unit = "v")
{
    assert(paramId >= 0 && static_cast<size_t>(paramId) < module->params.size() && static_cast<size_t>(paramId) < module->paramQuantities.size());
    if (module->paramQuantities[paramId]) {
        delete module->paramQuantities[paramId];
        module->paramQuantities[paramId] = nullptr;
    }
    auto q = new TSEPQ();
    q->module = module;
    q->paramId = paramId;
    q->minValue = minValue;
    q->maxValue = maxValue;
    q->defaultValue = defaultValue;
    q->name = name;
    q->unit = unit;
    q->displayPrecision = 4;
    assert(in_range((int)channel, 0, 15));
    q->channel = channel;
    assert(in_range((int)cc, 1, 127));
    q->cc = cc;
    module->paramQuantities[paramId] = q;
    module->getParam(paramId).value = q->getDefaultValue();
    return q;
}

// --  U14ccEmParamQuantity  --------------------
// Unipolar 14-bit cc

struct U14ccEmParamQuantity : ::rack::engine::ParamQuantity
{
    using Base = ::rack::engine::ParamQuantity;

    uint16_t em_value;
    uint8_t cc;
    uint8_t channel;

    U14ccEmParamQuantity();

    void send_midi();

    // Set value from em (does not send midi)
    void set_em_value(uint16_t value);

    // If changing greater than em granularity, send midi
    void setValue(float value) override;
};

inline U14ccEmParamQuantity* get_u14cc_em_param_quantity(rack::engine::Module* module, int param_id)
{
    if (!module) return nullptr;
    return dynamic_cast<U14ccEmParamQuantity*>(module->getParamQuantity(param_id));
}

template <typename TSEPQ = U14ccEmParamQuantity>
TSEPQ* configU14ccEmParam(uint8_t channel, uint8_t cc, ::rack::engine::Module* module, int paramId, float minValue, float maxValue, float defaultValue, std::string name = "", std::string unit = "v")
{
    assert(paramId >= 0 && static_cast<size_t>(paramId) < module->params.size() && static_cast<size_t>(paramId) < module->paramQuantities.size());
    if (module->paramQuantities[paramId]) {
        delete module->paramQuantities[paramId];
        module->paramQuantities[paramId] = nullptr;
    }
    auto q = new TSEPQ();
    q->module = module;
    q->paramId = paramId;
    q->minValue = minValue;
    q->maxValue = maxValue;
    q->defaultValue = defaultValue;
    q->name = name;
    q->unit = unit;
    q->displayPrecision = 4;
    assert(in_range((int)channel, 0, 15));
    q->channel = channel;
    assert(in_range((int)cc, 1, 127));
    q->cc = cc;
    module->paramQuantities[paramId] = q;
    module->getParam(paramId).value = q->getDefaultValue();
    return q;
}


}
