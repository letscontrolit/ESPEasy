#ifndef CONTROLLER_CONFIG_C023_CONFIG_H
#define CONTROLLER_CONFIG_C023_CONFIG_H

#include "../Helpers/_CPlugin_Helper.h"

#ifdef USES_C023

# include "../Helpers/_CPlugin_Helper_LoRa.h"

// Forward declaration
struct C023_data_struct;


# define C023_DEVICE_EUI_LEN          17
# define C023_DEVICE_ADDR_LEN         33
# define C023_NETWORK_SESSION_KEY_LEN 33
# define C023_APP_SESSION_KEY_LEN     33

// N.B. numerical value for OTAA/ABP differs from the enum value
# define C023_USE_ABP                 0
# define C023_USE_OTAA                1

struct C023_ConfigStruct
{


  C023_ConfigStruct() = default;

  void                               validate();

  void                               reset();

  // Send all to the web interface
  void                               webform_load(C023_data_struct*C023_data);

  // Collect all data from the web interface
  void                               webform_save();

  LoRa_Helper::LoRaWANclass_e        getClass() const       { return static_cast<LoRa_Helper::LoRaWANclass_e>(LoRaWAN_Class); }

  LoRa_Helper::DownlinkEventFormat_e getEventFormat() const { return static_cast<LoRa_Helper::DownlinkEventFormat_e>(eventFormat); }

  LoRa_Helper::LoRaWAN_DR            getDR() const          { return static_cast<LoRa_Helper::LoRaWAN_DR>(dr); }

  LoRa_Helper::LoRaWAN_JoinMethod    getJoinMethod() const  { 
    return static_cast<LoRa_Helper::LoRaWAN_JoinMethod>(joinmethod);
   }

  char          DeviceEUI[C023_DEVICE_EUI_LEN]                  = { 0 };
  char          DeviceAddr[C023_DEVICE_ADDR_LEN]                = { 0 };
  char          NetworkSessionKey[C023_NETWORK_SESSION_KEY_LEN] = { 0 };
  char          AppSessionKey[C023_APP_SESSION_KEY_LEN]         = { 0 };
  unsigned long baudrate                                        = 9600;
  int8_t        rxpin                                           = -1;
  int8_t        txpin                                           = -1;
  int8_t        resetpin                                        = -1;
  uint8_t       dr                                              = static_cast<uint8_t>(LoRa_Helper::LoRaWAN_DR::ADR);
  uint8_t       eventFormat                                     =
    static_cast<uint8_t>(LoRa_Helper::DownlinkEventFormat_e::PortNr_in_eventPar);
  uint8_t  joinmethod    = 0; // LoRa_Helper::LoRaWAN_JoinMethod::OTAA
  uint8_t  serialPort    = 0;
  uint8_t  LoRaWAN_Class = static_cast<uint8_t>(LoRa_Helper::LoRaWANclass_e::A);
  uint8_t  LoRa_module   = 0; // static_cast<uint8_t>(C023_AT_commands::LoRaModule_e::Dragino_LA66);
  uint32_t rx2_freq      = 0;

};

DEF_UP(C023_ConfigStruct);

#endif // ifdef USES_C023

#endif // ifndef CONTROLLER_CONFIG_C023_CONFIG_H
