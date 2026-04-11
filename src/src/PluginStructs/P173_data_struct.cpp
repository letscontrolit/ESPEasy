#include "../PluginStructs/P173_data_struct.h"

#ifdef USES_P173

# include "../Helpers/CRC_functions.h"

/**************************************************************************
* Constructor
**************************************************************************/
P173_data_struct::P173_data_struct(float tempOffset, bool lowPower) :
  _tempOffset(tempOffset), _lowPower(lowPower)
{}

bool P173_data_struct::init() {
  wakeup();
  softwareReset();
  delay(1);

  // Read sensor device id
  if (checkDeviceID()) {
    sleep();
    initialized = true;
  } else {
    addLog(LOG_LEVEL_ERROR, F("SHTC3: Sensor not recognized."));
  }

  return isInitialized();
}

/*****************************************************
* plugin_read
*****************************************************/
bool P173_data_struct::plugin_read(struct EventStruct *event)           {
  bool success = false;

  if (isInitialized()) {
    // Wake up the sensor
    wakeup();

    // Read the sensor data
    uint8_t data[6]{};
    const uint16_t command = _lowPower ? P173_SHTC3_READ_RH_T_LP : P173_SHTC3_READ_RH_T;

    if (!readValue(command, 6, data)) {
      addLog(LOG_LEVEL_ERROR, strformat(F("%s: Error reading sensor"), getTaskDeviceName(event->TaskIndex).c_str()));
    } else

    // Read succesful, data valid?
    if (calc_CRC8(data[0], data[1], data[2]) && calc_CRC8(data[3], data[4], data[5])) {
      const float temp = static_cast<float>(((uint16_t)data[0] << 8) | (uint16_t)data[1]);
      const float hum  = static_cast<float>(((uint16_t)data[3] << 8) | (uint16_t)data[4]);
      temperature = -45.0f + 175.0f * temp / 65535.0f;
      humidity    = 100.0f * hum / 65535.0f;

      if (definitelyLessThan(humidity, 0.0f)) { humidity = 0.0f; }

      if (definitelyGreaterThan(humidity, 100.0f)) { humidity = 100.0f; }

      UserVar.setFloat(event->TaskIndex, 0, temperature + _tempOffset); // Apply offset
      UserVar.setFloat(event->TaskIndex, 1, humidity);

      success    = true;
      errorCount = 0;

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, strformat(F("%s: Temperature: %s, Humidity: %s"),
                                         getTaskDeviceName(event->TaskIndex).c_str(),
                                         formatUserVarNoCheck(event, 0).c_str(),
                                         formatUserVarNoCheck(event, 1).c_str()));
      }
    } else {
      addLog(LOG_LEVEL_ERROR, concat(F("SHTC3: READ CRC Error, data: 0x"), formatToHex_array(data, 6)));
      errorCount++;

      if (errorCount > P173_MAX_ERRORCOUNT) {
        softwareReset();
        delay(1);
        addLog(LOG_LEVEL_ERROR, F("SHTC3: READ Error count reached, sensor reset."));
      }
    }

    if (!success) {
      UserVar.setFloat(event->TaskIndex, 0, NAN); // Read error or I/O error
      UserVar.setFloat(event->TaskIndex, 1, NAN);
    }

    // Send sensor to sleep mode for least power consumption
    sleep();
  }

  return success;
}

/********************************
 * Low level functions
 *******************************/
void P173_data_struct::softwareReset() {
  writeCommand(P173_SHTC3_RESET);
  delayMicroseconds(173);
}

void P173_data_struct::wakeup() {
  writeCommand(P173_SHTC3_WAKEUP);
  delayMicroseconds(500);
}

void P173_data_struct::sleep() {
  writeCommand(P173_SHTC3_SLEEP);
  delayMicroseconds(230);
}

bool P173_data_struct::checkDeviceID() {
  uint8_t idArray[3];

  readValue(P173_SHTC3_READ_DEVICEID, 3, idArray);

  if (calc_CRC8(idArray[0], idArray[1], idArray[2])) {
    const uint16_t id = (idArray[0] << 8) | idArray[1];

    if ((id & 0x807) == 0x807) {
      return true;
    }
  }
  return false;
}

void P173_data_struct::writeCommand(uint16_t command) {
  I2C_write16(_address, command);
}

bool P173_data_struct::readValue(uint16_t command, uint8_t readnum, uint8_t *readArray) {
  I2C_write16(_address, command);
  delayMicroseconds(12000);

  if (Wire.requestFrom(_address, readnum) == readnum) {
    for (uint8_t i = 0; i < readnum; ++i) {
      readArray[i] = Wire.read();
    }
    return true;
  }
  return false;
}

#endif // ifdef USES_P173
