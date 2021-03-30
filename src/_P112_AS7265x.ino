#include "_Plugin_Helper.h"
#ifdef USES_P112

// #######################################################################################################
// #################### Plugin 112 I2C AS7265X Triad Spectroscopy Sensor and White, IR and UV LED ########
// #######################################################################################################
//
// Triad Spectroscopy Sensor and White, IR and UV LED
// like this one: https://www.sparkfun.com/products/15050
// based on this library: https://github.com/sparkfun/SparkFun_AS7265x_Arduino_Library
// this code is based on 29 Mar 2019-03-29 version of the above library
//
// 2021-03-29 heinemannj: Initial commit
//

#include "src/PluginStructs/P112_data_struct.h"

#define PLUGIN_112
#define PLUGIN_ID_112         112
#define PLUGIN_NAME_112       "Color - AS7265X [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_112 "460"
#define PLUGIN_VALUENAME2_112 "535"
#define PLUGIN_VALUENAME3_112 "610"
#define PLUGIN_VALUENAME4_112 "860"

#define AS7265X_ADDR 0x49

boolean Plugin_112(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  String MeasurementStatus;
  MeasurementStatus = String("Not running");

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_112;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].DecimalsOnly       = true;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].OutputDataType     = Output_Data_type_t::All;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_112);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_112));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_112));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_112));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_112));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      int optionValues[1] = { AS7265X_ADDR };
      addFormSelectorI2C(F("i2c_addr"), 1, optionValues, AS7265X_ADDR);
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      // Set a default config here, which will be called when a plugin is assigned to a task.
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      byte   choiceMode = PCONFIG(0);
      {
        // sensor.setGain(AS7265X_GAIN_1X);  //Default
        // sensor.setGain(AS7265X_GAIN_37X); //This is 3.7x
        // sensor.setGain(AS7265X_GAIN_16X);
        // sensor.setGain(AS7265X_GAIN_64X);
        String optionsMode[4];
        optionsMode[0] = F("1x");
        optionsMode[1] = F("3.7x");
        optionsMode[2] = F("16x");
        optionsMode[3] = F("64x");
        int optionValuesMode[4];
        optionValuesMode[0] = AS7265X_GAIN_1X;
        optionValuesMode[1] = AS7265X_GAIN_37X;
        optionValuesMode[2] = AS7265X_GAIN_16X;
        optionValuesMode[3] = AS7265X_GAIN_64X;
        addFormSelector(F("Gain"), F("p112_Gain"), 4, optionsMode, optionValuesMode, choiceMode);
      }
      byte   choiceMode2 = PCONFIG(1);
      {
        // Integration cycles from 0 (2.78ms) to 255 (711ms)
        // sensor.setIntegrationCycles(49); //Default: 50*2.8ms = 140ms per reading
        // sensor.setIntegrationCycles(1);  //2*2.8ms = 5.6ms per reading
        String optionsMode2[6];
        optionsMode2[0] = F("2.8 ms");
        optionsMode2[1] = F("28 ms");
        optionsMode2[2] = F("56 ms");
        optionsMode2[3] = F("140 ms");
        optionsMode2[4] = F("280 ms");
        optionsMode2[5] = F("711 ms");
        int optionValuesMode2[6];
        optionValuesMode2[0] = 0;
        optionValuesMode2[1] = 9;
        optionValuesMode2[2] = 19;
        optionValuesMode2[3] = 49;
        optionValuesMode2[4] = 99;
        optionValuesMode2[5] = 254;
        addFormSelector(F("Integration Time"), F("p112_IntegrationTime"), 6, optionsMode2, optionValuesMode2, choiceMode2);
      }
      // The status indicator (Blue LED)
      // sensor.enableIndicator(); //Default
      // sensor.disableIndicator();
      addFormCheckBox(F("Blue Status LED"), F("p112_BlueStatusLED"), PCONFIG(2));
      // sensor.takeMeasurementsWithBulb();
      // sensor.takeMeasurements();
      addFormCheckBox(F("Integrated UV, White, and IR LEDs"), F("p112_IntegratedLEDs"), PCONFIG(3));
      // sensor.getCalibratedA();
      // sensor.getA();
      addFormCheckBox(F("Calibrated Measurements"), F("p112_CalibratedMeasurements"), PCONFIG(4));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("p112_Gain"));
      PCONFIG(1) = getFormItemInt(F("p112_IntegrationTime"));
      PCONFIG(2) = isFormItemChecked(F("p112_BlueStatusLED"));
      PCONFIG(3) = isFormItemChecked(F("p112_IntegratedLEDs"));
      PCONFIG(4) = isFormItemChecked(F("p112_CalibratedMeasurements"));
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P112_data_struct());
      P112_data_struct *P112_data =
        static_cast<P112_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P112_data) {
        P112_data->initialized = false; // Force re-init just in case the address changed.
        P112_data->begin();
        success = P112_data->initialized;
        P112_data->sensor.setGain(PCONFIG(0));
        P112_data->sensor.setIntegrationCycles(PCONFIG(1));
        if (PCONFIG(2)) // Blue Status LED?
        {
          P112_data->sensor.enableIndicator();
        } else {
          P112_data->sensor.disableIndicator();
        }
      }
      break;
    }
    case PLUGIN_ONCE_A_SECOND:
    {
      P112_data_struct *P112_data =
        static_cast<P112_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P112_data) {
        if (P112_data->sensor.dataAvailable()) {
          // Measurement was succesfull, schedule a read.
          MeasurementStatus = String("Ready");
          eventQueue.add("PLUGIN_ONCE_A_SECOND: MeasurementStatus = " + MeasurementStatus);
          Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
        } else {
          eventQueue.add("PLUGIN_ONCE_A_SECOND: No sensor data available");
        }
      }
      break;
    }
    case PLUGIN_READ:
    {
      P112_data_struct *P112_data =
        static_cast<P112_data_struct *>(getPluginTaskData(event->TaskIndex));

      eventQueue.add("PLUGIN_READ: MeasurementStatus = " + MeasurementStatus);

      if (MeasurementStatus == "Ready") {

        MeasurementStatus = "Done";
        eventQueue.add("PLUGIN_READ: MeasurementStatus = " + MeasurementStatus);
        String RuleEvent;
        RuleEvent  = getTaskDeviceName(event->TaskIndex);
        RuleEvent += '#';

        if (PCONFIG(3)) // Calibrated Measurements?
        {
          eventQueue.add(RuleEvent + "410=" + P112_data->sensor.getCalibratedA());
          eventQueue.add(RuleEvent + "435=" + P112_data->sensor.getCalibratedB());
          eventQueue.add(RuleEvent + "460=" + P112_data->sensor.getCalibratedC());

          eventQueue.add(RuleEvent + "485=" + P112_data->sensor.getCalibratedD());
          eventQueue.add(RuleEvent + "510=" + P112_data->sensor.getCalibratedE());
          eventQueue.add(RuleEvent + "535=" + P112_data->sensor.getCalibratedF());

          eventQueue.add(RuleEvent + "560=" + P112_data->sensor.getCalibratedG());
          eventQueue.add(RuleEvent + "585=" + P112_data->sensor.getCalibratedH());
          eventQueue.add(RuleEvent + "610=" + P112_data->sensor.getCalibratedR());

          eventQueue.add(RuleEvent + "645=" + P112_data->sensor.getCalibratedI());
          eventQueue.add(RuleEvent + "680=" + P112_data->sensor.getCalibratedS());
          eventQueue.add(RuleEvent + "705=" + P112_data->sensor.getCalibratedJ());

          eventQueue.add(RuleEvent + "730=" + P112_data->sensor.getCalibratedT());
          eventQueue.add(RuleEvent + "760=" + P112_data->sensor.getCalibratedU());
          eventQueue.add(RuleEvent + "810=" + P112_data->sensor.getCalibratedV());

          eventQueue.add(RuleEvent + "860=" + P112_data->sensor.getCalibratedW());
          eventQueue.add(RuleEvent + "900=" + P112_data->sensor.getCalibratedK());
          eventQueue.add(RuleEvent + "940=" + P112_data->sensor.getCalibratedL());

          UserVar[event->BaseVarIndex + 0] = P112_data->sensor.getCalibratedC();
          UserVar[event->BaseVarIndex + 1] = P112_data->sensor.getCalibratedF();
          UserVar[event->BaseVarIndex + 2] = P112_data->sensor.getCalibratedR();
          UserVar[event->BaseVarIndex + 3] = P112_data->sensor.getCalibratedW();
        } else {
          eventQueue.add(RuleEvent + "410=" + P112_data->sensor.getA());
          eventQueue.add(RuleEvent + "435=" + P112_data->sensor.getB());
          eventQueue.add(RuleEvent + "460=" + P112_data->sensor.getC());

          eventQueue.add(RuleEvent + "485=" + P112_data->sensor.getD());
          eventQueue.add(RuleEvent + "510=" + P112_data->sensor.getE());
          eventQueue.add(RuleEvent + "535=" + P112_data->sensor.getF());

          eventQueue.add(RuleEvent + "560=" + P112_data->sensor.getG());
          eventQueue.add(RuleEvent + "585=" + P112_data->sensor.getH());
          eventQueue.add(RuleEvent + "610=" + P112_data->sensor.getR());

          eventQueue.add(RuleEvent + "645=" + P112_data->sensor.getI());
          eventQueue.add(RuleEvent + "680=" + P112_data->sensor.getS());
          eventQueue.add(RuleEvent + "705=" + P112_data->sensor.getJ());

          eventQueue.add(RuleEvent + "730=" + P112_data->sensor.getT());
          eventQueue.add(RuleEvent + "760=" + P112_data->sensor.getU());
          eventQueue.add(RuleEvent + "810=" + P112_data->sensor.getV());

          eventQueue.add(RuleEvent + "860=" + P112_data->sensor.getW());
          eventQueue.add(RuleEvent + "900=" + P112_data->sensor.getK());
          eventQueue.add(RuleEvent + "940=" + P112_data->sensor.getL());

          UserVar[event->BaseVarIndex + 0] = P112_data->sensor.getC();
          UserVar[event->BaseVarIndex + 1] = P112_data->sensor.getF();
          UserVar[event->BaseVarIndex + 2] = P112_data->sensor.getR();
          UserVar[event->BaseVarIndex + 3] = P112_data->sensor.getW();
        }
      } else if (MeasurementStatus != "Running") {
        MeasurementStatus = String("Running");
        eventQueue.add("PLUGIN_READ: MeasurementStatus = " + MeasurementStatus);
        if (P112_data->begin()) {
          if (PCONFIG(3)) // Integrated LEDs?
          {
//            P112_data->sensor.takeMeasurementsWithBulb();
            P112_data->sensor.enableBulb(AS7265x_LED_WHITE);
            P112_data->sensor.enableBulb(AS7265x_LED_IR);
            P112_data->sensor.enableBulb(AS7265x_LED_UV);
            P112_data->sensor.setMeasurementMode(AS7265X_MEASUREMENT_MODE_6CHAN_ONE_SHOT);
            P112_data->sensor.disableBulb(AS7265x_LED_WHITE);
            P112_data->sensor.disableBulb(AS7265x_LED_IR);
            P112_data->sensor.disableBulb(AS7265x_LED_UV);
          } else {
//            P112_data->sensor.takeMeasurements();
            P112_data->sensor.setMeasurementMode(AS7265X_MEASUREMENT_MODE_6CHAN_ONE_SHOT);
          }
        }
      }
      success = true;
      break;
    }
  }
  return success;
}


//
// There are four measurement modes - the datasheet describes it best
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// sensor.setMeasurementMode(AS7265X_MEASUREMENT_MODE_4CHAN); //Channels STUV on x51
// sensor.setMeasurementMode(AS7265X_MEASUREMENT_MODE_4CHAN_2); //Channels RTUW on x51
// sensor.setMeasurementMode(AS7265X_MEASUREMENT_MODE_6CHAN_CONTINUOUS); //All 6 channels on all devices
// sensor.setMeasurementMode(AS7265X_MEASUREMENT_MODE_6CHAN_ONE_SHOT); //Default: All 6 channels, all devices, just once
//
// Drive current can be set for each LED
// 4 levels: 12.5, 25, 50, and 100mA
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// White LED has max forward current of 120mA
// sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_WHITE); //Default
// sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_25MA, AS7265x_LED_WHITE); //Allowed
// sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_50MA, AS7265x_LED_WHITE); //Allowed 
// sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_100MA, AS7265x_LED_WHITE); //Allowed
//
// UV LED has max forward current of 30mA so do not set the drive current higher
// sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_UV); //Default
// sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_25MA, AS7265x_LED_UV-bad); //Not allowed
// sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_50MA, AS7265x_LED_UV-bad); //Not allowed
// sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_100MA, AS7265x_LED_UV-bad); //Not allowed
//
// IR LED has max forward current of 65mA 
// sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_IR); //Default
// sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_25MA, AS7265x_LED_IR); //Allowed
// sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_50MA, AS7265x_LED_IR); //Allowed
// sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_100MA, AS7265x_LED_IR-bad); //Not allowed
//
// The status indicator (Blue LED) can be enabled/disabled and have its current set
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// sensor.enableIndicator(); //Default
// sensor.disableIndicator();
//
// sensor.setIndicatorCurrent(AS7265X_INDICATOR_CURRENT_LIMIT_1MA);
// sensor.setIndicatorCurrent(AS7265X_INDICATOR_CURRENT_LIMIT_2MA);
// sensor.setIndicatorCurrent(AS7265X_INDICATOR_CURRENT_LIMIT_4MA);
// sensor.setIndicatorCurrent(AS7265X_INDICATOR_CURRENT_LIMIT_8MA); //Default
//
// The interrupt pin is active low and can be enabled or disabled
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// sensor.enableInterrupt(); //Default
// sensor.disableInterrupt();
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// byte deviceType = sensor.getDeviceType();
// Serial.print("AMS Device Type: 0x");
// Serial.println(deviceType, HEX);
//
// byte hardwareVersion = sensor.getHardwareVersion();
// Serial.print("AMS Hardware Version: 0x");
// Serial.println(hardwareVersion, HEX);
//
// byte majorFirmwareVersion = sensor.getMajorFirmwareVersion();
// Serial.print("Major Firmware Version: 0x");
// Serial.println(majorFirmwareVersion, HEX);
//
// byte patchFirmwareVersion = sensor.getPatchFirmwareVersion();
// Serial.print("Patch Firmware Version: 0x");
// Serial.println(patchFirmwareVersion, HEX);
//
// byte buildFirmwareVersion = sensor.getBuildFirmwareVersion();
// Serial.print("Build Firmware Version: 0x");
// Serial.println(buildFirmwareVersion, HEX);
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// sensor.getTemperature(); //Returns the temperature of master IC
// Serial.print("Main IC temp: ");
// Serial.println(oneSensorTemp);
//
// float threeSensorTemp = sensor.getTemperatureAverage(); //Returns the average temperature of all three ICs
// Serial.print("Average IC temp: ");
// Serial.println(threeSensorTemp, 2);
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Rather than toggle the LEDs with each measurement, turn on LEDs all the time
// sensor.enableBulb(AS7265x_LED_WHITE);
// sensor.enableBulb(AS7265x_LED_IR);
// sensor.enableBulb(AS7265x_LED_UV);
//

#endif // USES_P112