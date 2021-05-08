/*
 * Plugin 115 MAX1704x I2C, Sparkfun Fuel Gauge Sensor
 * 
 * development version
 * by: jbaream
 * this plugin is based in example1 by Paul Clark
 * from SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library
 * provides Battery Voltage, Battery State of Charge (SOC) and Alert Flag when SOC is below a threehold
 * defined in device configuration webform
 * 
 */
#include "_Plugin_Helper.h"
#ifdef USES_P115

/******************************************************************************
Example1_Simple
By: Paul Clark
Date: October 23rd 2020

Based extensively on:
MAX17043_Simple_Serial.cpp
SparkFun MAX17043 Example Code
Jim Lindblom @ SparkFun Electronics
Original Creation Date: June 22, 2015

This file demonstrates the simple API of the SparkFun MAX17043 Arduino library.

This example will print the gauge's voltage and state-of-charge (SOC) readings
to Serial (115200 baud)

This code is released under the MIT license.

Distributed as-is; no warranty is given.
******************************************************************************/

#include <Wire.h> // Needed for I2C

#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h> // Click here to get the library: http://librarymanager/All#SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library

//uncomment one of the following as needed
//#ifdef PLUGIN_BUILD_DEVELOPMENT
//#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_115                          
#define PLUGIN_ID_115     115               //plugin id
#define PLUGIN_NAME_115   "Energy - Fuel Gauge MAX1704x [TESTING]"     
#define PLUGIN_VALUENAME1_115 "Voltage"     // Battery voltage
#define PLUGIN_VALUENAME2_115 "SOC"     // Battery state of charge in percentage
#define PLUGIN_VALUENAME3_115 "Alert"   // (0 or 1) Alert when the battery SoC gets too low 
#define PLUGIN_xxx_DEBUG  false  //set to true for extra log info in the debug

SFE_MAX1704X lipo; // Defaults to the MAX17043

//SFE_MAX1704X lipo(MAX1704X_MAX17043); // Create a MAX17043
//SFE_MAX1704X lipo(MAX1704X_MAX17044); // Create a MAX17044
//SFE_MAX1704X lipo(MAX1704X_MAX17048); // Create a MAX17048
//SFE_MAX1704X lipo(MAX1704X_MAX17049); // Create a MAX17049

double voltage = 0; // Variable to keep track of LiPo voltage
double soc = 0; // Variable to keep track of LiPo state-of-charge (SOC)
bool alert; // Variable to keep track of whether alert has been triggered

/*
//void setup()
{
	Serial.begin(115200); // Start serial, to output debug data
  while (!Serial)
    ; //Wait for user to open terminal
  Serial.println(F("MAX17043 Example"));

  Wire.begin();

  lipo.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

  // Set up the MAX17043 LiPo fuel gauge:
  if (lipo.begin() == false) // Connect to the MAX17043 using the default wire port
  {
    Serial.println(F("MAX17043 not detected. Please check wiring. Freezing."));
    while (1)
      ;
  }

	// Quick start restarts the MAX17043 in hopes of getting a more accurate
	// guess for the SOC.
	lipo.quickStart();

	// We can set an interrupt to alert when the battery SoC gets too low.
	// We can alert at anywhere between 1% - 32%:
	lipo.setThreshold(20); // Set alert threshold to 20%.
}
*/

//A plugin has to implement the following function
boolean Plugin_115(byte function, struct EventStruct *event, String& string)
{
  //function: reason the plugin was called
  //event: ??add description here??
  // string: ??add description here??
  
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
        //This case defines the device characteristics, edit appropriately

        Device[++deviceCount].Number = PLUGIN_ID_115;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;  //how the device is connected
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_TRIPLE; //type of value the plugin will return
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;             //number of output variables. The value shuld match the number of keys PLUGIN_VALUENAME1_xxx
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
       // Device[deviceCount].TimerOptional = false;
        Device[deviceCount].GlobalSyncOption = true;
        Device[deviceCount].DecimalsOnly = true;
        break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      //return the device name
      string = F(PLUGIN_NAME_115);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      //called when the user opens the module configuration page
      //it allows to add a new row for each output variable of the plugin
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_115));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_115));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_115));
      break;
    }
       
    case PLUGIN_INIT:
    {
      //this case defines code to be executed when the plugin is initialised

      //after the plugin has been initialised successfuly, set success and break
     
      //lipo.quickStart();
      //lipo.setThreshold(10); // Set alert threhold when battery soc is 10%
      int threhold = PCONFIG(0);
      lipo.setThreshold(threhold);
      break;      
      
    }
    
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      break;
    }    
    
    case PLUGIN_WEBFORM_LOAD:
    {
      //this case defines what should be displayed on the web form, when this plugin is selected
      //The user's selection will be stored in 
      //Settings.TaskDevicePluginConfig[event->TaskIndex][x] (custom configuration)
      
      //Use any of the following (defined at WebServer.ino):
      
      //addFormNote(string, F("not editable text added here"));

      //String dropdown[5] = { F("option1"), F("option2"), F("option3"), F("option4")};
      //addFormSelector(string, F("drop-down menu"), F("plugin_xxx_displtype"), 4, dropdown, NULL, Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

      //number selection (min-value - max-value)
      // addFormNumericBox(string, F("description"), F("plugin_xxx_description"), Settings.TaskDevicePluginConfig[event->TaskIndex][1], min-value, max-value);

      //HTML Code can also be added by defining a string variable and adding HTML code as follows:
      //string += F("<TR><TD>Line ");
        
      addFormNote(F("Alert threhold must be an integer between 1 and 30"));
      addFormNumericBox(F("Alert threhold"), F("p115_max1704x_threhold"), PCONFIG(0),1,30);
      addUnit(F("%")); 
      
      //after the form has been loaded, set success and break
      success = true;
      break;
    }
    
    case PLUGIN_WEBFORM_SAVE:
    {
      //this case defines the code to be executed when the form is submitted
      //the plugin settings should be saved to Settings.TaskDevicePluginConfig[event->TaskIndex][x]
      //ping configuration should be read from Settings.TaskDevicePin1[event->TaskIndex] and stored

      //after the form has been saved successfuly, set success and break
      PCONFIG(0) = getFormItemInt(F("p115_max1704x_threhold"));
      success = true;
      break;
          
    }

    case PLUGIN_READ:
    {
      //code to be executed to read data
      //It is executed according to the delay configured on the device configuration page, only once

      //after the plugin has read data successfuly, set success and break
      success = true;
      break;
      
    }

    case PLUGIN_WRITE:
    {
      //this case defines code to be executed when the plugin executes an action (command). 
      //Commands can be accessed via rules or via http.
      //As an example, http://192.168.1.12//control?cmd=dothis
      //implies that there exists the comamnd "dothis"

     /*
        if (plugin_not_initialised)
        break;

      //parse string to extract the command
      String tmpString  = string;
      int argIndex = tmpString.indexOf(',');
      if (argIndex)
        tmpString = tmpString.substring(0, argIndex);
     
      String tmpStr = string;
      int comma1 = tmpStr.indexOf(',');
      if (tmpString.equalsIgnoreCase(F("dothis"))) {
        //do something
        success = true;     //set to true only if plugin has executed a command successfully
      }
      */

       break;
    }

	case PLUGIN_EXIT:
	{
	  //perform cleanup tasks here. For example, free memory

	  break;

	}

    case PLUGIN_ONCE_A_SECOND:
    {
      //code to be executed once a second. Tasks which do not require fast response can be added here
 
        // lipo.getVoltage() returns a voltage value (e.g. 3.93)
        voltage = lipo.getVoltage();
        UserVar[event->BaseVarIndex + 0] = voltage;
        // lipo.getSOC() returns the estimated state of charge (e.g. 79%)
        soc = lipo.getSOC();
        UserVar[event->BaseVarIndex + 1] = soc;
        // lipo.getAlert() returns a 0 or 1 (0=alert not triggered)
        alert = lipo.getAlert();
        UserVar[event->BaseVarIndex + 2] = alert;

      String log = F("MAX1704x : Voltage: ");
      log += voltage;
      addLog(LOG_LEVEL_INFO,log);
      log = F("MAX1704x : SoC: ");
      log += soc;
      addLog(LOG_LEVEL_INFO,log);
      log = F("MAX1704x : Alert: ");
      log += alert;
      addLog(LOG_LEVEL_INFO,log);
        
      //  success = true;
      break;
      
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      //code to be executed 10 times per second. Tasks which require fast response can be added here
      //be careful on what is added here. Heavy processing will result in slowing the module down!

      success = true;
      
    }
  }   // switch
  return success;
  
}     //function

#endif    // USES_P115
