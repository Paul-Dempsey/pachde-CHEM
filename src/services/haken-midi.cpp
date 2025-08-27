// Copyright (C) Paul Chase Dempsey
#include "haken-midi.hpp"

namespace pachde {

void HakenMidi::control_change(ChemId tag, uint8_t channel, uint8_t cc, uint8_t value) {
    send_message(Tag(MakeCC(channel, cc, value), tag));
}

void HakenMidi::key_pressure(ChemId tag, uint8_t channel, uint8_t note, uint8_t pressure) {
    send_message(Tag(MakePolyKeyPressure(channel, note, pressure), tag));
}

void HakenMidi::begin_stream(ChemId tag, uint8_t stream)
{
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccStream, stream), tag));
}
void HakenMidi::stream_data(ChemId tag,uint8_t d1, uint8_t d2)
{
    send_message(Tag(MakePolyKeyPressure(Haken::ch16, d1, d2), tag));
}
void HakenMidi::end_stream(ChemId tag)
{
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccStream, 127), tag));
}

void HakenMidi::send_stream(ChemId tag, uint8_t stream, std::vector<PackedMidiMessage> &data)
{
    if (!data.empty()) return;

    begin_stream(tag, stream);
    for (auto msg: data) {
        send_message(msg);
    }
    if (!in_range(stream, U8(Haken::s_Mat_Poke), U8(Haken::s_Conv_Poke))) {
        end_stream(tag);
    }
}

void HakenMidi::disable_recirculator(ChemId tag, bool disable)
{
    begin_stream(tag, Haken::s_Mat_Poke);
    key_pressure(tag, Haken::ch16, Haken::idNoRecirc, disable);
    //end_stream(tag);
}

void HakenMidi::recirculator_type(ChemId tag, uint8_t kind)
{
    begin_stream(tag, Haken::s_Mat_Poke);
    key_pressure(tag, Haken::ch16, Haken::idReciType, kind);
    //end_stream(tag);
}

void HakenMidi::compressor_option(ChemId tag, bool tanh)
{
    begin_stream(tag, Haken::s_Mat_Poke);
    key_pressure(tag, Haken::ch16, Haken::idCompOpt, tanh);
    //end_stream(tag);
}

void HakenMidi::keep_pedals(ChemId tag, bool keep)
{
    begin_stream(tag, Haken::s_Mat_Poke);
    key_pressure(tag, Haken::ch16, Haken::idPresPed, keep);
    //end_stream(tag);
}

void HakenMidi::keep_surface(ChemId tag, bool keep)
{
    begin_stream(tag, Haken::s_Mat_Poke);
    key_pressure(tag, Haken::ch16, Haken::idPresSurf, keep);
    //end_stream(tag);
}

void HakenMidi::keep_midi(ChemId tag, bool keep)
{
    begin_stream(tag, Haken::s_Mat_Poke);
    key_pressure(tag, Haken::ch16, Haken::idPresEnc, keep);
    //end_stream(tag);
}

void HakenMidi::select_preset(ChemId tag, eaganmatrix::PresetId id)
{
    if (log) {
        log->log_message(">>H", "---- Select Preset");
    }
    assert(id.valid());
    // if (matrix) {
    //     matrix->set_preset_id(id);
    // }
    if (osmose_target) {
        send_message(Tag(MakeCC(Haken::ch1, Haken::ccBankH, id.bank_hi()), tag));
        send_message(Tag(MakeProgramChange(Haken::ch1, id.number()), tag));
    } else {
        send_message(Tag(MakeCC(Haken::ch16, Haken::ccBankH, id.bank_hi()), tag));
        send_message(Tag(MakeCC(Haken::ch16, Haken::ccBankL, id.bank_lo()), tag));
        send_message(Tag(MakeProgramChange(Haken::ch16, id.number()), tag));
    }
}

void HakenMidi::extended_macro(ChemId tag, uint8_t macro, uint16_t value)
{
    assert(in_range(macro, U8(7), U8(90)));
    send_message(Tag(MakeCC(Haken::ch1, macro_lsb_cc(macro), value & 0x7f), tag));
    send_message(Tag(MakeCC(Haken::ch1, macro_msb_cc(macro), value >> 7), tag));
}

void HakenMidi::midi_rate(ChemId tag, HakenMidiRate rate)
{
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, static_cast<uint8_t>(rate)), tag));
}

void HakenMidi::editor_present(ChemId tag) {
    if (log) {
        log->log_message(">>H", "---- EditorPresent");
    }
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccEditor, tick_tock ? 85 : 42), tag));
    tick_tock = !tick_tock;
}

void HakenMidi::request_configuration(ChemId tag) {
    if (log) {
        log->log_message(">>H", "---- RequestConfiguration");
    }
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, Haken::configToMidi), tag));
}

void HakenMidi::request_archive_0(ChemId tag)
{
    if (log) {
        log->log_message(">>H", "---- Request Archive 0");
    }
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, Haken::createLed), tag));
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccDInfo, Haken::cfCreateArch0), tag));
}

void HakenMidi::request_con_text(ChemId tag) {
    if (log) {
        log->log_message(">>H", "---- RequestConText");
    }
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, Haken::contTxtToMidi), tag));
}

void HakenMidi::request_updates(ChemId tag)
{
    if (log) {
        log->log_message(">>H", "---- RequestUpdates");
    }
    // send_message(MakeCC(Haken::ch16, Haken::ccEditor, tick_tock ? 85 : 42));
    // tick_tock = !tick_tock;
    // firmware 1009
    // control_change(Haken::ch16, 55, 1); // bit 1 means request config

    // firmware > 1009
    // begin_stream(tag, Haken::s_Mat_Poke);
    // key_pressure(tag, Haken::ch16, Haken::idCfgOut, 1);
    // end_stream(tag);
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, Haken::loadsToMidi), tag));
}

void HakenMidi::request_user(ChemId tag)
{
    if (log) {
        log->log_message(">>H", "---- Request User");
    }
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, Haken::userToMidi), tag));
}

void HakenMidi::request_system(ChemId tag)
{
    if (log) {
        log->log_message(">>H", "---- Request System");
    }
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, Haken::sysToMidi), tag));
}

void HakenMidi::remake_mahling(ChemId tag)
{
    if (log) {
        log->log_message(">>H", "---- Remake Mahling data");
    }
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, Haken::remakeSRMahl), tag));
}

void HakenMidi::previous_system_preset(ChemId tag)
{
    if (log) {
        log->log_message(">>H", "---- Previous sys preset");
    }
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, Haken::decPreset), tag));
}

void HakenMidi::next_system_preset(ChemId tag)
{
    if (log) {
        log->log_message(">>H", "---- Next sys preset");
    }
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, Haken::incPreset),tag));
}

void HakenMidi::reset_calibration(ChemId tag)
{
    if (log) {
        log->log_message(">>H", "---- Reset calibration");
    }
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, Haken::doResetCalib), tag));
}

void HakenMidi::refine_calibration(ChemId tag)
{
    if (log) {
        log->log_message(">>H", "---- Refine calibration");
    }
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, Haken::doRefineCalib), tag));
}

void HakenMidi::factory_calibration(ChemId tag)
{
    if (log) {
        log->log_message(">>H", "---- Factory calibration");
    }
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, Haken::doFactCalib), tag));
}

void HakenMidi::surface_alignment(ChemId tag)
{
    if (log) {
        log->log_message(">>H", "---- Surface alignment");
    }
    send_message(Tag(MakeCC(Haken::ch16, Haken::ccTask, Haken::surfAlign), tag));
}

}