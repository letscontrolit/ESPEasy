#ifdef USES_P098
//#######################################################################################################
//#################################### Plugin 098: Cache Reader #########################################
//#######################################################################################################

#include "_Plugin_Helper.h"

#define PLUGIN_098
#define PLUGIN_ID_098         98
#define PLUGIN_NAME_098       "Generic - Cache Reader"
#define PLUGIN_VALUENAME1_098 "CacheReader"

// Parse Byte, return 0 if parsed value = null
byte parse_byte(byte data){
  if (data){
    return data;
  } else {
    return 0;
  }
}
// Parse Float value, return 0 if parsed value = null
float *parse_float(byte data){
  float *current = (float*)(&data);

  if (*current){
    return current;
  } else {
    return 0;
  }
}

class Sample_t
{
public:
  unsigned long *timestamp;
  byte controller_idx;
  byte TaskIndex;
  byte sensorType;
  byte valueCount;
  float *val1;
  float *val2;
  float *val3;
  float *val4;

  Sample_t(){
    timestamp = new unsigned long();
    controller_idx = *(new byte);
    TaskIndex = *(new byte);
    sensorType = *(new byte);
    valueCount = *(new byte);
    val1 = new float();
    val2 = new float();
    val3 = new float();
    val4 = new float();
  }
  Sample_t(byte *data){
    this->parseSample(data);
  }
  // Destructor
   virtual ~ Sample_t(){
     // TODO:
   }

  void setTimestamp(byte *data){
    timestamp = (unsigned long*)data;
  }
  void setCtrlIdx(byte *data){
    byte current = data[4];
    controller_idx = parse_byte(current);
  }
  void setTaskIdx(byte *data){
    byte current = data[5];
    TaskIndex = parse_byte(current);
  }
  void setSensorType(byte *data){
    byte current = data[6];
    sensorType = parse_byte(current);
  }
  void setValueCount(byte *data){
    byte current = data[7];
    valueCount = parse_byte(current);
  }
  void setVal1(byte *data){
    byte current = data[8];
    val1 = parse_float(current);
  }
  void setVal2(byte *data){
    byte current = data[12];
    val2 = parse_float(current);
  }
  void setVal3(byte *data){
    byte current = data[16];
    val3 = parse_float(current);
  }
  void setVal4(byte *data){
    byte current = data[20];
    val4 = parse_float(current);
  }
  void parseSample(byte *data){
    // Set timestamp
    this->setTimestamp(data);
    // Set Byte Values
    this->setCtrlIdx(data);
    this->setTaskIdx(data);
    this->setSensorType(data);
    this->setValueCount(data);
    // Set float values
    this->setVal1(data);
    this->setVal2(data);
    this->setVal3(data);
    this->setVal4(data);
  }
  String toString(){
    // TODO: Build String from sample
    String returnValue = "BUILD THIS FUNCTION";
    return returnValue;
  }
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
          //float sample_index = UserVar[event->BaseVarIndex + 2];

          // Temporary check to make sure function is called
          String log = F("Cache Read Single - Called");
          addLog(LOG_LEVEL_INFO, log);

          String value = "";

          // Open Bin File
          // TODO: Should iterate over all bin files
          fs::File cache = tryOpenFile("cache_1.bin","r");
          byte *buffer = new byte[24];


          cache.seek(16);
          index += 16;
          cache.read(buffer,24);

          //unsigned long *timestamp = (unsigned long*)buffer;
          Sample_t *sample = new Sample_t(buffer);


          char *string_buffer = new char[128];
          std::sprintf(string_buffer, "%lu", *sample->timestamp);


          String publish_value = string_buffer;
          addLog(LOG_LEVEL_INFO, publish_value);

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
