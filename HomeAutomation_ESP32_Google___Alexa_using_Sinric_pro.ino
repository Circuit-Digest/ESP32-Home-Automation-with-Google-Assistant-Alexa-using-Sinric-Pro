#include <Arduino.h>
#include <WiFi.h>
#include "SinricPro.h"
#include "SinricProSwitch.h"

#define WIFI_SSID         "YOUR-WIFI-NAME"    
#define WIFI_PASS         "YOUR-WIFI-PASSWORD"
#define APP_KEY           "YOUR-APP-KEY"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "YOUR-APP-SECRET"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"

#define device_ID   "SWITCH_ID_HERE"

#define RELAY_PIN    22 // D22
#define SWITCH_PIN   12 // D12

#define wifiLed   2   //D2
 
#define BAUD_RATE    9600
#define DEBOUNCE_TIME 250

bool lastSwitchState = true;
unsigned long lastSwitchChange = 0;

bool relayState = false;

bool onPowerState(const String& deviceId, bool &state) {
  relayState = state;
  digitalWrite(RELAY_PIN, !relayState);
  return true;
}


void handleSwitch() {
  unsigned long currentMillis = millis();
  bool switchState = digitalRead(SWITCH_PIN);

  if (switchState != lastSwitchState) {
    if (currentMillis - lastSwitchChange > DEBOUNCE_TIME) {
      if (switchState) {
        relayState = !relayState;  // Toggle the relay state
        digitalWrite(RELAY_PIN, relayState);
        SinricProSwitch &mySwitch = SinricPro[device_ID];
        mySwitch.sendPowerStateEvent(relayState);
      }
      lastSwitchChange = currentMillis;
    }
    lastSwitchState = switchState;
  }
}

void setupWiFi()
{
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf(".");
    delay(250);
  }
  digitalWrite(wifiLed, HIGH);
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

void setupSinricPro() {
  SinricProSwitch& mySwitch = SinricPro[device_ID];
  mySwitch.onPowerState(onPowerState);

  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
}

void setupRelay() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN,HIGH); // Initialize relay in the OFF state
}

void setupSwitch() {
  pinMode(SWITCH_PIN, INPUT_PULLUP);
}


void setup() {
  Serial.begin(BAUD_RATE);

  pinMode(wifiLed, OUTPUT);
  digitalWrite(wifiLed, LOW);

  setupRelay();
  setupSwitch();
  setupWiFi();
  setupSinricPro();  
}
 

void loop() {
  SinricPro.handle();
  handleSwitch();
}
