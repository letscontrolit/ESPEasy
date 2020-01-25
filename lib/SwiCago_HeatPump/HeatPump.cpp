/*
  HeatPump.cpp - Mitsubishi Heat Pump control library for Arduino
  Copyright (c) 2017 Al Betschart.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "HeatPump.h"

// Structures //////////////////////////////////////////////////////////////////

bool operator==(const heatpumpSettings& lhs, const heatpumpSettings& rhs) {
  return lhs.power == rhs.power &&
         lhs.mode == rhs.mode &&
         lhs.temperature == rhs.temperature &&
         lhs.fan == rhs.fan &&
         lhs.vane == rhs.vane &&
         lhs.wideVane == rhs.wideVane &&
         lhs.iSee == rhs.iSee;
}

bool operator!=(const heatpumpSettings& lhs, const heatpumpSettings& rhs) {
  return lhs.power != rhs.power ||
         lhs.mode != rhs.mode ||
         lhs.temperature != rhs.temperature ||
         lhs.fan != rhs.fan ||
         lhs.vane != rhs.vane ||
         lhs.wideVane != rhs.wideVane ||
         lhs.iSee != rhs.iSee;
}

bool operator!(const heatpumpSettings& settings) {
  return !settings.power &&
         !settings.mode &&
         !settings.temperature &&
         !settings.fan &&
         !settings.vane &&
         !settings.wideVane &&
         !settings.iSee;
}

bool operator==(const heatpumpTimers& lhs, const heatpumpTimers& rhs) {
  return lhs.mode                == rhs.mode &&
         lhs.onMinutesSet        == rhs.onMinutesSet &&
         lhs.onMinutesRemaining  == rhs.onMinutesRemaining &&
         lhs.offMinutesSet       == rhs.offMinutesSet &&
         lhs.offMinutesRemaining == rhs.offMinutesRemaining;
}

bool operator!=(const heatpumpTimers& lhs, const heatpumpTimers& rhs) {
  return lhs.mode                != rhs.mode ||
         lhs.onMinutesSet        != rhs.onMinutesSet ||
         lhs.onMinutesRemaining  != rhs.onMinutesRemaining ||
         lhs.offMinutesSet       != rhs.offMinutesSet ||
         lhs.offMinutesRemaining != rhs.offMinutesRemaining;
}


// Constructor /////////////////////////////////////////////////////////////////

HeatPump::HeatPump() {
  lastSend = 0;
  infoMode = 0;
  lastRecv = millis() - (PACKET_SENT_INTERVAL_MS * 10);
  autoUpdate = false;
  firstRun = true;
  tempMode = false;
  waitForRead = false;
  externalUpdate = false;
  wideVaneAdj = false;
  currentStatus = {0, false, {TIMER_MODE_MAP[0], 0, 0, 0, 0}}; // initialise to all off, then it will update shortly after connect
}

// Public Methods //////////////////////////////////////////////////////////////

bool HeatPump::connect(ESPeasySerial *serial) {
	return connect(serial,true);
}

bool HeatPump::connect(ESPeasySerial *serial, bool retry) {
  if(serial != NULL) {
    _HardSerial = serial;
  }
  connected = false;
  _HardSerial->begin(bitrate, SERIAL_8E1);
  if(onConnectCallback) {
    onConnectCallback();
  }

  // settle before we start sending packets
  //delay(2000);

  // send the CONNECT packet twice - need to copy the CONNECT packet locally
  byte packet[CONNECT_LEN];
  memcpy(packet, CONNECT, CONNECT_LEN);
  //for(int count = 0; count < 2; count++) {
  writePacket(packet, CONNECT_LEN);
  while(!canRead()) { delay(10); }
  int packetType = readPacket();
  if(packetType != RCVD_PKT_CONNECT_SUCCESS && retry){
	  bitrate = (bitrate == 2400 ? 9600 : 2400);
	  return connect(serial, false);
  }
  return packetType == RCVD_PKT_CONNECT_SUCCESS;
  //}
}

bool HeatPump::update() {
  while(!canSend(false)) { delay(10); }

  byte packet[PACKET_LEN] = {};
  createPacket(packet, wantedSettings);
  writePacket(packet, PACKET_LEN);

  while(!canRead()) { delay(10); }
  int packetType = readPacket();

  if(packetType == RCVD_PKT_UPDATE_SUCCESS) {
    // call sync() to get the latest settings from the heatpump for autoUpdate, which should now have the updated settings
    if(autoUpdate) { //this sync will happen regardless, but autoUpdate needs it sooner than later.
	    while(!canSend(true)) {
		    delay(10);
	    }
	    sync(RQST_PKT_SETTINGS);
    }

    return true;
  } else {
    return false;
  }
}

void HeatPump::sync(byte packetType) {
  if((!connected) || (millis() - lastRecv > (PACKET_SENT_INTERVAL_MS * 10))) {
    connect(NULL);
  }
  else if(canRead()) {
    readPacket();
  }
  else if(autoUpdate && !firstRun && wantedSettings != currentSettings && packetType == PACKET_TYPE_DEFAULT) {
    update();
  }
  else if(canSend(true)) {
    byte packet[PACKET_LEN] = {};
    createInfoPacket(packet, packetType);
    writePacket(packet, PACKET_LEN);
  }
}

void HeatPump::enableExternalUpdate() {
  externalUpdate = true;
}

void HeatPump::enableAutoUpdate() {
  autoUpdate = true;
}

void HeatPump::disableAutoUpdate() {
  autoUpdate = false;
}

heatpumpSettings HeatPump::getSettings() {
  return currentSettings;
}

bool HeatPump::isConnected() const {
  return connected;
}

void HeatPump::setSettings(heatpumpSettings settings) {
  setPowerSetting(settings.power);
  setModeSetting(settings.mode);
  setTemperature(settings.temperature);
  setFanSpeed(settings.fan);
  setVaneSetting(settings.vane);
  setWideVaneSetting(settings.wideVane);
}

bool HeatPump::getPowerSettingBool() const {
  return currentSettings.power == POWER_MAP[1] ? true : false;
}

void HeatPump::setPowerSetting(bool setting) {
  wantedSettings.power = lookupByteMapIndex(POWER_MAP, 2, POWER_MAP[setting ? 1 : 0]) > -1 ? POWER_MAP[setting ? 1 : 0] : POWER_MAP[0];
}

const char* HeatPump::getPowerSetting() {
  return currentSettings.power;
}

void HeatPump::setPowerSetting(const char* setting) {
  int index = lookupByteMapIndex(POWER_MAP, 2, setting);
  if (index > -1) {
    wantedSettings.power = POWER_MAP[index];
  } else {
    wantedSettings.power = POWER_MAP[0];
  }
}

const char* HeatPump::getModeSetting() {
  return currentSettings.mode;
}

void HeatPump::setModeSetting(const char* setting) {
  int index = lookupByteMapIndex(MODE_MAP, 5, setting);
  if (index > -1) {
    wantedSettings.mode = MODE_MAP[index];
  } else {
    wantedSettings.mode = MODE_MAP[0];
  }
}

float HeatPump::getTemperature() const {
  return currentSettings.temperature;
}

void HeatPump::setTemperature(float setting) {
  if(!tempMode){
    wantedSettings.temperature = lookupByteMapIndex(TEMP_MAP, 16, (int)(setting + 0.5)) > -1 ? setting : TEMP_MAP[0];
  }
  else {
    setting = setting * 2;
    setting = round(setting);
    setting = setting / 2;
    wantedSettings.temperature = setting < 10 ? 10 : (setting > 31 ? 31 : setting);
  }
}

void HeatPump::setRemoteTemperature(float setting) {
  byte packet[PACKET_LEN] = {};
  for (int i = 0; i < 21; i++) {
    packet[i] = 0x00;
  }
  for (int i = 0; i < HEADER_LEN; i++) {
    packet[i] = HEADER[i];
  }
  packet[5] = 0x07;
  if(setting > 0) {
    packet[6] = 0x01;
    setting = setting * 2;
    setting = round(setting);
    setting = setting / 2;
    float temp1 = 3 + ((setting - 10) * 2);
    packet[7] = (int)temp1;
    float temp2 = (setting * 2) + 128;
    packet[8] = (int)temp2;
  }
  else {
    packet[6] = 0x00;
    packet[8] = 0x80; //MHK1 send 80, even though it could be 00, since ControlByte is 00
  }
  // add the checksum
  byte chkSum = checkSum(packet, 21);
  packet[21] = chkSum;
  while(!canSend(false)) { delay(10); }
  writePacket(packet, PACKET_LEN);
}

const char* HeatPump::getFanSpeed() {
  return currentSettings.fan;
}


void HeatPump::setFanSpeed(const char* setting) {
  int index = lookupByteMapIndex(FAN_MAP, 6, setting);
  if (index > -1) {
    wantedSettings.fan = FAN_MAP[index];
  } else {
    wantedSettings.fan = FAN_MAP[0];
  }
}

const char* HeatPump::getVaneSetting() {
  return currentSettings.vane;
}

void HeatPump::setVaneSetting(const char* setting) {
  int index = lookupByteMapIndex(VANE_MAP, 7, setting);
  if (index > -1) {
    wantedSettings.vane = VANE_MAP[index];
  } else {
    wantedSettings.vane = VANE_MAP[0];
  }
}

const char* HeatPump::getWideVaneSetting() {
  return currentSettings.wideVane;
}

void HeatPump::setWideVaneSetting(const char* setting) {
  int index = lookupByteMapIndex(WIDEVANE_MAP, 7, setting);
  if (index > -1) {
    wantedSettings.wideVane = WIDEVANE_MAP[index];
  } else {
    wantedSettings.wideVane = WIDEVANE_MAP[0];
  }
}

bool HeatPump::getIseeBool() { //no setter yet
  return currentSettings.iSee;
}

heatpumpStatus HeatPump::getStatus() {
  return currentStatus;
}

float HeatPump::getRoomTemperature() const {
  return currentStatus.roomTemperature;
}

bool HeatPump::getOperating() {
  return currentStatus.operating;
}

float HeatPump::FahrenheitToCelsius(int tempF) {
  float temp = (tempF - 32) / 1.8;
  return ((float)round(temp*2))/2;                 //Round to nearest 0.5C
}

int HeatPump::CelsiusToFahrenheit(float tempC) {
  float temp = (tempC * 1.8) + 32;                //round up if heat, down if cool or any other mode
  return (int)(temp + 0.5);
}

void HeatPump::setOnConnectCallback(ON_CONNECT_CALLBACK_SIGNATURE) {
  this->onConnectCallback = onConnectCallback;
}

void HeatPump::setSettingsChangedCallback(SETTINGS_CHANGED_CALLBACK_SIGNATURE) {
  this->settingsChangedCallback = settingsChangedCallback;
}

void HeatPump::setStatusChangedCallback(STATUS_CHANGED_CALLBACK_SIGNATURE) {
  this->statusChangedCallback = statusChangedCallback;
}

void HeatPump::setPacketCallback(PACKET_CALLBACK_SIGNATURE) {
  this->packetCallback = packetCallback;
}

void HeatPump::setRoomTempChangedCallback(ROOM_TEMP_CHANGED_CALLBACK_SIGNATURE) {
  this->roomTempChangedCallback = roomTempChangedCallback;
}

//#### WARNING, THE FOLLOWING METHOD CAN F--K YOUR HP UP, USE WISELY ####
void HeatPump::sendCustomPacket(byte data[], int packetLength) {
  while(!canSend(false)) { delay(10); }

  packetLength += 2; // +2 for first header byte and checksum
  packetLength = (packetLength > PACKET_LEN) ? PACKET_LEN : packetLength; // ensure we are not exceeding PACKET_LEN
  byte packet[packetLength];
  packet[0] = HEADER[0]; // add first header byte

  // add data
  for (int i = 0; i < packetLength; i++) {
    packet[(i+1)] = data[i];
  }

  // add checksum
  byte chkSum = checkSum(packet, (packetLength-1));
  packet[(packetLength-1)] = chkSum;

  writePacket(packet, packetLength);
}

// Private Methods //////////////////////////////////////////////////////////////

int HeatPump::lookupByteMapIndex(const int valuesMap[], int len, int lookupValue) {
  for (int i = 0; i < len; i++) {
    if (valuesMap[i] == lookupValue) {
      return i;
    }
  }
  return -1;
}

int HeatPump::lookupByteMapIndex(const char* valuesMap[], int len, const char* lookupValue) {
  for (int i = 0; i < len; i++) {
    if (strcmp(valuesMap[i], lookupValue) == 0) {
      return i;
    }
  }
  return -1;
}


const char* HeatPump::lookupByteMapValue(const char* valuesMap[], const byte byteMap[], int len, byte byteValue) {
  for (int i = 0; i < len; i++) {
    if (byteMap[i] == byteValue) {
      return valuesMap[i];
    }
  }
  return valuesMap[0];
}

int HeatPump::lookupByteMapValue(const int valuesMap[], const byte byteMap[], int len, byte byteValue) {
  for (int i = 0; i < len; i++) {
    if (byteMap[i] == byteValue) {
      return valuesMap[i];
    }
  }
  return valuesMap[0];
}

bool HeatPump::canSend(bool isInfo) {
  return (millis() - (isInfo ? PACKET_INFO_INTERVAL_MS : PACKET_SENT_INTERVAL_MS)) > lastSend;
}

bool HeatPump::canRead() {
  return (waitForRead && (millis() - PACKET_SENT_INTERVAL_MS) > lastSend);
}

byte HeatPump::checkSum(byte bytes[], int len) {
  byte sum = 0;
  for (int i = 0; i < len; i++) {
    sum += bytes[i];
  }
  return (0xfc - sum) & 0xff;
}

void HeatPump::createPacket(byte *packet, heatpumpSettings settings) {
  //preset all bytes to 0x00
  for (int i = 0; i < 21; i++) {
    packet[i] = 0x00;
  }
  for (int i = 0; i < HEADER_LEN; i++) {
    packet[i] = HEADER[i];
  }
  if(settings.power != currentSettings.power) {
    packet[8]  = POWER[lookupByteMapIndex(POWER_MAP, 2, settings.power)];
    packet[6] += CONTROL_PACKET_1[0];
  }
  if(settings.mode!= currentSettings.mode) {
    packet[9]  = MODE[lookupByteMapIndex(MODE_MAP, 5, settings.mode)];
    packet[6] += CONTROL_PACKET_1[1];
  }
  if(!tempMode && settings.temperature!= currentSettings.temperature) {
    packet[10] = TEMP[lookupByteMapIndex(TEMP_MAP, 16, settings.temperature)];
    packet[6] += CONTROL_PACKET_1[2];
  }
  else if(tempMode && settings.temperature!= currentSettings.temperature) {
    float temp = (settings.temperature * 2) + 128;
    packet[19] = (int)temp;
    packet[6] += CONTROL_PACKET_1[2];
  }
  if(settings.fan!= currentSettings.fan) {
    packet[11] = FAN[lookupByteMapIndex(FAN_MAP, 6, settings.fan)];
    packet[6] += CONTROL_PACKET_1[3];
  }
  if(settings.vane!= currentSettings.vane) {
    packet[12] = VANE[lookupByteMapIndex(VANE_MAP, 7, settings.vane)] | (wideVaneAdj ? 0x80 : 0x00);
    packet[6] += CONTROL_PACKET_1[4];
  }
  if(settings.wideVane!= currentSettings.wideVane) {
    packet[18] = WIDEVANE[lookupByteMapIndex(WIDEVANE_MAP, 7, settings.wideVane)];
    packet[7] += CONTROL_PACKET_2[0];
  }
  // add the checksum
  byte chkSum = checkSum(packet, 21);
  packet[21] = chkSum;
}

void HeatPump::createInfoPacket(byte *packet, byte packetType) {
  // add the header to the packet
  for (int i = 0; i < INFOHEADER_LEN; i++) {
    packet[i] = INFOHEADER[i];
  }

  // set the mode - settings or room temperature
  if(packetType != PACKET_TYPE_DEFAULT) {
    packet[5] = INFOMODE[packetType];
  } else {
    // request current infoMode, and increment for the next request
    packet[5] = INFOMODE[infoMode];
    if(infoMode == (INFOMODE_LEN - 1)) {
      infoMode = 0;
    } else {
      infoMode++;
    }
  }

  // pad the packet out
  for (int i = 0; i < 15; i++) {
    packet[i + 6] = 0x00;
  }

  // add the checksum
  byte chkSum = checkSum(packet, 21);
  packet[21] = chkSum;
}

void HeatPump::writePacket(byte *packet, int length) {
  for (int i = 0; i < length; i++) {
     _HardSerial->write((uint8_t)packet[i]);
  }

  if(packetCallback) {
    packetCallback(packet, length, (char*)"packetSent");
  }
  waitForRead = true;
  lastSend = millis();
}

int HeatPump::readPacket() {
  byte header[INFOHEADER_LEN] = {};
  byte data[PACKET_LEN] = {};
  bool foundStart = false;
  int dataSum = 0;
  byte checksum = 0;
  byte dataLength = 0;

  waitForRead = false;

  if(_HardSerial->available() > 0) {
    // read until we get start byte 0xfc
    while(_HardSerial->available() > 0 && !foundStart) {
      header[0] = _HardSerial->read();
      if(header[0] == HEADER[0]) {
        foundStart = true;
        delay(100); // found that this delay increases accuracy when reading, might not be needed though
      }
    }

    if(!foundStart) {
      return RCVD_PKT_FAIL;
    }

    //read header
    for(int i=1;i<5;i++) {
      header[i] =  _HardSerial->read();
    }

    //check header
    if(header[0] == HEADER[0] && header[2] == HEADER[2] && header[3] == HEADER[3]) {
      dataLength = header[4];

      for(int i=0;i<dataLength;i++) {
        data[i] = _HardSerial->read();
      }

      // read checksum byte
      data[dataLength] = _HardSerial->read();

      // sum up the header bytes...
      for (int i = 0; i < INFOHEADER_LEN; i++) {
        dataSum += header[i];
      }

      // ...and add to that the sum of the data bytes
      for (int i = 0; i < dataLength; i++) {
        dataSum += data[i];
      }

      // calculate checksum
      checksum = (0xfc - dataSum) & 0xff;

      if(data[dataLength] == checksum) {
        lastRecv = millis();
        if(packetCallback) {
          byte packet[37]; // we are going to put header[5] and data[32] into this, so the whole packet is sent to the callback
          for(int i=0; i<INFOHEADER_LEN; i++) {
            packet[i] = header[i];
          }
          for(int i=0; i<(dataLength+1); i++) { //must be dataLength+1 to pick up checksum byte
            packet[(i+5)] = data[i];
          }
          packetCallback(packet, PACKET_LEN, (char*)"packetRecv");
        }

        if(header[1] == 0x62) {
          switch(data[0]) {
            case 0x02: { // setting information
              heatpumpSettings receivedSettings;
              receivedSettings.power       = lookupByteMapValue(POWER_MAP, POWER, 2, data[3]);
              receivedSettings.iSee = data[4] > 0x08 ? true : false;
              receivedSettings.mode = lookupByteMapValue(MODE_MAP, MODE, 5, receivedSettings.iSee  ? (data[4] - 0x08) : data[4]);

              if(data[11] != 0x00) {
                int temp = data[11];
                temp -= 128;
                receivedSettings.temperature = (float)temp / 2;
                tempMode =  true;
              } else {
                receivedSettings.temperature = lookupByteMapValue(TEMP_MAP, TEMP, 16, data[5]);
              }

              receivedSettings.fan         = lookupByteMapValue(FAN_MAP, FAN, 6, data[6]);
              receivedSettings.vane        = lookupByteMapValue(VANE_MAP, VANE, 7, data[7]);
              receivedSettings.wideVane    = lookupByteMapValue(WIDEVANE_MAP, WIDEVANE, 7, data[10] & 0x0F);
		    wideVaneAdj = (data[10] & 0xF0) == 0x80 ? true : false;

              if(settingsChangedCallback && receivedSettings != currentSettings) {
                currentSettings = receivedSettings;
                settingsChangedCallback();
              } else {
                currentSettings = receivedSettings;
              }

              // if this is the first time we have synced with the heatpump, set wantedSettings to receivedSettings
              if(firstRun || (autoUpdate && externalUpdate)) {
                wantedSettings = currentSettings;
                firstRun = false;
              }

              return RCVD_PKT_SETTINGS;
            }

            case 0x03: { //Room temperature reading
              heatpumpStatus receivedStatus;

              if(data[6] != 0x00) {
                int temp = data[6];
                temp -= 128;
                receivedStatus.roomTemperature = (float)temp / 2;
              } else {
                receivedStatus.roomTemperature = lookupByteMapValue(ROOM_TEMP_MAP, ROOM_TEMP, 32, data[3]);
              }

              if((statusChangedCallback || roomTempChangedCallback) && currentStatus.roomTemperature != receivedStatus.roomTemperature) {
                currentStatus.roomTemperature = receivedStatus.roomTemperature;

                if(statusChangedCallback) {
                  statusChangedCallback(currentStatus);
                }

                if(roomTempChangedCallback) { // this should be deprecated - statusChangedCallback covers it
                  roomTempChangedCallback(currentStatus.roomTemperature);
                }
              } else {
                currentStatus.roomTemperature = receivedStatus.roomTemperature;
              }

              return RCVD_PKT_ROOM_TEMP;
            }

            case 0x04: { // unknown
                break;
            }

            case 0x05: { // timer packet
              heatpumpTimers receivedTimers;

              receivedTimers.mode                = lookupByteMapValue(TIMER_MODE_MAP, TIMER_MODE, 4, data[3]);
              receivedTimers.onMinutesSet        = data[4] * TIMER_INCREMENT_MINUTES;
              receivedTimers.onMinutesRemaining  = data[6] * TIMER_INCREMENT_MINUTES;
              receivedTimers.offMinutesSet       = data[5] * TIMER_INCREMENT_MINUTES;
              receivedTimers.offMinutesRemaining = data[7] * TIMER_INCREMENT_MINUTES;

              // callback for status change
              if(statusChangedCallback && currentStatus.timers != receivedTimers) {
                currentStatus.timers = receivedTimers;
                statusChangedCallback(currentStatus);
              } else {
                currentStatus.timers = receivedTimers;
              }

              return RCVD_PKT_TIMER;
            }

            case 0x06: { // status
              heatpumpStatus receivedStatus;
              receivedStatus.operating = data[4];
              receivedStatus.compressorFrequency = data[3];

              // callback for status change -- not triggered for compressor frequency at the moment
              if(statusChangedCallback && currentStatus.operating != receivedStatus.operating) {
                currentStatus.operating = receivedStatus.operating;
                currentStatus.compressorFrequency = receivedStatus.compressorFrequency;
                statusChangedCallback(currentStatus);
              } else {
                currentStatus.operating = receivedStatus.operating;
                currentStatus.compressorFrequency = receivedStatus.compressorFrequency;
              }

              return RCVD_PKT_STATUS;
            }

            case 0x09: { // standby mode maybe?
              break;
            }
          }
        }

        if(header[1] == 0x61) { //Last update was successful
          return RCVD_PKT_UPDATE_SUCCESS;
        } else if(header[1] == 0x7a) { //Last update was successful
          connected = true;
          return RCVD_PKT_CONNECT_SUCCESS;
        }
      }
    }
  }

  return RCVD_PKT_FAIL;
}
