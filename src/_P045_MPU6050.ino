#ifdef USES_P045
//#######################################################################################################
//#################################### Plugin 045: MPU6050 [Testing] ####################################
//#######################################################################################################

// Based on the works of Nolan Gilley @ https://home-assistant.io/blog/2016/08/03/laundry-automation-update/
// falling under the following license CC-BY-SA, https://creativecommons.org/licenses/by-sa/2.0/
// and the works of Jeff Rowberg @ https://www.i2cdevlib.com/devices/mpu6050, specifically his I2C Functions
// in this plugin are based on or are a copy from the following two libraries:
// I2Cdev: https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/I2Cdev
// MPU6050: https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/MPU6050
// Which contain the following license information:
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
// associated documentation files (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions: The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.


// This plugin enables the use of a MPU6050 sensor as e.g. used in the breakout-board GY-521.
// Using the webform you can set thresholds for the x-y-z axis and timeout values. If the thresholds are
// exceeded the sensor is on, if the thresholds are not met during the timeout period the sensor is off.

// Using this plugin you can get a notification from your home automation system when the monitored machine or
// device is no longer vibrating and thus this can be used as a signalling device for the end of a (dish)washer
// or dryer cycle.

// You can also use the plugin to read raw sensor values. You can use more then one instance of the plugin and
// you can set multiple movement alarms by giving each instance other threshold values if needed.

// Best practise: Create three custom sensors in your home controller (like domoticz) and let it plot the x, y and
// z range. Plot the sensor values while you use the washing machine and/or dryer. Also keep monitoring when they
// are not in use so you can determine the needed thresholds. When you have these you can select the movement
// detection function to setup the plugin for further use.

// The plugin can be used simultaneously with two MPU6050 devices by adding multiple instances.
// Originally released in the PlayGround as Plugin 118.

// Plugin var usage:
// Globals    - int16_t _P045_axis[3][5][2] Array to store sensorvalues of the axis
//              _P045_axis[0-2][x][x]  = x, y, z axis
//              _P045_axis[x][0-4][x]  = min values, max values, range (max-min), a-values, g-values.
//              _P045_axis[x][x][0-1]  = device address: 0=0x68, 1=0x69
//            - long _P045_time[2] = Timer to check values each 5 seconds for each used device address.

// Framework  - Settings.TaskDevicePluginConfig[x][0]     - Device address (0x68 | 0x69)
//              Settings.TaskDevicePluginConfig[x][1]     - Instance function
//              Settings.TaskDevicePluginConfig[x][2]     - ax threshold value
//              Settings.TaskDevicePluginConfig[x][3]     - ay threshold value
//              Settings.TaskDevicePluginConfig[x][4]     - az threshold value
//              Settings.TaskDevicePluginConfig[x][5]     - Minimal detection threshold value
//              Settings.TaskDevicePluginConfig[x][6]     - Detection threshold window value
//              Settings.TaskDevicePluginConfig[x][7]     - Last known status of switch
//              Settings.TaskDevicePluginConfigLong[x][0] - Minimal detection threshold counter
//              Settings.TaskDevicePluginConfigLong[x][1] - Detection threshold window counter


#define MPU6050_RA_GYRO_CONFIG              0x1B
#define MPU6050_RA_ACCEL_CONFIG             0x1C
#define MPU6050_RA_ACCEL_XOUT_H             0x3B
#define MPU6050_RA_PWR_MGMT_1               0x6B
#define MPU6050_ACONFIG_AFS_SEL_BIT         4
#define MPU6050_ACONFIG_AFS_SEL_LENGTH      2
#define MPU6050_GCONFIG_FS_SEL_BIT          4
#define MPU6050_GCONFIG_FS_SEL_LENGTH       2
#define MPU6050_CLOCK_PLL_XGYRO             0x01
#define MPU6050_GYRO_FS_250                 0x00
#define MPU6050_ACCEL_FS_2                  0x00
#define MPU6050_PWR1_SLEEP_BIT              6
#define MPU6050_PWR1_CLKSEL_BIT             2
#define MPU6050_PWR1_CLKSEL_LENGTH          3

#define PLUGIN_045
#define PLUGIN_ID_045                       45
#define PLUGIN_NAME_045                     "Gyro - MPU 6050 [TESTING]"
#define PLUGIN_VALUENAME1_045               ""

int16_t _P045_axis[3][5][2];                // [xyz], [min/max/range,a,g], [0x68/0x69]
unsigned long _P045_time[2];

boolean Plugin_045(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_045;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].ValueCount = 1;                   // Unfortunatly domoticz has no custom multivalue sensors.
        Device[deviceCount].SendDataOption = true;            //   and I use Domoticz ... so there.
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].FormulaOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_045);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_045));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        // Setup webform for address selection
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        /*
        String options[10];
        options[0] = F("0x68 - default settings (ADDR Low)");
        options[1] = F("0x69 - alternate settings (ADDR High)");
        */
        int optionValues[2];
        optionValues[0] = 0x68;
        optionValues[1] = 0x69;
        addFormSelectorI2C(F("plugin_045_address"), 2, optionValues, choice);
        addFormNote(F("ADDR Low=0x68, High=0x69"));

        choice = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        String options[10];
        options[0] = F("Movement detection");
        options[1] = F("Range acceleration X");
        options[2] = F("Range acceleration Y");
        options[3] = F("Range acceleration Z");
        options[4] = F("Acceleration X");
        options[5] = F("Acceleration Y");
        options[6] = F("Acceleration Z");
        options[7] = F("G-force X");
        options[8] = F("G-force Y");
        options[9] = F("G-force Z");
        addFormSelector(F("Function"), F("plugin_045_function"), 10, options, NULL, choice);

        if (choice == 0) {
          // If this is instance function 0, setup webform for additional vars
          // Show some user information about the webform and what the vars mean.
          addHtml(F("<TR><TD><TD>The thresholdvalues (0-65535) can be used to set a threshold for one or more<br>"));
          addHtml(F("axis. The axis will trigger when the range for that axis exceeds the threshold<br>"));
          addHtml(F("value. A value of 0 disables movement detection for that axis."));

        	addFormNumericBox(F("Detection threshold X"), F("plugin_045_threshold_x"), Settings.TaskDevicePluginConfig[event->TaskIndex][2], 0, 65535);
        	addFormNumericBox(F("Detection threshold Y"), F("plugin_045_threshold_y"), Settings.TaskDevicePluginConfig[event->TaskIndex][3], 0, 65535);
        	addFormNumericBox(F("Detection threshold Z"), F("plugin_045_threshold_z"), Settings.TaskDevicePluginConfig[event->TaskIndex][4], 0, 65535);

          addHtml(F("<TR><TD><TD>Each 30 seconds a counter for the detection window is increased plus all axis<br>"));
          addHtml(F("are checked and if they *all* exceeded the threshold values, a counter is increased.<br>"));
          addHtml(F("Each period, defined by the [detection window], the counter is checked against<br>"));
          addHtml(F("the [min. detection count] and if found equal or larger, movement is detected.<br>"));
          addHtml(F("If in the next window the [min. detection count] value is not met, movement has stopped."));
          addHtml(F("The [detection window] cannot be smaller than the [min. detection count]."));

        	addFormNumericBox(F("Min. detection count"), F("plugin_045_threshold_counter"), Settings.TaskDevicePluginConfig[event->TaskIndex][5], 0, 999999);
        	addFormNumericBox(F("Detection window"), F("plugin_045_threshold_window"), Settings.TaskDevicePluginConfig[event->TaskIndex][6], 0, 999999);

        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        // Save the vars
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_045_address"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_045_function"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_045_threshold_x"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = getFormItemInt(F("plugin_045_threshold_y"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = getFormItemInt(F("plugin_045_threshold_z"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][5] = getFormItemInt(F("plugin_045_threshold_counter"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][6] = getFormItemInt(F("plugin_045_threshold_window"));
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][6] < Settings.TaskDevicePluginConfig[event->TaskIndex][5]) {
          Settings.TaskDevicePluginConfig[event->TaskIndex][6] = Settings.TaskDevicePluginConfig[event->TaskIndex][5];
        }
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        // Initialize the MPU6050. This *can* be done multiple times per instance and device address.
        // We could make sure that this is only done once per device address, but why bother?
        uint8_t devAddr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        if ((devAddr < 0x68) || (devAddr > 0x69)) { //  Just in case the address is not initialized, set it anyway.
          devAddr = 0x68;
          Settings.TaskDevicePluginConfig[event->TaskIndex][0] = devAddr;
        }
        // Initialize the MPU6050, for details look at the MPU6050 library: MPU6050::Initialize
        _P045_writeBits(devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, MPU6050_CLOCK_PLL_XGYRO);
        _P045_writeBits(devAddr, MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, MPU6050_GYRO_FS_250);
        _P045_writeBits(devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, MPU6050_ACCEL_FS_2);
        _P045_writeBits(devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, 1, 0);

        // Read the MPU6050 once to clear out zeros (1st time reading MPU6050 returns all 0s)
        int16_t ax, ay, az, gx, gy, gz;
        _P045_getMotion6(devAddr, &ax, &ay, &az, &gx, &gy, &gz);

        // Reset vars
        Settings.TaskDevicePluginConfig[event->TaskIndex][7] = 0;       // Last known value of "switch" is off
        UserVar[event->BaseVarIndex] = 0;                               // Switch is off
        Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] = 0;   // Minimal detection counter is zero
        Settings.TaskDevicePluginConfigLong[event->TaskIndex][1] = 0;   // Detection window counter is zero
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        uint8_t devAddr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        byte dev = devAddr & 1;

        // Read the sensorvalues, we run this bit every 1/10th of a second
        _P045_getMotion6(devAddr, &_P045_axis[0][3][dev], &_P045_axis[1][3][dev], &_P045_axis[2][3][dev], &_P045_axis[0][4][dev], &_P045_axis[1][4][dev], &_P045_axis[2][4][dev]);
        // Set the minimum and maximum value for each axis a-value, overwrite previous values if smaller/larger
        _P045_trackMinMax(_P045_axis[0][3][dev], &_P045_axis[0][0][dev], &_P045_axis[0][1][dev]);
        _P045_trackMinMax(_P045_axis[1][3][dev], &_P045_axis[1][0][dev], &_P045_axis[1][1][dev]);
        _P045_trackMinMax(_P045_axis[2][3][dev], &_P045_axis[2][0][dev], &_P045_axis[2][1][dev]);
        //                              ^ current value @ 3     ^ min val @ 0           ^ max val @ 1

/*      // Uncomment this block if you want to debug your MPU6050, but be prepared for a log overload
        String log = F("MPU6050 : axis values: ");
        log += _P045_axis[0][3][dev];
        log += F(", ");
        log += _P045_axis[1][3][dev];
        log += F(", ");
        log += _P045_axis[2][3][dev];
        log += F(", g values: ");
        log += _P045_axis[0][4][dev];
        log += F(", ");
        log += _P045_axis[1][4][dev];
        log += F(", ");
        log += _P045_axis[2][4][dev];
        addLog(LOG_LEVEL_INFO,log);
*/
        // Run this bit every 5 seconds per deviceaddress (not per instance)
        if (timeOutReached(_P045_time[dev] + 5000))
        {
          _P045_time[dev] = millis();

          // Determine the maximum measured range of each axis
          for (uint8_t i=0; i<3; i++) {
            _P045_axis[i][2][dev] = abs(_P045_axis[i][1][dev] - _P045_axis[i][0][dev]);
            _P045_axis[i][0][dev] = _P045_axis[i][3][dev];
            _P045_axis[i][1][dev] = _P045_axis[i][3][dev];
          }
        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        int devAddr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        byte dev = devAddr & 1;
        int _P045_Function = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        switch (_P045_Function)
        {
          // Function 0 is for movement detection
          case 0:
          {
            // Check if all (enabled, so !=0) thresholds are exceeded, if one fails then thresexceed (thesholds exceeded) is reset to false;
            boolean thresexceed = true;
            byte count = 0;                 // Counter to check if not all thresholdvalues are set to 0 or disabled
            for (byte i=0; i<3; i++)
            {
              // for each axis:
              if (Settings.TaskDevicePluginConfig[event->TaskIndex][i + 2] != 0) {  // not disabled, check threshold
                if (_P045_axis[i][2][dev] < Settings.TaskDevicePluginConfig[event->TaskIndex][i + 2]) { thresexceed = false; }
              } else { count++; } // If disabled count + 1
            }
            if (count == 3) { thresexceed = false; }  // If we counted to three, all three axis are disabled.

            // If all enabled thresholds are exceeded the increase the counter
            if (thresexceed) { Settings.TaskDevicePluginConfigLong[event->TaskIndex][0]++; }
            // And increase the window counter
            Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]++;

            if (Settings.TaskDevicePluginConfigLong[event->TaskIndex][1] >= Settings.TaskDevicePluginConfig[event->TaskIndex][6]) {
              // Detection window has passed.
              Settings.TaskDevicePluginConfigLong[event->TaskIndex][1] = 0; // reset window counter

              // Did we count more times exceeded then the minimum detection value?
              if (Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] >= Settings.TaskDevicePluginConfig[event->TaskIndex][5]) {
                UserVar[event->BaseVarIndex] = 1; // x times threshold exceeded within window.
              } else {
                UserVar[event->BaseVarIndex] = 0; // reset because x times threshold within window not met.
              }

              // Check if UserVar changed so we do not overload homecontroller with the same readings
              if (Settings.TaskDevicePluginConfig[event->TaskIndex][7] != UserVar[event->BaseVarIndex]) {
                Settings.TaskDevicePluginConfig[event->TaskIndex][7] = UserVar[event->BaseVarIndex];
                success = true;
              } else {
                success = false;
              }
              Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] = 0; // reset threshold exceeded counter
            }
            // The default sensorType of the device is a single sensor value. But for detection movement we want it to be
            // a switch so we change the sensortype here. Looks like a legal thing to do because _P001_Switch does it as well.
            event->sensorType = SENSOR_TYPE_SWITCH;
            break;
          }
          // All other functions are reading values. So extract xyz value and wanted type from function number:
          default:  // [1-3]: range-values, [4-6]: a-values, [7-9]: g-values
          {
            uint8_t reqaxis = (_P045_Function - 1) % 3;       // xyz         -> eg: function 5(ay) (5-1) % 3 = 1           (y)
            uint8_t reqvar = ((_P045_Function - 1) / 3) + 2;  // range, a, g -> eg: function 9(gz) ((9-1) / 3 = 2) + 2 = 4 (g)
            UserVar[event->BaseVarIndex] = float(_P045_axis[reqaxis][reqvar][dev]);
            success = true;
            break;
          }
        }
        break;
      }
  }
  return success;
}

void _P045_trackMinMax(int16_t current, int16_t *min, int16_t *max)
// From nodemcu-laundry.ino by Nolan Gilley
{
  if (current > *max)
  {
    *max = current;
  }
  else if (current < *min)
  {
    *min = current;
  }
}


/** Get raw 6-axis motion sensor readings (accel/gyro).
 * Retrieves all currently available motion sensor values.
 * @param devAddr I2C slave device address
 * @param ax 16-bit signed integer container for accelerometer X-axis value
 * @param ay 16-bit signed integer container for accelerometer Y-axis value
 * @param az 16-bit signed integer container for accelerometer Z-axis value
 * @param gx 16-bit signed integer container for gyroscope X-axis value
 * @param gy 16-bit signed integer container for gyroscope Y-axis value
 * @param gz 16-bit signed integer container for gyroscope Z-axis value
 */
void _P045_getMotion6(uint8_t devAddr, int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz) {
    // From I2Cdev::readBytes and MPU6050::getMotion6, both by Jeff Rowberg
    uint8_t buffer[14];
    uint8_t count = 0;
    I2C_write8(devAddr, MPU6050_RA_ACCEL_XOUT_H);
    Wire.requestFrom(devAddr, (uint8_t)14);
    for (; Wire.available(); count++) {
        buffer[count] = Wire.read();
    }
    *ax = (((int16_t)buffer[0]) << 8) | buffer[1];
    *ay = (((int16_t)buffer[2]) << 8) | buffer[3];
    *az = (((int16_t)buffer[4]) << 8) | buffer[5];
    *gx = (((int16_t)buffer[8]) << 8) | buffer[9];
    *gy = (((int16_t)buffer[10]) << 8) | buffer[11];
    *gz = (((int16_t)buffer[12]) << 8) | buffer[13];
}

/** Write multiple bits in an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitStart First bit position to write (0-7)
 * @param length Number of bits to write (not more than 8)
 * @param data Right-aligned value to write
 */
void _P045_writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data) {
    // From I2Cdev::writeBits by Jeff Rowberg
    //      010 value to write
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    // 00011100 mask byte
    // 10101111 original value (sample)
    // 10100011 original & ~mask
    // 10101011 masked | value
    bool is_ok = true;
    uint8_t b = I2C_read8_reg(devAddr, regAddr, &is_ok);
    if (is_ok) {
      uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
      data <<= (bitStart - length + 1); // shift data into correct position
      data &= mask; // zero all non-important bits in data
      b &= ~(mask); // zero all important bits in existing byte
      b |= data; // combine data with existing byte
      I2C_write8_reg(devAddr, regAddr, b);
    }
}
#endif // USES_P045
