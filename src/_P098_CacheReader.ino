#ifdef USES_P098
//#######################################################################################################
//#################################### Plugin 098: Cache Reader #########################################
//#######################################################################################################

#include "_Plugin_Helper.h"

#define PLUGIN_098
#define PLUGIN_ID_098         98
#define PLUGIN_NAME_098       "Generic - Cache Reader"
#define PLUGIN_VALUENAME1_098 "CacheReader"

struct Sample_t
{
  unsigned long timestamp;
  byte controller_idx;
  byte TaskIndex;
  byte sensorType;
  byte valueCount;
  float val1;
  float val2;
  float val3;
  float val4;
};
/* Can probably delete
struct P098_data_struct : public PluginTaskData_base {
  P098_data_struct() : CacheReader(nullptr) {}

  ~P098_data_struct(){
    reset();
  }
}
void reset(){
  if (CacheReader != nullptr){
    delete CacheReader;
    CacheReader = nullptr;
  }
bool init(){
  reset();
  sample = new Sample_t();
  return isInitialized();
}
*/

boolean Plugin_098(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_098;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].DecimalsOnly = true;
        Device[deviceCount].ValueCount = 4;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_098);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_098));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        sensorTypeHelper_webformLoad_allTypes(event, 0);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        sensorTypeHelper_saveSensorType(event, 0);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        // Do not set the sensor type, or else it will be set for all instances of the Dummy plugin.
        //sensorTypeHelper_setSensorType(event, 0);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        bool fileFound = fileExists("cache_1.bin");
        String log = F("File Found: ");

        if (fileFound) {
          log = F("File Found: True");
        } else {
          log = F("File Found: False");
        }
        addLog(LOG_LEVEL_INFO,log);
        UserVar[event->BaseVarIndex + 1] = fileFound;

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string,1);
        if(command == F("readcachesingle"))
        {
            // Temporary check to make sure function is called
            String log = F("Cache Read Single - Called");
            addLog(LOG_LEVEL_INFO, log);

            /*
            if (!ControllerSettings.checkHostReachable(true)) {
                success = false;
                break;
            }
            */

            // Publish to MQTT
            //TODO: Check host reachable
            //      Set correct topic & value
            String tmppubname = "AUTOSEND";
            String value = "7777";
            bool publish_success = MQTTpublish(event->ControllerIndex, tmppubname.c_str(), value.c_str(), true);

            String publish_message = "";
            if (publish_success){
              publish_message = F("Publish success: True");
            } else {
              publish_message = F("Publish success: False");
            }
            addLog(LOG_LEVEL_INFO, publish_message);
            //ControllerSettings.mqtt_retainFlag() TODO: Should set retain flag from interface

            fs::File cache = tryOpenFile("cache_1.bin","r");
            byte buffer[24];
            cache.read(buffer, 24);
            Sample_t *sample = new Sample_t();
            Sample_t smpl = (Sample_t)(void*)buffer;
/*
            char* chr = (char*)buffer;
            char *arr[6];
            arr[0] = chr;
*/
            success = true;
        }
        break;
      }
  }
  return success;
}

#endif // USES_P098
