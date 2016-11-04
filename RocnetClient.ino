#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "RocNet.h"
#include "RocNetManufacturer.h"
#include "RocNetClass.h"
#include "ShiftRegister.h"

// Create a file Credentials.h and put the following lines into it, according to your WiFi network.
//char ssid[] = "your-ssid";
//char pass[] = "your-password";
#include "Credentials.h"

RocNet rocNet;
ShiftRegister shiftRegister;

const uint8_t numShiftRegisters = 4;
const uint8_t numIOs = numShiftRegisters * 8;
const uint8_t clkPin = 2;
const uint8_t dataPin = 0;
const uint8_t loadPin = 3;

void setup() {
  setUpSerial();
  setUpWiFi();
  setUpOTA();

  const uint16_t version = 0x0001;
  rocNet.begin(RocNetClass::accessory, RocNetManufacturer::rocrail, version, numIOs);

  shiftRegister.begin(numShiftRegisters, clkPin, dataPin, loadPin);
}

void loop() {
  ArduinoOTA.handle();

  bool ioStates[numIOs];
  static long ioDebounce[numIOs];

  shiftRegister.read(ioStates);

  for (uint8_t i = 0; i < numIOs; i++) {
    ioStates[i] = !ioStates[i]; // Negative logic
    if (ioStates[i] == true) {
      ioDebounce[i] = millis();
    } else if (millis() - ioDebounce[i] < 1000) {
      ioStates[i] = true;
    }
    yield();
  }

  rocNet.handle(ioStates);

  yield();
}

void setUpSerial (void) {
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  Serial.setDebugOutput(false);
  Serial.println();
  Serial.println();
}

void setUpOTA (void) {
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("");
    Serial.println("End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.print(".");
    //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void setUpWiFi (void) {
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
