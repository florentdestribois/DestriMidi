#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Preferences.h>
#include <MD_MAX72xx.h>

// MAX7219 Matrix Display Pins
#define MAX7219_DIN 21
#define MAX7219_CLK 18
#define MAX7219_CS 19

// Button Pins
#define PIN_BUTTON_1 32
#define PIN_BUTTON_2 33
#define PIN_BUTTON_3 25
#define PIN_BUTTON_4 26
#define PIN_BUTTON_5 27
#define PIN_BUTTON_6 14  // GPIO14 - Input/Output avec pull-up interne

// LED Pins
#define PIN_LED_CHARGING 15  // Green LED for charging indication
#define PIN_LED_ACTIVITY 4   // Orange LED for button press indication

// Analog Pins
#define PIN_BATTERY_VOLTAGE 35
// #define PIN_CHARGING_STATUS 34  // Non utilisé (TC4056 4-pins sans CHRG)

// Timing Constants
#define BUTTON_DEBOUNCE_MS 100
#define LONG_PRESS_MS 1000
#define FACTORY_RESET_MS 3000
#define BATTERY_READ_INTERVAL_MS 10000
#define SLEEP_TIMEOUT_MS 600000  // 10 min au lieu de 5 min
#define BATTERY_DISPLAY_TIME_MS 3000
#define ACTIVITY_LED_DURATION_MS 500
#define BLINK_INTERVAL_MS 500

// MIDI BLE Service UUIDs
#define MIDI_SERVICE_UUID        "03B80E5A-EDE8-4B33-A751-6CE34EC4C700"
#define MIDI_CHARACTERISTIC_UUID "7772E5DB-3868-4112-A1A9-F2669D106BF3"

// 8x8 Matrix Display Patterns
const byte digitPatterns_8x8[10][8] = {
  // 0
  {0b00111100, 0b01100110, 0b01101110, 0b01110110, 0b01100110, 0b01100110, 0b00111100, 0b00000000},
  // 1  
  {0b00011000, 0b00111000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b01111110, 0b00000000},
  // 2
  {0b00111100, 0b01100110, 0b00000110, 0b00001100, 0b00110000, 0b01100000, 0b01111110, 0b00000000},
  // 3
  {0b00111100, 0b01100110, 0b00000110, 0b00011100, 0b00000110, 0b01100110, 0b00111100, 0b00000000},
  // 4
  {0b00001100, 0b00011100, 0b00101100, 0b01001100, 0b01111110, 0b00001100, 0b00001100, 0b00000000},
  // 5
  {0b01111110, 0b01100000, 0b01111100, 0b00000110, 0b00000110, 0b01100110, 0b00111100, 0b00000000},
  // 6
  {0b00111100, 0b01100110, 0b01100000, 0b01111100, 0b01100110, 0b01100110, 0b00111100, 0b00000000},
  // 7
  {0b01111110, 0b00000110, 0b00001100, 0b00011000, 0b00110000, 0b00110000, 0b00110000, 0b00000000},
  // 8
  {0b00111100, 0b01100110, 0b01100110, 0b00111100, 0b01100110, 0b01100110, 0b00111100, 0b00000000},
  // 9
  {0b00111100, 0b01100110, 0b01100110, 0b00111110, 0b00000110, 0b01100110, 0b00111100, 0b00000000}
};

// Special patterns
const byte batteryPatterns[6][8] = {
  // Battery 0%
  {0b11111111, 0b10000001, 0b10000001, 0b10000001, 0b10000001, 0b10000001, 0b10000001, 0b11111111},
  // Battery 20%  
  {0b11111111, 0b10000001, 0b10000001, 0b10000001, 0b10000001, 0b10000001, 0b11111111, 0b11111111},
  // Battery 40%
  {0b11111111, 0b10000001, 0b10000001, 0b10000001, 0b10000001, 0b11111111, 0b11111111, 0b11111111},
  // Battery 60%
  {0b11111111, 0b10000001, 0b10000001, 0b10000001, 0b11111111, 0b11111111, 0b11111111, 0b11111111},
  // Battery 80%
  {0b11111111, 0b10000001, 0b10000001, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111},
  // Battery 100%
  {0b11111111, 0b10000001, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111}
};

const byte pairingPattern[8] = {0b01110011, 0b01100110, 0b01100110, 0b01110011, 0b01100000, 0b01100000, 0b01100000, 0b00000000}; // P
const byte channelCPattern[8] = {0b00111100, 0b01100110, 0b01100000, 0b01100000, 0b01100000, 0b01100110, 0b00111100, 0b00000000}; // C

// Display Modes
enum DisplayMode {
  MODE_CHANNEL,
  MODE_BATTERY,
  MODE_PAIRING,
  MODE_ERROR
};

// Global Variables
Preferences preferences;
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::GENERIC_HW, MAX7219_CS, 1);

// Function Prototypes
void displayMatrix(const byte pattern[8]);
void displayDigit(int number);
void displayOff();
void updateChannelDisplay();
void showBatteryLevel();
void blinkDisplay();
void readBatteryVoltage();
void handleButton(int index);
void handleShortPress(int index);
void handleLongPress(int index);
void enterPairingMode();
void factoryReset();
void checkSleepTimeout();
void enterDeepSleep();
void sendMidiControlChange(uint8_t channel, uint8_t control, uint8_t value);
void flashActivityLED();
void connectionLightShow();

// Button Structure
struct Button {
  uint8_t pin;
  bool pressed;
  bool longPressed;
  unsigned long pressTime;
  unsigned long lastDebounceTime;
  bool lastState;
};

Button buttons[6] = {
  {PIN_BUTTON_1, false, false, 0, 0, HIGH},
  {PIN_BUTTON_2, false, false, 0, 0, HIGH},
  {PIN_BUTTON_3, false, false, 0, 0, HIGH},
  {PIN_BUTTON_4, false, false, 0, 0, HIGH},
  {PIN_BUTTON_5, false, false, 0, 0, HIGH},
  {PIN_BUTTON_6, false, false, 0, 0, HIGH}
};

// State Variables
uint8_t midiChannel = 1;
uint8_t ccNumbers[6] = {1, 2, 3, 4, 5, 6};
float batteryVoltage = 0;
bool isCharging = false;
unsigned long lastActivityTime = 0;
unsigned long lastBatteryReadTime = 0;
unsigned long batteryDisplayEndTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long activityLEDOffTime = 0;
DisplayMode currentDisplayMode = MODE_PAIRING;
bool blinkState = false;

// BLE Server Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
      deviceConnected = true;
      Serial.println("*** BLE DEVICE CONNECTED ***");
      Serial.print("Connected devices count: ");
      Serial.println(pServer->getConnectedCount());
      
      // Lancer le show de lumières de connexion
      connectionLightShow();
      
      // Passer du mode pairing au mode channel
      currentDisplayMode = MODE_CHANNEL;
      updateChannelDisplay();
    };

    void onDisconnect(BLEServer* pServer) override {
      deviceConnected = false;
      Serial.println("*** BLE DEVICE DISCONNECTED ***");
      Serial.print("Reason: Connection timeout or client disconnect");
      // Retourner en mode pairing avec LEDs alternées
      currentDisplayMode = MODE_PAIRING;
      Serial.println("BLE Disconnected - returning to pairing mode");
    }
};

void setup() {
  Serial.begin(115200);
  
  // Initialize MAX7219 Matrix Display
  mx.begin();            // Initialize MAX7219
  mx.control(MD_MAX72XX::INTENSITY, 8);  // Set brightness (0-15)
  mx.clear();            // Clear display
  Serial.println("MAX7219 Matrix Display initialized");
  
  // Initialize LED Pins
  pinMode(PIN_LED_CHARGING, OUTPUT);
  pinMode(PIN_LED_ACTIVITY, OUTPUT);
  digitalWrite(PIN_LED_CHARGING, LOW);
  digitalWrite(PIN_LED_ACTIVITY, LOW);
  
  Serial.println("Hardware initialization completed");
  
  // Initialize Button Pins
  for (int i = 0; i < 6; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
  
  // Initialize Analog Pins
  pinMode(PIN_BATTERY_VOLTAGE, INPUT);
  // pinMode(PIN_CHARGING_STATUS, INPUT_PULLUP);  // Non utilisé (TC4056 4-pins)
  
  // Show startup pattern
  displayMatrix(pairingPattern);
  delay(500);
  displayOff();
  delay(200);
  
  // Load preferences
  preferences.begin("destrimidi", false);
  midiChannel = preferences.getUChar("channel", 1);
  for (int i = 0; i < 6; i++) {
    String key = "cc" + String(i);
    ccNumbers[i] = preferences.getUChar(key.c_str(), i + 1);
  }
  
  // Initialize BLE with power settings
  Serial.println("Starting BLE initialization...");
  BLEDevice::init("DestriMidi");
  BLEDevice::setPower(ESP_PWR_LVL_P9); // Max power for stable connection
  Serial.println("BLE Device initialized");
  
  pServer = BLEDevice::createServer();
  Serial.println("BLE Server created");
  
  pServer->setCallbacks(new MyServerCallbacks());
  Serial.println("BLE Server callbacks set");
  
  BLEService *pService = pServer->createService(MIDI_SERVICE_UUID);
  Serial.println("BLE Service created");
  
  pCharacteristic = pService->createCharacteristic(
                      MIDI_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE_NR
                    );
  Serial.println("BLE Characteristic created");
  
  pCharacteristic->addDescriptor(new BLE2902());
  Serial.println("BLE Descriptor added");
  
  pService->start();
  Serial.println("BLE Service started");
  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(MIDI_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMaxPreferred(0x12);
  
  // Configuration pour reconnexion automatique Mac
  pAdvertising->setMinInterval(0x20);
  pAdvertising->setMaxInterval(0x40);
  Serial.println("BLE Advertising configured");
  
  BLEDevice::startAdvertising();
  Serial.println("BLE Advertising started - Device should be visible now!");
  
  // Start in pairing mode - show blinking P
  currentDisplayMode = MODE_PAIRING;
  Serial.println("Starting in pairing mode - P will blink until connected");
  
  
  lastActivityTime = millis();
}

// 8x8 Matrix Display Functions
void displayMatrix(const byte pattern[8]) {
  for (int i = 0; i < 8; i++) {
    mx.setRow(i, pattern[i]);
  }
}

void displayDigit(int number) {
  if (number >= 0 && number <= 9) {
    displayMatrix(digitPatterns_8x8[number]);
  }
}

void displayOff() {
  mx.clear();
}

void updateChannelDisplay() {
  // Affichage canal avec "C" suivi du numéro
  if (midiChannel <= 9) {
    // Afficher "C" + chiffre pour canaux 1-9
    byte channelPattern[8];
    // Combiner pattern "C" avec chiffre
    for (int i = 0; i < 8; i++) {
      channelPattern[i] = (channelCPattern[i] >> 1) | (digitPatterns_8x8[midiChannel][i] << 4);
    }
    displayMatrix(channelPattern);
  } else {
    // Pour canaux >9, afficher juste le chiffre
    displayDigit(midiChannel);
  }
}

void showBatteryLevel() {
  int batteryPercent = (int)((batteryVoltage - 3.0) / (4.2 - 3.0) * 100);
  batteryPercent = constrain(batteryPercent, 0, 100);
  int batteryLevel = batteryPercent / 20;  // 0-5 levels
  
  if (batteryLevel >= 0 && batteryLevel <= 5) {
    displayMatrix(batteryPatterns[batteryLevel]);
  }
}

void blinkDisplay() {
  if (millis() - lastBlinkTime > BLINK_INTERVAL_MS) {
    blinkState = !blinkState;
    lastBlinkTime = millis();
    
    if (currentDisplayMode == MODE_PAIRING) {
      if (blinkState) {
        displayMatrix(pairingPattern);
        // LEDs clignotent en alternance pendant le pairing
        digitalWrite(PIN_LED_ACTIVITY, HIGH);
        digitalWrite(PIN_LED_CHARGING, LOW);
      } else {
        displayOff();
        // LEDs inversées
        digitalWrite(PIN_LED_ACTIVITY, LOW);
        digitalWrite(PIN_LED_CHARGING, HIGH);
      }
    }
  }
}

void flashActivityLED() {
  digitalWrite(PIN_LED_ACTIVITY, HIGH);
  activityLEDOffTime = millis() + 1000; // 1 seconde
}

void connectionLightShow() {
  Serial.println("Starting connection LED light show...");
  
  // LEDs indépendantes qui se superposent naturellement
  // LED jaune: 1x par seconde pendant 5 secondes = 5 flashs total
  // LED verte: 3x par seconde pendant 5 secondes = 15 flashs total
  
  unsigned long startTime = millis();
  unsigned long duration = 5000; // 5 secondes
  
  while (millis() - startTime < duration) {
    unsigned long elapsed = millis() - startTime;
    
    // LED jaune : clignote toutes les 1000ms (1x par seconde)
    unsigned long yellowCycle = elapsed % 1000;
    bool yellowOn = (yellowCycle < 100); // Flash de 100ms
    
    // LED verte : clignote toutes les 333ms (3x par seconde)  
    unsigned long greenCycle = elapsed % 333;
    bool greenOn = (greenCycle < 100); // Flash de 100ms
    
    digitalWrite(PIN_LED_ACTIVITY, yellowOn ? HIGH : LOW);
    digitalWrite(PIN_LED_CHARGING, greenOn ? HIGH : LOW);
    
    delay(10); // Petite pause pour la boucle
  }
  
  // Éteindre toutes les LEDs
  digitalWrite(PIN_LED_ACTIVITY, LOW);
  digitalWrite(PIN_LED_CHARGING, LOW);
  Serial.println("Connection LED light show complete");
}

// Battery and Charging Functions
void readBatteryVoltage() {
  int adcValue = analogRead(PIN_BATTERY_VOLTAGE);
  batteryVoltage = (adcValue / 4095.0) * 3.3 * 2;
  
  // Détection de charge par analyse de tension (pas de pin CHRG sur ce module)
  static float previousVoltage = 0;
  static unsigned long voltageStableTime = 0;
  
  if (abs(batteryVoltage - previousVoltage) < 0.05) { // Tension stable
    if (millis() - voltageStableTime > 5000) { // Stable depuis 5 sec
      isCharging = (batteryVoltage > 4.0); // >4V probablement en charge
    }
  } else {
    voltageStableTime = millis();
    previousVoltage = batteryVoltage;
  }
  
  // Debug batterie toutes les 2 secondes + clignotement LED pour indication
  static unsigned long lastBatteryDebug = 0;
  if (millis() - lastBatteryDebug > 2000) {
    Serial.print("Battery: ADC=");
    Serial.print(adcValue);
    Serial.print(" Voltage=");
    Serial.print(batteryVoltage);
    Serial.print("V Charging=");
    Serial.println(isCharging ? "YES" : "NO");
    
    
    lastBatteryDebug = millis();
  }
  
  // Update charging LED - Only green LED for charging indication
  if (isCharging) {
    // En charge : LED verte clignotante
    digitalWrite(PIN_LED_CHARGING, (millis() / 1000) % 2);
  } else if (batteryVoltage > 4.1) {
    // Charge terminée : LED verte fixe pendant 5 secondes
    static unsigned long chargeCompleteTime = 0;
    static bool wasCharging = false;
    
    if (wasCharging && !isCharging) {
      chargeCompleteTime = millis();
    }
    
    if (millis() - chargeCompleteTime < 5000) {
      digitalWrite(PIN_LED_CHARGING, HIGH);
    } else {
      digitalWrite(PIN_LED_CHARGING, LOW);
    }
    
    wasCharging = isCharging;
  } else {
    // Fonctionnement normal : LED éteinte
    digitalWrite(PIN_LED_CHARGING, LOW);
  }
}

// Button Handling Functions
void handleButton(int index) {
  Button& btn = buttons[index];
  bool currentState = digitalRead(btn.pin);
  
  if (currentState != btn.lastState) {
    btn.lastDebounceTime = millis();
    Serial.print("Button ");
    Serial.print(index + 1);
    Serial.print(" state change: ");
    Serial.println(currentState == LOW ? "PRESSED" : "RELEASED");
  }
  
  if ((millis() - btn.lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
    if (currentState == LOW && !btn.pressed) {
      btn.pressed = true;
      btn.pressTime = millis();
      lastActivityTime = millis();
      Serial.print("Button ");
      Serial.print(index + 1);
      Serial.println(" press STARTED");
    } else if (currentState == HIGH && btn.pressed) {
      unsigned long pressDuration = millis() - btn.pressTime;
      
      // Ignorer les pressions trop courtes (probable rebond)
      if (pressDuration < 30) {
        Serial.print("Button ");
        Serial.print(index + 1);
        Serial.println(" press too short - ignored");
        btn.pressed = false;
        btn.longPressed = false;
        btn.lastState = currentState;
        return;
      }
      
      Serial.print("Button ");
      Serial.print(index + 1);
      Serial.print(" press ENDED (duration: ");
      Serial.print(pressDuration);
      Serial.println("ms)");
      
      if (pressDuration >= LONG_PRESS_MS) {
        // Long press détecté au relâchement
        Serial.print("Button ");
        Serial.print(index + 1);
        Serial.println(" -> LONG PRESS");
        handleLongPress(index);
      } else {
        // Short press au relâchement
        Serial.print("Button ");
        Serial.print(index + 1);
        Serial.println(" -> SHORT PRESS");
        handleShortPress(index);
      }
      
      btn.pressed = false;
      btn.longPressed = false;
    }
    
    // Marquer comme long press pendant l'appui (sans traiter l'action)
    if (btn.pressed && (millis() - btn.pressTime) >= LONG_PRESS_MS && !btn.longPressed) {
      btn.longPressed = true;
      Serial.print("Long press detected on button ");
      Serial.println(index + 1);
    }
  }
  
  btn.lastState = currentState;
}

void handleShortPress(int index) {
  Serial.print("handleShortPress called for button ");
  Serial.println(index + 1);
  
  // Check for button combinations
  if (buttons[0].pressed && buttons[1].pressed) {
    Serial.println("Button combination B1+B2 -> Entering pairing mode");
    enterPairingMode();
    return;
  }
  
  if (buttons[2].pressed && buttons[3].pressed) {
    Serial.println("Button combination B3+B4 -> Show battery level");
    // Show battery level for 3 seconds
    currentDisplayMode = MODE_BATTERY;
    batteryDisplayEndTime = millis() + BATTERY_DISPLAY_TIME_MS;
    showBatteryLevel();
    return;
  }
  
  // Send MIDI CC
  Serial.print("Sending MIDI CC: Channel=");
  Serial.print(midiChannel);
  Serial.print(", CC#=");
  Serial.print(ccNumbers[index]);
  Serial.print(", Value=127");
  Serial.println();
  
  sendMidiControlChange(midiChannel - 1, ccNumbers[index], 127);
  
  flashActivityLED();
}

void handleLongPress(int index) {
  Serial.print("handleLongPress called for button ");
  Serial.print(index + 1);
  Serial.print(" (index ");
  Serial.print(index);
  Serial.println(")");
  
  if (index == 4) {  // Button 5 - Channel Down
    Serial.println("Button 5 - Channel DOWN");
    midiChannel = (midiChannel == 1) ? 9 : midiChannel - 1;
    preferences.putUChar("channel", midiChannel);
    updateChannelDisplay();
    flashActivityLED();
    Serial.print("New channel: ");
    Serial.println(midiChannel);
  } else if (index == 5) {  // Button 6 - Channel Up  
    Serial.println("Button 6 - Channel UP");
    midiChannel = (midiChannel == 9) ? 1 : midiChannel + 1;
    preferences.putUChar("channel", midiChannel);
    updateChannelDisplay();
    flashActivityLED();
    Serial.print("New channel: ");
    Serial.println(midiChannel);
  } else {
    Serial.print("Long press on button ");
    Serial.print(index + 1);
    Serial.println(" - no channel change (only B5/B6)");
  }
}

// MIDI Functions
void sendMidiControlChange(uint8_t channel, uint8_t control, uint8_t value) {
  if (!deviceConnected) return;
  
  uint8_t midiPacket[5];
  midiPacket[0] = 0x80;  // Header
  midiPacket[1] = 0x80;  // Timestamp
  midiPacket[2] = 0xB0 | (channel & 0x0F);  // Control Change
  midiPacket[3] = control & 0x7F;
  midiPacket[4] = value & 0x7F;
  
  pCharacteristic->setValue(midiPacket, 5);
  pCharacteristic->notify();
}

// System Functions
void enterPairingMode() {
  currentDisplayMode = MODE_PAIRING;
  BLEDevice::startAdvertising();
  
  unsigned long pairingEndTime = millis() + 5000;
  while (millis() < pairingEndTime) {
    blinkDisplay();
    delay(10);
  }
  
  currentDisplayMode = MODE_CHANNEL;
  updateChannelDisplay();
}

void factoryReset() {
  // Show E pattern for reset
  byte resetPattern[8] = {0b01111110, 0b01100000, 0b01100000, 0b01111100, 0b01100000, 0b01100000, 0b01111110, 0b00000000}; // E
  displayMatrix(resetPattern);
  delay(1000);
  
  preferences.clear();
  midiChannel = 1;
  for (int i = 0; i < 6; i++) {
    ccNumbers[i] = i + 1;
  }
  
  delay(1000);
  ESP.restart();
}

void checkSleepTimeout() {
  // Disable sleep if BLE connected to prevent disconnections
  if (deviceConnected) {
    lastActivityTime = millis(); // Reset timer when connected
    return;
  }
  
  if ((millis() - lastActivityTime) > SLEEP_TIMEOUT_MS) {
    enterDeepSleep();
  }
}

void enterDeepSleep() {
  displayOff();
  digitalWrite(PIN_LED_CHARGING, LOW);
  digitalWrite(PIN_LED_ACTIVITY, LOW);
  
  uint64_t wakeupMask = 0;
  for (int i = 0; i < 6; i++) {
    wakeupMask |= (1ULL << buttons[i].pin);
  }
  esp_sleep_enable_ext1_wakeup(wakeupMask, ESP_EXT1_WAKEUP_ANY_HIGH);
  
  esp_deep_sleep_start();
}

void loop() {
  // Handle all buttons
  for (int i = 0; i < 6; i++) {
    handleButton(i);
  }
  
  // Update battery voltage periodically
  if ((millis() - lastBatteryReadTime) > BATTERY_READ_INTERVAL_MS) {
    readBatteryVoltage();
    lastBatteryReadTime = millis();
  }
  
  // Handle display modes
  if (currentDisplayMode == MODE_BATTERY) {
    if (millis() > batteryDisplayEndTime) {
      currentDisplayMode = MODE_CHANNEL;
      updateChannelDisplay();
    flashActivityLED();
    } else {
      showBatteryLevel();
    }
  } else if (currentDisplayMode == MODE_PAIRING) {
    blinkDisplay();
  } else if (currentDisplayMode == MODE_CHANNEL) {
    // Keep channel displayed
  }
  
  // Turn off activity LED after duration
  if (activityLEDOffTime > 0 && millis() > activityLEDOffTime) {
    digitalWrite(PIN_LED_ACTIVITY, LOW);
    activityLEDOffTime = 0;
  }
  
  // Handle BLE connection changes
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    // Restart advertising pour reconnexion automatique
    BLEDevice::startAdvertising();
    Serial.println("Device disconnected - restarting advertising for auto-reconnect");
    oldDeviceConnected = deviceConnected;
  }
  
  if (deviceConnected && !oldDeviceConnected) {
    Serial.println("Device connected successfully");
    oldDeviceConnected = deviceConnected;
  }
  
  // Check for sleep timeout
  checkSleepTimeout();
  
  delay(10);
}