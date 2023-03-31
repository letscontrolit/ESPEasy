// ----------------------------------------------------------------------------
// P144 "Dust - PM1006(K) (Vindriktning)"
// Implementation of sensor abstraction
// 2022 By flashmark
// ----------------------------------------------------------------------------
#include "../PluginStructs/P144_data_struct.h"

#ifdef USES_P144
#include <ESPeasySerial.h>

// ----------------------------------------------------------------------------
// Function setSerial
// Connect the serial port used for the sensor
// The ESPeasySerial class is used to handle the connection
// ----------------------------------------------------------------------------
bool P144_data_struct::setSerial(ESPEasySerialPort portType, int8_t rxPin, int8_t txPin) 
{
  bool success = false;

  if (easySerial != nullptr)
  {
    delete easySerial;
  }
  // Try to open the assocaited serial port
  easySerial = new (std::nothrow) ESPeasySerial(portType, rxPin, txPin);
  if (easySerial != nullptr) 
  {
    easySerial->begin(9600);
    success = true;
  }

  #ifdef PLUGIN_144_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log = F("P144 : Init: ");
    if (success)
    {
      log += F("  ESP GPIO-pin RX:");
      log += rxPin;
      log += F(" TX:");
      log += txPin;  
    }
    else
    {
      log += F("Failed opening serial port");
    }
    addLogMove(LOG_LEVEL_INFO, log);
  }
  #endif
  return(success);
}

// ----------------------------------------------------------------------------
// Function exit
// Detaches the serial port
// ----------------------------------------------------------------------------
void P144_data_struct::disconnectSerial() 
{
  if (easySerial != nullptr) 
  {
    delete(easySerial);
    easySerial = nullptr;
  }
}

// ----------------------------------------------------------------------------
// Function processSensor
// This function must be called regularly to process the serial data received from the sensor
// It relies on processing serial RX characters fast enough to prever overrunning the buffer
// Function stops processing once the end of a message is reached, leaving remaining 
// characters to be processed next call.
// If the message contains sensor values these will be updated (pm25Value)
// ----------------------------------------------------------------------------
bool P144_data_struct::processSensor() {
  bool new_data = false;
  for (int charAvailable = easySerial->available(); ((charAvailable > 0) && !new_data); charAvailable--)
    {
      // Process received characters, return true when a complete message is received 
      // Message rate on Vindriktning is a few per minute
      new_data = processRx(easySerial->read());
      if (new_data) 
      {
        pm25Value = float((serialRxBuffer[3] << 8) + serialRxBuffer[4]);
        #ifdef PLUGIN_144_DEBUG
        if (loglevelActiveFor(LOG_LEVEL_INFO))
        {
          String log = F("P144 : New value received ");
          log += pm25Value;
          addLogMove(LOG_LEVEL_INFO, log);
        }
        #endif
      }
    }
    return(true);
}

// ----------------------------------------------------------------------------
// Function getValue
// Return the last received value of the sensor as stored in pm25Value
// ----------------------------------------------------------------------------
int P144_data_struct::getValue() 
{
  return(pm25Value);
}

// ----------------------------------------------------------------------------
// Function processRx
// Handles a single character received from the PM1006 in the Vindrikning
// It expects a response to the poll message sent by the Vindriknings own CPU
// 0x16 <len> <data> <chksum>
// <len>  = size of <data> in char [1 char]
// <data> = message payload, array of <len> chars
// <csum> = checksum, 256-(sum of all received characters) [1 char]
// ----------------------------------------------------------------------------
bool P144_data_struct::processRx(char c)
{
  switch (rxState) {
  case PM1006_HEADER:
    // Waiting for the expected message header (0x16)
    rxChecksum = c;
    if (c == 0x16) {
      rxState = PM1006_LENGTH;
    }
    break;

  case PM1006_LENGTH:
    // Waiting for the message length
    rxChecksum += c;
    if (c <= P144_bufferSize) {
      rxlen = c;
      rxIndex = 0;
      rxState = (rxlen > 0) ? PM1006_DATA : PM1006_CHECK;
    } 
    else 
    {
      rxState = PM1006_HEADER;
    }
    break;

  case PM1006_DATA:
    // Receiving the data part of the message
    rxChecksum += c;
    serialRxBuffer[rxIndex++] = c;
    if (rxIndex == rxlen) {
      rxState = PM1006_CHECK;
    }
    break;

  case PM1006_CHECK:
    // Waiting for the checksum of the message
    rxChecksum += c;
    rxState = PM1006_HEADER;
#ifdef PLUGIN_144_DEBUG
    dump();
#endif
    return ((rxChecksum & 0xFF) == 0);

  default:
    // Unexpected state, reset statemachine trashing pending data
    rxState = PM1006_HEADER;
    break;
  }
  return false;
}

#ifdef PLUGIN_144_DEBUG
// ----------------------------------------------------------------------------
// Function toHex
// Helper to convert uint8/char to HEX representation
// ----------------------------------------------------------------------------
char * P144_data_struct::toHex(char c, char * ptr)
{
  static const char hex[] = "0123456789ABCDEF";
  *ptr++ = hex[c>>4];
  *ptr++ = hex[c&0x7];
  return ptr;
}

// ----------------------------------------------------------------------------
// Function dump
// Dump the received buffer
// Note: Contents may be inconsistent unless alingned with the decoder
// ----------------------------------------------------------------------------
void P144_data_struct::dump()
{
  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log = F("P144 : Dump message: ");
    char *ptr = debugBuffer;
    for (int n=0; n< rxlen; n++)
    {
      ptr = toHex(serialRxBuffer[n], ptr);
      *ptr++ = ' ';
    }
    *ptr++ = '\0';
    log += debugBuffer;
    log += " size ";
    log += rxlen;
    log += " csum ";
    log += (rxChecksum & 0xFF);
    addLogMove(LOG_LEVEL_INFO, log);
  }
}
#endif // PLUGIN_144_DEBUG

#endif // USES_P144