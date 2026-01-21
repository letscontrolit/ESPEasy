#pragma once

#include "../../ESPEasy_common.h"

#if defined(USES_C018) || defined(USES_C023)

class LoRa_Helper
{
public:

  /*************************************************
  * LoRaWAN Join Method
  *************************************************/

  enum class LoRaWAN_JoinMethod {
    OTAA = 0,
    ABP = 1, // N.B. C023 (AT-command LoRaWAN) uses 1 = OTAA, 0 = ABP

    //    ,P2P

  };

  static const __FlashStringHelper* toString(LoRaWAN_JoinMethod joinMethod);

  static void                       addLoRaWAN_JoinMethod_FormSelector(
    const __FlashStringHelper *label,
    const __FlashStringHelper *id,
    LoRaWAN_JoinMethod         selectedIndex);

  static void add_joinChanged_script_element_line(const String     & id,
                                                  LoRaWAN_JoinMethod joinMethod);

  /*************************************************
  * LoRaWAN Class
  *************************************************/

  enum class LoRaWANclass_e {
    A, // Class A: Bi-directional, allows downlink only after an uplink transmission
    B, // Class B: Bi-directional, allows periodic downlink windows
    C  // Class C: Bi-directional, allows continuous listening for downlinks

  };

  static void addLoRaWANclass_FormSelector(
    const __FlashStringHelper *label,
    const __FlashStringHelper *id,
    LoRaWANclass_e             selectedIndex);

  /*************************************************
  * LoRaWAN Downlink Event Format
  *************************************************/

  enum class DownlinkEventFormat_e {
    PortNr_in_eventPar,
    PortNr_as_first_eventvalue,
    PortNr_both_eventPar_eventvalue

  };

  static void addEventFormatStructure_FormSelector(
    const __FlashStringHelper *label,
    const __FlashStringHelper *id,
    DownlinkEventFormat_e      selectedIndex);

  /*************************************************
  * LoRaWAN DataRate
  *************************************************/

  enum class LoRaWAN_DR {
    SF12_BW125 = 0,  // (KR920, AS923, EU868)
    SF11_BW125 = 1,  // (KR920, AS923, EU868)
    SF10_BW125 = 2,  // (KR920, AS923, EU868)
    SF9_BW125  = 3,  // (KR920, AS923, EU868)
    SF8_BW125  = 4,  // (KR920, AS923, EU868)
    SF7_BW125  = 5,  // (KR920, AS923, EU868)
    SF7_BW250  = 6,  // (AS923, EU868)
    FSK        = 7,  // (AS923, EU868)
    ADR        = 127 // Just used as internal setting, either it is set to ADR or to a fixed value

  };


  static const __FlashStringHelper* toString(LoRaWAN_DR dr);

  static uint8_t                    getSF(LoRaWAN_DR dr);

  // Get bandwidth in kHz from given data rate
  static uint16_t                   getBW(LoRaWAN_DR dr);

  static void                       addLoRaWAN_DR_FormSelector(
    const __FlashStringHelper *label,
    const __FlashStringHelper *id,
    LoRaWAN_DR                 selectedIndex);

  /*************************************************
  * LoRaWAN Air Time
  *************************************************/

  // Compute the air time of a LoRa packet based on packet length and data rate
  static float getLoRaAirTime(uint8_t    pl,
                              LoRaWAN_DR dr);

}; // class LoRa_Helper

#endif // if defined(USES_C018) || defined(USES_C023)
