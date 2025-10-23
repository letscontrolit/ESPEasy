
#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS_FAC

# include <ESPeasySerial.h>
# include "Modbus_mgr.h"

//# define MODBUS_DEBUG
# ifdef BUILD_NO_DEBUG
#  undef MODBUS_DEBUG // Debugging switched off
# endif // ifdef BUILD_NO_DEBUG

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Singleton administration object for Modbus manager
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusMGR_struct_t ModbusMGR_singleton = {}; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusMGR_struct::~ModbusMGR_struct()
{
  reset();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusMGR_struct::reset()
{

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

  # ifdef MODBUS_DEBUG
  String log = F("Modbus: Manager, Connect port=");

  log += (int)port;
  # endif // ifdef MODBUS_DEBUG

  // Check if link is already used by another device
  for (int i = 0; i < MAX_MODBUS_LINKS; i++) {
    if ((_modbus_links[i] != nullptr) && (_modbus_links[i]->port == port))  {
      // Found existing link with matching port identifier
      linkInfoPtr = _modbus_links[i];
      # ifdef MODBUS_DEBUG
      log += strformat(F(", Found existing link= %d for port=%s"), i, ESPEasySerialPort_toString(_modbus_links[i]->port));
      # endif // ifdef MODBUS_DEBUG
    }
  }

  if (linkInfoPtr == nullptr) {
    linkInfoPtr = new (std::nothrow) ModbusLinkInfo_struct();

    for (int i = 0; i < MAX_MODBUS_LINKS; i++) {
      if (_modbus_links[i] == nullptr) {
        _modbus_links[i] = linkInfoPtr;
        # ifdef MODBUS_DEBUG
        log += strformat(F(", Created new link= %d for port=%s"), i, ESPEasySerialPort_toString(_modbus_links[i]->port));
        # endif // ifdef MODBUS_DEBUG
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

  for (int i = 0; i < MAX_MODBUS_DEVICES; i++) {
    if (_modbus_devices[i] == nullptr) {
      // Found an available device slot
      _modbus_devices[i]           = new (std::nothrow) ModbusDeviceInfo_struct();
      _modbus_devices[i]->deviceID = i + 1; // Assign a unique device ID (1-MAX_MODBUS_DEVICES)
      _modbus_devices[i]->link     = linkInfoPtr;
      *deviceID                    = _modbus_devices[i]->deviceID;
      *link                        = linkInfoPtr->link;
      #ifdef MODBUS_DEBUG
      log                         += F(" Assigned deviceID= ");
      log                         += *deviceID;
      #endif
      break;
    }
  }
  # ifdef MODBUS_DEBUG
  addLogMove(LOG_LEVEL_INFO, log);
  dumpAdminInfo();
  # endif // ifdef MODBUS_DEBUG
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::disconnect(uint8_t deviceID) {
  dumpAdminInfo();
  # ifdef MODBUS_DEBUG
  String log = F("Modbus: Manager, Disconnect device=");

  log += (int)deviceID;
  # endif // ifdef MODBUS_DEBUG

  for (int i = 0; i < MAX_MODBUS_DEVICES; i++) {
    if ((_modbus_devices[i] != nullptr) && (_modbus_devices[i]->deviceID == deviceID)) {
      // Found the device to disconnect
      ModbusLinkInfo_struct *linkInfoPtr = _modbus_devices[i]->link;

      // Remove the device entry
      _modbus_devices[i] = nullptr;

      // Check if any other devices are using the same link
      bool linkInUse = false;

      for (int j = 0; j < MAX_MODBUS_DEVICES; j++) {
        if ((_modbus_devices[j] != nullptr) && (_modbus_devices[j]->link == linkInfoPtr)) {
          linkInUse = true;
          break;
        }
      }

      if (!linkInUse) {
        # ifdef MODBUS_DEBUG
        log += F(", No other devices using link, deleting link");
        # endif // ifdef MODBUS_DEBUG

        // No other devices are using this link, so we can delete it
        for (int k = 0; k < MAX_MODBUS_LINKS; k++) {
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
  # ifdef MODBUS_DEBUG
  addLogMove(LOG_LEVEL_INFO, log);
  # endif // ifdef MODBUS_DEBUG
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusMGR_struct::dumpAdminInfo()
{
  addLogMove(LOG_LEVEL_INFO, F("Modbus: Manager, Dumping admin info"));
  #ifdef MODBUS_DEBUG
  // Iterate over the modbus links and dump their info
  for (int i = 0; i < MAX_MODBUS_LINKS; i++) {
    if (_modbus_links[i] != nullptr)  {
      addLogMove(LOG_LEVEL_INFO,
                 strformat(F("Modbus_mgr: Link[%d] Port=%s, RX=%d, TX=%d, Baudrate=%d, DerePin=%d, RS485Mode=%s, CollisionDetect=%s"),
                           i,
                           ESPEasySerialPort_toString(_modbus_links[i]->port),
                           _modbus_links[i]->serial_rx,
                           _modbus_links[i]->serial_tx,
                           _modbus_links[i]->baudrate,
                           _modbus_links[i]->dere_pin,
                           _modbus_links[i]->rs485_mode ? F("Yes") : F("No"),
                           _modbus_links[i]->collision_detect ? F("Yes") : F("No")
                           ));
    }
    else {
      addLogMove(LOG_LEVEL_INFO, strformat(F("Modbus_mgr: Link[%d] <not used>"), i));
    }
  }

  // Iterate over the modbus devices and dump their info
  for (int i = 0; i < MAX_MODBUS_DEVICES; i++) {
    if (_modbus_devices[i] != nullptr)  {
      addLogMove(LOG_LEVEL_INFO,
                 strformat(F("Modbus_mgr: Device[%d] DeviceID=%d, LinkPort=%s"),
                           i,
                           _modbus_devices[i]->deviceID,
                           ESPEasySerialPort_toString(_modbus_devices[i]->link->port)
                           ));
    }
    else {
      addLogMove(LOG_LEVEL_INFO, strformat(F("Modbus_mgr: Device[%d] <not used>"), i));
    }
  }
  #endif // MODBUS_DEBUG
}

#endif // FEATURE_MODBUS_FAC
