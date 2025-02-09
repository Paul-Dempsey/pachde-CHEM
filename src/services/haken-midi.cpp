// Copyright (C) Paul Chase Dempsey
#include "haken-midi.hpp"

namespace pachde {

void HakenMidi::control_change(uint8_t channel, uint8_t cc, uint8_t value) {
    send_message(MakeCC(channel, cc, value));
}

void HakenMidi::key_pressure(uint8_t channel, uint8_t note, uint8_t pressure) {
    send_message(MakePolyKeyPressure(channel, note, pressure));
}

void HakenMidi::select_preset(PresetId id)
{
    if (log) {
        log->logMessage(">>H", "---- Select Preset");
    }
    assert(id.valid());
    //send_message(MakeCC(Haken::ch16, Haken::ccEditor, tick_tock ? 85 : 42)); tick_tock = !tick_tock;
    //send_message(MakeCC(Haken::ch16, Haken::ccTask, Haken::gridToFlash)); //is this still necessary to get pedals?
    send_message(MakeCC(Haken::ch16, Haken::ccBankH, id.bank_hi()));
    send_message(MakeCC(Haken::ch16, Haken::ccBankL, id.bank_lo()));
    send_message(MakeProgramChange(Haken::ch16, id.number()));
}

void HakenMidi::midi_rate(HakenMidiRate rate)
{
    send_message(MakeCC(Haken::ch16, Haken::ccTask, static_cast<uint8_t>(rate)));
}

void HakenMidi::editor_present() {
    if (log) {
        log->logMessage(">>H", "---- EditorPresent");
    }
    send_message(MakeCC(Haken::ch16, Haken::ccEditor, tick_tock ? 85 : 42));
    tick_tock = !tick_tock;
}

void HakenMidi::request_configuration() {
    if (log) {
        log->logMessage(">>H", "---- RequestConfiguration");
    }
    //doMessage(MakeCC(Haken::ch16, Haken::ccTask, Haken::gridToFlash)); //is this still necessary to get pedals?
    send_message(MakeCC(Haken::ch16, Haken::ccTask, Haken::configToMidi));
}

void HakenMidi::request_con_text() {
    if (log) {
        log->logMessage(">>H", "---- RequestConText");
    }
    send_message(MakeCC(Haken::ch16, Haken::ccTask, Haken::contTxtToMidi));
}

void HakenMidi::request_updates()
{
    if (log) {
        log->logMessage(">>H", "---- RequestUpdates");
    }
    // send_message(MakeCC(Haken::ch16, Haken::ccEditor, tick_tock ? 85 : 42));
    // tick_tock = !tick_tock;

    // firmware 1009
//    control_change(Haken::ch16, 55, 1); // bit 1 means request config

    // firmware > 1009
    send_message(MakeCC(Haken::ch16, Haken::ccStream, Haken::s_Mat_Poke)); //s_Mat_Poke
    key_pressure(Haken::ch16, Haken::idCfgOut, 1);
    send_message(MakeCC(Haken::ch16, Haken::ccStream, 127)); //end
}

void HakenMidi::request_user()
{
    if (log) {
        log->logMessage(">>H", "---- Request User");
    }
    send_message(MakeCC(Haken::ch16, Haken::ccTask, Haken::userToMidi));
}

void HakenMidi::request_system()
{
    if (log) {
        log->logMessage(">>H", "---- Request System");
    }
    send_message(MakeCC(Haken::ch16, Haken::ccTask, Haken::sysToMidi));
}

void HakenMidi::remake_mahling()
{
    if (log) {
        log->logMessage(">>H", "---- Remake Mahling data");
    }
    send_message(MakeCC(Haken::ch16, Haken::ccTask, Haken::remakeSRMahl));
}

void HakenMidi::previous_system_preset()
{
    if (log) {
        log->logMessage(">>H", "---- Previous sys preset");
    }
    send_message(MakeCC(Haken::ch16, Haken::ccTask, Haken::decPreset));
}

void HakenMidi::next_system_preset()
{
    if (log) {
        log->logMessage(">>H", "---- Next sys preset");
    }
    send_message(MakeCC(Haken::ch16, Haken::ccTask, Haken::incPreset));
}

void HakenMidi::reset_calibration()
{
    if (log) {
        log->logMessage(">>H", "---- Reset calibration");
    }
    send_message(MakeCC(Haken::ch16, Haken::ccTask, Haken::doResetCalib));
}

void HakenMidi::refine_calibration()
{
    if (log) {
        log->logMessage(">>H", "---- Refine calibration");
    }
    send_message(MakeCC(Haken::ch16, Haken::ccTask, Haken::doRefineCalib));
}

void HakenMidi::factory_calibration()
{
    if (log) {
        log->logMessage(">>H", "---- Factory calibration");
    }
    send_message(MakeCC(Haken::ch16, Haken::ccTask, Haken::doFactCalib));
}

void HakenMidi::surface_alignment()
{
    if (log) {
        log->logMessage(">>H", "---- Surface alignment");
    }
    send_message(MakeCC(Haken::ch16, Haken::ccTask, Haken::surfAlign));
}

}