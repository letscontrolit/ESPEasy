#ifdef USES_P258
//#######################################################################################################
//######################## Plugin 258 HMC5883L I2C XAxis/IR Sensor
//#########################################
//#######################################################################################################
//
// by: https://github.com/krikk
// this plugin is based on the adafruit library
// written based on version 1.0.2 from
// https://github.com/adafruit/Adafruit_HMC5883L_Library
// does need Adafruit Sensors Library
// added XAxis calculation improvement
// https://github.com/adafruit/Adafruit_HMC5883L_Library/issues/14
// added fix for issue
// https://github.com/adafruit/Adafruit_HMC5883L_Library/issues/17

#define PLUGIN_258
#define PLUGIN_ID_258 258
#define PLUGIN_NAME_258 "Magnetometer  - HMC5883L [TESTING]"
#define PLUGIN_VALUENAME1_258 "XAxis"
#define PLUGIN_VALUENAME2_258 "YAxis"
#define PLUGIN_VALUENAME3_258 "ZAxis"
#define PLUGIN_VALUENAME4_258 "Bearing"

#include "Adafruit_HMC5883L.h"
#include <Adafruit_Sensor.h>

struct P258_data_struct : public PluginTaskData_base {

  P258_data_struct() {
    hmc = Adafruit_HMC5883L(5883); // pass in a number for the sensor identifier
                                  // (for your use later)
  }


  void setRangeR(int range) {
    switch (range) {
    default:
    case 0: hmc.setRange(HMC5883L_RANGE_0_88GA); break;
    case 1: hmc.setRange(HMC5883L_RANGE_1_3GA); break;
    case 2: hmc.setRange(HMC5883L_RANGE_1_9GA); break;
    case 3: hmc.setRange(HMC5883L_RANGE_2_5GA); break;
    case 4: hmc.setRange(HMC5883L_RANGE_4GA); break;
    case 5: hmc.setRange(HMC5883L_RANGE_4_7GA); break;
    case 6: hmc.setRange(HMC5883L_RANGE_5_6GA); break;
    case 7: hmc.setRange(HMC5883L_RANGE_8_1GA); break;
    }
  }


  void setRate(int rate) {
    switch (rate) {
    default:
    case 0: hmc.setRate(HMC5883L_DATARATE_0_75HZ);  break;
    case 1: hmc.setRate(HMC5883L_DATARATE_1_5HZ);  break;
    case 2: hmc.setRate(HMC5883L_DATARATE_3HZ);  break;
    case 3: hmc.setRate(HMC5883L_DATARATE_7_50HZ);  break;
    case 4: hmc.setRate(HMC5883L_DATARATE_15HZ);  break;
    case 5: hmc.setRate(HMC5883L_DATARATE_30HZ);  break;
    case 6: hmc.setRate(HMC5883L_DATARATE_75HZ);  break;
      }
  }

  void setSamples(int samples) {
    switch (samples) {
    default:
    case 0: hmc.setSamples(HMC5883L_SAMPLES_1);  break;
    case 1: hmc.setSamples(HMC5883L_SAMPLES_2);  break;
    case 2: hmc.setSamples(HMC5883L_SAMPLES_4); break;
    case 3: hmc.setSamples(HMC5883L_SAMPLES_8);  break;
      }
  }

  Adafruit_HMC5883L hmc;
};

boolean Plugin_258(byte function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number = PLUGIN_ID_258;
      Device[deviceCount].Type = DEVICE_TYPE_I2C;
      Device[deviceCount].Ports = 0;
      Device[deviceCount].VType = SENSOR_TYPE_QUAD;
      Device[deviceCount].PullUpOption = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption = true;
      Device[deviceCount].ValueCount = 4;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption = true;
      Device[deviceCount].TimerOptional = false;
      Device[deviceCount].GlobalSyncOption = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_258);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_258));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_258));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_258));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_258));
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      int optionValues[1] = {HMC5883L_ADDR};
      addFormSelectorI2C(F("p258_i2c_addr"), 1, optionValues,
                         HMC5883L_ADDR);

      String optionsRange[8] = {F("0,88Ga"), F("1,3Ga"), F("1,9Ga"), F("2,5Ga"),
                               F("4Ga"), F("4,7Ga"), F("5,6Ga"), F("8,1Ga")};
      addFormSelector(F("Range"), F("p258_range"), 8, optionsRange,
                      NULL, PCONFIG(1));



      String optionsRate[7] = {F("0,75HZ"), F("1,5Hz"), F("4Hz"), F("7,5Hz"),
                               F("15Hz"), F("30Hz"), F("75Hz")};
      addFormSelector(F("Data Rate"), F("p258_rate"), 7, optionsRate, NULL,
                      PCONFIG(2));

      String optionsSamples[4] = {F("1"), F("2", F("4"), F("8")};
      addFormSelector(F("Samples"), F("p258_samples"), 4, optionsSamples, NULL,
                      PCONFIG(3));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      // PCONFIG(0) = getFormItemInt(F("p258_i2c_addr"));
      PCONFIG(1) = getFormItemInt(F("p258_range"));
      PCONFIG(2) = getFormItemInt(F("p258_rate"));
      PCONFIG(3) = getFormItemInt(F("p258_samples"));

      success = true;
      break;
    }

    case PLUGIN_INIT: {
      initPluginTaskData(event->TaskIndex, new P258_data_struct());
      P258_data_struct *P258_data =
          static_cast<P258_data_struct *>(getPluginTaskData(event->TaskIndex));
      if (nullptr == P258_data) {
        return success;
      }
      if (P258_data->hmc.begin()) {
        P258_data->setRangeR(PCONFIG(1));
        P258_data->setRate(PCONFIG(2));
        P258_data->setSamples(PCONFIG(2));
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("HMC5883L: Address: 0x");
          log += String(HMC5883L_ADDR, HEX);
          log += F(": Integration Time: ");
          log += String((P258_data->hmc.getTiming() + 1) * 100, DEC);
          log += F(" ms");

          /* Display the gain and integration time for reference sake */
          log += (F(" Gain: "));
          switch (P258_data->hmc.getGain()) {
            default:
            case HMC5883L_GAIN_LOW:  log += F("1x (Low)");     break; // 1x gain (bright light)
            case HMC5883L_GAIN_MED:  log += F("25x (Medium)"); break;
            case HMC5883L_GAIN_HIGH: log += F("428x (High)");  break;
            case HMC5883L_GAIN_MAX:  log += F("9876x (Max)");  break;
          }
          addLog(LOG_LEVEL_INFO, log);
        }
      } else {
        clearPluginTaskData(event->TaskIndex);
        addLog(LOG_LEVEL_ERROR,
               F("HMC5883L: No sensor found ... check your wiring?"));
      }

      success = true;
      break;
    }

    case PLUGIN_READ: {
      P258_data_struct *P258_data =
          static_cast<P258_data_struct *>(getPluginTaskData(event->TaskIndex));
      if (nullptr != P258_data) {
        // Simple data read example. Just read the infrared, fullspecrtrum diode
        // or 'visible' (difference between the two) channels.
        // This can take 100-600 milliseconds! Uncomment whichever of the
        // following you want to read
        float XAxis, full, visible, ir;
        XAxis = P258_data->hmc.getLuminosity(HMC5883L_VISIBLE);
        YAxis      = P258_data->hmc.getLuminosity(HMC5883L_INFRARED);
        ZAxis    = P258_data->hmc.getLuminosity(HMC5883L_FULLSPECTRUM);
        Bearing     = P258_data->hmc.calculateXAxisf(full, ir); // get XAxis

        UserVar[event->BaseVarIndex + 0] = XAxis;
        UserVar[event->BaseVarIndex + 1] = YAxis;
        UserVar[event->BaseVarIndex + 2] = ZAxis;
        UserVar[event->BaseVarIndex + 3] = Bearing;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("HMC5883L: XAxis: ");
          log += String(XAxis);
          log += F(" YAxis: ");
          log += String(full);
          log += F(" ZAxis: ");
          log += String(visible);
          log += F(" Bearing: ");
          log += String(ir);
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
      } else {
        addLog(LOG_LEVEL_ERROR, F("HMC5883L: Sensor not initialized!?"));
      }
      break;
    }
  }
  return success;
}
#endif // USES_P258
