/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MODBUS manager class
// This class implements a singleton administration object for Modbus devices & links.
// It is part of the Modbus facilities supporting multiple Modbus devices on multiple serial Modbus links.
// It associates Modbus devices with Modbus links and manages their lifecycle.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS_FAC

# include <ESPeasySerial.h>
# include "Modbus_mgr.h"

# define MODBUS_DEBUG
# ifdef BUILD_NO_DEBUG
#  undef MODBUS_DEBUG // Debugging switched off
# endif // ifdef BUILD_NO_DEBUG

#define MODBUS_MAX_BAUDRATE_SEL 8

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Singleton instance of the Modbus manager
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusMGR_struct_t ModbusMGR_singleton = {};

int modbus_storageValueToBaudrate(uint8_t baudrate_setting);
uint8_t modbus_baudrateToStorageValue(int baudrate);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor of the Modbus manager class, should not be called as this is intended to be a singleton
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusMGR_struct::~ModbusMGR_struct()
{
  // This class is a singleton, so destructor should not be called.
  // However, in case it is called, we clean up the allocated resources.
  // Note that deletion of a device will also delete the associated  link if no other device is using it.
  for (int i = 0; i < MAX_MODBUS_DEVICES; i++) {
    if (_modbus_devices[i] != nullptr) {
      delete _modbus_devices[i];
      _modbus_devices[i] = nullptr;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Connect a Modbus device to a Modbus link with the given parameters. The link will be created if it does not exist yet.
// The function returns true if the connection was successful.
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
// Connect a Modbus device to a Modbus link with the given parameters. The link will be created if it does not exist yet.
// The function returns true if the connection was successful.
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

  if (linkInfoPtr != nullptr) {         // Sanity check for successful link admin creation
    if (linkInfoPtr->link == nullptr) { // Check if link object already exists
      // No existing link, create a new one
      linkInfoPtr->link = new (std::nothrow) ModbusLINK_struct();
    }

    if (linkInfoPtr->link != nullptr) { // Sanity check for successful creation
      // (re)initialize the new link
      if (!linkInfoPtr->link->init(port, serial_rx, serial_tx, baudrate, dere_pin, collision_detect)) {
        // Initialization failed, clean up
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

  for (int i = 0; i < MAX_MODBUS_DEVICES; i++) {
    if (_modbus_devices[i] == nullptr) {
      // Found an available device slot
      _modbus_devices[i]           = new (std::nothrow) ModbusDeviceInfo_struct();
      _modbus_devices[i]->deviceID = i + 1; // Assign a unique device ID (1-MAX_MODBUS_DEVICES)
      _modbus_devices[i]->link     = linkInfoPtr;
      *deviceID                    = _modbus_devices[i]->deviceID;
      *link                        = linkInfoPtr->link;
      # ifdef MODBUS_DEBUG
      log += F(" Assigned deviceID= ");
      log += *deviceID;
      # endif // ifdef MODBUS_DEBUG
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
// Disconnect the Modbus device with the given device ID.
// If no other devices are using the same link, the link is also deleted.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::disconnect(uint8_t deviceID) {
  dumpAdminInfo();
  # ifdef MODBUS_DEBUG
  String log = F("Modbus: Manager, Disconnect device=");

  log += deviceID;
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
// This function should be called periodically to allow the Modbus manager to process the Modbus links
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusMGR_struct::processLinks()
{
  ModbusLinkInfo_struct   *linkInfoPtr   = nullptr;
  for (int i = 0; i < MAX_MODBUS_LINKS; i++) {
    if (_modbus_links[i] != nullptr)  {
      linkInfoPtr = _modbus_links[i];
      linkInfoPtr->link->processCommand(); // Trigger processing of the command queue on the link
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dump the Modbus manager admin information to the log for debugging purposes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusMGR_struct::dumpAdminInfo()
{
  addLogMove(LOG_LEVEL_INFO, F("Modbus: Manager, Dumping admin info"));
  # ifdef MODBUS_DEBUG

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
  # endif // ifdef MODBUS_DEBUG
}

void ModbusMGR_struct::show_modbus_interfaces()
{
  String options_baudrate[MODBUS_MAX_BAUDRATE_SEL]; // Array to hold the baudrate options for the selector
  String options_port[static_cast<size_t>(ESPEasySerialPort::MAX_SERIAL_TYPE)]; // Port otions for the selector, only valid ports will be filled.
  int portMap[static_cast<size_t>(ESPEasySerialPort::MAX_SERIAL_TYPE)];  // Map to keep track of valid ports and their indices in the options_port array

  constexpr size_t optionBaudCount = NR_ELEMENTS(options_baudrate);
  for (int i = 0; i < optionBaudCount; ++i) {
    options_baudrate[i] = modbus_storageValueToBaudrate(i);
  }
  const FormSelectorOptions baudselector(optionBaudCount, options_baudrate);

  int optionPortCount = 1;
  options_port[0] = F("Not set");
  portMap[0] = 0; // Map the "Not set" option to index 0
  for (int i = 1; i < NR_ELEMENTS(options_port); i++) {
    if (validSerialPort(static_cast<ESPEasySerialPort>(i))) {
      options_port[optionPortCount] = ESPEasySerialPort_toString(static_cast<ESPEasySerialPort>(i));
      portMap[i] = optionPortCount; // Store the index of the valid port in the options_port array
      optionPortCount++;
    }
    else {
      options_port[i] = F("Invalid");
      portMap[i] = 0; // Map invalid/unused ports to the "Not set" option
    }
  }
  const FormSelectorOptions portSelector(optionPortCount, options_port);
  for (int i =0; i < NR_ELEMENTS(options_port); i++) {
    addLogMove(LOG_LEVEL_INFO,
            strformat(F("Modbus_mgr: portMap[%d]= %d Port=%s"),
                      i,
                      portMap[i],
                      options_port[portMap[i]].c_str()
                      ));
  }

  // Iterate over the modbus links and show their configuration on the web page
  for (int link = 0; link < MAX_MODBUS_LINKS; ++link)
  {
    if (_modbus_links[link] != nullptr) { 
      addFormSubHeader(strformat(F("Modbus %u"), link));

      int idx = static_cast<int>(_modbus_links[link]->port);
      portSelector.addFormSelector(F("Port"), strformat(F("MBport%u"), link), portMap[idx]);
      
      String id = strformat(F("MBtx%u"), link);
      addRowLabel_tr_id(formatGpioName_serialTX(false), id);
      addPinSelect(PinSelectPurpose::Serial_input, id, _modbus_links[link]->serial_tx);

      id = strformat(F("MBrx%u"), link);
      addRowLabel_tr_id(formatGpioName_serialRX(false), id);
      addPinSelect(PinSelectPurpose::Serial_output, id, _modbus_links[link]->serial_rx);

      id = strformat(F("MBde%u"), link);
      addRowLabel_tr_id(formatGpioName_output_optional(F("~RE/DE")), id);
      addPinSelect(PinSelectPurpose::Generic_output, id, _modbus_links[link]->dere_pin);

      baudselector.addFormSelector(F("Baud Rate"), strformat(F("MBbaud%u"), link), modbus_baudrateToStorageValue(_modbus_links[link]->baudrate));
      addUnit(F("baud"));
      # ifdef ESP32
      addFormCheckBox(F("Enable Collision Detection"), strformat(F("MBcoll%u"), link), _modbus_links[link]->collision_detect);
      addFormNote(F("/RE connected to GND, only supported on hardware serial"));
      # endif // ifdef ESP32
    }
  }
}



// Convert stored baudrate setting (enumeration value) to actual baudrate value
// Returns the actual baudrate value.
int modbus_storageValueToBaudrate(uint8_t baudrate_setting) {
  int baudrate = 9600;

  switch (baudrate_setting)
  {
    case 0:
      baudrate = 1200;   break;
    case 1:
      baudrate = 2400;   break;
    case 2:
      baudrate = 4800;   break;
    case 3:
      baudrate = 9600;   break;
    case 4:
      baudrate = 19200;  break;
    case 5:
      baudrate = 38400;  break;
    case 6:
      baudrate = 57600;  break;
    case 7:
      baudrate = 115200; break;
    default:
      baudrate = 9600;   break; // Default value for fallback
  }
  return baudrate;
}

uint8_t modbus_baudrateToStorageValue(int baudrate) {
  if (baudrate <= 1200) {return 0;}
  else if (baudrate <= 2400) {return 1;}
  else if (baudrate <= 4800) {return 2;}
  else if (baudrate <= 9600) {return 3;}
  else if (baudrate <= 19200) {return 4;}
  else if (baudrate <= 38400) {return 5;}
  else if (baudrate <= 57600) {return 6;}
  else if (baudrate <= 115200) {return 7;}
  else {return 3;} // Default to 9600 baud for unsupported values
}

bool ModbusMGR_struct::save_modbus_interfaces(String &error)
{
  int portCount = 1;
  // Create mapping table from dropdown enum index to actual port identifier, index 0 is reserved for "Not set"
  int portMap[static_cast<size_t>(ESPEasySerialPort::MAX_SERIAL_TYPE)];
  portMap[0] = 0; // Map the "Not set" option to index 0
  for (int i = 1; i < NR_ELEMENTS(portMap); i++) {
    if (validSerialPort(static_cast<ESPEasySerialPort>(i))) {
      portMap[portCount++] = i; 
    }
  }

   for (int link = 0; link < MAX_MODBUS_LINKS; ++link)  {
    int port = 0;
    if (_modbus_links[link] != nullptr) { 
      if (update_whenset_FormItemInt(strformat(F("MBport%u"), link), port)) {
        _modbus_links[link]->port = static_cast<ESPEasySerialPort>(portMap[port]);
        addLogMove(LOG_LEVEL_INFO,
            strformat(F("Modbus_mgr: New Port[%d]= %d (%s)"),
                      link,
                      portMap[port],
                      ESPEasySerialPort_toString(_modbus_links[link]->port)
                      ));
      }
      int baudrate_setting = modbus_baudrateToStorageValue(_modbus_links[link]->baudrate);
      if (update_whenset_FormItemInt(strformat(F("MBbaud%u"), link), baudrate_setting)) {
        _modbus_links[link]->baudrate = modbus_storageValueToBaudrate(baudrate_setting);
      }
      int tx_setting = _modbus_links[link]->serial_tx;
      if (update_whenset_FormItemInt(strformat(F("MBtx%u"), link), tx_setting)) {
        _modbus_links[link]->serial_tx = tx_setting;
      }
      int rx_setting = _modbus_links[link]->serial_rx;
      if (update_whenset_FormItemInt(strformat(F("MBrx%u"), link), rx_setting)) {
        _modbus_links[link]->serial_rx = rx_setting;
      }
      int dere_setting = _modbus_links[link]->dere_pin;
      if (update_whenset_FormItemInt(strformat(F("MBde%u"), link), dere_setting)) {
        _modbus_links[link]->dere_pin = dere_setting;
        # ifdef ESP32
        // Checkbox existence cannot be determined from the HTML response. Assume its there when dere_setting is detected.
        // The Collision detection setting is only available on ESP32 and only when a DE/RE pin is configured.
        bool collision_detect_setting = isFormItemChecked(strformat(F("MBcoll%u"), link)); 
        _modbus_links[link]->collision_detect = collision_detect_setting; 
        # endif // ifdef ESP32
      }
    }
  }
  dumpAdminInfo();  //TODO: remove after debugging
  return false;
}

#endif // if FEATURE_MODBUS_FAC
