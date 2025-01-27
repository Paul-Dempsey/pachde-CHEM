#include "haken-midi.hpp"
#include "../em/wrap-haken-midi.hpp"

namespace pachde {

void HakenMidiOutput::dispatch(float sampleTime)
{
    float midi_time = midi_timer.process(sampleTime);
    if (midi_time < MIDI_RATE) return;
    midi_timer.reset();

    while (!ring.empty()) {
        auto message = ring.shift();
        if (log) {
            log->logMessage(">>H", message);
        }
        ++message_count;
        em->onMessage(message);
        output.sendMessage(rackFromPacked(message));
    }
}

void HakenMidiOutput::queueMessage(PackedMidiMessage msg)
{
    if (ring.full()) {
        assert(false);
    } else {
        ring.push(msg);
    }
}

void HakenMidiOutput::sendControlChange(uint8_t channel, uint8_t cc, uint8_t value) {
    queueMessage(MakeCC(channel, cc, value));
}

void HakenMidiOutput::sendKeyPressure(uint8_t channel, uint8_t note, uint8_t pressure) {
    queueMessage(MakePolyKeyPressure(channel, note, pressure));
}

void HakenMidiOutput::sendMidiRate(HakenMidiRate rate)
{
    queueMessage(MakeCC(Haken::ch16, Haken::ccTask, static_cast<uint8_t>(rate)));
}

void HakenMidiOutput::sendEditorPresent() {
    if (log) {
        log->logMessage(">>H", "---- EditorPresent");
    }
    queueMessage(MakeCC(Haken::ch16, Haken::ccEditor, tick_tock ? 85 : 42));
    tick_tock = !tick_tock;
}

void HakenMidiOutput::sendRequestConfiguration() {
    if (log) {
        log->logMessage(">>H", "---- RequestConfiguration");
    }
    //doMessage(MakeCC(Haken::ch16, Haken::ccTask, Haken::gridToFlash)); //is this still necessary to get pedals?
    queueMessage(MakeCC(Haken::ch16, Haken::ccTask, Haken::configToMidi));
}

void HakenMidiOutput::sendRequestConText() {
    if (log) {
        log->logMessage(">>H", "---- RequestConText");
    }
    //doMessage(MakeCC(Haken::ch16, Haken::ccTask, Haken::gridToFlash)); //is this still necessary to get pedals?
    queueMessage(MakeCC(Haken::ch16, Haken::ccTask, Haken::contTxtToMidi));
}

void HakenMidiOutput::sendRequestUpdates()
{
    if (log) {
        log->logMessage(">>H", "---- RequestUpdates");
    }
    queueMessage(MakeCC(Haken::ch16, Haken::ccEditor, tick_tock ? 85 : 42));
    tick_tock = !tick_tock;

    // firmware 1009
//    sendControlChange(Haken::ch16, 55, 1); // bit 1 means request config

    // firmware > 1009
    queueMessage(MakeCC(Haken::ch16, Haken::ccStream, Haken::s_Mat_Poke)); //s_Mat_Poke
    sendKeyPressure(Haken::ch16, Haken::idCfgOut, 1);
    queueMessage(MakeCC(Haken::ch16, Haken::ccStream, 127)); //end
}

void HakenMidiOutput::sendRequestUser()
{
    if (log) {
        log->logMessage(">>H", "---- Request User");
    }
    queueMessage(MakeCC(Haken::ch16, Haken::ccTask, Haken::userToMidi));
}

void HakenMidiOutput::sendRequestSystem()
{
    if (log) {
        log->logMessage(">>H", "---- Request System");
    }
    queueMessage(MakeCC(Haken::ch16, Haken::ccTask, Haken::sysToMidi));
}


void HakenMidiOutput::doMessage(PackedMidiMessage message)
{
    queueMessage(message);
}


}
