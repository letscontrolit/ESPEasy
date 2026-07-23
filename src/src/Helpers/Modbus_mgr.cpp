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


// Macro to check if a linkId is valid and if the link is initialized
# define MODBUS_MGR_VALID_LINK_ID(id) (validLinkId(id) && (_modbus_links[id].link != nullptr))

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

uint32_t modbus_storageValueToBaudrate(uint8_t baudrate_setting);
uint8_t  modbus_baudrateToStorageValue(uint32_t baudrate);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor of the Modbus manager class, initializes the internal data structures
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModbusMGR_struct::ModbusMGR_struct()
{}

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

      if (_modbus_links[i].kvs != nullptr) {
        _modbus_links[i].kvs->load(SettingsType::Enum::ModbusInterfaceSettings_Type, i, 0, 0);
        _modbus_links[i].kvs->getValue(MODBUS_PORT_KEY_INDEX, val);
        _modbus_links[i].port = static_cast<ESPEasySerialPort>(val);
        _modbus_links[i].kvs->getValue(MODBUS_RX_KEY_INDEX, _modbus_links[i].serial_rx);
        _modbus_links[i].kvs->getValue(MODBUS_TX_KEY_INDEX, _modbus_links[i].serial_tx);
        _modbus_links[i].kvs->getValue(MODBUS_BAUDRATE_KEY_INDEX, _modbus_links[i].baudrate);
        _modbus_links[i].kvs->getValue(MODBUS_DERE_PIN_KEY_INDEX, _modbus_links[i].dere_pin);
        _modbus_links[i].kvs->getValue(MODBUS_COLLISION_DETECT_KEY_INDEX, _modbus_links[i].collision_detect);
      }

      // Create the Modbus link object if a port is configured for the link
      if (_modbus_links[i].port != ESPEasySerialPort::not_set) {
        _modbus_links[i].link = new (std::nothrow) ModbusLINK_struct(i);

        if (_modbus_links[i].link != nullptr) {
          _modbus_links[i].link->init(_modbus_links[i].port,
                                      _modbus_links[i].serial_rx,
                                      _modbus_links[i].serial_tx,
                                      _modbus_links[i].baudrate,
                                      _modbus_links[i].dere_pin,
                                      _modbus_links[i].collision_detect);
        }
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
bool ModbusMGR_struct::connect(int linkId, const ModbusDEVICE_struct *device)
{
  bool returnValue = false;

  # ifdef MODBUS_DEBUG
  String log = strformat(F("Modbus Manager: Connect device= %p to linkId=%d"), device, linkId);
  # endif // ifdef MODBUS_DEBUG

  initialize(); // TODO Initialization sequence to be refactored.

  if (!MODBUS_MGR_VALID_LINK_ID(linkId)) {
    # ifdef MODBUS_DEBUG
    log += F(" Invalid linkId");
    addLogMove(LOG_LEVEL_ERROR, log);
    # endif // ifdef MODBUS_DEBUG
  }
  else if (_modbus_links[linkId].port == ESPEasySerialPort::not_set) {
      # ifdef MODBUS_DEBUG
    log += F(" No link available at linkIndex= ");
    log += linkId;
    addLogMove(LOG_LEVEL_ERROR, log);
      # endif // ifdef MODBUS_DEBUG
  }
  else {
    // No administration to be done on manager or link side
    returnValue = true;
  }

  # ifdef MODBUS_DEBUG
  addLogMove(LOG_LEVEL_INFO, log);
  dumpAdminInfo();
  # endif // ifdef MODBUS_DEBUG

  return returnValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Disconnect the Modbus device with the given device ID.
// If no other devices are using the same link, the link is also deleted.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::disconnect(int linkId, const ModbusDEVICE_struct*device) {

  bool returnValue = false;

  # ifdef MODBUS_DEBUG
  dumpAdminInfo();
  String log = strformat(F("Modbus Manager: Disconnect device= %p from linkId= %d "), device, linkId);
  # endif // ifdef MODBUS_DEBUG

  if (!MODBUS_MGR_VALID_LINK_ID(linkId)) {
    # ifdef MODBUS_DEBUG
    log += F("Invalid linkId");
    addLogMove(LOG_LEVEL_ERROR, log);
    # endif // ifdef MODBUS_DEBUG
  }
  else {
    _modbus_links[linkId].link->freeTransactions(device);
    returnValue = true;
  }

  # ifdef MODBUS_DEBUG
  addLogMove(LOG_LEVEL_INFO, log);
  # endif // ifdef MODBUS_DEBUG
  return returnValue;
}

Modbus_transaction_ptr ModbusMGR_struct::newTransaction(int linkId, ModbusDEVICE_struct *device)
{
  Modbus_transaction_ptr transaction = nullptr;

  # ifdef MODBUS_DEBUG
  String log = strformat(F("Modbus Manager: New transaction for linkId= %d, device= %p"), linkId, device);
  # endif // ifdef MODBUS_DEBUG

  if (!MODBUS_MGR_VALID_LINK_ID(linkId)) {
    # ifdef MODBUS_DEBUG
    log += F("Invalid linkId");
    # endif // ifdef MODBUS_DEBUG
  }
  else {
    transaction = _modbus_links[linkId].link->newTransaction(device);
    # ifdef MODBUS_DEBUG
    log += strformat(F(" Transaction pointer= %p"), transaction);
    transaction->print(); // Print the transaction details for debugging
    # endif // ifdef MODBUS_DEBUG
  }

  # ifdef MODBUS_DEBUG
  addLogMove(LOG_LEVEL_INFO, log);
  # endif // ifdef MODBUS_DEBUG
  return transaction;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::queueTransaction(int linkId, Modbus_Transaction *transaction)
{
  bool returnValue = false;

  # ifdef MODBUS_DEBUG
  String log = strformat(F("Modbus Manager: Queue transaction for linkId= %d "), linkId);
  # endif // ifdef MODBUS_DEBUG

  if (!MODBUS_MGR_VALID_LINK_ID(linkId)) {
    # ifdef MODBUS_DEBUG
    log += F("Invalid linkId");
    addLogMove(LOG_LEVEL_ERROR, log);
    # endif // ifdef MODBUS_DEBUG
  }
  else {
    returnValue = _modbus_links[linkId].link->queueTransaction(transaction);
  }

  # ifdef MODBUS_DEBUG
  addLogMove(LOG_LEVEL_INFO, log);
  # endif // ifdef MODBUS_DEBUG
  return returnValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This function should be called periodically to allow the Modbus manager to process the Modbus links
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusMGR_struct::processLinks()
{
  if (isInitialized()) {
    for (int i = 0; i < MAX_MODBUS_LINKS; i++) {
      if ((_modbus_links[i].link != nullptr)) {
        _modbus_links[i].link->processQueue(); // Trigger processing of the command queue on the link
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dump the Modbus manager admin information to the log for debugging purposes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModbusMGR_struct::dumpAdminInfo()
{
  # ifdef MODBUS_DEBUG
  addLogMove(LOG_LEVEL_INFO, F("Modbus Manager: Dumping admin info"));

  // Iterate over the modbus links and dump their info
  for (int i = 0; i < MAX_MODBUS_LINKS; i++) {

    addLogMove(LOG_LEVEL_INFO,
               strformat(F("Modbus Admin: Link[%d] Port=%s, RX=%d, TX=%d, Baudrate=%d, DerePin=%d, RS485Mode=%s, CollisionDetect=%s"),
                         i,
                         ESPEasySerialPort_toString(_modbus_links[i].port),
                         _modbus_links[i].serial_rx,
                         _modbus_links[i].serial_tx,
                         _modbus_links[i].baudrate,
                         _modbus_links[i].dere_pin,
                         FsP(_modbus_links[i].rs485_mode ? F("Yes") : F("No")),
                         FsP(_modbus_links[i].collision_detect ? F("Yes") : F("No"))
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
    options_baudrate[i] = modbus_storageValueToBaudrate(i); // Fill the baudrate options with actual baudrate values
  }
  const FormSelectorOptions baudselector(optionBaudCount, options_baudrate);

  // Iterate over the modbus links and show their configuration on the web page
  for (int link = 0; link < MAX_MODBUS_LINKS; ++link)
  {
    addFormSubHeader(strformat(F("Modbus %u"), link));
    addFormDetailsStart(link == 0 || _modbus_links[link].port != ESPEasySerialPort::not_set);

////    serialHelper_webformLoad(link, _modbus_links[link].port, static_cast<int>(_modbus_links[link].serial_rx),
////                             static_cast<int>(_modbus_links[link].serial_tx), true);

    String id = strformat(F("MBde%u"), link);
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
uint32_t modbus_storageValueToBaudrate(uint8_t baudrate_setting) {
  if ((baudrate_setting > 7) || (baudrate_setting < 0)) { return 9600; }

  switch (baudrate_setting)
  {
    case 0: return 1200;
    case 1: return 2400;
    case 2: return 4800;
    case 3: return 9600;
    case 4: return 19200;
    case 5: return 38400;
    case 6: return 57600;
    case 7: return 115200;
    default:
      return 9600; // Default to a safe value if the setting is invalid
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert actual baudrate value to stored baudrate setting (enumeration value)
// Returns the stored baudrate setting.
// The first value is 1200 baud, and each subsequent value doubles the baudrate up to 115200 baud.
// Values outside this range will be mapped to the closest valid value.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t modbus_baudrateToStorageValue(uint32_t baudrate) {
  if (baudrate <= 1200) { return 0; }

  if (baudrate <= 2400) { return 1; }

  if (baudrate <= 4800) { return 2; }

  if (baudrate <= 9600) { return 3; }

  if (baudrate <= 19200) { return 4; }

  if (baudrate <= 38400) { return 5; }

  if (baudrate <= 57600) { return 6; }
  return 7; // 115200
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process the Modbus link configuration from the web page save action and update the Modbus manager configuration
// This is called from the interfaces configuration page to show the Modbus link configuration.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ModbusMGR_struct::save_modbus_interfaces(String& error)
{
  for (int link = 0; link < MAX_MODBUS_LINKS; ++link)  {
    int  port_setting             = static_cast<int>(_modbus_links[link].port);
    int  tx_setting               = _modbus_links[link].serial_tx;
    int  rx_setting               = _modbus_links[link].serial_rx;
    int  baudrate_setting         = modbus_baudrateToStorageValue(_modbus_links[link].baudrate);
    int  dere_setting             = _modbus_links[link].dere_pin;
    bool collision_detect_setting = _modbus_links[link].collision_detect;
    bool settingsChanged          = false;

    int8_t  rxPin = rx_setting;
    int8_t  txPin = tx_setting;
    uint8_t port  = port_setting;
////    serialHelper_webformSave(link, port, rxPin, txPin);

    if (port != port_setting) {
      port_setting    = port;
      settingsChanged = true;
    }

    if (rxPin != rx_setting) {
      rx_setting      = rxPin;
      settingsChanged = true;
    }

    if (txPin != tx_setting) {
      tx_setting      = txPin;
      settingsChanged = true;
    }

    update_whenset_FormItemInt(strformat(F("MBbaud%u"), link), baudrate_setting, &settingsChanged);

    if (update_whenset_FormItemInt(strformat(F("MBde%u"), link), dere_setting, &settingsChanged))
    {
      # ifdef ESP32

      // Checkbox existence cannot be determined from the HTML response. Assume its there when dere_setting is detected.
      // The Collision detection setting is only available on ESP32 and only when a DE/RE pin is configured.
      collision_detect_setting = isFormItemChecked(strformat(F("MBcoll%u"), link));
      settingsChanged         |= (collision_detect_setting != _modbus_links[link].collision_detect);
      # endif // ifdef ESP32
    }

    if (settingsChanged) {
      setLink(link,
              static_cast<ESPEasySerialPort>(port_setting),
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

  if (!validLinkId(linkIndex)) {
    # ifdef MODBUS_DEBUG
    log += strformat(F("Invalid link for linkIndex=%d"), linkIndex);
    addLogMove(LOG_LEVEL_INFO, log);
    # endif // ifdef MODBUS_DEBUG
    return false; // Invalid link index
  }

  // Check of the link is being disabled (port set to not_set) or enabled (port set to a valid port)
  if (_modbus_links[linkIndex].port == ESPEasySerialPort::not_set) {
    if ((port != ESPEasySerialPort::not_set) && (_modbus_links[linkIndex].link == nullptr)) {
      _modbus_links[linkIndex].link = new (std::nothrow) ModbusLINK_struct(linkIndex);
    }
  }
  else if ((port == ESPEasySerialPort::not_set) && (_modbus_links[linkIndex].link != nullptr)) {
    delete _modbus_links[linkIndex].link;
    _modbus_links[linkIndex].link = nullptr;
    }

  // If the link object is enabled (has a valid pointer), initialize it with the new parameters
  if (_modbus_links[linkIndex].link != nullptr) {
    if (!_modbus_links[linkIndex].link->init(port, serial_rx, serial_tx, baudrate, dere_pin, collision_detect)) {
      # ifdef MODBUS_DEBUG
      log += strformat(F("Link initialization failed"));
      addLogMove(LOG_LEVEL_INFO, log);
      # endif // ifdef MODBUS_DEBUG
      return false; // Initialization failed
    }
  }

  // Store the updated link parameters in the Modbus manager's internal data structure
  _modbus_links[linkIndex].port             = port;
  _modbus_links[linkIndex].serial_rx        = serial_rx;
  _modbus_links[linkIndex].serial_tx        = serial_tx;
  _modbus_links[linkIndex].baudrate         = baudrate;
  _modbus_links[linkIndex].dere_pin         = dere_pin;
  _modbus_links[linkIndex].rs485_mode       = (dere_pin != -1);
  _modbus_links[linkIndex].collision_detect = collision_detect;

  // Store the link configuration parameters in the key-value store for persistence
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
