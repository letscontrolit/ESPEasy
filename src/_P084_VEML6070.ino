#ifdef USES_P084
//#######################################################################################################
//#################################### Plugin 084: VEML6070 UV ##########################################
//#######################################################################################################

// ESPEasy Plugin to for UV with chip VEML6070
// written by Remco van Essen (https://github.com/RemCom)
// Based on VEML6070 plugin from Sonoff-Tasmota (https://github.com/arendst/Sonoff-Tasmota)


#define PLUGIN_084
#define PLUGIN_ID_084         84
#define PLUGIN_NAME_084       "UV - VEML6070 [TESTING]"
#define PLUGIN_VALUENAME1_084 "UV-Raw"
#define PLUGIN_VALUENAME2_084 "UV-Risk"
#define PLUGIN_VALUENAME3_084 "UV-Power"

#define VEML6070_ADDR_H             0x39           
#define VEML6070_ADDR_L             0x38     
#define VEML6070_RSET_DEFAULT       270000          // 270K default resistor value 270000 ohm, range from 220K..1Meg
#define VEML6070_UV_MAX_INDEX       15              // normal 11, internal on weather laboratories and NASA it's 15 so far the sensor is linear
#define VEML6070_UV_MAX_DEFAULT     11              // 11 = public default table values
#define VEML6070_POWER_COEFFCIENT   0.025           // based on calculations from Karel Vanicek and reorder by hand
#define VEML6070_TABLE_COEFFCIENT   32.86270591     // calculated by hand with help from a friend of mine, a professor which works in aero space things
                                                    // (resistor, differences, power coefficients and official UV index calculations (LAT & LONG will be added later)
#define D_UV_INDEX_1  "Low"      // = sun->fun
#define D_UV_INDEX_2  "Mid"      // = sun->glases advised
#define D_UV_INDEX_3  "High"     // = sun->glases a must
#define D_UV_INDEX_4  "Danger"   // = sun->skin burns Level 1
#define D_UV_INDEX_5  "BurnL1/2" // = sun->skin burns level 1..2
#define D_UV_INDEX_6  "BurnL3"   // = sun->skin burns with level 3
#define D_UV_INDEX_7  "OoR"      // = out of range or unknown

#include <math.h>  

double     uv_risk_map[VEML6070_UV_MAX_INDEX] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
char       str_uvrisk_text[10];

boolean Plugin_084(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_084;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = false;
        Device[deviceCount].GlobalSyncOption = true;
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
        addFormSelector(F("Refresh Time Determination"), F("itime"), 4, optionsMode, NULL, PCONFIG(1));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        //PCONFIG(0) = getFormItemInt(F("i2c_addr"));
        PCONFIG(1) = getFormItemInt(F("itime"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        bool error = VEML6070_Init(PCONFIG(1));
        if(error){
          String log = F("VEML6070: Not available!");
          addLog(LOG_LEVEL_INFO, log);
        }
        
        VEML6070_UvTableInit();

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        uint16_t uv_raw;
        double uv_risk, uv_power;

        uv_raw   = VEML6070_ReadUv();             // get UV raw values
        uv_risk  = VEML6070_UvRiskLevel(uv_raw);  // get UV risk level
        uv_power = VEML6070_UvPower(uv_risk);     // get UV power in W/m2

        if (isnan(uv_raw) || uv_raw == (-1) || uv_raw == 65535) {
          String log = F("VEML6070: no data read!");
          addLog(LOG_LEVEL_INFO, log);
          UserVar[event->BaseVarIndex + 0] = NAN;
          UserVar[event->BaseVarIndex + 1] = NAN;
          UserVar[event->BaseVarIndex + 2] = NAN;
		      success = false;
        } else {
          UserVar[event->BaseVarIndex + 0] = uv_raw;
          UserVar[event->BaseVarIndex + 1] = uv_risk;
          UserVar[event->BaseVarIndex + 2] = uv_power;
          String log = F("VEML6070: UV: ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);
		      success = true;
        }

        success = true;
        break;
      }

  }
  return success;
}


//////////////
// VEML6070 //
//////////////

//get UV raw values
uint16_t VEML6070_ReadUv(void)
{
  uint16_t uv_raw = 0;
  // read high byte
  if (Wire.requestFrom(VEML6070_ADDR_H, 1) != 1) {
    return -1;
  }
  uv_raw   = Wire.read();
  uv_raw <<= 8;
  // read low byte
  if (Wire.requestFrom(VEML6070_ADDR_L, 1) != 1) {
    return -1;
  }
  uv_raw  |= Wire.read();
  // high and low done
  return uv_raw;

}


bool VEML6070_Init(byte it)
{
  byte error;
  
  Wire.begin();
  Wire.beginTransmission(VEML6070_ADDR_L);
  Wire.write((it << 2) | 0x02);
  error = Wire.endTransmission();
  delay(500);

  return (error == 0);
}

double VEML6070_UvRiskLevel(uint16_t uv_level)
{
  double risk = 0;
  if (uv_level < uv_risk_map[VEML6070_UV_MAX_INDEX-1]) {
    risk = (double)uv_level / uv_risk_map[0];
    // generate uv-risk string
    if ( (risk >= 0) && (risk <= 2.9) ) { snprintf_P(str_uvrisk_text, sizeof(str_uvrisk_text), D_UV_INDEX_1); }
    else if ( (risk >= 3.0)  && (risk <= 5.9) )  { snprintf_P(str_uvrisk_text, sizeof(str_uvrisk_text), D_UV_INDEX_2); }
    else if ( (risk >= 6.0)  && (risk <= 7.9) )  { snprintf_P(str_uvrisk_text, sizeof(str_uvrisk_text), D_UV_INDEX_3); }
    else if ( (risk >= 8.0)  && (risk <= 10.9) ) { snprintf_P(str_uvrisk_text, sizeof(str_uvrisk_text), D_UV_INDEX_4); }
    else if ( (risk >= 11.0) && (risk <= 12.9) ) { snprintf_P(str_uvrisk_text, sizeof(str_uvrisk_text), D_UV_INDEX_5); }
    else if ( (risk >= 13.0) && (risk <= 25.0) ) { snprintf_P(str_uvrisk_text, sizeof(str_uvrisk_text), D_UV_INDEX_6); }
    else { snprintf_P(str_uvrisk_text, sizeof(str_uvrisk_text), D_UV_INDEX_7); }
    return risk;
  } else {
    // out of range and much to high - it must be outerspace or sensor damaged
    snprintf_P(str_uvrisk_text, sizeof(str_uvrisk_text), D_UV_INDEX_7);
    String log = F("VEML6070 out of range: ");
    log += risk;
    addLog(LOG_LEVEL_DEBUG, log);
    return ( risk = 99 );
  }
}

double VEML6070_UvPower(double uvrisk)
{
  // based on calculations for effective irradiation from Karel Vanicek
  double power = 0;
  return ( power = VEML6070_POWER_COEFFCIENT * uvrisk );
}

void VEML6070_UvTableInit(void)
{
  // fill the uv-risk compare table once, based on the coefficient calculation
  for (uint8_t i = 0; i < VEML6070_UV_MAX_INDEX; i++) {
#ifdef USE_VEML6070_RSET
    if ( (USE_VEML6070_RSET >= 220000) && (USE_VEML6070_RSET <= 1000000) ) {
      uv_risk_map[i] = ( (USE_VEML6070_RSET / VEML6070_TABLE_COEFFCIENT) / VEML6070_UV_MAX_DEFAULT ) * (i+1);
    } else {
      uv_risk_map[i] = ( (VEML6070_RSET_DEFAULT / VEML6070_TABLE_COEFFCIENT) / VEML6070_UV_MAX_DEFAULT ) * (i+1);
    }
#else
    uv_risk_map[i] = ( (VEML6070_RSET_DEFAULT / VEML6070_TABLE_COEFFCIENT) / VEML6070_UV_MAX_DEFAULT ) * (i+1);

#endif
  }
}

#endif // USES_P084