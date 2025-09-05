#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Preferences.h>

// 7-Segment Display Pins
#define PIN_SEG_A 21
#define PIN_SEG_B 22
#define PIN_SEG_C 19
#define PIN_SEG_D 23
#define PIN_SEG_E 18
#define PIN_SEG_F 5
#define PIN_SEG_G 17
#define PIN_SEG_DP 16

// Button Pins
#define PIN_BUTTON_1 32
#define PIN_BUTTON_2 33
#define PIN_BUTTON_3 25
#define PIN_BUTTON_4 26
#define PIN_BUTTON_5 27
#define PIN_BUTTON_6 0  // Changé de 14 vers 0 (GPIO fiable)

// LED Pins
#define PIN_LED_POWER 12
#define PIN_LED_BLUETOOTH 2  // Changé de 13 vers 2 (GPIO fiable)
#define PIN_LED_CHARGING 15
#define PIN_LED_ACTIVITY 4

// Analog Pins
#define PIN_BATTERY_VOLTAGE 34
#define PIN_CHARGING_STATUS 35

// Timing Constants
#define BUTTON_DEBOUNCE_MS 50
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

// 7-Segment Display Pins Array
const int segmentPins[] = {PIN_SEG_A, PIN_SEG_B, PIN_SEG_C, PIN_SEG_D, PIN_SEG_E, PIN_SEG_F, PIN_SEG_G};

// 7-Segment Digit Patterns (Common Cathode)
// Format: 0bGFEDCBA
const byte digitPatterns[16] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111, // 9
  0b01110111, // A (10)
  0b01111100, // b (11)
  0b00111001, // C (12)
  0b01011110, // d (13)
  0b01111001, // E (14)
  0b01110001  // F (15)
};

// Special Patterns
#define PATTERN_P 0b01110011  // P pattern
#define PATTERN_BLANK 0b00000000  // All segments off

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

// Function Prototypes
void displaySegments(byte pattern);
void displayDigit(int number);
void displayHex(int number);
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
unsigned long activityLEDOffTime = 0;
unsigned long lastBlinkTime = 0;
DisplayMode currentDisplayMode = MODE_PAIRING;
bool blinkState = false;

// BLE Server Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
      deviceConnected = true;
      digitalWrite(PIN_LED_BLUETOOTH, HIGH);  // LED bleue fixe
      // Passer du mode pairing au mode channel
      currentDisplayMode = MODE_CHANNEL;
      updateChannelDisplay();
      Serial.println("*** BLE DEVICE CONNECTED ***");
      Serial.print("Connected devices count: ");
      Serial.println(pServer->getConnectedCount());
    };

    void onDisconnect(BLEServer* pServer) override {
      deviceConnected = false;
      digitalWrite(PIN_LED_BLUETOOTH, LOW);
      Serial.println("*** BLE DEVICE DISCONNECTED ***");
      Serial.print("Reason: Connection timeout or client disconnect");
      // Retourner en mode pairing
      currentDisplayMode = MODE_PAIRING;
      Serial.println("BLE Disconnected - returning to pairing mode");
    }
};

void setup() {
  Serial.begin(115200);
  
  // Initialize 7-Segment Display Pins
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], LOW);
  }
  pinMode(PIN_SEG_DP, OUTPUT);
  digitalWrite(PIN_SEG_DP, LOW);
  
  // Initialize LED Pins
  pinMode(PIN_LED_POWER, OUTPUT);
  pinMode(PIN_LED_BLUETOOTH, OUTPUT);
  pinMode(PIN_LED_CHARGING, OUTPUT);
  pinMode(PIN_LED_ACTIVITY, OUTPUT);
  digitalWrite(PIN_LED_POWER, HIGH);
  digitalWrite(PIN_LED_BLUETOOTH, LOW);
  digitalWrite(PIN_LED_CHARGING, LOW);
  digitalWrite(PIN_LED_ACTIVITY, LOW);
  
  // Test LED bleue au démarrage (maintenant sur GPIO 2)
  Serial.println("Testing Bluetooth LED on GPIO 2...");
  digitalWrite(PIN_LED_BLUETOOTH, HIGH);
  delay(1000);
  digitalWrite(PIN_LED_BLUETOOTH, LOW);
  Serial.println("Bluetooth LED test completed");
  
  // Initialize Button Pins
  for (int i = 0; i < 6; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
  
  // Initialize Analog Pins
  pinMode(PIN_BATTERY_VOLTAGE, INPUT);
  pinMode(PIN_CHARGING_STATUS, INPUT_PULLUP);
  
  // Show startup pattern
  displaySegments(PATTERN_P);
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

// 7-Segment Display Functions
void displaySegments(byte pattern) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], (pattern >> i) & 0x01);
  }
}

void displayDigit(int number) {
  if (number >= 0 && number <= 9) {
    displaySegments(digitPatterns[number]);
  }
}


void displayOff() {
  displaySegments(PATTERN_BLANK);
  digitalWrite(PIN_SEG_DP, LOW);
}

void updateChannelDisplay() {
  // Affichage simplifié pour canaux 1-9 uniquement
  displayDigit(midiChannel);
  digitalWrite(PIN_SEG_DP, LOW);
}

void showBatteryLevel() {
  int batteryPercent = (int)((batteryVoltage - 3.0) / (4.2 - 3.0) * 100);
  batteryPercent = constrain(batteryPercent, 0, 100);
  int displayLevel = batteryPercent / 10;
  
  displayDigit(min(displayLevel, 9));
  digitalWrite(PIN_SEG_DP, isCharging);
}

void blinkDisplay() {
  if (millis() - lastBlinkTime > BLINK_INTERVAL_MS) {
    blinkState = !blinkState;
    lastBlinkTime = millis();
    
    if (currentDisplayMode == MODE_PAIRING) {
      if (blinkState) {
        displaySegments(PATTERN_P);
      } else {
        displayOff();
      }
    }
  }
}

void flashActivityLED() {
  // S'assurer que la LED est vraiment éteinte avant de l'allumer
  digitalWrite(PIN_LED_ACTIVITY, LOW);
  delay(1);  // Pause très courte
  digitalWrite(PIN_LED_ACTIVITY, HIGH);
  activityLEDOffTime = millis() + ACTIVITY_LED_DURATION_MS;
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
  
  // Update charging LED
  if (isCharging) {
    // En charge : LED verte clignotante
    digitalWrite(PIN_LED_POWER, (millis() / 1000) % 2);
    digitalWrite(PIN_LED_CHARGING, LOW);
  } else if (batteryVoltage < 3.0) {
    // LED rouge fixe - arrêt imminent
    digitalWrite(PIN_LED_CHARGING, HIGH);
    Serial.println("*** CRITICAL LOW BATTERY - SHUTDOWN IMMINENT ***");
  } else if (batteryVoltage < 3.1) {
    // LED rouge clignotante - batterie faible
    digitalWrite(PIN_LED_CHARGING, (millis() / 1000) % 2);
    Serial.println("*** LOW BATTERY WARNING ***");
  } else {
    // Fonctionnement normal : LED verte fixe, LED rouge éteinte
    digitalWrite(PIN_LED_POWER, HIGH);
    digitalWrite(PIN_LED_CHARGING, LOW);
  }
}

// Button Handling Functions
void handleButton(int index) {
  Button& btn = buttons[index];
  bool currentState = digitalRead(btn.pin);
  
  if (currentState != btn.lastState) {
    btn.lastDebounceTime = millis();
  }
  
  if ((millis() - btn.lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
    if (currentState == LOW && !btn.pressed) {
      btn.pressed = true;
      btn.pressTime = millis();
      lastActivityTime = millis();
    } else if (currentState == HIGH && btn.pressed) {
      unsigned long pressDuration = millis() - btn.pressTime;
      
      if (pressDuration >= LONG_PRESS_MS) {
        // Long press détecté au relâchement
        handleLongPress(index);
      } else {
        // Short press au relâchement
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
  // Check for button combinations
  if (buttons[0].pressed && buttons[1].pressed) {
    enterPairingMode();
    return;
  }
  
  if (buttons[2].pressed && buttons[3].pressed) {
    // Show battery level for 3 seconds
    currentDisplayMode = MODE_BATTERY;
    batteryDisplayEndTime = millis() + BATTERY_DISPLAY_TIME_MS;
    showBatteryLevel();
    return;
  }
  
  // Send MIDI CC
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
  displaySegments(digitPatterns[14]);  // E
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
  digitalWrite(PIN_LED_POWER, LOW);
  digitalWrite(PIN_LED_BLUETOOTH, LOW);
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