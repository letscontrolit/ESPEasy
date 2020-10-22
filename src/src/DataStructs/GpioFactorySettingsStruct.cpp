#include "GpioFactorySettingsStruct.h"

#include "../CustomBuild/ESPEasyDefaults.h"

GpioFactorySettingsStruct::GpioFactorySettingsStruct(DeviceModel model)
  :
  status_led(DEFAULT_PIN_STATUS_LED),
  i2c_sda(DEFAULT_PIN_I2C_SDA),
  i2c_scl(DEFAULT_PIN_I2C_SCL),
  eth_phyaddr(DEFAULT_ETH_PHY_ADDR),
  eth_phytype(DEFAULT_ETH_PHY_TYPE),
  eth_mdc(DEFAULT_ETH_PIN_MDC),
  eth_mdio(DEFAULT_ETH_PIN_MDIO),
  eth_power(DEFAULT_ETH_PIN_POWER),
  eth_clock_mode(DEFAULT_ETH_CLOCK_MODE),
  active_network_medium(DEFAULT_NETWORK_MEDIUM)

{
  for (int i = 0; i < 4; ++i) {
    button[i] = -1;
    relais[i] = -1;
  }

  switch (model) {
    case DeviceModel_Sonoff_Basic:
    case DeviceModel_Sonoff_TH1x:
    case DeviceModel_Sonoff_S2x:
    case DeviceModel_Sonoff_TouchT1:
    case DeviceModel_Sonoff_POWr2:
      button[0]  = 0;  // Single Button
      relais[0]  = 12; // Red Led and Relay (0 = Off, 1 = On)
      status_led = 13; // Green/Blue Led (0 = On, 1 = Off)
      i2c_sda    = -1;
      i2c_scl    = -1;
      break;
    case DeviceModel_Sonoff_POW:
      button[0]  = 0;  // Single Button
      relais[0]  = 12; // Red Led and Relay (0 = Off, 1 = On)
      status_led = 15; // Blue Led (0 = On, 1 = Off)
      i2c_sda    = -1;
      i2c_scl    = -1; // GPIO5 conflicts with HLW8012 Sel output
      break;
    case DeviceModel_Sonoff_TouchT2:
      button[0]  = 0;  // Button 1
      button[1]  = 9;  // Button 2
      relais[0]  = 12; // Led and Relay1 (0 = Off, 1 = On)
      relais[1]  = 4;  // Led and Relay2 (0 = Off, 1 = On)
      status_led = 13; // Blue Led (0 = On, 1 = Off)
      i2c_sda    = -1; // GPIO4 conflicts with GPIO_REL3
      i2c_scl    = -1; // GPIO5 conflicts with GPIO_REL2
      break;
    case DeviceModel_Sonoff_TouchT3:
      button[0]  = 0;  // Button 1
      button[1]  = 10; // Button 2
      button[2]  = 9;  // Button 3
      relais[0]  = 12; // Led and Relay1 (0 = Off, 1 = On)
      relais[1]  = 5;  // Led and Relay2 (0 = Off, 1 = On)
      relais[2]  = 4;  // Led and Relay3 (0 = Off, 1 = On)
      status_led = 13; // Blue Led (0 = On, 1 = Off)
      i2c_sda    = -1; // GPIO4 conflicts with GPIO_REL3
      i2c_scl    = -1; // GPIO5 conflicts with GPIO_REL2
      break;

    case DeviceModel_Sonoff_4ch:
      button[0]  = 0;             // Button 1
      button[1]  = 9;             // Button 2
      button[2]  = 10;            // Button 3
      button[3]  = 14;            // Button 4
      relais[0]  = 12;            // Red Led and Relay1 (0 = Off, 1 = On)
      relais[1]  = 5;             // Red Led and Relay2 (0 = Off, 1 = On)
      relais[2]  = 4;             // Red Led and Relay3 (0 = Off, 1 = On)
      relais[3]  = 15;            // Red Led and Relay4 (0 = Off, 1 = On)
      status_led = 13;            // Blue Led (0 = On, 1 = Off)
      i2c_sda    = -1;            // GPIO4 conflicts with GPIO_REL3
      i2c_scl    = -1;            // GPIO5 conflicts with GPIO_REL2
      break;
    case DeviceModel_Shelly1:
      button[0]  = 5;             // Single Button
      relais[0]  = 4;             // Red Led and Relay (0 = Off, 1 = On)
      status_led = 15;            // Blue Led (0 = On, 1 = Off)
      i2c_sda    = -1;            // GPIO4 conflicts with relay control.
      i2c_scl    = -1;            // GPIO5 conflicts with SW input
      break;
    case DeviceModel_ShellyPLUG_S:
      button[0]  = 13;            // Single Button
      relais[0]  = 15;            // Red Led and Relay (0 = Off, 1 = On)
      status_led = 2;             // Blue Led (0 = On, 1 = Off)
      i2c_sda    = -1;            // GPIO4 conflicts with relay control.
      i2c_scl    = -1;            // GPIO5 conflicts with SW input
      break;
    case DeviceMode_Olimex_ESP32_PoE:
      button[0]             = 34; // BUT1 Button
      relais[0]             = -1; // No LED's or relays on board
      status_led            = -1;
      i2c_sda               = 13;
      i2c_scl               = 16;
      eth_phyaddr           = 0;
      eth_phytype           = EthPhyType_t::LAN8710;
      eth_mdc               = 23;
      eth_mdio              = 18;
      eth_power             = 12;
      eth_clock_mode        = EthClockMode_t::Int_50MHz_GPIO_17_inv;
      active_network_medium = NetworkMedium_t::Ethernet;
      break;
    case DeviceMode_Olimex_ESP32_EVB:
      button[0] = 34; // BUT1 Button
      relais[0] = 32; // LED1 + Relay1 (0 = Off, 1 = On)
      relais[1] = 33; // LED2 + Relay2 (0 = Off, 1 = On)

      status_led            = -1;
      i2c_sda               = 13;
      i2c_scl               = 16;
      eth_phyaddr           = 0;
      eth_phytype           = EthPhyType_t::LAN8710;
      eth_mdc               = 23;
      eth_mdio              = 18;
      eth_power             = -1; // No Ethernet power pin
      eth_clock_mode        = EthClockMode_t::Ext_crystal_osc;
      active_network_medium = NetworkMedium_t::Ethernet;
      break;

    case DeviceMode_Olimex_ESP32_GATEWAY:
      button[0]             = 34; // BUT1 Button
      relais[0]             = -1; // No LED's or relays on board
      status_led            = 33;
      i2c_sda               = -1;
      i2c_scl               = -1;
      eth_phyaddr           = 0;
      eth_phytype           = EthPhyType_t::LAN8710;
      eth_mdc               = 23;
      eth_mdio              = 18;
      eth_power             = 5;
      eth_clock_mode        = EthClockMode_t::Int_50MHz_GPIO_17_inv;
      active_network_medium = NetworkMedium_t::Ethernet;
      // Rev A to E:
      // GPIO 5, 17 can be used only if Ethernet functionality is not used
      // GPIO 6, 7, 8, 9, 10, 11 used for internal flash and SD card
      // GPIO 33 - Status LED
      // GPIO 34 - User button
      // GPIO 16, 32, 35, 36, 39 free to use

      // Rev F and up:
      // GPIO 5, 17 can be used only if Ethernet functionality is not used
      // GPIO 2, 14, 15 are used for SD card, they are free to use if SD is not used.
      // GPIO 33 - Status LED
      // GPIO 34 - User button
      // GPIO 4, 12, 13, 32, 35, 36, 39 free to use

      // ESPEasy default setting:
      // No GPIO pins selected in profile for I2C.
      // Since there are none free to use in all revisions capable of input/output.
      // N.B. GPIO 35 and up are input only.

      break;

    case DeviceModel_default:
    case DeviceModel_MAX:
      break;

      // Do not use default: as this allows the compiler to detect any missing cases.
      // default: break;
  }
}
