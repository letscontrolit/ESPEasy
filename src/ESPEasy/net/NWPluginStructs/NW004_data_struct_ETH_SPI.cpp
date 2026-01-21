#include "../NWPluginStructs/NW004_data_struct_ETH_SPI.h"

#ifdef USES_NW004

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

# define NW_PLUGIN_ID  4

namespace ESPEasy {
namespace net {
namespace eth {


// Keys as used in the Key-value-store
# define NW004_KEY_ETH_INDEX                         1
# define NW004_KEY_ETH_PHY_TYPE                      2
# define NW004_KEY_ETH_PHY_ADDR                      3
# define NW004_KEY_ETH_PIN_CS                        4
# define NW004_KEY_ETH_PIN_IRQ                       5
# define NW004_KEY_ETH_PIN_RST                       6
# define NW004_KEY_IP                                7
# define NW004_KEY_GW                                8
# define NW004_KEY_SN                                9
# define NW004_KEY_DNS                               10

# define NW004_MAX_KEY                               11

const __FlashStringHelper * NW004_data_struct_ETH_SPI::getLabelString(uint32_t key, bool displayString, KVS_StorageType::Enum& storageType)
{
  storageType = KVS_StorageType::Enum::int8_type;

  switch (key)
  {
    case NW004_KEY_ETH_INDEX:  return F("Index");
    case NW004_KEY_ETH_PHY_TYPE: return displayString ? F("Ethernet PHY type") : F("phytype");
    case NW004_KEY_ETH_PHY_ADDR: return displayString ? F("Ethernet PHY Address") : F("phyaddr");
    case NW004_KEY_ETH_PIN_CS: return displayString ? F("Ethernet CS pin") : F("CS");
    case NW004_KEY_ETH_PIN_IRQ: return displayString ? F("Ethernet IRQ pin") : F("IRQ");
    case NW004_KEY_ETH_PIN_RST: return displayString ? F("Ethernet RST pin") : F("RST");
    case NW004_KEY_IP:
      storageType = KVS_StorageType::Enum::string_type;
      return F("IP");
    case NW004_KEY_GW:
      storageType = KVS_StorageType::Enum::string_type;
      return displayString ? F("Gateway") : F("gw");
    case NW004_KEY_SN:
      storageType = KVS_StorageType::Enum::string_type;
      return displayString ? F("Subnetmask") : F("sn");
    case NW004_KEY_DNS:
      storageType = KVS_StorageType::Enum::string_type;
      return F("DNS");
  }
  return F("");
}

int32_t NW004_data_struct_ETH_SPI::getNextKey(int32_t key)
{
  if (key == -1) { return NW004_KEY_ETH_INDEX; }

  if (key >= NW004_MAX_KEY) { return -2; }
  return key + 1;
}

String NW004_data_struct_ETH_SPI::formatGpioLabel(uint32_t          key,
                                                  PinSelectPurpose& purpose,
                                                  bool              shortNotation)
{
  String label;

  if ((key >= NW004_KEY_ETH_PIN_CS) && (key <= NW004_KEY_ETH_PIN_RST))
  {
    KVS_StorageType::Enum storageType;
    purpose = PinSelectPurpose::Generic;
    label   = getLabelString(key, true, storageType);

    const bool optional = !shortNotation;

    switch (key)
    {
      case NW004_KEY_ETH_PIN_IRQ:
        purpose = PinSelectPurpose::Generic_input;
        label   = formatGpioName(
          label, gpio_direction::gpio_input, optional);
        break;
      case NW004_KEY_ETH_PIN_CS:
      case NW004_KEY_ETH_PIN_RST:
        purpose = PinSelectPurpose::Generic_output;
        label   = formatGpioName(
          label, gpio_direction::gpio_output, optional);
        break;
    }
  }
  return label;
}

WebFormItemParams NW004_makeWebFormItemParams(uint32_t key) {
  KVS_StorageType::Enum storageType;
  const __FlashStringHelper*label = NW004_data_struct_ETH_SPI::getLabelString(key, true, storageType);
  const __FlashStringHelper*id    = NW004_data_struct_ETH_SPI::getLabelString(key, false, storageType);

  return WebFormItemParams(label, id, storageType, key);
}

NW004_data_struct_ETH_SPI::NW004_data_struct_ETH_SPI(networkIndex_t networkIndex, NetworkInterface *netif)
  : NWPluginData_base(nwpluginID_t(NW_PLUGIN_ID), networkIndex, netif)
{}

NW004_data_struct_ETH_SPI::~NW004_data_struct_ETH_SPI()
{
  ESPEasy::net::eth::ETH_NWPluginData_static_runtime::exit(_networkIndex);
}

void NW004_data_struct_ETH_SPI::loadDefaults(ESPEasy_key_value_store     *kvs,
                                             ESPEasy::net::networkIndex_t networkIndex,
                                             ESPEasy::net::nwpluginID_t   nwPluginID)
{
  // Only load the defaults when the settings are empty
  if (kvs && kvs->isEmpty()) {
    if (isValid(Settings.ETH_Phy_Type) &&
        ESPEasy::net::isSPI_EthernetType(Settings.ETH_Phy_Type))
    {
      kvs->setValue(NW004_KEY_ETH_INDEX,    static_cast<int8_t>(-1));
      kvs->setValue(NW004_KEY_ETH_PHY_TYPE, static_cast<int8_t>(Settings.ETH_Phy_Type));
      kvs->setValue(NW004_KEY_ETH_PHY_ADDR, static_cast<int8_t>(Settings.ETH_Phy_Addr));
      kvs->setValue(NW004_KEY_ETH_PIN_CS,   static_cast<int8_t>(Settings.ETH_Pin_mdc_cs));
      kvs->setValue(NW004_KEY_ETH_PIN_IRQ,  static_cast<int8_t>(Settings.ETH_Pin_mdio_irq));
      kvs->setValue(NW004_KEY_ETH_PIN_RST,  static_cast<int8_t>(Settings.ETH_Pin_power_rst));

      // TODO TD-er: Copy IP info

      store_nwpluginTaskData_KVS(kvs, networkIndex, nwPluginID);
    }
    # if defined(DEFAULT_ETH_PHY_TYPE) && defined(DEFAULT_ETH_CLOCK_MODE) && defined(DEFAULT_ETH_PHY_ADDR) && \
    defined(DEFAULT_ETH_PIN_MDC) && defined(DEFAULT_ETH_PIN_MDIO)
    else if (isValid(DEFAULT_ETH_PHY_TYPE) &&
             ESPEasy::net::isSPI_EthernetType(DEFAULT_ETH_PHY_TYPE))
    {
      kvs->setValue(NW004_KEY_ETH_INDEX,    static_cast<int8_t>(-1));
      kvs->setValue(NW004_KEY_ETH_PHY_TYPE, static_cast<int8_t>(DEFAULT_ETH_PHY_TYPE));
      kvs->setValue(NW004_KEY_ETH_PHY_ADDR, static_cast<int8_t>(DEFAULT_ETH_PHY_ADDR));
      kvs->setValue(NW004_KEY_ETH_PIN_CS,   static_cast<int8_t>(DEFAULT_ETH_PIN_MDC));
      kvs->setValue(NW004_KEY_ETH_PIN_IRQ,  static_cast<int8_t>(DEFAULT_ETH_PIN_MDIO));
      kvs->setValue(NW004_KEY_ETH_PIN_RST,  static_cast<int8_t>(DEFAULT_ETH_PIN_POWER));

      // TODO TD-er: Copy IP info

      store_nwpluginTaskData_KVS(kvs, networkIndex, nwPluginID);
    }
    # endif // if defined(DEFAULT_ETH_PHY_TYPE) && defined(DEFAULT_ETH_CLOCK_MODE) && defined(DEFAULT_ETH_PHY_ADDR) &&
    // defined(DEFAULT_ETH_PIN_MDC) && defined(DEFAULT_ETH_PIN_MDIO)
  }
}

void NW004_data_struct_ETH_SPI::webform_load(EventStruct *event)
{
  _load();
  NW004_data_struct_ETH_SPI::loadDefaults(_kvs, event->NetworkIndex, nwpluginID_t(4));
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
# if ESP_IDF_VERSION_MAJOR >= 5
#  if CONFIG_ETH_SPI_ETHERNET_DM9051
      toString(EthPhyType_t::DM9051),
#  endif // if CONFIG_ETH_SPI_ETHERNET_DM9051
#  if CONFIG_ETH_SPI_ETHERNET_W5500
      toString(EthPhyType_t::W5500),
#  endif // if CONFIG_ETH_SPI_ETHERNET_W5500
#  if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
      toString(EthPhyType_t::KSZ8851),
#  endif // if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
# endif // if ESP_IDF_VERSION_MAJOR >= 5
    };
    const int ids[] = {
      static_cast<int>(EthPhyType_t::notSet),

# if ESP_IDF_VERSION_MAJOR >= 5
#  if CONFIG_ETH_SPI_ETHERNET_DM9051
      static_cast<int>(EthPhyType_t::DM9051),
#  endif // if CONFIG_ETH_SPI_ETHERNET_DM9051
#  if CONFIG_ETH_SPI_ETHERNET_W5500
      static_cast<int>(EthPhyType_t::W5500),
#  endif // if CONFIG_ETH_SPI_ETHERNET_W5500
#  if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
      static_cast<int>(EthPhyType_t::KSZ8851),
#  endif // if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
# endif // if ESP_IDF_VERSION_MAJOR >= 5
    };

    FormSelectorOptions selector(NR_ELEMENTS(ids), options, ids);
    auto params = NW004_makeWebFormItemParams(NW004_KEY_ETH_PHY_TYPE);
    params._defaultIntValue = static_cast<int>(EthPhyType_t::notSet);
    showFormSelector(*_kvs, selector, params);
  }
  {
    auto params = NW004_makeWebFormItemParams(NW004_KEY_ETH_PHY_ADDR);
    params._min             = -1;
    params._max             = 127;
    params._defaultIntValue = 1;
    showWebformItem(*_kvs, params);
    addFormNote(F("I&sup2;C-address of Ethernet PHY"));
  }
  {
    const int gpio_keys[] = {
      NW004_KEY_ETH_PIN_CS,
      NW004_KEY_ETH_PIN_IRQ,
      NW004_KEY_ETH_PIN_RST
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
}

void NW004_data_struct_ETH_SPI::webform_save(EventStruct *event)
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

bool NW004_data_struct_ETH_SPI::webform_getPort(KeyValueWriter *writer) { return true; }

bool NW004_data_struct_ETH_SPI::init(EventStruct *event)
{
  _load();
  ETHConnectRelaxed();

  return true;
}

bool                          NW004_data_struct_ETH_SPI::exit(EventStruct *event)         { return true; }

NWPluginData_static_runtime * NW004_data_struct_ETH_SPI::getNWPluginData_static_runtime() {
  return ESPEasy::net::eth::ETH_NWPluginData_static_runtime::getNWPluginData_static_runtime(_networkIndex);
}

bool NW004_data_struct_ETH_SPI::write_Eth_HW_Address(KeyValueWriter *writer)
{
  if (writer == nullptr) { return false; }
  auto data  = getNWPluginData_static_runtime();
  auto iface = ESPEasy::net::eth::ETH_NWPluginData_static_runtime::getInterface(_networkIndex);

  if (!(data && iface)) { return false; }
  const int phyType_int = _kvs->getValueAsInt_or_default(NW004_KEY_ETH_PHY_TYPE, -1);

  if (phyType_int == -1) { return false; }
  return ESPEasy::net::write_Eth_HW_Address(
    static_cast<ESPEasy::net::EthPhyType_t>(phyType_int),
    iface,
    writer);
}

bool NW004_data_struct_ETH_SPI::write_Eth_port(KeyValueWriter *writer)
{
  if (writer == nullptr) { return false; }
  const ESPEasy::net::EthPhyType_t phyType = static_cast<ESPEasy::net::EthPhyType_t>(_kvs->getValueAsInt(NW004_KEY_ETH_PHY_TYPE));

  if (!isValid(phyType)) { return false; }

  int8_t spi_gpios[3]{};

  if (!Settings.getSPI_pins(spi_gpios)) { return false; }
  const __FlashStringHelper*labels[] = {
    F("CLK"), F("MISO"), F("MOSI"), F("CS"), F("IRQ"), F("RST") };
  const int pins[] = {
    spi_gpios[0],
    spi_gpios[1],
    spi_gpios[2],
    (int)_kvs->getValueAsInt(NW004_KEY_ETH_PIN_CS),
    (int)_kvs->getValueAsInt(NW004_KEY_ETH_PIN_IRQ),
    (int)_kvs->getValueAsInt(NW004_KEY_ETH_PIN_RST) };
  return write_NetworkPort(labels, pins, NR_ELEMENTS(labels), writer);
}

void NW004_data_struct_ETH_SPI::ethResetGPIOpins() {
  // fix an disconnection issue after rebooting Olimex POE - this forces a clean state for all GPIO involved in RMII
  // Thanks to @s-hadinger and @Jason2866
  // Resetting state of power pin is done in ethPower()
  addLog(LOG_LEVEL_INFO, F("ethResetGPIOpins()"));
  gpio_reset_pin((gpio_num_t)_kvs->getValueAsInt(NW004_KEY_ETH_PIN_CS));
  gpio_reset_pin((gpio_num_t)_kvs->getValueAsInt(NW004_KEY_ETH_PIN_IRQ));
# if CONFIG_ETH_USE_ESP32_EMAC && FEATURE_ETHERNET

  gpio_reset_pin(GPIO_NUM_19); // EMAC_TXD0 - hardcoded
  gpio_reset_pin(GPIO_NUM_21); // EMAC_TX_EN - hardcoded
  gpio_reset_pin(GPIO_NUM_22); // EMAC_TXD1 - hardcoded
  gpio_reset_pin(GPIO_NUM_25); // EMAC_RXD0 - hardcoded
  gpio_reset_pin(GPIO_NUM_26); // EMAC_RXD1 - hardcoded
  gpio_reset_pin(GPIO_NUM_27); // EMAC_RX_CRS_DV - hardcoded
# endif // if CONFIG_ETH_USE_ESP32_EMAC && FEATURE_ETHERNET

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

bool NW004_data_struct_ETH_SPI::ethCheckSettings() {
  const ESPEasy::net::EthPhyType_t phyType = static_cast<ESPEasy::net::EthPhyType_t>(_kvs->getValueAsInt(NW004_KEY_ETH_PHY_TYPE));

  const int rstPin = _kvs->getValueAsInt_or_default(NW004_KEY_ETH_PIN_RST, -1);
  const int irqPin = _kvs->getValueAsInt_or_default(NW004_KEY_ETH_PIN_IRQ, -1);

  return isValid(phyType)
         && validGpio(_kvs->getValueAsInt(NW004_KEY_ETH_PIN_CS))
         && ((validGpio(irqPin) || (irqPin == -1)) &&
             (validGpio(rstPin) || (rstPin == -1))
             ); // IRQ and RST are optional
}

void NW004_data_struct_ETH_SPI::ethPrintSettings() {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;

    if (log.reserve(115)) {

      const ESPEasy::net::EthPhyType_t phyType = static_cast<ESPEasy::net::EthPhyType_t>(_kvs->getValueAsInt(NW004_KEY_ETH_PHY_TYPE));

      log += F("ETH PHY Type: ");
      log += toString(phyType);
      log += F(" PHY Addr: ");
      log += _kvs->getValueAsInt(NW004_KEY_ETH_PHY_ADDR);
      log += strformat(F(" CS: %d IRQ: %d RST: %d"),
                       _kvs->getValueAsInt(NW004_KEY_ETH_PIN_CS),
                       _kvs->getValueAsInt(NW004_KEY_ETH_PIN_IRQ),
                       _kvs->getValueAsInt(NW004_KEY_ETH_PIN_RST));
      addLogMove(LOG_LEVEL_INFO, log);
    }
  }
}

bool NW004_data_struct_ETH_SPI::ETHConnectRelaxed() {
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

  data->mark_begin_establish_connection();

  bool success = data->started();

  if (!success) {
# if FEATURE_USE_IPV6

    if (Settings.EnableIPv6()) {
      iface->enableIPv6(true);
    }
# endif // if FEATURE_USE_IPV6

    const ESPEasy::net::EthPhyType_t phyType = static_cast<ESPEasy::net::EthPhyType_t>(_kvs->getValueAsInt(NW004_KEY_ETH_PHY_TYPE));

    const int phy_addr = _kvs->getValueAsInt(NW004_KEY_ETH_PHY_ADDR);
    const int rstPin   = _kvs->getValueAsInt(NW004_KEY_ETH_PIN_RST);
    const int csPin    = _kvs->getValueAsInt(NW004_KEY_ETH_PIN_CS);
    const int irqPin   = _kvs->getValueAsInt(NW004_KEY_ETH_PIN_IRQ);


    spi_host_device_t SPI_host = Settings.getSPI_host();

    if (SPI_host == spi_host_device_t::SPI_HOST_MAX) {
      addLog(LOG_LEVEL_ERROR, F("SPI not enabled"));
        # ifdef ESP32C3

      // FIXME TD-er: Fallback for ETH01-EVO board
      SPI_host              = spi_host_device_t::SPI2_HOST;
      Settings.InitSPI      = static_cast<int>(SPI_Options_e::UserDefined);
      Settings.SPI_SCLK_pin = 7;
      Settings.SPI_MISO_pin = 3;
      Settings.SPI_MOSI_pin = 10;
        # endif // ifdef ESP32C3
    }

    // else
    {
# if ETH_SPI_SUPPORTS_CUSTOM
      success = iface->begin(
        to_ESP_phy_type(phyType),
        phy_addr,
        csPin,
        irqPin,
        rstPin,
        SPI);
# else // if ETH_SPI_SUPPORTS_CUSTOM
      success = iface->begin(
        to_ESP_phy_type(phyType),
        phy_addr,
        csPin,
        irqPin,
        rstPin,
        SPI_host,
        static_cast<int>(Settings.SPI_SCLK_pin),
        static_cast<int>(Settings.SPI_MISO_pin),
        static_cast<int>(Settings.SPI_MOSI_pin));
# endif // if ETH_SPI_SUPPORTS_CUSTOM
    }
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

#endif // ifdef USES_NW004
