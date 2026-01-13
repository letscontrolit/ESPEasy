#include "../NWPluginStructs/NW003_data_struct_ETH_RMII.h"

#ifdef USES_NW003


# include "../../../src/ESPEasyCore/ESPEasyGPIO.h"

# include "../../../src/Globals/Settings.h"

# include "../../../src/Helpers/ESPEasy_time_calc.h"
# include "../../../src/Helpers/Hardware_GPIO.h"
# include "../../../src/Helpers/LongTermOnOffTimer.h"
# include "../../../src/Helpers/StringConverter.h"

# include "../../../src/WebServer/Markup.h"
# include "../../../src/WebServer/Markup_Forms.h"
# include "../../../src/WebServer/ESPEasy_key_value_store_webform.h"

# include "../Globals/NetworkState.h"

# include "../Helpers/NW_info_writer.h"

# include "../eth/ESPEasyEth.h"

# define NW_PLUGIN_ID  3

namespace ESPEasy {
namespace net {
namespace eth {


// Keys as used in the Key-value-store
# define NW003_KEY_ETH_INDEX                         1
# define NW003_KEY_ETH_PHY_TYPE                      2
# define NW003_KEY_ETH_PHY_ADDR                      3
# define NW003_KEY_ETH_PIN_MDC                       4
# define NW003_KEY_ETH_PIN_MDIO                      5
# define NW003_KEY_ETH_PIN_POWER                     6
# define NW003_KEY_CLOCK_MODE                        7
# define NW003_KEY_IP                                8
# define NW003_KEY_GW                                9
# define NW003_KEY_SN                                10
# define NW003_KEY_DNS                               11

# define NW003_MAX_KEY                               12

const __FlashStringHelper * NW003_data_struct_ETH_RMII::getLabelString(uint32_t key, bool displayString, KVS_StorageType::Enum& storageType)
{
  storageType = KVS_StorageType::Enum::int8_type;

  switch (key)
  {
    case NW003_KEY_ETH_INDEX:  return F("Index");
    case NW003_KEY_ETH_PHY_TYPE: return displayString ? F("Ethernet PHY type") : F("phytype");
    case NW003_KEY_ETH_PHY_ADDR: return displayString ? F("Ethernet PHY Address") : F("phyaddr");
    case NW003_KEY_ETH_PIN_MDC: return displayString ? F("Ethernet MDC pin") : F("MDC");
    case NW003_KEY_ETH_PIN_MDIO: return displayString ? F("Ethernet MDIO pin") : F("MDIO");
    case NW003_KEY_ETH_PIN_POWER: return displayString ? F("Ethernet Power pin") : F("pwr");
    case NW003_KEY_CLOCK_MODE: return displayString ? F("Ethernet Clock") : F("clock");
    case NW003_KEY_IP:
      storageType = KVS_StorageType::Enum::string_type;
      return F("IP");
    case NW003_KEY_GW:
      storageType = KVS_StorageType::Enum::string_type;
      return displayString ? F("Gateway") : F("gw");
    case NW003_KEY_SN:
      storageType = KVS_StorageType::Enum::string_type;
      return displayString ? F("Subnetmask") : F("sn");
    case NW003_KEY_DNS:
      storageType = KVS_StorageType::Enum::string_type;
      return F("DNS");
  }
  return F("");
}

int32_t NW003_data_struct_ETH_RMII::getNextKey(int32_t key)
{
  if (key == -1) { return NW003_KEY_ETH_INDEX; }

  if (key >= NW003_MAX_KEY) { return -2; }
  return key + 1;
}

String NW003_data_struct_ETH_RMII::formatGpioLabel(uint32_t          key,
                                                   PinSelectPurpose& purpose,
                                                   bool              shortNotation)
{
  String label;

  if ((key >= NW003_KEY_ETH_PIN_MDC) && (key <= NW003_KEY_ETH_PIN_POWER))
  {
    KVS_StorageType::Enum storageType;
    purpose = PinSelectPurpose::Generic;
    label   = getLabelString(key, true, storageType);

    const bool optional = !shortNotation;

    switch (key)
    {
      case NW003_KEY_ETH_PIN_MDIO:
        purpose = PinSelectPurpose::Generic_input;
        label   = formatGpioName(
          label, gpio_direction::gpio_input, optional);
        break;
      case NW003_KEY_ETH_PIN_MDC:
      case NW003_KEY_ETH_PIN_POWER:
        purpose = PinSelectPurpose::Generic_output;
        label   = formatGpioName(
          label, gpio_direction::gpio_output, optional);
        break;
    }
  }
  return label;
}

WebFormItemParams NW003_makeWebFormItemParams(uint32_t key) {
  KVS_StorageType::Enum storageType;
  const __FlashStringHelper*label = NW003_data_struct_ETH_RMII::getLabelString(key, true, storageType);
  const __FlashStringHelper*id    = NW003_data_struct_ETH_RMII::getLabelString(key, false, storageType);

  return WebFormItemParams(label, id, storageType, key);
}

NW003_data_struct_ETH_RMII::NW003_data_struct_ETH_RMII(networkIndex_t networkIndex, NetworkInterface *netif)
  : NWPluginData_base(nwpluginID_t(NW_PLUGIN_ID), networkIndex, netif)
{}

NW003_data_struct_ETH_RMII::~NW003_data_struct_ETH_RMII()
{
  ESPEasy::net::eth::ETH_NWPluginData_static_runtime::exit(_networkIndex);
}

void NW003_data_struct_ETH_RMII::loadDefaults(ESPEasy_key_value_store     *kvs,
                                              ESPEasy::net::networkIndex_t networkIndex,
                                              ESPEasy::net::nwpluginID_t   nwPluginID)
{
  // Only load the defaults when the settings are empty
  if (kvs && kvs->isEmpty()) {
    if (isValid(Settings.ETH_Phy_Type) &&
        !ESPEasy::net::isSPI_EthernetType(Settings.ETH_Phy_Type))
    {
      kvs->setValue(NW003_KEY_ETH_INDEX,     static_cast<int8_t>(-1));
      kvs->setValue(NW003_KEY_ETH_PHY_TYPE,  static_cast<int8_t>(Settings.ETH_Phy_Type));
      kvs->setValue(NW003_KEY_ETH_PHY_ADDR,  static_cast<int8_t>(Settings.ETH_Phy_Addr));
      kvs->setValue(NW003_KEY_ETH_PIN_MDC,   static_cast<int8_t>(Settings.ETH_Pin_mdc_cs));
      kvs->setValue(NW003_KEY_ETH_PIN_MDIO,  static_cast<int8_t>(Settings.ETH_Pin_mdio_irq));
      kvs->setValue(NW003_KEY_ETH_PIN_POWER, static_cast<int8_t>(Settings.ETH_Pin_power_rst));
      kvs->setValue(NW003_KEY_CLOCK_MODE,    static_cast<int8_t>(Settings.ETH_Clock_Mode));

      // TODO TD-er: Copy IP info

      store_nwpluginTaskData_KVS(kvs, networkIndex, nwPluginID);
    }
    # if defined(DEFAULT_ETH_PHY_TYPE) && defined(DEFAULT_ETH_CLOCK_MODE) && defined(DEFAULT_ETH_PHY_ADDR) && \
    defined(DEFAULT_ETH_PIN_MDC) && defined(DEFAULT_ETH_PIN_MDIO)
    else if (isValid(DEFAULT_ETH_PHY_TYPE) &&
             !ESPEasy::net::isSPI_EthernetType(DEFAULT_ETH_PHY_TYPE))
    {
      kvs->setValue(NW003_KEY_ETH_INDEX,     static_cast<int8_t>(-1));
      kvs->setValue(NW003_KEY_ETH_PHY_TYPE,  static_cast<int8_t>(DEFAULT_ETH_PHY_TYPE));
      kvs->setValue(NW003_KEY_ETH_PHY_ADDR,  static_cast<int8_t>(DEFAULT_ETH_PHY_ADDR));
      kvs->setValue(NW003_KEY_ETH_PIN_MDC,   static_cast<int8_t>(DEFAULT_ETH_PIN_MDC));
      kvs->setValue(NW003_KEY_ETH_PIN_MDIO,  static_cast<int8_t>(DEFAULT_ETH_PIN_MDIO));
      kvs->setValue(NW003_KEY_ETH_PIN_POWER, static_cast<int8_t>(DEFAULT_ETH_PIN_POWER));
      kvs->setValue(NW003_KEY_CLOCK_MODE,    static_cast<int8_t>(DEFAULT_ETH_CLOCK_MODE));

      // TODO TD-er: Copy IP info

      store_nwpluginTaskData_KVS(kvs, networkIndex, nwPluginID);
    }
    # endif // if defined(DEFAULT_ETH_PHY_TYPE) && defined(DEFAULT_ETH_CLOCK_MODE) && defined(DEFAULT_ETH_PHY_ADDR) &&
    // defined(DEFAULT_ETH_PIN_MDC) && defined(DEFAULT_ETH_PIN_MDIO)
  }
}

void NW003_data_struct_ETH_RMII::webform_load(EventStruct *event)
{
  _load();
  NW003_data_struct_ETH_RMII::loadDefaults(_kvs, event->NetworkIndex, nwpluginID_t(3));

  addFormSubHeader(F("Ethernet IP Settings"));

  addFormIPBox(F("ESP Ethernet IP"),         F("espethip"),      Settings.ETH_IP);
  addFormIPBox(F("ESP Ethernet Gateway"),    F("espethgateway"), Settings.ETH_Gateway);
  addFormIPBox(F("ESP Ethernet Subnetmask"), F("espethsubnet"),  Settings.ETH_Subnet);
  addFormIPBox(F("ESP Ethernet DNS"),        F("espethdns"),     Settings.ETH_DNS);
  addFormNote(F("Leave empty for DHCP"));

  addFormSubHeader(F("Ethernet"));

  {
    const __FlashStringHelper *options[] = {
      toString(EthPhyType_t::notSet),

      toString(EthPhyType_t::LAN8720),
# if ETH_PHY_LAN867X_SUPPORTED
      toString(ESPEasy::net::EthPhyType_t::LAN867X),
# endif
      toString(EthPhyType_t::TLK110),
      toString(EthPhyType_t::RTL8201),
# if ETH_TYPE_JL1101_SUPPORTED
      toString(EthPhyType_t::JL1101),
# endif
      toString(EthPhyType_t::DP83848),
      toString(EthPhyType_t::KSZ8041),
      toString(EthPhyType_t::KSZ8081),
    };
    const int ids[] = {
      static_cast<int>(EthPhyType_t::notSet),

      static_cast<int>(EthPhyType_t::LAN8720),
# if ETH_PHY_LAN867X_SUPPORTED
      static_cast<int>(ESPEasy::net::EthPhyType_t::LAN867X),
# endif
      static_cast<int>(EthPhyType_t::TLK110),
      static_cast<int>(EthPhyType_t::RTL8201),
# if ETH_TYPE_JL1101_SUPPORTED
      static_cast<int>(EthPhyType_t::JL1101),
# endif
      static_cast<int>(EthPhyType_t::DP83848),
      static_cast<int>(EthPhyType_t::KSZ8041),
      static_cast<int>(EthPhyType_t::KSZ8081),

    };
    const FormSelectorOptions selector(NR_ELEMENTS(ids), options, ids);
    auto params = NW003_makeWebFormItemParams(NW003_KEY_ETH_PHY_TYPE);
    params._defaultIntValue = static_cast<int>(EthPhyType_t::notSet);
    showFormSelector(*_kvs, selector, params);
  }
  {
    auto params = NW003_makeWebFormItemParams(NW003_KEY_ETH_PHY_ADDR);
    params._min             = -1;
    params._max             = 127;
    params._defaultIntValue = 1;
    showWebformItem(*_kvs, params);
    addFormNote(F("I&sup2;C-address of Ethernet PHY"
                  " (0 or 1 for LAN8720, 31 for TLK110, -1 autodetect)"
                  ));

  }
  {
    const int gpio_keys[] = {
      NW003_KEY_ETH_PIN_MDC,
      NW003_KEY_ETH_PIN_MDIO,
      NW003_KEY_ETH_PIN_POWER
    };

    for (int i = 0; i < NR_ELEMENTS(gpio_keys); ++i)
    {
      const int key = gpio_keys[i];
      KVS_StorageType::Enum storageType;
      const __FlashStringHelper *id = getLabelString(key, false, storageType);
      PinSelectPurpose purpose      = PinSelectPurpose::Ethernet;
      String label                  = formatGpioLabel(key, purpose);

      int8_t pin = -1;
      _kvs->getValue(key, pin);

      addFormPinSelect(purpose, label, id, pin);
    }
  }
  {
# if CONFIG_IDF_TARGET_ESP32
    const __FlashStringHelper *ethClockOptions[] = {
      toString(EthClockMode_t::Ext_crystal_osc),
      toString(EthClockMode_t::Int_50MHz_GPIO_0),
      toString(EthClockMode_t::Int_50MHz_GPIO_16),
      toString(EthClockMode_t::Int_50MHz_GPIO_17_inv)
    };
    const int indices[] = {
      static_cast<int>(EthClockMode_t::Ext_crystal_osc),
      static_cast<int>(EthClockMode_t::Int_50MHz_GPIO_0),
      static_cast<int>(EthClockMode_t::Int_50MHz_GPIO_16),
      static_cast<int>(EthClockMode_t::Int_50MHz_GPIO_17_inv)
    };
# endif // if CONFIG_IDF_TARGET_ESP32
# if CONFIG_IDF_TARGET_ESP32P4
    const __FlashStringHelper *ethClockOptions[] = {
      //          toString(EthClockMode_t::Default),
      toString(EthClockMode_t::Ext_crystal),
      toString(EthClockMode_t::Int_50MHz)
    };
    const int indices[] = {
      //          static_cast<int>(EthClockMode_t::Default),
      static_cast<int>(EthClockMode_t::Ext_crystal),
      static_cast<int>(EthClockMode_t::Int_50MHz)
    };
# endif // if CONFIG_IDF_TARGET_ESP32P4
    const FormSelectorOptions selector(
      NR_ELEMENTS(ethClockOptions),
      ethClockOptions,
      indices);
    auto params = NW003_makeWebFormItemParams(NW003_KEY_CLOCK_MODE);
# if CONFIG_IDF_TARGET_ESP32
    params._defaultIntValue = static_cast<int>(EthClockMode_t::Ext_crystal_osc);
#endif
# if CONFIG_IDF_TARGET_ESP32P4
    params._defaultIntValue = static_cast<int>(EthClockMode_t::Ext_crystal);
#endif

    showFormSelector(*_kvs, selector, params);

  }
}

void NW003_data_struct_ETH_RMII::webform_save(EventStruct *event)
{
  // TODO TD-er: Move this to a central function, like done with import/export
  int32_t key = getNextKey(-1);

  while (key >= 0) {
    KVS_StorageType::Enum storageType;
    const __FlashStringHelper *id = getLabelString(key, false, storageType);
    storeWebformItem(*_kvs, key, storageType, id);
    key = getNextKey(key);
  }
  _store();
}

bool NW003_data_struct_ETH_RMII::webform_getPort(KeyValueWriter *writer) { return true; }

bool NW003_data_struct_ETH_RMII::init(EventStruct *event)
{
  _load();
  ETHConnectRelaxed();

  return true;
}

bool                          NW003_data_struct_ETH_RMII::exit(EventStruct *event)         { return true; }

NWPluginData_static_runtime * NW003_data_struct_ETH_RMII::getNWPluginData_static_runtime() {
  return ESPEasy::net::eth::ETH_NWPluginData_static_runtime::getNWPluginData_static_runtime(_networkIndex);
}

bool NW003_data_struct_ETH_RMII::write_Eth_HW_Address(KeyValueWriter *writer)
{
  if (writer == nullptr) { return false; }
  auto data  = getNWPluginData_static_runtime();
  auto iface = ESPEasy::net::eth::ETH_NWPluginData_static_runtime::getInterface(_networkIndex);

  if (!(data && iface)) { return false; }
  const int phyType_int = _kvs->getValueAsInt_or_default(NW003_KEY_ETH_PHY_TYPE, -1);

  if (phyType_int == -1) { return false; }
  return ESPEasy::net::write_Eth_HW_Address(
    static_cast<ESPEasy::net::EthPhyType_t>(phyType_int),
    iface,
    writer);
}

bool NW003_data_struct_ETH_RMII::write_Eth_port(KeyValueWriter *writer)
{
  if (writer == nullptr) { return false; }
  const ESPEasy::net::EthPhyType_t phyType = static_cast<ESPEasy::net::EthPhyType_t>(_kvs->getValueAsInt(NW003_KEY_ETH_PHY_TYPE));

  if (!isValid(phyType)) { return false; }

  const __FlashStringHelper*labels[] = {
    F("MDC"), F("MDIO"), F("Power") };
  const int pins[] = {
    (int)_kvs->getValueAsInt(NW003_KEY_ETH_PIN_MDC),
    (int)_kvs->getValueAsInt(NW003_KEY_ETH_PIN_MDIO),
    (int)_kvs->getValueAsInt(NW003_KEY_ETH_PIN_POWER)
  };
  return write_NetworkPort(labels, pins, NR_ELEMENTS(labels), writer);
}

void NW003_data_struct_ETH_RMII::ethResetGPIOpins() {
  // fix an disconnection issue after rebooting Olimex POE - this forces a clean state for all GPIO involved in RMII
  // Thanks to @s-hadinger and @Jason2866
  // Resetting state of power pin is done in ethPower()
  addLog(LOG_LEVEL_INFO, F("ethResetGPIOpins()"));
  gpio_reset_pin((gpio_num_t)_kvs->getValueAsInt(NW003_KEY_ETH_PIN_MDC));
  gpio_reset_pin((gpio_num_t)_kvs->getValueAsInt(NW003_KEY_ETH_PIN_MDIO));
# ifdef CONFIG_IDF_TARGET_ESP32

  gpio_reset_pin(GPIO_NUM_19); // EMAC_TXD0 - hardcoded
  gpio_reset_pin(GPIO_NUM_21); // EMAC_TX_EN - hardcoded
  gpio_reset_pin(GPIO_NUM_22); // EMAC_TXD1 - hardcoded
  gpio_reset_pin(GPIO_NUM_25); // EMAC_RXD0 - hardcoded
  gpio_reset_pin(GPIO_NUM_26); // EMAC_RXD1 - hardcoded
  gpio_reset_pin(GPIO_NUM_27); // EMAC_RX_CRS_DV - hardcoded
# endif // ifdef CONFIG_IDF_TARGET_ESP32

  /*
     switch (Settings.ETH_Clock_Mode) {
     case EthClockMode_t::Ext_crystal_osc:       // ETH_CLOCK_GPIO0_IN
     case EthClockMode_t::Int_50MHz_GPIO_0:      // ETH_CLOCK_GPIO0_OUT
      gpio_reset_pin(GPIO_NUM_0);
      break;
     case EthClockMode_t::Int_50MHz_GPIO_16:     // ETH_CLOCK_GPIO16_OUT
      gpio_reset_pin(GPIO_NUM_16);
      break;
     case EthClockMode_t::Int_50MHz_GPIO_17_inv: // ETH_CLOCK_GPIO17_OUT
      gpio_reset_pin(GPIO_NUM_17);
      break;
     }
   */
  delay(1);

}

void NW003_data_struct_ETH_RMII::ethPower(bool enable)
{
  const int powerpin = _kvs->getValueAsInt(NW003_KEY_ETH_PIN_POWER);

  if (powerpin == -1) { return; }

  if (GPIO_Internal_Read(powerpin) == enable) {
    // Already the desired state
    return;
  }
  addLog(LOG_LEVEL_INFO, enable ? F("ETH power ON") : F("ETH power OFF"));

  /*
      if (!enable) {
        EthEventData.ethInitSuccess = false;
        EthEventData.clearAll();
   # ifdef ESP_IDF_VERSION_MAJOR

        // FIXME TD-er: See: https://github.com/espressif/arduino-esp32/issues/6105
        // Need to store the last link state, as it will be cleared after destructing the object.
        EthEventData.setEthDisconnected();

        if (ETH.linkUp()) {
          EthEventData.setEthConnected();
        }
   # endif // ifdef ESP_IDF_VERSION_MAJOR

        //      ETH = ETHClass();
      }
   */

  if (enable) {
    //      ethResetGPIOpins();
  }

  //    gpio_reset_pin((gpio_num_t)powerpin);

  GPIO_Write(PLUGIN_GPIO, powerpin, enable ? 1 : 0);

  if (!enable) {
    const EthClockMode_t ETH_ClockMode = static_cast<EthClockMode_t>(_kvs->getValueAsInt(NW003_KEY_CLOCK_MODE));
      # if CONFIG_IDF_TARGET_ESP32P4
    const bool isExternalCrystal = ETH_ClockMode == EthClockMode_t::Ext_crystal;
      # else
    const bool isExternalCrystal = ETH_ClockMode == EthClockMode_t::Ext_crystal_osc;
      # endif // if CONFIG_IDF_TARGET_ESP32P4

    if (isExternalCrystal) {
      delay(600); // Give some time to discharge any capacitors
      // Delay is needed to make sure no clock signal remains present which may cause the ESP to boot into flash mode.
    }
  } else {
    delay(400); // LAN chip needs to initialize before calling Eth.begin()
  }
}

bool NW003_data_struct_ETH_RMII::ethCheckSettings() {
  const EthClockMode_t ETH_ClockMode       = static_cast<EthClockMode_t>(_kvs->getValueAsInt(NW003_KEY_CLOCK_MODE));
  const ESPEasy::net::EthPhyType_t phyType = static_cast<ESPEasy::net::EthPhyType_t>(_kvs->getValueAsInt(NW003_KEY_ETH_PHY_TYPE));

  const int powerPin = _kvs->getValueAsInt(NW003_KEY_ETH_PIN_POWER);

  return isValid(phyType)
         && (isValid(ETH_ClockMode) /* || isSPI_EthernetType(Settings.ETH_Phy_Type)*/)
         && validGpio(_kvs->getValueAsInt(NW003_KEY_ETH_PIN_MDC))
         && (validGpio(_kvs->getValueAsInt(NW003_KEY_ETH_PIN_MDIO)) &&
             (validGpio(powerPin) || (powerPin == -1))
             ); // Some boards have fixed power
}

void NW003_data_struct_ETH_RMII::ethPrintSettings() {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;

    if (log.reserve(115)) {

      const EthClockMode_t ETH_ClockMode       = static_cast<EthClockMode_t>(_kvs->getValueAsInt(NW003_KEY_CLOCK_MODE));
      const ESPEasy::net::EthPhyType_t phyType = static_cast<ESPEasy::net::EthPhyType_t>(_kvs->getValueAsInt(NW003_KEY_ETH_PHY_TYPE));

      log += F("ETH PHY Type: ");
      log += toString(phyType);
      log += F(" PHY Addr: ");
      log += _kvs->getValueAsInt(NW003_KEY_ETH_PHY_ADDR);

      log += F(" Eth Clock mode: ");
      log += ESPEasy::net::toString(ETH_ClockMode);
      log += strformat(F(" MDC: %d MIO: %d PWR: %d"),
                       _kvs->getValueAsInt(NW003_KEY_ETH_PIN_MDC),
                       _kvs->getValueAsInt(NW003_KEY_ETH_PIN_MDIO),
                       _kvs->getValueAsInt(NW003_KEY_ETH_PIN_POWER));
      addLogMove(LOG_LEVEL_INFO, log);
    }
  }
}

bool NW003_data_struct_ETH_RMII::ETHConnectRelaxed() {
  auto data = getNWPluginData_static_runtime();

  auto iface = ESPEasy::net::eth::ETH_NWPluginData_static_runtime::getInterface(_networkIndex);

  if (!(data && iface)) { return false; }

  if (data->started() && data->connected()) {
    return EthLinkUp();
  }
  ethPrintSettings();

  if (!ethCheckSettings())
  {
    addLog(LOG_LEVEL_ERROR, F("ETH: Settings not correct!!!"));
    data->mark_stop();
    return false;
  }

  ethPower(true);
  data->mark_begin_establish_connection();

  bool success = data->started();

  if (!success) {
# if FEATURE_USE_IPV6

    if (Settings.EnableIPv6()) {
      iface->enableIPv6(true);
    }
# endif // if FEATURE_USE_IPV6

    const EthClockMode_t ETH_ClockMode       = static_cast<EthClockMode_t>(_kvs->getValueAsInt(NW003_KEY_CLOCK_MODE));
    const ESPEasy::net::EthPhyType_t phyType = static_cast<ESPEasy::net::EthPhyType_t>(_kvs->getValueAsInt(NW003_KEY_ETH_PHY_TYPE));

    const int phy_addr = _kvs->getValueAsInt(NW003_KEY_ETH_PHY_ADDR);
    const int powerPin = _kvs->getValueAsInt(NW003_KEY_ETH_PIN_POWER);
    const int mdcPin   = _kvs->getValueAsInt(NW003_KEY_ETH_PIN_MDC);
    const int mdioPin  = _kvs->getValueAsInt(NW003_KEY_ETH_PIN_MDIO);

# ifndef ESP32P4
    ethResetGPIOpins();
# endif
    success = iface->begin(
      to_ESP_phy_type(phyType),
      phy_addr,
      mdcPin,
      mdioPin,
      powerPin,
      (eth_clock_mode_t)Settings.ETH_Clock_Mode);
  }

  if (success) {
    // FIXME TD-er: Not sure if this is correctly set to false
    // EthEventData.ethConnectAttemptNeeded = false;

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      PrintToString p2s;
      KeyValueWriter_WebForm writer(false, &p2s);
      writer.setSummaryValueOnly();

      if (write_Eth_Show_Connected(*iface, &writer)) {
        addLog(LOG_LEVEL_INFO, concat(F("ETH  : "), p2s.get()));
      }
    }

    if (EthLinkUp()) {
      // We might miss the connected event, since we are already connected.
      data->mark_connected();
    }
  } else {
    addLog(LOG_LEVEL_ERROR, F("ETH  : Failed to initialize ETH"));
  }
  return success;
}

} // namespace eth
} // namespace net
} // namespace ESPEasy

#endif // ifdef USES_NW003
