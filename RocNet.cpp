#include "RocNet.h"
#include "EEPROM.h"

#define GEN_BUFFER_LEN(DATA_LEN) (DATA_LEN + 8)

RocNet::RocNet() {
  
}

void RocNet::begin(RocNetClass _class, RocNetManufacturer manufacturer, uint16_t version, uint8_t numIOs) {
  m_class = _class;
  m_manufacturer = manufacturer;
  m_version = version;
  m_numIOs = numIOs;
  m_oldIOStates = new bool[numIOs];
  memset(m_oldIOStates, false, m_numIOs * sizeof(*m_oldIOStates));
  
  this->readRocNetId();

  this->startUdp();

  this->sayHello();
}

void RocNet::handleIOStates(bool ioStates[]) {
  /*
  Serial.print("new: ");
  for (uint8_t i = 0; i < m_numIOs; i++) {
    Serial.print(ioStates[i]);
    Serial.print(" ");
  }
  Serial.println("");
  
  Serial.print("old: ");
  for (uint8_t i = 0; i < m_numIOs; i++) {
    Serial.print(m_oldIOStates[i]);
    Serial.print(" ");
  }
  Serial.println("");
  */
  
  for (uint8_t i = 0; i < m_numIOs; i++) {
    if (m_oldIOStates[i] != ioStates[i]) {
      this->sendSensorState(i + 1, ioStates[i]);
      m_oldIOStates[i] = ioStates[i];
      yield();
    }
  }
}

void RocNet::handle(bool ioStates[]) {
  this->handleIOStates(ioStates);
  
  int packetLength = m_udp.parsePacket();
  if (!packetLength) {
    return;
  }
  Serial.print("Packet received, length=");
  Serial.println(packetLength);

  uint8_t ctrl[8];
  m_udp.read(ctrl, 8);

  if (ctrl[0] != m_networkId) {
    // Not my network.
    Serial.print("Different network: ");
    Serial.println(ctrl[0], HEX);
    return;
  }
  if (!(ctrl[1] == 0 && ctrl[2] == 0) && !(ctrl[1] == ((m_rocNetId >> 8) & 0xFF) && ctrl[2] == (m_rocNetId & 0xFF))) {
    // Not sent to me.
    Serial.print("Was sent to: ");
    Serial.print(ctrl[1], HEX);
    Serial.print(" ");
    Serial.print(ctrl[2], HEX);
    Serial.print(", but I am: ");
    Serial.println(m_rocNetId, HEX);
    
    return;
  }
  
  uint16_t sender = (ctrl[3] << 8) | ctrl[4];
  
  RocNetGroup group = static_cast<RocNetGroup>(ctrl[5]);

  RocNetActionType type = static_cast<RocNetActionType>((ctrl[6] >> 5) & 0b11);
  
  uint8_t code = ctrl[6] & 0b11111;

  uint16_t dataLength = ctrl[7];
  uint8_t *data;
  if (dataLength > 0) {
    data = new uint8_t[dataLength];
    m_udp.read(data, dataLength);
  }

  if (group == RocNetGroup::clock) {
    return;
  }
  
  for (byte i = 0; i < 8; i++) {
    Serial.print(ctrl[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
  for (byte i = 0; i < 8; i++) {
    Serial.print(ctrl[i], DEC);
    Serial.print(" ");
  }
  Serial.println("");
  for (byte i = 0; i < dataLength; i++) {
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
  for (byte i = 0; i < dataLength; i++) {
    Serial.print(data[i], DEC);
    Serial.print(" ");
  }
  Serial.println("");
  
  // Check for identify packet
  if (group == RocNetGroup::stationary_decoders && type == RocNetActionType::request && code == 8) { // Identify
    Serial.println("Received identify request");
    this->sendIdentifier(sender);
    
    return;
  }

  if (group == RocNetGroup::stationary_decoders && type == RocNetActionType::request && code == 10) { // Acknowledge
    Serial.println("Received acknowledge.");

    // Command == data[0]

    return;
  }

  if (group == RocNetGroup::stationary_decoders && type == RocNetActionType::request && code == 11) { // Show
    Serial.println("Received show request");

    /* This breaks the Serial output, because the LED is connected to TXD.
    pinMode(LED_BUILTIN, OUTPUT);
    for (uint8_t i = 0; i < 10; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    }
    pinMode(LED_BUILTIN, INPUT);
    */
    
    return;
  }

  if (group == RocNetGroup::command_station && type == RocNetActionType::request && code == 2) { // Track power
    Serial.println("Received track power change");
    // 1 if on, 0 if off.
    bool onOff = data[0];

    return;
  }

  if (group == RocNetGroup::host && type == RocNetActionType::request && code == 2) { // Ping
    Serial.println("Received ping");
    this->pong(sender);
  
    return;
  }

  if (group == RocNetGroup::stationary_decoders && type == RocNetActionType::request && code == 12) { // Start of day
    Serial.println("Received start of day");
    this->sayHello();

    return;
  }
  
  IPAddress myIP = WiFi.localIP();
  // Check for set rocnet id packet
  if (group == RocNetGroup::programming_stationary && type == RocNetActionType::request && code == 6 && /* Set Rocnet ID */ data[2] == myIP[2] && data[3] == myIP[3]) { 
    Serial.println("Setting RocnetID");
    this->changeRocNetId((data[0] << 8) | data[1]);

    if (dataLength >= 5) {
      // @todo Also set location.
    }
    if (dataLength >= 6) {
      // @todo Also set uid.
    }
    
    this->sendIdentifier(sender);

    return;
  }
}

void RocNet::startUdp() {
  Serial.println("Starting UDP");
  if (!m_udp.begin(m_port)) {
    this->error("UDP failed to start!");
  }
  Serial.print("Local port: ");
  Serial.println(m_udp.localPort());
}

void RocNet::error (String txt) {
  Serial.println();
  Serial.println();
  Serial.println("ERROR! " + txt);
  Serial.println();
  Serial.println();
}

void RocNet::readRocNetId() {
  uint8_t initialized;
  EEPROM.begin(3);
  EEPROM.get(0, initialized);
  EEPROM.get(1, m_rocNetId);
    
  if (initialized != 77) {
    // EEPROM is fresh.
    Serial.println("EEPROM is fresh");
    EEPROM.put(0, 77);
    this->changeRocNetId(0xFFFF);
  } else {
    Serial.print("Read rocnet id: ");
    Serial.println(m_rocNetId, HEX);
  }
}

void RocNet::changeRocNetId(uint16_t newId) {
  Serial.print("Changing rocnet id to ");
  Serial.println(newId, HEX);
  
  m_rocNetId = newId;
  EEPROM.put(1, m_rocNetId);
  EEPROM.commit();
}

void RocNet::sayHello() {
  this->sendIdentifier(0x0000);

  bool *states = new bool[m_numIOs];
  memset(states, false, m_numIOs * sizeof(*states));
  
  this->sendSensorStates(states);
}

void RocNet::pong(uint16_t recipient) {
  const uint8_t len = 0;
  uint8_t packetBuffer[GEN_BUFFER_LEN(0)];

  this->buildPacket(recipient, RocNetGroup::host, RocNetActionType::request, 3, NULL, len, packetBuffer);
  this->sendPacket(packetBuffer, len);
}

void RocNet::sendSensorStates(bool states[]) {
  for (uint8_t i = 0; i < m_numIOs; i++) {
    this->sendSensorState(i + 1, states[i]);
    yield();
  }
}

void RocNet::sendSensorState(uint8_t address, boolean state) {
  const uint8_t len = 4;
  uint8_t packetBuffer[GEN_BUFFER_LEN(len)];

  uint8_t data[len];
  data[0] = 0x00;    // Loco Address H
  data[1] = 0x00;    // Loco Address L
  data[2] = state;   // State
  data[3] = address; // Address                     
  
  this->buildPacket(0x0000, RocNetGroup::sensor, RocNetActionType::event, 1, data, len, packetBuffer);
  this->sendPacket(packetBuffer, len);
}

void RocNet::sendIdentifier(uint16_t recipient) {
  const uint8_t len = 7;
  uint8_t packetBuffer[GEN_BUFFER_LEN(len)];

  uint8_t data[len];
  data[0] = static_cast<uint8_t>(m_class);        // class
  data[1] = static_cast<uint8_t>(m_manufacturer); // manufacturer id
  data[2] = (m_version >> 8) & 0xFF;              // versionH
  data[3] = m_version & 0xFF;                     // versionL
  data[4] = m_numIOs;                             // nr I/O
  
  IPAddress myIP = WiFi.localIP();
  data[5] = myIP[2]; // subipH
  data[6] = myIP[3]; // subipL
  
  this->buildPacket(recipient, RocNetGroup::stationary_decoders, RocNetActionType::event, 0b01000, data, len, packetBuffer);
  this->sendPacket(packetBuffer, len);
}

void RocNet::buildPacket(uint16_t recipient, RocNetGroup actionGroup, RocNetActionType actionType, uint8_t code, uint8_t data[], uint8_t dataLength, uint8_t packetBuffer[]) {
  packetBuffer[0] = m_networkId;              // Network ID
  
  packetBuffer[1] = (recipient >> 8) & 0xFF;  // Recipient H
  packetBuffer[2] = recipient & 0xFF;         // Recipient L
  
  packetBuffer[3] = (m_rocNetId >> 8) & 0xFF; // Bus (Sender H)
  packetBuffer[4] = m_rocNetId & 0xFF;        // Bus (Sender L)
  
  packetBuffer[5] = static_cast<uint8_t>(actionGroup);              // Action Group
  packetBuffer[6] = (static_cast<uint8_t>(actionType) << 5) | code; // Action Code
  
  packetBuffer[7] = dataLength;                   // Data Length
  memcpy(&packetBuffer[8], &data[0], dataLength); // Data
}

void RocNet::sendPacket(uint8_t packet[], uint16_t dataLength) {
  Serial.println("Sending packet...");
  for (uint8_t i = 0; i < GEN_BUFFER_LEN(dataLength); i++) {
    Serial.print(packet[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
  for (uint8_t i = 0; i < GEN_BUFFER_LEN(dataLength); i++) {
    Serial.print(packet[i], DEC);
    Serial.print(" ");
  }
  Serial.println("");
  
  if (!m_udp.beginPacket(m_destIp, m_port)) {
    this->error("UDP packet could not be started.");
  }
  if (!m_udp.write(packet, GEN_BUFFER_LEN(dataLength))) {
    this->error("UDP packet not written correctly.");
  }
  if (!m_udp.endPacket()) {
    this->error("UDP packet not sent correctly.");
  }
}

