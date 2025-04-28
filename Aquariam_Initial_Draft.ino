/* 
  Wiring Details - Visag Smart Aquarium Controller (LilyGO T-Display-S3)

  - DS18B20 Temp Sensor -> GPIO21 (with 4.7kΩ pull-up resistor)
  - Fan (via relay/MOSFET) -> GPIO1
  - Light (via relay/MOSFET) -> GPIO3
  - Filter (via relay/MOSFET) -> GPIO10
  - Push Button 1 (onboard) -> GPIO0
  - Push Button 2 (onboard) -> GPIO14
  - TFT Display -> Built-in
  - Power -> 5V USB or external supply
  
  Notes:
    - Use relays/MOSFETs for controlling high-current devices.
    - Ensure DS18B20 uses a pull-up resistor on the data line.
    - Buttons are already connected internally to the ESP32.
*/


#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TFT_eSPI.h>
#include "time.h"

// WiFi credentials
const char* ssid = "wifi_name";
const char* password = "password";

// Timezone Settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // India GMT+5:30
const int daylightOffset_sec = 0;

// GPIO Pins
#define ONE_WIRE_BUS 21
#define FAN_PIN 1
#define LED_PIN 3
#define FILTER_PIN 10      // New Filter
#define BUTTON_PIN 0       // Built-in button IO0

// Libraries
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
TFT_eSPI tft = TFT_eSPI();

// Variables
bool fanStatus = false;
bool ledStatus = false;
bool filterStatus = false;
bool manualControl = false;
unsigned long manualControlTime = 0;
unsigned long previousMillis = 0;
const long interval = 1000; // 1 sec

void setup() {
  Serial.begin(115200);

  // Initialize Display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(20, 60);
  tft.println("Welcome to");
  tft.setCursor(20, 90);
  tft.println("Visag Smart Aquarium");
  delay(3000);
  tft.fillScreen(TFT_BLACK);

  // Initialize GPIOs
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(FILTER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize DS18B20
  sensors.begin();

  // Connect WiFi
  WiFi.begin(ssid, password);
  tft.setCursor(0, 20);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Connecting WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  tft.println("WiFi Connected!");

  // Time Setup
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  tft.println("Time Synced!");
  delay(2000);
  tft.fillScreen(TFT_BLACK);

  drawStaticLabels();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    checkManualControl();

    if (manualControl && (millis() - manualControlTime > 10000)) {  // 10s test mode
      manualControl = false;  // return to auto mode after 10s
    }

    // Read temperature
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);

    if (!manualControl) {
      controlFan(temperatureC);
      controlLight();
      controlFilter();
    }

    // Get Time
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }

    updateDynamicData(temperatureC, timeinfo);
  }
}

// Manual Button Handling
void checkManualControl() {
  static bool lastButtonState = HIGH;
  bool buttonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && buttonState == LOW) {
    manualControl = true;
    manualControlTime = millis();

    // Turn all ON when button pressed
    fanStatus = true;
    ledStatus = true;
    filterStatus = true;

    digitalWrite(FAN_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(FILTER_PIN, HIGH);
  }
  lastButtonState = buttonState;
}

// Auto Fan Control
void controlFan(float temp) {
  if (temp > 26.0) {
    digitalWrite(FAN_PIN, HIGH);
    fanStatus = true;
  } else if (temp < 24.0) {
    digitalWrite(FAN_PIN, LOW);
    fanStatus = false;
  }
}

// Auto Light Control
void controlLight() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time for Light");
    return;
  }
  int hour = timeinfo.tm_hour;
  if (hour >= 7 && hour < 22) {
    digitalWrite(LED_PIN, HIGH);
    ledStatus = true;
  } else {
    digitalWrite(LED_PIN, LOW);
    ledStatus = false;
  }
}

// Auto Filter Control (Same Logic as Light)
void controlFilter() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time for Filter");
    return;
  }
  int hour = timeinfo.tm_hour;
  if (hour >= 7 && hour < 22) {
    digitalWrite(FILTER_PIN, HIGH);
    filterStatus = true;
  } else {
    digitalWrite(FILTER_PIN, LOW);
    filterStatus = false;
  }
}

// Static labels
void drawStaticLabels() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, 0);
  tft.print("Time:");

  tft.setCursor(0, 20);
  tft.print("Date:");

  tft.setCursor(0, 50);
  tft.print("Water Temp:");  // Updated label here

  tft.setCursor(0, 80);
  tft.print("Cooling Fan:");

  tft.setCursor(0, 110);
  tft.print("Light:");

  tft.setCursor(0, 140);
  tft.print("Filter:");
}

// Dynamic updates
void updateDynamicData(float temp, struct tm timeinfo) {
  char timeString[16];
  char ampm[3];
  int displayHour = timeinfo.tm_hour;

  if (displayHour >= 12) {
    strcpy(ampm, "PM");
    if (displayHour > 12) displayHour -= 12;
  } else {
    strcpy(ampm, "AM");
    if (displayHour == 0) displayHour = 12;
  }

  sprintf(timeString, "%02d:%02d:%02d %s", displayHour, timeinfo.tm_min, timeinfo.tm_sec, ampm);

  char dateString[16];
  strftime(dateString, sizeof(dateString), "%d-%m-%Y", &timeinfo);

  // Update Time
  tft.fillRect(80, 0, 160, 20, TFT_BLACK);
  tft.setCursor(80, 0);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.print(timeString);

  // Update Date
  static int lastDay = -1;
  if (timeinfo.tm_mday != lastDay) {
    tft.fillRect(80, 20, 160, 20, TFT_BLACK);
    tft.setCursor(80, 20);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.print(dateString);
    lastDay = timeinfo.tm_mday;
  }

  // Update Temperature (Water Temp now)
  tft.fillRect(160, 50, 80, 20, TFT_BLACK);
  tft.setCursor(160, 50);
  tft.setTextColor(getTempColor(temp), TFT_BLACK);
  if (temp > -100 && temp < 150) {
    tft.printf("%.1f C", temp);
  } else {
    tft.print("Error");
  }

  // Update Fan Status (Fixing the issue with colon)
  tft.fillRect(160, 80, 80, 20, TFT_BLACK);
  tft.setCursor(160, 80);
  tft.setTextColor(fanStatus ? TFT_GREEN : TFT_RED, TFT_BLACK);
  if (fanStatus) {
    tft.print(manualControl ? "ON-Test" : "ON");
  } else {
    tft.print("OFF");
  }

  // Update Light Status (Fixing the issue with colon)
  tft.fillRect(160, 110, 80, 20, TFT_BLACK);
  tft.setCursor(160, 110);
  tft.setTextColor(ledStatus ? TFT_GREEN : TFT_RED, TFT_BLACK);
  if (ledStatus) {
    tft.print(manualControl ? "ON-Test" : "ON");
  } else {
    tft.print("OFF");
  }

  // Update Filter Status (Fixing the issue with colon)
  tft.fillRect(160, 140, 80, 20, TFT_BLACK);
  tft.setCursor(160, 140);
  tft.setTextColor(filterStatus ? TFT_GREEN : TFT_RED, TFT_BLACK);
  if (filterStatus) {
    tft.print(manualControl ? "ON-Test" : "ON");
  } else {
    tft.print("OFF");
  }
}

// Temp Color
uint16_t getTempColor(float temp) {
  if (temp > 30) {
    return TFT_RED;
  } else if (temp > 26) {
    return TFT_YELLOW;
  } else if (temp < 5) {
    return TFT_RED;
  } else if (temp < 20) {
    return TFT_YELLOW;
  } else {
    return TFT_GREEN;
  }
}
