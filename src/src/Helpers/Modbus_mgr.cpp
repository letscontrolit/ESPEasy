
#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS_FAC

# include <ESPeasySerial.h>
# include "Modbus_mgr.h"


// ModbusMGR structure representing the singleton Modbus Management entity
// Thw manager has an overview of all Modbus links and the conneted devices.
// The manager allows multiple Modbus devices to connect to a single Modbus link while supporting multiple links.
// The modbus manager is not involved in the actual data transport, this is handled by a direct relation between Modbus device and
// ModbusLINK object.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusMGR_struct::~ModbusMGR_struct()
{
  reset();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusMGR_struct::reset()
{
  //////_modbus_link->;
  //////delete _modbus_link;
  /////_modbus_link = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::connect(const ESPEasySerialPort port,
                               const int16_t           serial_rx,
                               const int16_t           serial_tx,
                               int16_t                 baudrate,
                               ModbusLINK_struct     **link,
                               uint8_t                *deviceID) {
  return connect(port, serial_rx, serial_tx, baudrate, -1, false, link, deviceID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::connect(const ESPEasySerialPort port,
                               const int16_t           serial_rx,
                               const int16_t           serial_tx,
                               int16_t                 baudrate,
                               int8_t                  dere_pin,
                               bool                    collision_detect,
                               ModbusLINK_struct     **link,
                               uint8_t                *deviceID) {
  ModbusLinkInfo_struct   *linkInfoPtr   = nullptr;
  ModbusDeviceInfo_struct *deviceInfoPtr = nullptr;

  String log = F("-MGR-> Connect: port=");

  log += (int)port;

  // Check if link is already used by another device
  for (int i = 0; i < 5; i++) {
    if ((_modbus_links[i] != nullptr) && (_modbus_links[i]->port == port))  {
      // Found existing link with matching port identifier
      linkInfoPtr = _modbus_links[i];
      log        += F(" Found existing link= ");
      log        += i;
    }
  }

  if (linkInfoPtr == nullptr) {
    linkInfoPtr = new (std::nothrow) ModbusLinkInfo_struct();

    for (int i = 0; i < 5; i++) {
      if (_modbus_links[i] == nullptr) {
        _modbus_links[i] = linkInfoPtr;
        log             += F(" Created new link= ");
        log             += i;
        break;
      }
    }
  }

  if (linkInfoPtr != nullptr) {
    if (linkInfoPtr->link == nullptr) {
      // No existing link, create a new one
      linkInfoPtr->link = new (std::nothrow) ModbusLINK_struct();

      if (linkInfoPtr->link != nullptr) {
        // Initialize the new link
        if (!linkInfoPtr->link->init(port, serial_rx, serial_tx, baudrate, dere_pin, collision_detect)) {
          delete linkInfoPtr->link;
          linkInfoPtr->link = nullptr;
          delete linkInfoPtr;
          linkInfoPtr = nullptr;
          return false; // Initialization failed
        }
        else {
          // Store the link parameters
          linkInfoPtr->port             = port;
          linkInfoPtr->serial_rx        = serial_rx;
          linkInfoPtr->serial_tx        = serial_tx;
          linkInfoPtr->baudrate         = baudrate;
          linkInfoPtr->dere_pin         = dere_pin;
          linkInfoPtr->rs485_mode       = (dere_pin != -1);
          linkInfoPtr->collision_detect = collision_detect;
        }
      }
    }
  }

  for (int i = 0; i < 16; i++) {
    if (_modbus_devices[i] == nullptr) {
      // Found an available device slot
      _modbus_devices[i]           = new (std::nothrow) ModbusDeviceInfo_struct();
      _modbus_devices[i]->deviceID = i + 1; // Assign a unique device ID (1-16)
      _modbus_devices[i]->link     = linkInfoPtr;
      *deviceID                    = _modbus_devices[i]->deviceID;
      *link                        = linkInfoPtr->link;
      log                         += F(" Assigned deviceID= ");
      log                         += *deviceID;
      break;
    }
  }
  addLogMove(LOG_LEVEL_INFO, log);
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::disconnect(uint8_t deviceID) {
  for (int i = 0; i < 16; i++) {
    if ((_modbus_devices[i] != nullptr) && (_modbus_devices[i]->deviceID == deviceID)) {
      // Found the device to disconnect
      ModbusLinkInfo_struct *linkInfoPtr = _modbus_devices[i]->link;

      // Remove the device entry
      _modbus_devices[i] = nullptr;

      // Check if any other devices are using the same link
      bool linkInUse = false;

      for (int j = 0; j < 16; j++) {
        if ((_modbus_devices[j] != nullptr) && (_modbus_devices[j]->link == linkInfoPtr)) {
          linkInUse = true;
          break;
        }
      }

      if (!linkInUse) {
        // No other devices are using this link, so we can delete it
        for (int k = 0; k < 5; k++) {
          if (_modbus_links[k] == linkInfoPtr) {
            delete _modbus_links[k];
            _modbus_links[k] = nullptr;
            break;
          }
        }
      }

      return true; // Successfully disconnected
    }
  }

  return false; // TODO: implement
}

#endif // FEAURE_MODBUS
