#include "em-midi-port.hpp"
#include "rack-em-convert.hpp"

namespace pachde {

void EmControlPort::send(IChemHost* chem, ChemId tag, bool force)
{
    if (!chem) return;
    if (no_send()) return;
    if (!pending() && !force) return;
    if (!chem->host_matrix()->is_ready()) return;
    assert (em_value <= Haken::max14);

    auto haken = chem->host_haken();
    assert(haken);

    un_pend();
    if (is_cc()) {
        if (!low_resolution) {
            uint8_t lo = em_value & 0x7f;
            if (lo) {
                haken->control_change(tag, channel, Haken::ccFracPed, lo);
            }
        }
        haken->control_change(tag, channel, cc_id, em_value >> 7);
    } else {
        assert(is_stream_poke());
        haken->begin_stream(tag, stream);
        haken->stream_data(tag, cc_id, em_value >> 7);
        //haken->end_stream(tag); // supposedly optional
    }
}

void EmControlPort::pull_param_cv(Module* module, int param_id, int input_id)
{
    if (!module) return;
    cv = module->getInput(input_id).getVoltage();
    set_param_and_em(module->getParam(param_id).getValue());
}

void EmControlPort::set_mod_amount(float amount)
{
    assert(in_range(amount, -100.f, 100.f));
    mod_amount = amount;
}

void EmControlPort::set_param_and_em(float value)
{
    param_value = value;
    em_value = unipolar_rack_to_unipolar_14(modulate());
}

void EmControlPort::set_em_and_param(uint16_t u14)
{
    em_value = last_em_value = u14;
    param_value = unipolar_14_to_rack(u14);
}

float EmControlPort::modulate()
{
    mod_value = modulated_value(param_value, cv, mod_amount);
    return mod_value;
}


// ----  Modulation -----------------------------

bool Modulation::sync_params_ready(const rack::engine::Module::ProcessArgs &args, float rate)
{
    if (midi_timer.process(args.sampleTime) > rate) {
        midi_timer.reset();
        return true;
    }
    return false;
}

Modulation::Modulation(ChemModule *module, ChemId client_tag) : 
    module(module),
    mod_target(-1),
    last_mod_target(-1),
    mod_param(-1),
    count(-1),
    first_param(-1),
    first_input(-1),
    first_light(-1),
    have_stream(false),
    client_tag(client_tag)
{
    midi_timer.time = (random::uniform() * MIDI_RATE); // jitter
}

void Modulation::configure(int mod_param_id, int first_param_id, int first_input_id, int first_light_id, int data_length, const EmccPortConfig *data)
{
    mod_param = mod_param_id;
    first_param = first_param_id;
    first_input = first_input_id;
    first_light = first_light_id;
    count = data_length;
    ports.reserve(data_length);
    for (int i = 0; i < data_length; ++i) {
        EmControlPort port;
        port.config(*data);
        if (port.is_stream_poke()) { have_stream = true; }
        ports.push_back(port);
    }
}

void Modulation::set_em_and_param(int index, uint16_t em_value, bool with_module)
{
    EmControlPort& port = ports[index];
    port.set_em_and_param(em_value);
    if (with_module) {
        module->getParam(first_param + index).setValue(port.param());
    }
}

void Modulation::set_em_and_param_low(int index, uint8_t em_value, bool with_module)
{
    EmControlPort& port = ports[index];
    port.set_em_and_param_low(em_value);
    if (with_module) {
        module->getParam(first_param + index).setValue(port.param());
    }
}

void Modulation::onPortChange(const ::rack::engine::Module::PortChangeEvent &e)
{
    if (e.type == Port::OUTPUT) return;
    if (e.connecting) {
        mod_target = e.portId;
        auto pq = module->getParamQuantity(mod_param);
        if (pq) {
            pq->setImmediateValue(ports[mod_target].amount());
        }
    } else {
        ports[e.portId].set_mod_amount(0.f);
        for (int i = first_input; i < first_input + count; ++i) {
            if (module->getInput(i).isConnected()) {
                mod_target = i;
                auto pq = module->getParamQuantity(mod_param);
                if (pq) {
                    pq->setImmediateValue(ports[i].amount());
                }
                return;
            }
        }
        mod_target = -1;
        auto pq = module->getParamQuantity(mod_param);
        if (pq) {
            pq->setImmediateValue(0.f);
        }
    }
}

void Modulation::set_modulation_target(int target)
{
    if (!module->getInput(target).isConnected()) return;

    mod_target = target;
    auto pq = module->getParamQuantity(mod_param);
    if (pq) {
        pq->setImmediateValue(ports[mod_target].amount());
    }        
}

void Modulation::sync_send()
{
//   if (have_stream) {
// todo: consolidate multiple pending items in the same stream

//    } else {
        auto pit = ports.begin();
        for (int i = 0; i < count; ++i) {
            pit->pull_param_cv(module, first_param + i, first_input + i );
            if (pit->pending()) {
                pit->send(module->chem_host, client_tag);
            }
            pit++;
        }
//    }
}

void Modulation::pull_mod_amount()
{
    if (mod_target >= 0) {
        auto pq = module->getParamQuantity(mod_param);
        if (pq) {
            ports[mod_target].set_mod_amount(pq->getValue());
        }
    }
}

void Modulation::update_lights() {
    if (last_mod_target != mod_target) {
        for (int i = first_light; i < first_light+count; ++i) {
            module->getLight(i).setSmoothBrightness((i - first_light) == mod_target ? .6f : 0.f, 90);
        }
    }
    last_mod_target = mod_target;
}

void Modulation::mod_to_json(json_t *root)
{
    auto port = ports.begin();
    auto jaru = json_array();
    for (int i = 0; i < count; ++i) {
        json_array_append_new(jaru, json_real(port->amount()));
        port++;
    }
    json_object_set_new(root, "mod-amount", jaru);
}

void Modulation::mod_from_json(json_t *root)
{
    auto jar = json_object_get(root, "mod-amount");
    if (jar) {
        json_t* jp;
        size_t index;
        auto port = ports.begin();
        json_array_foreach(jar, index, jp) {
            if (index >= static_cast<size_t>(count)) break;
            port->set_mod_amount(json_real_value(jp));
            port++;
        }
    }
}

}