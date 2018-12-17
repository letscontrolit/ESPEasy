// #######################################################################################################
// #################################### Plugin 081: CRON tasks Scheduler       ###########################
// #######################################################################################################

// Maxim Integrated
#ifdef USES_P081

#include <ctype.h>
#include <time.h>

extern "C"
{
#include "ccronexpr.h"
}


#define PLUGIN_081
#define PLUGIN_ID_081      81                 //plugin id
#define PLUGIN_NAME_081   "Generic - CRON [TESTING]" //"Plugin Name" is what will be displayed in the selection list
#define PLUGIN_VALUENAME1_081 "LastExecution"
#define PLUGIN_VALUENAME2_081 "NextExecution"
#ifndef PLUGIN_081_DEBUG
  #define PLUGIN_081_DEBUG  false                //set to true for extra log info in the debug
#endif
#define PLUGIN_081_EXPRESSION_SIZE 41
#define LASTEXECUTION UserVar[event->BaseVarIndex]
#define NEXTEXECUTION UserVar[event->BaseVarIndex+1]

union timeToFloat
{
  time_t time;
  float value;
};


//A plugin has to implement the following function

boolean Plugin_081(byte function, struct EventStruct *event, String& string)
{
  timeToFloat converter;
  boolean success = false;
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
        //This case defines the device characteristics, edit appropriately

        Device[++deviceCount].Number = PLUGIN_ID_081;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;  //how the device is connected
        Device[deviceCount].VType = SENSOR_TYPE_NONE; //type of value the plugin will return, used only for Domoticz
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 2;             //number of output variables. The value should match the number of keys PLUGIN_VALUENAME1_xxx
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].TimerOptional = false;
        Device[deviceCount].GlobalSyncOption = true;
        Device[deviceCount].DecimalsOnly = true;
        break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      //return the device name
      string = F(PLUGIN_NAME_081);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      //called when the user opens the module configuration page
      //it allows to add a new row for each output variable of the plugin
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_081));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_081));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      char expression[PLUGIN_081_EXPRESSION_SIZE];
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&expression, PLUGIN_081_EXPRESSION_SIZE);

      addFormTextBox(F("CRON Expression")
        , F("p081_cron_exp")
        , expression
        , 39);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      String log;
      char expression[PLUGIN_081_EXPRESSION_SIZE];
      strncpy(expression,  WebServer.arg(F("p081_cron_exp")).c_str() , sizeof(expression));
      if(/*strcmp(expression, state.Expression)*/1 != 0)
      {
        cron_expr expr;
        const char* err = NULL;
        memset(&expr, 0, sizeof(expr));
        cron_parse_expr(expression, &expr, &err);
        if (!err)
        {
          unsigned long time __attribute__((unused)) = now();
          time_t last   = mktime((struct tm *)&tm);
          time_t next   = cron_next((cron_expr *)&expr, last);
          converter.time = last;
          LASTEXECUTION = converter.value; // Set current time;
          converter.time = next;
          NEXTEXECUTION = converter.value;

#if PLUGIN_081_DEBUG
          serialPrintln(last);
          converter.value = LASTEXECUTION;
          serialPrintln(converter.time);
          serialPrintln(getDateTimeString(*gmtime(&last)));
          serialPrintln(next);
          converter.value = NEXTEXECUTION;
          serialPrintln(converter.time);
          serialPrintln(getDateTimeString(*gmtime(&next)));
          PrintCronExp(expr);
#endif
        }
        else
        {
          log = String(F("CRON Expression: Error ")) + String(err);
          addHtmlError(log);
          addLog(LOG_LEVEL_ERROR, log);
        }
        log = SaveCustomTaskSettings(event->TaskIndex, (byte*)&expression, PLUGIN_081_EXPRESSION_SIZE);
        if(log != "")
        {
            log = String(PSTR(PLUGIN_NAME_081)) + String(F(": Saving ")) + log;
            addLog(LOG_LEVEL_ERROR, log);
        }
      }
      success = true;
      break;

    }
    case PLUGIN_WEBFORM_SHOW_VALUES:
    {
      converter.value = LASTEXECUTION;
      time_t last = converter.time;
      converter.value = NEXTEXECUTION;
      time_t next = converter.time;
      string += F("<div class=\"div_l\">");
      string += ExtraTaskSettings.TaskDeviceValueNames[0];
      string += F(":</div><div class=\"div_r\">");
      string += getDateTimeString(*gmtime(&last));
      string += F("</div><div class=\"div_br\"></div><div class=\"div_l\">");
      string += ExtraTaskSettings.TaskDeviceValueNames[1];
      string += F(":</div><div class=\"div_r\">");
      string += getDateTimeString(*gmtime(&next));
      string += F("</div>");
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      //this case defines code to be executed when the plugin is initialised
      //after the plugin has been initialised successfuly, set success and break
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
       break;
    }

    case PLUGIN_EXIT:
    {
      //perform cleanup tasks here. For example, free memory

      break;

    }

    case PLUGIN_TIME_CHANGE:
    case PLUGIN_ONCE_A_SECOND:
    {
      //code to be executed once a second. Tasks which do not require fast response can be added here
      String log;
      char expression[PLUGIN_081_EXPRESSION_SIZE];
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&expression, PLUGIN_081_EXPRESSION_SIZE);
      converter.value = LASTEXECUTION;
      time_t last = converter.time;
      converter.value = NEXTEXECUTION;
      time_t next = converter.time;
      unsigned long time __attribute__((unused)) = now();
      struct tm current = tm;
      time_t  current_t = mktime((struct tm *)&current);
      #if PLUGIN_081_DEBUG
        serialPrintln(F("CRON Debug info:"));
        serialPrint(F("Next execution:"));
        serialPrintln(getDateTimeString(*gmtime(&next)));
        serialPrint(F("Current Time:"));
        serialPrintln(getDateTimeString(current));
        serialPrint(F("Triggered:"));
        serialPrintln(next <=  current_t);
      #endif
      if(function == PLUGIN_TIME_CHANGE || next <=  current_t)
      {
        cron_expr expr;
        memset(&expr, 0, sizeof(expr));
        const char* error;
        addLog(LOG_LEVEL_DEBUG, F("Cron Elapsed"));
        cron_parse_expr(expression, &expr, &error);

#if PLUGIN_081_DEBUG
        PrintCronExp(expr);
        serialPrint(F("Expression:"));
        serialPrintln(expression);
#endif
        if(error)
        {
          //TODO: Notify at ui de error
#if PLUGIN_081_DEBUG
          serialPrint(F("Errors:"));
          serialPrintln(String(error));
#endif
          addLog(LOG_LEVEL_ERROR, String(F("CRON Expression:")) + String(error));
        }
        else
        {
          last = current_t; // Set current time;
          next = cron_next((cron_expr *)&expr, last);
          if(next != CRON_INVALID_INSTANT)
          {
#if PLUGIN_081_DEBUG
            serialPrint(F("LastExecution:"));
            serialPrintln(getDateTimeString(*gmtime(&last)));
            serialPrint(F("NextExecution:"));
            serialPrintln(getDateTimeString(*gmtime(&next)));
#endif

            converter.time = last;
            LASTEXECUTION = converter.value;
            converter.time = next;
            NEXTEXECUTION = converter.value;
#if PLUGIN_081_DEBUG
            serialPrintln(F("Check Conversion"));
            serialPrintln(last);
            converter.value = LASTEXECUTION;
            serialPrintln(converter.time);
            serialPrintln(next);
            converter.value = NEXTEXECUTION;
            serialPrintln(converter.time);
#endif
            addLog(LOG_LEVEL_DEBUG, String(F("Next execution:")) + getDateTimeString(*gmtime(&next)));
            LoadTaskSettings(event->TaskIndex);
            if(function != PLUGIN_TIME_CHANGE)
              rulesProcessing(String(F("Cron#")) + String(ExtraTaskSettings.TaskDeviceName));
          }
          else
          {
            log = String(F("CRON: INVALID INSTANT"));
            addLog(LOG_LEVEL_ERROR, log);
          }
        }

      }
      success = true;

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

#if PLUGIN_081_DEBUG
void PrintCronExp(struct cron_expr_t e) {
  serialPrintln(F("===DUMP Cron Expression==="));
  serialPrint(F("Seconds:"));
  for (int i = 0; i < 8; i++)
  {
    serialPrint(e.seconds[i]);
    serialPrint(",");
  }
  serialPrintln();
  serialPrint(F("Minutes:"));
  for (int i = 0; i < 8; i++)
  {
    serialPrint(e.minutes[i]);
    serialPrint(",");
  }
  serialPrintln();
  serialPrint(F("hours:"));
  for (int i = 0; i < 3; i++)
  {
    serialPrint(e.hours[i]);
    serialPrint(",");
  }
  serialPrintln();
  serialPrint(F("months:"));
  for (int i = 0; i < 2; i++)
  {
    serialPrint(e.months[i]);
    serialPrint(",");
  }
  serialPrintln();
  serialPrint(F("days_of_week:"));
  for (int i = 0; i < 1; i++)
  {
    serialPrint(e.days_of_week[i]);
    serialPrint(",");
  }
  serialPrintln();
  serialPrint(F("days_of_month:"));
  for (int i = 0; i < 4; i++)
  {
    serialPrint(e.days_of_month[i]);
    serialPrint(",");
  }
  serialPrintln();
  serialPrintln(F("END=DUMP Cron Expression==="));

}
#endif


#endif // USES_P081
