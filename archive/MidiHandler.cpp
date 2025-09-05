/*
 * MIDI Handler Module Implementation
 */

#include "MidiHandler.h"

MidiHandler::MidiHandler() {
    pCharacteristic = nullptr;
}

void MidiHandler::begin(BLECharacteristic* characteristic) {
    pCharacteristic = characteristic;
    Serial.println("MIDI Handler initialized");
}

void MidiHandler::sendMidiMessage(uint8_t* data, size_t length) {
    if (pCharacteristic && length <= 5) {
        pCharacteristic->setValue(data, length);
        pCharacteristic->notify();
    }
}

void MidiHandler::sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (channel < 1 || channel > 16) return;
    
    midiPacket[0] = 0x80;  // Header
    midiPacket[1] = 0x80;  // Timestamp (not used)
    midiPacket[2] = 0x90 | (channel - 1);  // Note On + channel
    midiPacket[3] = note & 0x7F;
    midiPacket[4] = velocity & 0x7F;
    
    sendMidiMessage(midiPacket, 5);
    
    Serial.printf("MIDI Note On - Ch:%d Note:%d Vel:%d\n", channel, note, velocity);
}

void MidiHandler::sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (channel < 1 || channel > 16) return;
    
    midiPacket[0] = 0x80;  // Header
    midiPacket[1] = 0x80;  // Timestamp
    midiPacket[2] = 0x80 | (channel - 1);  // Note Off + channel
    midiPacket[3] = note & 0x7F;
    midiPacket[4] = velocity & 0x7F;
    
    sendMidiMessage(midiPacket, 5);
    
    Serial.printf("MIDI Note Off - Ch:%d Note:%d Vel:%d\n", channel, note, velocity);
}

void MidiHandler::sendControlChange(uint8_t channel, uint8_t control, uint8_t value) {
    if (channel < 1 || channel > 16) return;
    
    midiPacket[0] = 0x80;  // Header
    midiPacket[1] = 0x80;  // Timestamp
    midiPacket[2] = 0xB0 | (channel - 1);  // Control Change + channel
    midiPacket[3] = control & 0x7F;
    midiPacket[4] = value & 0x7F;
    
    sendMidiMessage(midiPacket, 5);
    
    Serial.printf("MIDI CC - Ch:%d CC#%d Val:%d\n", channel, control, value);
}

void MidiHandler::sendProgramChange(uint8_t channel, uint8_t program) {
    if (channel < 1 || channel > 16) return;
    
    midiPacket[0] = 0x80;  // Header
    midiPacket[1] = 0x80;  // Timestamp
    midiPacket[2] = 0xC0 | (channel - 1);  // Program Change + channel
    midiPacket[3] = program & 0x7F;
    midiPacket[4] = 0;  // Not used for program change
    
    sendMidiMessage(midiPacket, 4);
    
    Serial.printf("MIDI Program Change - Ch:%d Prog:%d\n", channel, program);
}

void MidiHandler::sendPitchBend(uint8_t channel, int16_t bend) {
    if (channel < 1 || channel > 16) return;
    
    // Convert bend value to 14-bit MIDI format
    uint16_t bendValue = bend + 8192;  // Center at 8192
    uint8_t lsb = bendValue & 0x7F;
    uint8_t msb = (bendValue >> 7) & 0x7F;
    
    midiPacket[0] = 0x80;  // Header
    midiPacket[1] = 0x80;  // Timestamp
    midiPacket[2] = 0xE0 | (channel - 1);  // Pitch Bend + channel
    midiPacket[3] = lsb;
    midiPacket[4] = msb;
    
    sendMidiMessage(midiPacket, 5);
    
    Serial.printf("MIDI Pitch Bend - Ch:%d Bend:%d\n", channel, bend);
}

void MidiHandler::sendSystemExclusive(uint8_t* data, size_t length) {
    // For SysEx messages, we need to send them in chunks
    // This is a simplified implementation
    // Full implementation would require proper chunking
    
    if (length > 0 && data[0] == 0xF0) {
        Serial.println("SysEx messages not fully implemented in this version");
    }
}