#ifndef PLUGINSTRUCTS_P103_DATA_STRUCT_H
#define PLUGINSTRUCTS_P103_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"

#ifdef USES_P103

# include "src/Helpers/Rules_calculate.h"

enum class AtlasEZO_Sensors_e : uint8_t {
  UNKNOWN = 0u,
  PH      = 1u,
  ORP     = 2u,
  EC      = 3u,
  DO      = 4u,
};

# define P103_BOARD_TYPE                PCONFIG(0)
# define P103_I2C_ADDRESS               PCONFIG(1)
# define P103_STATUS_LED                PCONFIG(2)
# define P103_SENSOR_VERSION            PCONFIG_FLOAT(0)
# define P103_CALIBRATION_SINGLE        PCONFIG_FLOAT(1)
# define P103_CALIBRATION_LOW           PCONFIG_FLOAT(2)
# define P103_CALIBRATION_HIGH          PCONFIG_FLOAT(3)

# define ATLAS_EZO_RETURN_ARRAY_SIZE    33 // Max expected result 32 bytes + \0

# define P103_ATLASEZO_I2C_NB_OPTIONS   4  // was: 6 see comment below at 'const int i2cAddressValues'

# define P103_FIXED_TEMP_VALUE          20 // Temperature correction for pH and EC sensor if no temperature is given from calculation

bool P103_send_I2C_command(uint8_t       I2Caddress,
                           const String& cmd,
                           char         *sensordata); // Forward declarations

void P103_addDisabler();
void P103_html_color_message(const __FlashStringHelper *color,
                             const String             & message);
void P103_html_red(const String& message);
void P103_html_orange(const String& message);
void P103_html_green(const String& message);
int  P103_getCalibrationPoints(uint8_t i2cAddress);
void P103_addClearCalibration();
int  P103_addDOCalibration(uint8_t I2Cchoice);
void P103_addCreateDryCalibration();
int  P103_addCreateSinglePointCalibration(AtlasEZO_Sensors_e  board_type,
                                          struct EventStruct *event,
                                          uint8_t             I2Cchoice,
                                          String              unit,
                                          float               min,
                                          float               max,
                                          uint8_t             nrDecimals,
                                          float               stepsize);
int P103_addCreate3PointCalibration(AtlasEZO_Sensors_e  board_type,
                                    struct EventStruct *event,
                                    uint8_t             I2Cchoice,
                                    String              unit,
                                    float               min,
                                    float               max,
                                    uint8_t             nrDecimals,
                                    float               stepsize);

#endif  // ifdef USED_P103
#endif  // ifndef PLUGINSTRUCTS_P103_DATA_STRUCT_H
