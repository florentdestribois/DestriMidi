/*
 * Battery Manager Module Implementation
 */

#include "BatteryManager.h"

BatteryManager::BatteryManager() {
    batteryVoltage = 0.0;
    batteryPercentage = 100;
    chargingStatus = false;
    lastReadTime = 0;
    lastWarningTime = 0;
    sampleIndex = 0;
    
    // Initialize samples array
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        voltageSamples[i] = BATTERY_MAX_VOLTAGE;
    }
}

void BatteryManager::begin() {
    // Configure ADC for battery voltage reading
    analogReadResolution(12);  // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db);  // Full scale 0-3.3V
    
    // Initialize with first reading
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        voltageSamples[i] = readBatteryVoltage();
        delay(10);
    }
    
    Serial.println("Battery Manager initialized");
}

void BatteryManager::update() {
    // Read battery voltage every 1 second
    if (millis() - lastReadTime > 1000) {
        lastReadTime = millis();
        
        // Add new sample to moving average
        voltageSamples[sampleIndex] = readBatteryVoltage();
        sampleIndex = (sampleIndex + 1) % SAMPLE_COUNT;
        
        // Calculate average voltage
        float sum = 0;
        for (int i = 0; i < SAMPLE_COUNT; i++) {
            sum += voltageSamples[i];
        }
        batteryVoltage = sum / SAMPLE_COUNT;
        
        // Convert to percentage
        batteryPercentage = voltageToPercentage(batteryVoltage);
        
        // Read charging status
        chargingStatus = (digitalRead(PIN_CHARGING_STATUS) == LOW);
        
        // Update charging LED
        digitalWrite(PIN_LED_CHARGING, chargingStatus ? HIGH : LOW);
        
        // Check for low battery warning (every 30 seconds)
        if (isLowBattery() && !chargingStatus) {
            if (millis() - lastWarningTime > 30000) {
                lastWarningTime = millis();
                Serial.printf("Low Battery Warning: %.2fV (%d%%)\n", 
                            batteryVoltage, batteryPercentage);
            }
        }
    }
}

float BatteryManager::readBatteryVoltage() {
    // Read ADC value
    int adcValue = analogRead(PIN_BATTERY_VOLTAGE);
    
    // Convert to voltage
    // With voltage divider (1:2), actual battery voltage is 2x the measured voltage
    float measuredVoltage = (adcValue * ADC_REFERENCE_VOLTAGE) / ADC_RESOLUTION;
    float actualVoltage = measuredVoltage * 2.0;  // Account for voltage divider
    
    // Clamp to valid range
    if (actualVoltage < BATTERY_MIN_VOLTAGE) {
        actualVoltage = BATTERY_MIN_VOLTAGE;
    } else if (actualVoltage > BATTERY_MAX_VOLTAGE) {
        actualVoltage = BATTERY_MAX_VOLTAGE;
    }
    
    return actualVoltage;
}

uint8_t BatteryManager::voltageToPercentage(float voltage) {
    // Linear approximation of LiPo discharge curve
    // More accurate curves can be implemented if needed
    
    if (voltage >= BATTERY_MAX_VOLTAGE) {
        return 100;
    } else if (voltage <= BATTERY_MIN_VOLTAGE) {
        return 0;
    }
    
    // Improved discharge curve approximation for LiPo
    // Using piecewise linear approximation
    if (voltage >= 4.0) {
        // 4.2V to 4.0V = 100% to 90%
        return map(voltage * 100, 400, 420, 90, 100);
    } else if (voltage >= 3.8) {
        // 4.0V to 3.8V = 90% to 70%
        return map(voltage * 100, 380, 400, 70, 90);
    } else if (voltage >= 3.6) {
        // 3.8V to 3.6V = 70% to 40%
        return map(voltage * 100, 360, 380, 40, 70);
    } else if (voltage >= 3.3) {
        // 3.6V to 3.3V = 40% to 10%
        return map(voltage * 100, 330, 360, 10, 40);
    } else {
        // 3.3V to 3.0V = 10% to 0%
        return map(voltage * 100, 300, 330, 0, 10);
    }
}

float BatteryManager::getBatteryVoltage() {
    return batteryVoltage;
}

uint8_t BatteryManager::getBatteryPercentage() {
    return batteryPercentage;
}

bool BatteryManager::isCharging() {
    return chargingStatus;
}

bool BatteryManager::isLowBattery() {
    return batteryPercentage <= BATTERY_LOW_THRESHOLD;
}

bool BatteryManager::isCriticalBattery() {
    return batteryPercentage <= BATTERY_CRITICAL_THRESHOLD;
}