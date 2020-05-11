#ifdef USES_P098
//#######################################################################################################
//#################################### Plugin 098: Cache Reader #########################################
//#######################################################################################################

#include "_Plugin_Helper.h"

#define PLUGIN_098
#define PLUGIN_ID_098         98
#define PLUGIN_NAME_098       "Generic - Cache Reader"
#define PLUGIN_VALUENAME1_098 "CacheReader"

// Parse Sample from pointer to Sample (24 byte)
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
  float *zerofloat;
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

    zerofloat = new float();
  }
  Sample_t(byte *data){
    timestamp = new unsigned long();
    controller_idx = *(new byte);
    TaskIndex = *(new byte);
    sensorType = *(new byte);
    valueCount = *(new byte);
    val1 = new float();
    val2 = new float();
    val3 = new float();
    val4 = new float();

    zerofloat = new float();

    this->parseSample(data);
  }
  // Destructor
   virtual ~Sample_t(){
     // TODO:
   }
   // Parse Float value, return 0 if parsed value = null
   float *parse_float(byte *data){
     float *current = (float*)data;
     if (*current && *current != '\0'){
       return current;
     } else {
       return this->zerofloat;
     }
   }
   // Parse Byte, return 0 if parsed value = null
   byte parse_byte(byte data){
     if (data){
       return data;
     } else {
       return 0;
     }
   }
  void setTimestamp(byte *data){
    timestamp = (unsigned long*)data;
  }
  void setCtrlIdx(byte *data){
    byte current = data[4];
    controller_idx = this->parse_byte(current);
  }
  void setTaskIdx(byte *data){
    byte current = data[5];
    TaskIndex = this->parse_byte(current);
  }
  void setSensorType(byte *data){
    byte current = data[6];
    sensorType = this->parse_byte(current);
  }
  void setValueCount(byte *data){
    byte current = data[7];
    valueCount = this->parse_byte(current);
  }
  void setVal1(byte *data){
    data += 8;
    val1 = this->parse_float(data);
  }
  void setVal2(byte *data){
    data += 12;
    val2 = this->parse_float(data);
  }
  void setVal3(byte *data){
    data += 16;
    val3 = this->parse_float(data);
  }
  void setVal4(byte *data){
    data += 20;
    val4 = this->parse_float(data);
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
  bool validateSample(){
    //TODO: Check that sample is valid
    return false;
  }
  bool sendSample(){
    // TODO:
    return false;
  }
  String toString(){
    // TODO: Build String from sample
    String returnValue = "BUILD THIS FUNCTION";
    return returnValue;
  }
};

class Cache_t
{
public:
  fs::File cache;
  Sample_t *sample;
  byte *buffer;
  float *fileNr;
  float *offset;
  float *sampleCount;
  float *sampleIndex;
  bool loadFromServer;

  Cache_t(struct EventStruct *event){
    // Create Sample
    sample = new Sample_t();
    // Load values from UserVar array
    fileNr = &UserVar[event->BaseVarIndex];
    offset = &UserVar[event->BaseVarIndex +1];
    sampleCount = &UserVar[event->BaseVarIndex +2];
    sampleIndex = &UserVar[event->BaseVarIndex +3];
    // Initialize buffer for sample data
    buffer = new byte[24];
    for (int i=0; i<24; i++){
      buffer[i] = '\0';
    }
    // Load values if UserVar array values were 0
    this->initialize(event);
    // Open File (Defaults to bin 1 if no values were loaded)
    this->setData();
    // Seek to current sample index
    // Defaults to sample 1 if no values were loaded
    cache.seek(24 * (*sampleIndex) + (*offset));
    // Read sample data into buffer
    cache.read(buffer, 24);
    // Parse buffer into Sample_t
    sample->parseSample(buffer);
    // Check and send sample
    bool validated = sample->validateSample();
    if (validated){
      bool sendSuccess = sample->sendSample();
      if (sendSuccess){
        this->increment();
      }
    }
  }
  void initialize(struct EventStruct *event){
    // TODO:
    if (*fileNr < 1){
      fileNr = this->fetchFileNr();
      UserVar[event->BaseVarIndex] = *fileNr;
    }
    if (*sampleIndex < 1){
      sampleIndex = this->fetchSampleIndex(event);
      UserVar[event->BaseVarIndex +3] = *sampleIndex;
    }
    if (*offset < 1){
      offset = this->getOffset(event);
      UserVar[event->BaseVarIndex +1] = *offset;
    }
    if (*sampleCount < 1){
      sampleCount = this->getSampleCount(event);
      UserVar[event->BaseVarIndex +2] = *sampleCount;
    }
  }
  float *fetchFileNr(){
    // TODO: fetch file nr from server
    // Return 1 if no value available
    return 0;
  }
  float *fetchSampleIndex(struct EventStruct *event){
    // TODO: fetch current sample index
    // Return 1 if no value available
    UserVar[event->BaseVarIndex +3] = 1;
    return &UserVar[event->BaseVarIndex +3];
  }
  bool setData(){
    const char *filename = "cache_1.bin";
    /*
    char * fileindex_str;
    sprintf(fileindex_str, "%f", *fileNr);
    strcat(filename, fileindex_str);
    strcat(filename, ".bin");
    */
    if (!fileExists(filename)){ return false; }
    cache = tryOpenFile(filename,"r");

    return true;
  }
  bool readSample(){
    // TODO:
    return false;
  }
  float *getSampleCount(struct EventStruct *event){
    float totalBytes = std::streamsize(cache);
    UserVar[event->BaseVarIndex +1] = floor((totalBytes-(*offset))/24);
    return &UserVar[event->BaseVarIndex +1];
  }
  float *getOffset(struct EventStruct *event){
    // TODO: Find offset instead of static 16 byte offset
    if (UserVar[event->BaseVarIndex +1] != 16){
      UserVar[event->BaseVarIndex +1] = 16;
    }
    return &UserVar[event->BaseVarIndex +1];
  }
  void setIndex(int index){
    // TODO:
  }
  void increment(){
    // TODO:
  }
  bool deleteFile(){
    // TODO:
    // Move to next file before leaving function
    // Don't delete cache_1.bin, just move to cache_2.bin
    return false;
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
          //float index = UserVar[event->BaseVarIndex + 3];
          //float sample_index = UserVar[event->BaseVarIndex + 2];

          // Temporary check to make sure function is called
          String log = F("Cache Read Single - Called");
          addLog(LOG_LEVEL_INFO, log);

          //String value = "";

          // Open Bin File
          // TODO: Should iterate over all bin files
          /*
          fs::File cache = tryOpenFile("cache_1.bin","r");
          byte *buffer = new byte[24];
          cache.seek(16);
          index += 16;
          cache.read(buffer,24);
          */


          Cache_t *cache = new Cache_t(event);

          //unsigned long *timestamp = (unsigned long*)buffer;

          //Sample_t *sample = new Sample_t(buffer);

          char *string_buffer = new char[128];
          for (int i = 0 ; i < 128 ; i++){
            string_buffer[i] = '\0';
          }

          std::sprintf(string_buffer, "%lu", *(cache->sample->timestamp));
          //std::sprintf(string_buffer, "%f", *sample->val3);

          String publish_value = string_buffer;
          addLog(LOG_LEVEL_INFO, publish_value);


          /*
          if (!ControllerSettings.checkHostReachable(true)) {
              success = false;
              break;
          }
          */

          // Publish to MQTT
          //TODO: Check host reachable
          //      Set correct topic & value

          String tmppubname = "Debug_publish";
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

          success = true;
        }
        break;
      }
  }
  return success;
}

#endif // USES_P098
