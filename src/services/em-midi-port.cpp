#include "em-midi-port.hpp"
#include "rack-em-convert.hpp"

namespace pachde {

void EmControlPort::send(IChemHost* chem, ChemId tag, bool force)
{
    if (!chem) return;
    if (!pending() && !force) return;
    if (!chem->host_matrix()->is_ready()) return;
    assert (em_value <= Haken::max14);

    auto haken = chem->host_haken();
    assert(haken);

    un_pend();

    switch (kind) {
    case PortKind::NoSend:
        return;

    case PortKind::CC:
        if (!low_resolution) {
            uint8_t lo = em_value & 0x7f;
            if (lo) {
                haken->control_change(tag, channel_stream, Haken::ccFracIM48, lo);
            }
        }
        haken->control_change(tag, channel_stream, cc_id, em_low());
        break;

    case PortKind::Stream:
        haken->begin_stream(tag, channel_stream);
        haken->stream_data(tag, cc_id, em_low());
        //haken->end_stream(tag); // supposedly optional
        break;

    default:
        assert(false);
        break;
    }
}

void EmControlPort::pull_param_cv(Module* module)
{
    if (!module) return;
    cv = (input_id < 0) ? 0.f : module->getInput(input_id).getVoltage();
    set_param_and_em(module->getParam(param_id).getValue());
}

void EmControlPort::set_mod_amount(float amount)
{
    assert(in_range(amount, -100.f, 100.f));
    mod_amount = amount;
    mod_value = modulated_value(param_value, cv, mod_amount); 
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
    have_stream(false),
    client_tag(client_tag)
{
    assert(module);
    midi_timer.time = (random::uniform() * MIDI_RATE); // jitter
}

void Modulation::configure(int mod_param_id, int data_length, const EmccPortConfig *data)
{
    mod_param = mod_param_id;
    count = data_length;
    ports.reserve(data_length);
    for (int i = 0; i < data_length; ++i) {
        EmControlPort port(i, data++);
        if (port.is_stream_poke()) { have_stream = true; }
        ports.push_back(port);
    }
}

void Modulation::set_em_and_param(int index, uint16_t em_value, bool with_module)
{
    EmControlPort& port = ports[index];
    port.set_em_and_param(em_value);
    if (with_module) {
        module->getParam(port.param_id).setValue(port.parameter());
    }
}

void Modulation::set_em_and_param_low(int index, uint8_t em_value, bool with_module)
{
    EmControlPort& port = ports[index];
    port.set_em_and_param_low(em_value);
    if (with_module) {
        module->getParam(port.param_id).setValue(port.parameter());
    }
}

void Modulation::onPortChange(const ::rack::engine::Module::PortChangeEvent &e)
{
    if (e.type == Port::OUTPUT) return;
    if (e.connecting) {
        auto port = get_port_for_input(e.portId);
        mod_target = port->index;
        auto pq = module->getParamQuantity(mod_param);
        if (pq) {
            pq->setImmediateValue(ports[mod_target].modulation());
        }
    } else {
        ports[e.portId].set_mod_amount(0.f);

        int i = 0;
        for (auto port: ports) {
            if (port.input_id >= 0 && module->getInput(port.input_id).isConnected()) {
                mod_target = i;
                auto pq = module->getParamQuantity(mod_param);
                if (pq) {
                    pq->setImmediateValue(port.modulation());
                }
                return;
            }
            ++i;
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
    EmControlPort& port = ports[target];
    if (port.input_id < 0 || !module->getInput(port.input_id).isConnected()) return;

    mod_target = target;
    auto pq = module->getParamQuantity(mod_param);
    if (pq) {
        pq->setImmediateValue(port.modulation());
    }        
}

void Modulation::sync_send()
{
    if (!module->chem_host) return;
    if (have_stream) {
        uint8_t stream{INVALID_STREAM};
        std::vector<PackedMidiMessage> stream_data;

        // pass 1: send all plain CCs and capture stream
        for (auto pit = ports.begin(); pit != ports.end(); pit++) {
            switch (pit->kind) {
            case PortKind::NoSend:
                continue;

            case PortKind::CC:
                pit->pull_param_cv(module);
                if (pit->pending() && pit->is_cc()) {
                    pit->send(module->chem_host, client_tag);
                }
                break;

            case PortKind::Stream:
                if (stream == INVALID_STREAM) {
                    stream = pit->channel_stream;
                    stream_data.push_back(Tag(MakeCC(Haken::ch16, Haken::ccStream, stream), client_tag));
                } else {
                    assert(stream == pit->channel_stream); // multiple streams in one modulation not implemented
                }
                pit->pull_param_cv(module);
                if (pit->pending()) {
                    stream_data.push_back(Tag(MakePolyKeyPressure(Haken::ch16, pit->cc_id, pit->em_low()), client_tag));
                    pit->un_pend();
                }
                break;

            default:
                assert(false);
            }
        }
        // pass 2: send pending data stream
        assert (stream != INVALID_STREAM);
        if (stream_data.size() > 1) { // send only if data (always have the stream start)
            auto haken{module->chem_host->host_haken()};
            assert(haken);
            for (auto msg : stream_data) {
                haken->send_message(msg);
            }
        }
    } else {
        for (auto pit = ports.begin();pit != ports.end(); pit++) {
            pit->pull_param_cv(module);
            if (pit->pending()) {
                pit->send(module->chem_host, client_tag);
            }
        }
    }
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

void Modulation::update_mod_lights()
{
    if (last_mod_target != mod_target) {
        int i = 0;
        for (auto port: ports) {
            if (port.light_id >= 0) {
                module->getLight(port.light_id).setSmoothBrightness((i == mod_target) ? 1.f : 0.f, 90);
            }
            ++i;
        }
        last_mod_target = mod_target;
    }
}

void Modulation::zero_modulation()
{
    if (mod_param >= 0) {
        module->getParam(mod_param).setValue(0.f);
    }
    for (auto pit = ports.begin(); pit != ports.end(); pit++) {
        if (pit->has_input()) {
            pit->set_mod_amount(0.f);
        }
    }
}

void Modulation::mod_to_json(json_t *root)
{
    auto port = ports.begin();
    auto jaru = json_array();
    for (int i = 0; i < count; ++i) {
        json_array_append_new(jaru, json_real(port->modulation()));
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