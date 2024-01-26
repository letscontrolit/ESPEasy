#include "../DataStructs/FactoryDefault_Network_NVS.h"

#ifdef ESP32

# include "../Globals/Settings.h"
# include "../Helpers/StringConverter.h"

// Max. 15 char keys for ESPEasy Factory Default marked keys
# define FACTORY_DEFAULT_NVS_WIFI_IP_KEY       "WiFI_IP"
# ifdef FEATURE_ETHERNET
#  define FACTORY_DEFAULT_NVS_ETH_IP_KEY       "ETH_IP"
#  define FACTORY_DEFAULT_NVS_ETH_HW_CONF_KEY  "ETH_HW_CONF"
# endif // ifdef FEATURE_ETHERNET

bool FactoryDefault_Network_NVS::applyToSettings_from_NVS(ESPEasy_NVS_Helper& preferences)
{
  bool res = false;

  uint8_t *write = reinterpret_cast<uint8_t *>(&Settings);

  if (preferences.getPreference(F(FACTORY_DEFAULT_NVS_WIFI_IP_KEY), IP_data, sizeof(IP_data))) {
    // TD-er: This data is stored in sequence, so we can do a single memcpy call
    constexpr unsigned int offset = offsetof(SettingsStruct, IP);
    memcpy(write + offset, IP_data, sizeof(IP_data));
    res = true;
  }
# ifdef FEATURE_ETHERNET

  if (preferences.getPreference(F(FACTORY_DEFAULT_NVS_ETH_IP_KEY), IP_data, sizeof(IP_data))) {
    constexpr unsigned int offset = offsetof(SettingsStruct, ETH_IP);

    memcpy(write + offset, IP_data, sizeof(IP_data));
    res = true;
  }

  if (preferences.getPreference(F(FACTORY_DEFAULT_NVS_ETH_HW_CONF_KEY), ETH_HW_conf)) {
    Settings.ETH_Phy_Addr   = bits.ETH_Phy_Addr;
    Settings.ETH_Pin_mdc_cs    = bits.ETH_Pin_mdc_cs;
    Settings.ETH_Pin_mdio_irq   = bits.ETH_Pin_mdio_irq;
    Settings.ETH_Pin_power_rst  = bits.ETH_Pin_power_rst;
    Settings.ETH_Phy_Type   = static_cast<EthPhyType_t>(bits.ETH_Phy_Type);
    Settings.ETH_Clock_Mode = static_cast<EthClockMode_t>(bits.ETH_Clock_Mode);
    Settings.NetworkMedium  = static_cast<NetworkMedium_t>(bits.NetworkMedium);
    res                     = true;
  }
# endif // ifdef FEATURE_ETHERNET
  return res;
}

void FactoryDefault_Network_NVS::fromSettings_to_NVS(ESPEasy_NVS_Helper& preferences)
{
  const uint8_t *read = reinterpret_cast<const uint8_t *>(&Settings);

  {
    constexpr unsigned int offset = offsetof(SettingsStruct, IP);
    memcpy(IP_data, read + offset, sizeof(IP_data));
    preferences.setPreference(F(FACTORY_DEFAULT_NVS_WIFI_IP_KEY), IP_data, sizeof(IP_data));
  }
# ifdef FEATURE_ETHERNET
  {
    constexpr unsigned int offset = offsetof(SettingsStruct, ETH_IP);
    memcpy(IP_data, read + offset, sizeof(IP_data));
    preferences.setPreference(F(FACTORY_DEFAULT_NVS_ETH_IP_KEY), IP_data, sizeof(IP_data));
  }
  {
    bits.ETH_Phy_Addr   = Settings.ETH_Phy_Addr;
    bits.ETH_Pin_mdc_cs    = Settings.ETH_Pin_mdc_cs;
    bits.ETH_Pin_mdio_irq   = Settings.ETH_Pin_mdio_irq;
    bits.ETH_Pin_power_rst  = Settings.ETH_Pin_power_rst;
    bits.ETH_Phy_Type   = static_cast<uint8_t>(Settings.ETH_Phy_Type);
    bits.ETH_Clock_Mode = static_cast<uint8_t>(Settings.ETH_Clock_Mode);
    bits.NetworkMedium  = static_cast<uint8_t>(Settings.NetworkMedium);
    preferences.setPreference(F(FACTORY_DEFAULT_NVS_ETH_HW_CONF_KEY), ETH_HW_conf);
  }
# endif // ifdef FEATURE_ETHERNET
}

void FactoryDefault_Network_NVS::clear_from_NVS(ESPEasy_NVS_Helper& preferences)
{
  preferences.remove(F(FACTORY_DEFAULT_NVS_WIFI_IP_KEY));
# ifdef FEATURE_ETHERNET
  preferences.remove(F(FACTORY_DEFAULT_NVS_ETH_IP_KEY));
  preferences.remove(F(FACTORY_DEFAULT_NVS_ETH_IP_KEY));
# endif // ifdef FEATURE_ETHERNET
}

#endif  // ifdef ESP32
