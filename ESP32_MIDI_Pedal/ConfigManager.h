/*
 * Configuration Manager Module
 * Handles persistent storage of settings using ESP32 Preferences
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

class ConfigManager {
private:
    Preferences* preferences;
    const char* namespaceName = "midipedal";
    
    // Configuration keys
    const char* KEY_MIDI_CHANNEL = "midi_ch";
    const char* KEY_DEVICE_NAME = "dev_name";
    const char* KEY_FIRST_BOOT = "first_boot";
    const char* KEY_BT_PAIRED = "bt_paired";
    
public:
    ConfigManager(Preferences* prefs);
    void begin();
    
    // MIDI Channel
    uint8_t getMidiChannel();
    void setMidiChannel(uint8_t channel);
    
    // Device Name
    String getDeviceName();
    void setDeviceName(String name);
    
    // Bluetooth pairing
    bool isBluetoothPaired();
    void setBluetoothPaired(bool paired);
    
    // Factory reset
    void factoryReset();
    
    // First boot check
    bool isFirstBoot();
    void setFirstBootComplete();
};

#endif