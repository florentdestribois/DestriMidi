/*
 * Configuration Manager Module Implementation
 */

#include "ConfigManager.h"
#include "config.h"

ConfigManager::ConfigManager(Preferences* prefs) {
    preferences = prefs;
}

void ConfigManager::begin() {
    // Open preferences with namespace in RW mode
    preferences->begin(namespaceName, false);
    
    // Check if this is first boot
    if (isFirstBoot()) {
        Serial.println("First boot detected, initializing defaults...");
        
        // Set default values
        setMidiChannel(DEFAULT_MIDI_CHANNEL);
        setDeviceName(DEVICE_NAME);
        setBluetoothPaired(false);
        setFirstBootComplete();
        
        Serial.println("Default configuration loaded");
    } else {
        Serial.println("Configuration loaded from flash");
    }
    
    // Log current configuration
    Serial.printf("Current Config - MIDI Channel: %d, Device Name: %s\n", 
                  getMidiChannel(), getDeviceName().c_str());
}

uint8_t ConfigManager::getMidiChannel() {
    return preferences->getUChar(KEY_MIDI_CHANNEL, DEFAULT_MIDI_CHANNEL);
}

void ConfigManager::setMidiChannel(uint8_t channel) {
    if (channel >= 1 && channel <= 16) {
        preferences->putUChar(KEY_MIDI_CHANNEL, channel);
        Serial.printf("MIDI Channel saved: %d\n", channel);
    }
}

String ConfigManager::getDeviceName() {
    return preferences->getString(KEY_DEVICE_NAME, DEVICE_NAME);
}

void ConfigManager::setDeviceName(String name) {
    if (name.length() > 0 && name.length() <= 30) {
        preferences->putString(KEY_DEVICE_NAME, name);
        Serial.printf("Device name saved: %s\n", name.c_str());
    }
}

bool ConfigManager::isBluetoothPaired() {
    return preferences->getBool(KEY_BT_PAIRED, false);
}

void ConfigManager::setBluetoothPaired(bool paired) {
    preferences->putBool(KEY_BT_PAIRED, paired);
    Serial.printf("Bluetooth paired status: %s\n", paired ? "true" : "false");
}

void ConfigManager::factoryReset() {
    Serial.println("Performing factory reset of configuration...");
    
    // Clear all preferences
    preferences->clear();
    
    // Set defaults
    setMidiChannel(DEFAULT_MIDI_CHANNEL);
    setDeviceName(DEVICE_NAME);
    setBluetoothPaired(false);
    setFirstBootComplete();
    
    Serial.println("Factory reset complete");
}

bool ConfigManager::isFirstBoot() {
    return !preferences->getBool(KEY_FIRST_BOOT, false);
}

void ConfigManager::setFirstBootComplete() {
    preferences->putBool(KEY_FIRST_BOOT, true);
}