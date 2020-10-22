#include "_Plugin_Helper.h"
#ifdef USES_P084

// #######################################################################################################
// #################################### Plugin 084: VEML6070 UV ##########################################
// #######################################################################################################

// ESPEasy Plugin to for UV with chip VEML6070
// written by Remco van Essen (https://github.com/RemCom)
// Based on VEML6070 plugin from Sonoff-Tasmota (https://github.com/arendst/Sonoff-Tasmota)
// Datasheet: https://www.vishay.com/docs/84277/veml6070.pdf


#define PLUGIN_084
#define PLUGIN_ID_084         84
#define PLUGIN_NAME_084       "UV - VEML6070 [TESTING]"
#define PLUGIN_VALUENAME1_084 "UV-Raw"
#define PLUGIN_VALUENAME2_084 "UV-Risk"
#define PLUGIN_VALUENAME3_084 "UV-Power"

#define VEML6070_ADDR_H             0x39
#define VEML6070_ADDR_L             0x38
#define VEML6070_RSET_DEFAULT       270000      // 270K default resistor value 270000 ohm, range from 220K..1Meg
#define VEML6070_UV_MAX_INDEX       15          // normal 11, internal on weather laboratories and NASA it's 15 so far the sensor is linear
#define VEML6070_UV_MAX_DEFAULT     11          // 11 = public default table values
#define VEML6070_POWER_COEFFCIENT   0.025       // based on calculations from Karel Vanicek and reorder by hand
#define VEML6070_TABLE_COEFFCIENT   32.86270591 // calculated by hand with help from a friend of mine, a professor which works in aero space
                                                // things
                                                // (resistor, differences, power coefficients and official UV index calculations (LAT & LONG
                                                // will be added later)

#define VEML6070_base_value ((VEML6070_RSET_DEFAULT / VEML6070_TABLE_COEFFCIENT) / VEML6070_UV_MAX_DEFAULT) * (1)
#define VEML6070_max_value  ((VEML6070_RSET_DEFAULT / VEML6070_TABLE_COEFFCIENT) / VEML6070_UV_MAX_DEFAULT) * (VEML6070_UV_MAX_INDEX)

boolean Plugin_084(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_084;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = false;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_084);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_084));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_084));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_084));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      String optionsMode[4] = { F("1/2T"), F("1T"), F("2T"), F("4T (Default)") };
      addFormSelector(F("Refresh Time Determination"), F("itime"), 4, optionsMode, NULL, PCONFIG(0));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("itime"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      bool status = VEML6070_Init(PCONFIG(0));

      if (!status) {
        addLog(LOG_LEVEL_INFO, F("VEML6070: Not available!"));
      }

      success = status;
      break;
    }

    case PLUGIN_READ:
    {
      uint16_t uv_raw;
      double   uv_risk, uv_power;
      bool     read_status;

      uv_raw   = VEML6070_ReadUv(&read_status); // get UV raw values
      uv_risk  = VEML6070_UvRiskLevel(uv_raw);  // get UV risk level
      uv_power = VEML6070_UvPower(uv_risk);     // get UV power in W/m2

      if (isnan(uv_raw) || (uv_raw == 65535) || !read_status) {
        addLog(LOG_LEVEL_INFO, F("VEML6070: no data read!"));
        UserVar[event->BaseVarIndex + 0] = NAN;
        UserVar[event->BaseVarIndex + 1] = NAN;
        UserVar[event->BaseVarIndex + 2] = NAN;
        success                          = false;
      } else {
        UserVar[event->BaseVarIndex + 0] = uv_raw;
        UserVar[event->BaseVarIndex + 1] = uv_risk;
        UserVar[event->BaseVarIndex + 2] = uv_power;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("VEML6070: UV: ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);
        }

        success = true;
      }

      break;
    }
  }
  return success;
}

//////////////
// VEML6070 //
//////////////

// get UV raw values
uint16_t VEML6070_ReadUv(bool *status)
{
  uint16_t uv_raw      = 0;
  bool     wire_status = false;

  uv_raw   = I2C_read8(VEML6070_ADDR_H, &wire_status);
  *status  = wire_status;
  uv_raw <<= 8;
  uv_raw  |= I2C_read8(VEML6070_ADDR_L, &wire_status);
  *status &= wire_status;

  return uv_raw;
}

bool VEML6070_Init(byte it)
{
  boolean succes = I2C_write8(VEML6070_ADDR_L, ((it << 2) | 0x02));

  return succes;
}

// Definition of risk numbers
//  0.0 - 2.9  "Low"      = sun->fun
//  3.0 - 5.9  "Mid"      = sun->glases advised
//  6.0 - 7.9  "High"     = sun->glases a must
//  8.0 - 10.9 "Danger"   = sun->skin burns Level 1
// 11.0 - 12.9 "BurnL1/2" = sun->skin burns level 1..2
// 13.0 - 25.0 "BurnL3"   = sun->skin burns with level 3

double VEML6070_UvRiskLevel(uint16_t uv_level)
{
  double risk = 0;

  if (uv_level < VEML6070_max_value) {
    return (double)uv_level / VEML6070_base_value;
  } else {
    // out of range and much to high - it must be outerspace or sensor damaged
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("VEML6070 out of range: ");
      log += risk;
      addLog(LOG_LEVEL_INFO, log);
    }

    return 99;
  }
}

double VEML6070_UvPower(double uvrisk)
{
  // based on calculations for effective irradiation from Karel Vanicek
  return VEML6070_POWER_COEFFCIENT * uvrisk;
}

#endif // USES_P084
