// Copyright (C) Paul Chase Dempsey
#include "em-param-quantity.hpp"
#include "../chem.hpp"

namespace pachde {

U7EmParamQuantity::U7EmParamQuantity() :
    em_value(0),
    cc(0),          // not a valid parameter cc
    channel(0xff)   // not a valid channel
{
}

void U7EmParamQuantity::send_midi()
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
void U7EmParamQuantity::set_em_midi_value(uint8_t value)
{
    em_value = value;
    float param_value = unipolar_7_to_rack(value);
    Base::setValue(param_value);
}

void U7EmParamQuantity::setValue(float value)
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

// --  U14ccEmParamQuantity  --------------------

U14ccEmParamQuantity::U14ccEmParamQuantity() :
    em_value(0),
    cc(0),          // not a valid parameter cc
    channel(0xff)   // not a valid channel
{
}

void U14ccEmParamQuantity::send_midi()
{
    assert(module && (0 != cc) && (channel < 16));
    auto chem = dynamic_cast<ChemModule*>(module);
    assert(chem);

    auto chem_host = chem->get_host();
    if (!chem_host || !chem_host->host_matrix()->is_ready()) return;
    auto haken = chem_host->host_haken();
    if (!haken) return;
    uint8_t lo = em_value & 0x7f;
    if (lo) {
        haken->control_change(channel, Haken::ccFracPed, lo);
    }
    haken->control_change(channel, cc, em_value >> 7);
}

// Set value from em (does not send midi) (TODO:rename not "midi")
void U14ccEmParamQuantity::set_em_midi_value(uint16_t value)
{
    em_value = value;
    float param_value = unipolar_14_to_rack(value);
    Base::setValue(param_value);
}

// If changing greater than em granularity, send midi
void U14ccEmParamQuantity::setValue(float value)
{
    Base::setValue(value);
    auto p = getParam();
    if (!p) return;
    auto new_value = unipolar_rack_to_unipolar_14(value);
    if (new_value != em_value) {
        em_value = new_value;
        send_midi();
    }

}
}
