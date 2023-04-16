#ifndef CONTROLLER_CONFIG_C018_CONFIG_H
#define CONTROLLER_CONFIG_C018_CONFIG_H

#include "src/Helpers/_CPlugin_Helper.h"

#ifdef USES_C018

// Forward declaration
struct C018_data_struct;

# include <rn2xx3.h>


# define C018_DEVICE_EUI_LEN          17
# define C018_DEVICE_ADDR_LEN         33
# define C018_NETWORK_SESSION_KEY_LEN 33
# define C018_APP_SESSION_KEY_LEN     33
# define C018_USE_OTAA                0
# define C018_USE_ABP                 1

struct C018_ConfigStruct
{
  C018_ConfigStruct() = default;

  void validate();

  void reset();

  // Send all to the web interface
  void webform_load(C018_data_struct* C018_data);

  // Collect all data from the web interface
  void webform_save();

  char          DeviceEUI[C018_DEVICE_EUI_LEN]                  = { 0 };
  char          DeviceAddr[C018_DEVICE_ADDR_LEN]                = { 0 };
  char          NetworkSessionKey[C018_NETWORK_SESSION_KEY_LEN] = { 0 };
  char          AppSessionKey[C018_APP_SESSION_KEY_LEN]         = { 0 };
  unsigned long baudrate                                        = 57600;
  int8_t        rxpin                                           = -1;
  int8_t        txpin                                           = -1;
  int8_t        resetpin                                        = -1;
  uint8_t       sf                                              = 7;
  uint8_t       frequencyplan                                   = RN2xx3_datatypes::Freq_plan::TTN_EU;
  uint8_t       joinmethod                                      = C018_USE_OTAA;
  uint8_t       serialPort                                      = 0;
  uint8_t       stackVersion                                    = RN2xx3_datatypes::TTN_stack_version::TTN_v2;
  uint8_t       adr                                             = 0;
  uint32_t      rx2_freq                                        = 0;
};


#endif // ifdef USES_C018

#endif // ifndef CONTROLLER_CONFIG_C018_CONFIG_H
