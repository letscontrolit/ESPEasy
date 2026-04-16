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

# define MODBUS_MAX_BAUDRATE_SEL 8

// Key indices for storing Modbus link settings in the key-value store
# define MODBUS_PORT_KEY_INDEX                   1
# define MODBUS_RX_KEY_INDEX                     2
# define MODBUS_TX_KEY_INDEX                     3
# define MODBUS_BAUDRATE_KEY_INDEX               4
# define MODBUS_DERE_PIN_KEY_INDEX               5
# define MODBUS_COLLISION_DETECT_KEY_INDEX       6


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Singleton instance of the Modbus manager
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusMGR_struct_t ModbusMGR_singleton = {};

int     modbus_storageValueToBaudrate(uint8_t baudrate_setting);
uint8_t modbus_baudrateToStorageValue(int baudrate);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor of the Modbus manager class, initializes the internal data structures
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusMGR_struct::ModbusMGR_struct()
{
  # ifdef MODBUS_DEBUG
  String log = F("Modbus: Manager, Constructor ");
  # endif // ifdef MODBUS_DEBUG

  for (int i = 0; i < MAX_MODBUS_LINKS; i++) {
    _modbus_links[i] = nullptr;
  }

  for (int i = 0; i < MAX_MODBUS_DEVICES; i++) {
    _modbus_devices[i] = nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor of the Modbus manager class, should not be called as this is intended to be a singleton
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusMGR_struct::~ModbusMGR_struct()
{
  // This class is a singleton, so destructor should not be called.
  // However, in case it is called, we clean up the allocated resources.

  for (int i = 0; i < MAX_MODBUS_DEVICES; i++) {
    if (_modbus_devices[i] != nullptr) {
      delete _modbus_devices[i];
      _modbus_devices[i] = nullptr;
    }
  }

  for (int i = 0; i < MAX_MODBUS_LINKS; i++) {
    if (_modbus_links[i] != nullptr) {
      if (_modbus_links[i]->link != nullptr) {
        delete _modbus_links[i]->link;
        _modbus_links[i]->link = nullptr;
      }

      if (_modbus_links[i]->kvs != nullptr) {
        delete _modbus_links[i]->kvs;
        _modbus_links[i]->kvs = nullptr;
      }
      delete _modbus_links[i];
      _modbus_links[i] = nullptr;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize the Modbus manager link administration.
// This will read the persisted data for each link and create the link objects for the links that are configured.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::initialize()
{
  for (int i = 0; i < MAX_MODBUS_LINKS; i++) {
    if (_modbus_links[i] == nullptr) {
      int8_t val;
      _modbus_links[i]       = new (std::nothrow) ModbusLinkInfo_struct();
      _modbus_links[i]->link = nullptr;
      _modbus_links[i]->kvs  = new (std::nothrow) ESPEasy_key_value_store;
      _modbus_links[i]->kvs->load(SettingsType::Enum::ModbusInterfaceSettings_Type, i, 0, 0);
      # ifdef MODBUS_DEBUG
      _modbus_links[i]->kvs->dump();
      # endif
      _modbus_links[i]->kvs->getValue(MODBUS_PORT_KEY_INDEX, val);
      _modbus_links[i]->port = static_cast<ESPEasySerialPort>(val);
      _modbus_links[i]->kvs->getValue(MODBUS_RX_KEY_INDEX, _modbus_links[i]->serial_rx);
      _modbus_links[i]->kvs->getValue(MODBUS_TX_KEY_INDEX, _modbus_links[i]->serial_tx);
      _modbus_links[i]->kvs->getValue(MODBUS_BAUDRATE_KEY_INDEX, _modbus_links[i]->baudrate);
      _modbus_links[i]->kvs->getValue(MODBUS_DERE_PIN_KEY_INDEX, _modbus_links[i]->dere_pin);
      _modbus_links[i]->kvs->getValue(MODBUS_COLLISION_DETECT_KEY_INDEX, _modbus_links[i]->collision_detect);

      if (_modbus_links[i]->port != ESPEasySerialPort::not_set) {
        _modbus_links[i]->link = new (std::nothrow) ModbusLINK_struct();
        _modbus_links[i]->link->init(_modbus_links[i]->port,
                                     _modbus_links[i]->serial_rx,
                                     _modbus_links[i]->serial_tx,
                                     modbus_storageValueToBaudrate(_modbus_links[i]->baudrate),
                                     _modbus_links[i]->dere_pin,
                                     _modbus_links[i]->collision_detect);
      }
    }
  }

  # ifdef MODBUS_DEBUG
  dumpAdminInfo();
  # endif // ifdef MODBUS_DEBUG

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Connect a Modbus device to a Modbus link. A unique device ID is assigned to the device.
// Returns a pointer to the Modbus link object and the assigned device ID if connection is successful, otherwise returns false.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::connect(int linkId, ModbusLINK_struct **link, uint8_t *deviceID)
{
  # ifdef MODBUS_DEBUG
  String log = F("Modbus: Manager, Connect linkId=");
  log += linkId;
  # endif // ifdef MODBUS_DEBUG

  initialize(); // TODO Initialization sequence to be refactored.

  if ((linkId < 0) || (linkId >= MAX_MODBUS_LINKS)) {
    # ifdef MODBUS_DEBUG
    log += F("Invalid linkId");
    addLogMove(LOG_LEVEL_ERROR, log);
    # endif // ifdef MODBUS_DEBUG
    return false;
  }

  *deviceID = -1; // Default to -1, will be set to a valid device ID if connection is successful
  *link     = nullptr;

  if (_modbus_links[linkId] != nullptr) {
    for (int i = 0; i < MAX_MODBUS_DEVICES; i++) {
      if (_modbus_devices[i] == nullptr) {
        // Found an available device slot
        _modbus_devices[i]           = new (std::nothrow) ModbusDeviceInfo_struct();
        _modbus_devices[i]->deviceID = i + 1; // Assign a unique device ID (1-MAX_MODBUS_DEVICES)
        _modbus_devices[i]->link     = _modbus_links[linkId];
        *deviceID                    = _modbus_devices[i]->deviceID;
        *link                        = _modbus_devices[i]->link->link;
        # ifdef MODBUS_DEBUG
        log += F(" Assigned deviceID= ");
        log += *deviceID;
        # endif // ifdef MODBUS_DEBUG
        break;
      }
    }
  }
  else {
    # ifdef MODBUS_DEBUG
    log += F(" No link available at linkIndex= ");
    log += linkId;
    addLogMove(LOG_LEVEL_ERROR, log);
    # endif // ifdef MODBUS_DEBUG
    return false;
  }

  if (*deviceID == -1) {
    // No available device slot found, connection failed
    # ifdef MODBUS_DEBUG
    log += F(" Failed to assign device ID, no available device slots");
    addLogMove(LOG_LEVEL_ERROR, log);
    # endif // ifdef MODBUS_DEBUG
    return false;
  }

  # ifdef MODBUS_DEBUG
  addLogMove(LOG_LEVEL_INFO, log);
  dumpAdminInfo();
  # endif // ifdef MODBUS_DEBUG

  return false;
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
      delete _modbus_devices[i];
      _modbus_devices[i] = nullptr;

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
  ModbusLinkInfo_struct *linkInfoPtr = nullptr;

  for (int i = 0; i < MAX_MODBUS_LINKS; i++) {
    if (_modbus_links[i] != nullptr)  {
      linkInfoPtr = _modbus_links[i];

      /////////////     linkInfoPtr->link->processCommand(); // Trigger processing of the command queue on the link
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Put the Modbus link configuration on the web page
// This is called from the interfaces configuration page to show the Modbus link configuration.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusMGR_struct::show_modbus_interfaces()
{
  String options_baudrate[MODBUS_MAX_BAUDRATE_SEL];                             // Array to hold the baudrate options for the selector
  String options_port[static_cast<size_t>(ESPEasySerialPort::MAX_SERIAL_TYPE)]; // Port otions for the selector, only valid ports will be
                                                                                // filled.
  int portMap[static_cast<size_t>(ESPEasySerialPort::MAX_SERIAL_TYPE)];         // Map to keep track of valid ports and their indices in the
                                                                                // options_port array

  constexpr size_t optionBaudCount = NR_ELEMENTS(options_baudrate);

  for (int i = 0; i < optionBaudCount; ++i) {
    options_baudrate[i] = modbus_storageValueToBaudrate(i);
  }
  const FormSelectorOptions baudselector(optionBaudCount, options_baudrate);

  int optionPortCount = 1;
  options_port[0] = F("Not set");
  portMap[0]      = 0; // Map the "Not set" option to index 0

  for (int i = 1; i < NR_ELEMENTS(options_port); i++) {
    if (validSerialPort(static_cast<ESPEasySerialPort>(i))) {
      options_port[optionPortCount] = ESPEasySerialPort_toString(static_cast<ESPEasySerialPort>(i));
      portMap[i]                    = optionPortCount; // Store the index of the valid port in the options_port array
      optionPortCount++;
    }
    else {
      options_port[i] = F("Invalid");
      portMap[i]      = 0; // Map invalid/unused ports to the "Not set" option
    }
  }
  const FormSelectorOptions portSelector(optionPortCount, options_port);

  // Iterate over the modbus links and show their configuration on the web page
  for (int link = 0; link < MAX_MODBUS_LINKS; ++link)
  {
    if (_modbus_links[link] != nullptr) {
      addFormSubHeader(strformat(F("Modbus %u"), link));
      addFormDetailsStart(link == 0);

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

      baudselector.addFormSelector(F("Baud Rate"),
                                   strformat(F("MBbaud%u"), link),
                                   modbus_baudrateToStorageValue(_modbus_links[link]->baudrate));
      addUnit(F("baud"));
      
      # ifdef ESP32
      addFormCheckBox(F("Enable Collision Detection"), strformat(F("MBcoll%u"), link), _modbus_links[link]->collision_detect);
      addFormNote(F("/RE connected to GND, only supported on hardware serial"));
      # endif // ifdef ESP32

      addFormDetailsEnd();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert stored baudrate setting (enumeration value) to actual baudrate value
// Returns the actual baudrate value.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int modbus_storageValueToBaudrate(uint8_t baudrate_setting) {
  int baudrate = 9600;

  switch (baudrate_setting)
  {
    case 0:
      baudrate = 1200;
      break;
    case 1:
      baudrate = 2400;
      break;
    case 2:
      baudrate = 4800;
      break;
    case 3:
      baudrate = 9600;
      break;
    case 4:
      baudrate = 19200;
      break;
    case 5:
      baudrate = 38400;
      break;
    case 6:
      baudrate = 57600;
      break;
    case 7:
      baudrate = 115200;
      break;
    default:
      baudrate = 9600;
      break; // Default value for fallback
  }
  return baudrate;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert actual baudrate value to stored baudrate setting (enumeration value)
// Returns the stored baudrate setting.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t modbus_baudrateToStorageValue(int baudrate) {
  if (baudrate <= 1200) { return 0; }
  else if (baudrate <= 2400) { return 1; }
  else if (baudrate <= 4800) { return 2; }
  else if (baudrate <= 9600) { return 3; }
  else if (baudrate <= 19200) { return 4; }
  else if (baudrate <= 38400) { return 5; }
  else if (baudrate <= 57600) { return 6; }
  else if (baudrate <= 115200) { return 7; }
  else { return 3; } // Default to 9600 baud for unsupported values
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process the Modbus link configuration from the web page save action and update the Modbus manager configuration
// This is called from the interfaces configuration page to show the Modbus link configuration.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::save_modbus_interfaces(String& error)
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
    int  port_setting             = 0;
    int  baudrate_setting         = 0;
    int  tx_setting               = 0;
    int  rx_setting               = 0;
    int  dere_setting             = 0;
    bool collision_detect_setting = false;
    bool settingsChanged          = false;

    if (_modbus_links[link] != nullptr) {
      for (int i = 0; i < NR_ELEMENTS(portMap); i++) {
        if (portMap[i] == static_cast<int>(_modbus_links[link]->port)) {
          port_setting = i;
          break;
        }
      }

      if (update_whenset_FormItemInt(strformat(F("MBport%u"), link), port_setting)) {
        settingsChanged |= (portMap[port_setting] != static_cast<int>(_modbus_links[link]->port));
      }
      baudrate_setting = modbus_baudrateToStorageValue(_modbus_links[link]->baudrate);

      if (update_whenset_FormItemInt(strformat(F("MBbaud%u"), link), baudrate_setting)) {
        settingsChanged |= (modbus_storageValueToBaudrate(baudrate_setting) != _modbus_links[link]->baudrate);
      }
      tx_setting = _modbus_links[link]->serial_tx;

      if (update_whenset_FormItemInt(strformat(F("MBtx%u"), link), tx_setting)) {
        settingsChanged |= (tx_setting != _modbus_links[link]->serial_tx);
      }
      rx_setting = _modbus_links[link]->serial_rx;

      if (update_whenset_FormItemInt(strformat(F("MBrx%u"), link), rx_setting)) {
        settingsChanged |= (rx_setting != _modbus_links[link]->serial_rx);
      }
      dere_setting = _modbus_links[link]->dere_pin;

      if (update_whenset_FormItemInt(strformat(F("MBde%u"), link), dere_setting)) {
        settingsChanged |= (dere_setting != _modbus_links[link]->dere_pin);
        # ifdef ESP32

        // Checkbox existence cannot be determined from the HTML response. Assume its there when dere_setting is detected.
        // The Collision detection setting is only available on ESP32 and only when a DE/RE pin is configured.
        collision_detect_setting = isFormItemChecked(strformat(F("MBcoll%u"), link));
        settingsChanged         |= (collision_detect_setting != _modbus_links[link]->collision_detect);
        # endif // ifdef ESP32
      }
    }

    if (settingsChanged) {
      setLink(link,
              static_cast<ESPEasySerialPort>(portMap[port_setting]),
              rx_setting,
              tx_setting,
              modbus_storageValueToBaudrate(baudrate_setting),
              dere_setting,
              collision_detect_setting);
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup the Modbus link with the specified parameters. Settings will be persisted on disk.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::setLink(const int               linkIndex,
                               const ESPEasySerialPort port,
                               const int16_t           serial_rx,
                               const int16_t           serial_tx,
                               int16_t                 baudrate,
                               int8_t                  dere_pin,
                               bool                    collision_detect)
{
  ModbusLinkInfo_struct *linkInfoPtr = nullptr;


  # ifdef MODBUS_DEBUG
  String log = F("Modbus_mgr: SetLink. ");

  addLogMove(LOG_LEVEL_INFO,
             strformat(F("*** setlink***: LinkID=%d, Port=%s, RX=%d, TX=%d, Baudrate=%d, DerePin=%d, CollisionDetect=%s"),
                       linkIndex,
                       ESPEasySerialPort_toString(port),
                       serial_rx,
                       serial_tx,
                       baudrate,
                       dere_pin,
                       collision_detect ? F("Yes") : F("No")
                       ));
  # endif // ifdef MODBUS_DEBUG

  if ((linkIndex >= 0) && (linkIndex < MAX_MODBUS_LINKS)) {
    if (_modbus_links[linkIndex] == nullptr) {
      linkInfoPtr              = new (std::nothrow) ModbusLinkInfo_struct();
      _modbus_links[linkIndex] = linkInfoPtr;
      # ifdef MODBUS_DEBUG
      log += strformat(F("New link for linkIndex=%d"), linkIndex);
      # endif // ifdef MODBUS_DEBUG
    }
    else {
      linkInfoPtr = _modbus_links[linkIndex]; // Link admin already exists, will be reused for the new link configuration
      # ifdef MODBUS_DEBUG
      log += strformat(F("Existing link for linkIndex=%d"), linkIndex);
      # endif // ifdef MODBUS_DEBUG
    }
  }
  else {
    log += strformat(F("Invalid link for linkIndex=%d"), linkIndex);
    # ifdef MODBUS_DEBUG
    addLogMove(LOG_LEVEL_INFO, log);
    # endif // ifdef MODBUS_DEBUG
    return false;                       // Invalid link index
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
  else {
    return false; // Failed to create link admin
  }

  if (_modbus_links[linkIndex]->kvs == nullptr) {
    _modbus_links[linkIndex]->kvs = new (std::nothrow) ESPEasy_key_value_store;
  }

  if (_modbus_links[linkIndex]->kvs) {
    // Store the link configuration parameters in the key-value store for persistence
    linkInfoPtr->kvs->setValue(MODBUS_PORT_KEY_INDEX,             static_cast<int8_t>(port));
    linkInfoPtr->kvs->setValue(MODBUS_RX_KEY_INDEX,               static_cast<int8_t>(serial_rx));
    linkInfoPtr->kvs->setValue(MODBUS_TX_KEY_INDEX,               static_cast<int8_t>(serial_tx));
    linkInfoPtr->kvs->setValue(MODBUS_BAUDRATE_KEY_INDEX,         static_cast<int16_t>(baudrate));
    linkInfoPtr->kvs->setValue(MODBUS_DERE_PIN_KEY_INDEX,         static_cast<int8_t>(dere_pin));
    linkInfoPtr->kvs->setValue(MODBUS_COLLISION_DETECT_KEY_INDEX, static_cast<bool>(collision_detect));

    linkInfoPtr->kvs->store(SettingsType::Enum::ModbusInterfaceSettings_Type, linkIndex, 0, 0);
  }

  # ifdef MODBUS_DEBUG
  addLogMove(LOG_LEVEL_INFO, F("Modbus: setlink successfull"));
  dumpAdminInfo();
  # endif // ifdef MODBUS_DEBUG
  return true;
}

#endif // if FEATURE_MODBUS_FAC
