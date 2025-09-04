/*
 * Battery Manager Module
 * Handles battery voltage monitoring and charging status
 */

#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>
#include "config.h"

class BatteryManager {
private:
    float batteryVoltage;
    uint8_t batteryPercentage;
    bool chargingStatus;
    unsigned long lastReadTime;
    unsigned long lastWarningTime;
    
    // Moving average for stable readings
    static const int SAMPLE_COUNT = 10;
    float voltageSamples[SAMPLE_COUNT];
    int sampleIndex;
    
    float readBatteryVoltage();
    uint8_t voltageToPercentage(float voltage);
    
public:
    BatteryManager();
    void begin();
    void update();
    float getBatteryVoltage();
    uint8_t getBatteryPercentage();
    bool isCharging();
    bool isLowBattery();
    bool isCriticalBattery();
};

#endif