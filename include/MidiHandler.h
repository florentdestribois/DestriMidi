/*
 * MIDI Handler Module
 * Handles all MIDI communication over BLE
 */

#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include <Arduino.h>
#include <BLECharacteristic.h>

class MidiHandler {
private:
    BLECharacteristic* pCharacteristic;
    uint8_t midiPacket[5];
    
    void sendMidiMessage(uint8_t* data, size_t length);
    
public:
    MidiHandler();
    void begin(BLECharacteristic* characteristic);
    void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
    void sendControlChange(uint8_t channel, uint8_t control, uint8_t value);
    void sendProgramChange(uint8_t channel, uint8_t program);
    void sendPitchBend(uint8_t channel, int16_t bend);
    void sendSystemExclusive(uint8_t* data, size_t length);
};

#endif