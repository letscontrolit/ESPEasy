/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MODBUS manager class
// This class implements a singleton administration object for Modbus devices & links.
// It is part of the Modbus facilities supporting multiple Modbus devices on multiple serial Modbus links.
// It associates Modbus devices with Modbus links and manages their lifecycle.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS_FAC

# include <ESPeasySerial.h>
# include "../Helpers/Modbus_mgr.h"

////# define MODBUS_DEBUG
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

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor of the Modbus manager class, should not be called as this is intended to be a singleton
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusMGR_struct::~ModbusMGR_struct()
{
  // This class is a singleton, so destructor should not be called.
  // However, in case it is called, we clean up the allocated resources.

  for (int i = 0; i < MAX_MODBUS_LINKS; i++) {

    if (_modbus_links[i].link != nullptr) {
      delete _modbus_links[i].link;
      _modbus_links[i].link = nullptr;
    }

    if (_modbus_links[i].kvs != nullptr) {
      delete _modbus_links[i].kvs;
      _modbus_links[i].kvs = nullptr;
    }
  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize the Modbus manager link administration.
// This will read the persisted data for each link and create the link objects for the links that are configured.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::initialize()
{
  if (!_initialized) {
    for (int i = 0; i < MAX_MODBUS_LINKS; i++) {

      int8_t val;
      _modbus_links[i].link = nullptr; // Initialize link pointer to a safe value
      // Create a key-value store for the link and load the persisted settings
      _modbus_links[i].kvs = new (std::nothrow) ESPEasy_key_value_store;
      _modbus_links[i].kvs->load(SettingsType::Enum::ModbusInterfaceSettings_Type, i, 0, 0);
      _modbus_links[i].kvs->getValue(MODBUS_PORT_KEY_INDEX, val);
      _modbus_links[i].port = static_cast<ESPEasySerialPort>(val);
      _modbus_links[i].kvs->getValue(MODBUS_RX_KEY_INDEX, _modbus_links[i].serial_rx);
      _modbus_links[i].kvs->getValue(MODBUS_TX_KEY_INDEX, _modbus_links[i].serial_tx);
      _modbus_links[i].kvs->getValue(MODBUS_BAUDRATE_KEY_INDEX, _modbus_links[i].baudrate);
      _modbus_links[i].kvs->getValue(MODBUS_DERE_PIN_KEY_INDEX, _modbus_links[i].dere_pin);
      _modbus_links[i].kvs->getValue(MODBUS_COLLISION_DETECT_KEY_INDEX, _modbus_links[i].collision_detect);

      // Create the Modbus link object if a port is configured for the link
      if (_modbus_links[i].port != ESPEasySerialPort::not_set) {
        _modbus_links[i].link = new (std::nothrow) ModbusLINK_struct();
        _modbus_links[i].link->init(_modbus_links[i].port,
                                    _modbus_links[i].serial_rx,
                                    _modbus_links[i].serial_tx,
                                    _modbus_links[i].baudrate,
                                    _modbus_links[i].dere_pin,
                                    _modbus_links[i].collision_detect);
      }

    }
    _initialized = true;
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

  initialize();   // TODO Initialization sequence to be refactored.

  *deviceID = -1; // Default to -1, currently not used anymore
  *link     = nullptr;

  if ((linkId < 0) || (linkId >= MAX_MODBUS_LINKS)) {
    # ifdef MODBUS_DEBUG
    log += F("Invalid linkId");
    addLogMove(LOG_LEVEL_ERROR, log);
    # endif // ifdef MODBUS_DEBUG
    return false;
  }

  if (_modbus_links[linkId].port == ESPEasySerialPort::not_set) {
    # ifdef MODBUS_DEBUG
    log += F(" No link available at linkIndex= ");
    log += linkId;
    addLogMove(LOG_LEVEL_ERROR, log);
    # endif // ifdef MODBUS_DEBUG
    return false;
  }

  *deviceID = ++_deviceCounter;
  *link     = _modbus_links[linkId].link;

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
  if (isInitialized()) {
    for (int i = 0; i < MAX_MODBUS_LINKS; i++) {
      if ((_modbus_links[i].link != nullptr)) {
        _modbus_links[i].link->processCommand(); // Trigger processing of the command queue on the link
      }
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

    addLogMove(LOG_LEVEL_INFO,
               strformat(F("Modbus_mgr: Link[%d] Port=%s, RX=%d, TX=%d, Baudrate=%d, DerePin=%d, RS485Mode=%s, CollisionDetect=%s"),
                         i,
                         ESPEasySerialPort_toString(_modbus_links[i].port),
                         _modbus_links[i].serial_rx,
                         _modbus_links[i].serial_tx,
                         _modbus_links[i].baudrate,
                         _modbus_links[i].dere_pin,
                         _modbus_links[i].rs485_mode ? F("Yes") : F("No"),
                         _modbus_links[i].collision_detect ? F("Yes") : F("No")
                         ));
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

  constexpr int optionBaudCount = static_cast<int>(NR_ELEMENTS(options_baudrate));

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

    addFormSubHeader(strformat(F("Modbus %u"), link));
    addFormDetailsStart(link == 0);

    int idx = static_cast<int>(_modbus_links[link].port);
    portSelector.addFormSelector(F("Port"), strformat(F("MBport%u"), link), portMap[idx]);

    String id = strformat(F("MBtx%u"), link);
    addRowLabel_tr_id(formatGpioName_serialTX(false), id);
    addPinSelect(PinSelectPurpose::Serial_input, id, _modbus_links[link].serial_tx);

    id = strformat(F("MBrx%u"), link);
    addRowLabel_tr_id(formatGpioName_serialRX(false), id);
    addPinSelect(PinSelectPurpose::Serial_output, id, _modbus_links[link].serial_rx);

    id = strformat(F("MBde%u"), link);
    addRowLabel_tr_id(formatGpioName_output_optional(F("~RE/DE")), id);
    addPinSelect(PinSelectPurpose::Generic_output, id, _modbus_links[link].dere_pin);

    baudselector.addFormSelector(F("Baud Rate"),
                                 strformat(F("MBbaud%u"), link),
                                 modbus_baudrateToStorageValue(_modbus_links[link].baudrate));
    addUnit(F("baud"));

      # ifdef ESP32
    addFormCheckBox(F("Enable Collision Detection"), strformat(F("MBcoll%u"), link), _modbus_links[link].collision_detect);
    addFormNote(F("/RE connected to GND, only supported on hardware serial"));
      # endif // ifdef ESP32

    addFormDetailsEnd();

  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert stored baudrate setting (enumeration value) to actual baudrate value
// Returns the actual baudrate value.
// The first value is 1200 baud, and each subsequent value doubles the baudrate up to 115200 baud.
// Values outside this range will be mapped to the closest valid value.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int modbus_storageValueToBaudrate(uint8_t baudrate_setting) {
  if ((baudrate_setting > 7) || (baudrate_setting < 0)) { return 9600; }
  return 1200 << baudrate_setting;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert actual baudrate value to stored baudrate setting (enumeration value)
// Returns the stored baudrate setting.
// The first value is 1200 baud, and each subsequent value doubles the baudrate up to 115200 baud.
// Values outside this range will be mapped to the closest valid value.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t modbus_baudrateToStorageValue(int baudrate) {
  if ((baudrate < 1200) || (baudrate > 115200)) { return modbus_baudrateToStorageValue(9600); }
  int bd        = 1200;
  uint8_t index = 0;

  while (bd < baudrate) {
    bd *= 2;
    ++index;
  }
  return index;
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


    for (int i = 0; i < NR_ELEMENTS(portMap); i++) {
      if (portMap[i] == static_cast<int>(_modbus_links[link].port)) {
        port_setting = i;
        break;
      }
    }

    if (update_whenset_FormItemInt(strformat(F("MBport%u"), link), port_setting)) {
      settingsChanged |= (portMap[port_setting] != static_cast<int>(_modbus_links[link].port));
    }
    baudrate_setting = modbus_baudrateToStorageValue(_modbus_links[link].baudrate);

    if (update_whenset_FormItemInt(strformat(F("MBbaud%u"), link), baudrate_setting)) {
      settingsChanged |= (modbus_storageValueToBaudrate(baudrate_setting) != _modbus_links[link].baudrate);
    }
    tx_setting = _modbus_links[link].serial_tx;

    if (update_whenset_FormItemInt(strformat(F("MBtx%u"), link), tx_setting)) {
      settingsChanged |= (tx_setting != _modbus_links[link].serial_tx);
    }
    rx_setting = _modbus_links[link].serial_rx;

    if (update_whenset_FormItemInt(strformat(F("MBrx%u"), link), rx_setting)) {
      settingsChanged |= (rx_setting != _modbus_links[link].serial_rx);
    }
    dere_setting = _modbus_links[link].dere_pin;

    if (update_whenset_FormItemInt(strformat(F("MBde%u"), link), dere_setting)) {
      settingsChanged |= (dere_setting != _modbus_links[link].dere_pin);
        # ifdef ESP32

      // Checkbox existence cannot be determined from the HTML response. Assume its there when dere_setting is detected.
      // The Collision detection setting is only available on ESP32 and only when a DE/RE pin is configured.
      collision_detect_setting = isFormItemChecked(strformat(F("MBcoll%u"), link));
      settingsChanged         |= (collision_detect_setting != _modbus_links[link].collision_detect);
        # endif // ifdef ESP32
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
                               const int8_t            serial_rx,
                               const int8_t            serial_tx,
                               uint16_t                baudrate,
                               int8_t                  dere_pin,
                               bool                    collision_detect)
{
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

  if ((linkIndex < 0) || (linkIndex >= MAX_MODBUS_LINKS)) {
    # ifdef MODBUS_DEBUG
    log += strformat(F("Invalid link for linkIndex=%d"), linkIndex);
    addLogMove(LOG_LEVEL_INFO, log);
    # endif // ifdef MODBUS_DEBUG
    return false;                                 // Invalid link index
  }

  if (_modbus_links[linkIndex].link != nullptr) { // Sanity check for successful creation
    // (re)initialize the new link
    if (!_modbus_links[linkIndex].link->init(port, serial_rx, serial_tx, baudrate, dere_pin, collision_detect)) {
      return false;                               // Initialization failed
    }
    else {
      // Store the link parameters
      _modbus_links[linkIndex].port             = port;
      _modbus_links[linkIndex].serial_rx        = serial_rx;
      _modbus_links[linkIndex].serial_tx        = serial_tx;
      _modbus_links[linkIndex].baudrate         = baudrate;
      _modbus_links[linkIndex].dere_pin         = dere_pin;
      _modbus_links[linkIndex].rs485_mode       = (dere_pin != -1);
      _modbus_links[linkIndex].collision_detect = collision_detect;
    }
  }

  if (_modbus_links[linkIndex].kvs) {
    // Store the link configuration parameters in the key-value store for persistence
    _modbus_links[linkIndex].kvs->setValue(MODBUS_PORT_KEY_INDEX,             static_cast<int8_t>(port));
    _modbus_links[linkIndex].kvs->setValue(MODBUS_RX_KEY_INDEX,               static_cast<int8_t>(serial_rx));
    _modbus_links[linkIndex].kvs->setValue(MODBUS_TX_KEY_INDEX,               static_cast<int8_t>(serial_tx));
    _modbus_links[linkIndex].kvs->setValue(MODBUS_BAUDRATE_KEY_INDEX,         static_cast<int16_t>(baudrate));
    _modbus_links[linkIndex].kvs->setValue(MODBUS_DERE_PIN_KEY_INDEX,         static_cast<int8_t>(dere_pin));
    _modbus_links[linkIndex].kvs->setValue(MODBUS_COLLISION_DETECT_KEY_INDEX, static_cast<bool>(collision_detect));

    _modbus_links[linkIndex].kvs->store(SettingsType::Enum::ModbusInterfaceSettings_Type, linkIndex, 0, 0);
  }

  # ifdef MODBUS_DEBUG
  addLogMove(LOG_LEVEL_INFO, F("Modbus: setlink successfull"));
  dumpAdminInfo();
  # endif // ifdef MODBUS_DEBUG
  return true;
}

#endif // if FEATURE_MODBUS_FAC
