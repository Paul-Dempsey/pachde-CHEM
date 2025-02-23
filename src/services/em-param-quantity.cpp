// Copyright (C) Paul Chase Dempsey
#include "em-param-quantity.hpp"
#include "../chem.hpp"

namespace pachde {

SimpleEmParamQuantity::SimpleEmParamQuantity() :
    em_value(0),
    cc(0),          // not a valid parameter cc
    channel(0xff)   // not a valid channel
{
}

void SimpleEmParamQuantity::send_midi()
{
    assert(module && (0 != cc) && (channel < 16));
    auto chem = dynamic_cast<ChemModule*>(module);
    assert(chem);

    auto chem_host = chem->get_host();
    if (!chem_host || !chem_host->host_matrix()->is_ready()) return;
    auto haken = chem_host->host_haken();
    if (!haken) return;
    haken->control_change(channel, cc, em_value);
}

// do not send midi
void SimpleEmParamQuantity::set_em_midi_value(uint8_t value)
{
    em_value = value;
    float param_value = unipolar_7_to_rack(value);
    Base::setValue(param_value);
}

void SimpleEmParamQuantity::setValue(float value)
{
    Base::setValue(value);
    auto p = getParam();
    if (!p) return;
    auto new_value = unipolar_rack_to_unipolar_7(value);
    if (new_value != em_value) {
        em_value = new_value;
        send_midi();
    }
}


}
