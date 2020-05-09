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

// Seek next non null value
float nextNonNull(fs::File cache, byte *buffer, float current){
  while (!buffer[0]){
    cache.seek(current);
    cache.read(buffer, 1);
    current++;
    if (buffer[0] == EOF){
      break;
    }
    return current;
  }
  return 0;
}
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
            float index = UserVar[event->BaseVarIndex + 3];
            // Temporary check to make sure function is called
            String log = F("Cache Read Single - Called");
            addLog(LOG_LEVEL_INFO, log);

            String value = "";

            fs::File cache = tryOpenFile("cache_1.bin","r");
            byte *buffer = new byte[4];

            /*
            // Seek to next non null value
            while (!buffer[0]){
              cache.seek(index);
              cache.read(buffer, 1);
              index++;
              if (buffer[0] == EOF){
                break;
              }
            }
            */
            cache.seek(16);
            index += 16;
            cache.read(buffer,4);

            //unsigned long timestamp = (unsigned long)*(void *)buffer;

            // Convert to little endian
            byte *timestamp_arr = new byte[4];
            timestamp_arr[0] = buffer[0];
            timestamp_arr[1] = buffer[1];
            timestamp_arr[2] = buffer[2];
            timestamp_arr[3] = buffer[3];

            unsigned long *timestamp = (unsigned long*)timestamp_arr;


            /*
            Sample_t *sample = new Sample_t();
            sample->timestamp = (unsigned long)buffer;
            sample->controller_idx = buffer[4];
            sample->TaskIndex = buffer[5];
            sample->sensorType = buffer[6];
            sample->valueCount = buffer[7];
            sample->val1 = buffer[8];
            sample->val2 = buffer[12];
            sample->val3 = buffer[16];
            sample->val4 = buffer[20];
            */

            char *string_buffer = new char[4];

            std::sprintf(string_buffer, "%lu", *timestamp);

            String publish_value = string_buffer;
            addLog(LOG_LEVEL_INFO, publish_value);

            /*
            // Seek to next start of sample
            while (buffer[0] != 11100001 &&
                   buffer[1] != 11111010 &&
                   buffer[2] != 11000111 &&
                   buffer[3] != 1000010)
                   {
                     index++;
                     cache.seek(index+1);
                     cache.read(buffer,4);
                   }
            */

            UserVar[event->BaseVarIndex + 3] = index;

            /*
            if (!ControllerSettings.checkHostReachable(true)) {
                success = false;
                break;
            }
            */

            // Publish to MQTT
            //TODO: Check host reachable
            //      Set correct topic & value
            String tmppubname = "AUTOSEND_BIN";
            bool publish_success = MQTTpublish(event->ControllerIndex, tmppubname.c_str(), string_buffer, true);
            //value.c_str()
            String publish_message = "";
            if (publish_success){
              publish_message = F("Publish success: True");
            } else {
              publish_message = F("Publish success: False");
            }
            addLog(LOG_LEVEL_INFO, publish_message);
            //ControllerSettings.mqtt_retainFlag() TODO: Should set retain flag from interface

            delete []buffer;

            success = true;
        }
        break;
      }
  }
  return success;
}

#endif // USES_P098
