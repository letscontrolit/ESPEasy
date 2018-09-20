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
#define PLUGIN_NAME_081   "CRON"              //"Plugin Name" is what will be dislpayed in the selection list
#define PLUGIN_VALUENAME1_081 "LastExecution"
#define PLUGIN_VALUENAME2_081 "NextExecution"
#define PLUGIN_081_DEBUG  false                //set to true for extra log info in the debug
#define PLUGIN_081_EXPRESSION_SIZE 41


typedef struct
{
  char Expression[PLUGIN_081_EXPRESSION_SIZE];
  time_t LastExecution;
  time_t NextExecution;
} CronState;

//A plugin has to implement the following function

boolean Plugin_081(byte function, struct EventStruct *event, String& string)
{

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
      CronState state;
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&state, sizeof(state));

      addFormTextBox(F("CRON Expression")
        , F("plugin_081_cron_exp")
        , state.Expression
        , 39);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      CronState state;
      String log;
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&state, sizeof(state));
      char expression[PLUGIN_081_EXPRESSION_SIZE];
      strncpy(expression,  WebServer.arg(F("plugin_081_cron_exp")).c_str() , sizeof(expression));
      if(/*strcmp(expression, state.Expression)*/1 != 0)
      {
        strncpy(state.Expression, expression, sizeof(state.Expression));
        cron_expr expr;
        const char* err = NULL;
        memset(&expr, 0, sizeof(expr));
        cron_parse_expr(expression, &expr, &err);
        if (!err)
        {
          state.LastExecution = mktime((struct tm *)&tm); // Set current time;
          state.NextExecution = cron_next((cron_expr *)&expr, state.LastExecution);
#if PLUGIN_081_DEBUG
          PrintCronExp(expr);
#endif
        }
        else
        {
          log = String(F("CRON Expression: Error ")) + String(err);
          addHtmlError(log);
          addLog(LOG_LEVEL_ERROR, log);
        }
        log = SaveCustomTaskSettings(event->TaskIndex, (byte*)&state, sizeof(state));
        if(log != F(""))
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
      CronState state;
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&state, sizeof(state));

      string += F("<div class=\"div_l\">");
      string += ExtraTaskSettings.TaskDeviceValueNames[0];
      string += F(":</div><div class=\"div_r\">");
      string += getDateTimeString(*gmtime(&state.LastExecution));
      string += F("</div><div class=\"div_br\"></div><div class=\"div_l\">");
      string += ExtraTaskSettings.TaskDeviceValueNames[1];
      string += F(":</div><div class=\"div_r\">");
      string += getDateTimeString(*gmtime(&state.NextExecution));
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
      //code to be executed to read data
      //It is executed according to the delay configured on the device configuration page, only once
      CronState state;
      String log;
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&state, sizeof(state));
      //after the plugin has read data successfuly, set success and break
      UserVar[event->BaseVarIndex] = (float)state.LastExecution;
      UserVar[event->BaseVarIndex + 1] = (float)state.NextExecution;
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
      CronState state;
      String log;
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&state, sizeof(state));
      unsigned long time __attribute__((unused)) = now();
      struct tm current = tm;
      #if PLUGIN_081_DEBUG
        Serial.println(F("CRON Debug info:"));
        Serial.print(F("Next execution:"));
        Serial.println(getDateTimeString(*gmtime(&state.NextExecution)));
        Serial.print(F("Current Time:"));
        Serial.println(getDateTimeString(current));
        Serial.print(F("Triggered:"));
        Serial.println(state.NextExecution <=  mktime((struct tm *)&current));
      #endif
      if(function == PLUGIN_TIME_CHANGE || state.NextExecution <=  mktime((struct tm *)&current))
      {
        cron_expr expr;
        memset(&expr, 0, sizeof(expr));
        const char* error;
        addLog(LOG_LEVEL_DEBUG, F("Cron Elapsed"));
        cron_parse_expr(state.Expression, &expr, &error);

#if PLUGIN_081_DEBUG
        PrintCronExp(expr);
        Serial.print(F("Expression:"));
        Serial.println(state.Expression);
#endif
        if(error)
        {
          //TODO: Notify at ui de error
#if PLUGIN_081_DEBUG
          Serial.print(F("Errors:"));
          Serial.println(String(error));
#endif
          addLog(LOG_LEVEL_ERROR, String(F("CRON Expression:")) + String(error));
        }
        else
        {

          state.LastExecution = mktime((struct tm *)&current); // Set current time;
          state.NextExecution = cron_next((cron_expr *)&expr, state.LastExecution);
          if(state.NextExecution != CRON_INVALID_INSTANT)
          {
#if PLUGIN_081_DEBUG
            Serial.print(F("LastExecution:"));
            Serial.println(getDateTimeString(*gmtime(&state.LastExecution)));
            Serial.print(F("NextExecution:"));
            Serial.println(getDateTimeString(*gmtime(&state.NextExecution)));
#endif
            String log = SaveCustomTaskSettings(event->TaskIndex, (byte*)&state, sizeof(state));
            if(log != F(""))
            {
              log = String(PSTR(PLUGIN_NAME_081)) + String(F(":")) + log;
              addLog(LOG_LEVEL_ERROR, log);
            }
            addLog(LOG_LEVEL_DEBUG, String(F("Next execution:")) + getDateTimeString(*gmtime(&state.NextExecution)));
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
  Serial.println(F("===DUMP Cron Expression==="));
  Serial.print(F("Seconds:"));
  for (int i = 0; i < 8; i++)
  {
    Serial.print(e.seconds[i]);
    Serial.print(F(","));
  }
  Serial.println();
  Serial.print(F("Minutes:"));
  for (int i = 0; i < 8; i++)
  {
    Serial.print(e.minutes[i]);
    Serial.print(F(","));
  }
  Serial.println();
  Serial.print(F("hours:"));
  for (int i = 0; i < 3; i++)
  {
    Serial.print(e.hours[i]);
    Serial.print(F(","));
  }
  Serial.println();
  Serial.print(F("months:"));
  for (int i = 0; i < 2; i++)
  {
    Serial.print(e.months[i]);
    Serial.print(F(","));
  }
  Serial.println();
  Serial.print(F("days_of_week:"));
  for (int i = 0; i < 1; i++)
  {
    Serial.print(e.days_of_week[i]);
    Serial.print(F(","));
  }
  Serial.println();
  Serial.print(F("days_of_month:"));
  for (int i = 0; i < 4; i++)
  {
    Serial.print(e.days_of_month[i]);
    Serial.print(F(","));
  }
  Serial.println();
  Serial.println(F("END=DUMP Cron Expression==="));

}
#endif


#endif // USES_P081
