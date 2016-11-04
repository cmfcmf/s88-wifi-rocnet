#ifndef ROCNET_H
#define ROCNET_H

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "RocNetActionType.h"
#include "RocNetClass.h"
#include "RocNetManufacturer.h"
#include "RocNetGroup.h"

class RocNet {
  // Constants
  const uint16_t m_port = 4321;
  const IPAddress m_destIp = IPAddress(224, 0, 0, 1);
  const uint8_t m_networkId = 0;

  // Constant like; these are set by begin().
  RocNetClass m_class;
  RocNetManufacturer m_manufacturer;
  uint16_t m_version;
  uint8_t m_numIOs;

  // These need to be saved in EEPROM.
  uint16_t m_rocNetId;
  uint8_t m_location = 0; // @todo Save to EEPROM
  String m_uid = ""; // @todo Save to EEPROM

  // Objects
  WiFiUDP m_udp;
  bool *m_oldIOStates;
  
  public:
    RocNet();
    void begin(RocNetClass _class, RocNetManufacturer manufacturerId, uint16_t version, uint8_t numIOs);
    void handle(bool ioStates[]);
  private:
    void sendIdentifier(uint16_t recipient);
    void sendSensorState(uint8_t address, boolean state);
    void sendSensorStates(bool states[]);
    void pong(uint16_t recipient);
    void handleIOStates(bool ioStates[]);
    void sayHello();
    void error (String txt);
    void startUdp();
    void readRocNetId();
    void changeRocNetId(uint16_t newId);
    void sendPacket(uint8_t packet[], uint16_t length);
    void buildPacket(uint16_t recipient, RocNetGroup actionGroup, RocNetActionType actionType, uint8_t code, uint8_t data[], uint8_t dataLength, uint8_t packetBuffer[]);
    uint8_t buildActionCode(RocNetActionType type, uint8_t code);
};

#endif
