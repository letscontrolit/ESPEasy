#ifdef USES_P098
//#######################################################################################################
//#################################### Plugin 098: Cache Reader #########################################
//#######################################################################################################

#include "_Plugin_Helper.h"

#define PLUGIN_098
#define PLUGIN_ID_098         98
#define PLUGIN_NAME_098       "Generic - Cache Reader"
#define PLUGIN_VALUENAME1_098 "fileNr"
#define PLUGIN_VALUENAME2_098 "offset"
#define PLUGIN_VALUENAME3_098 "sampleCount"
#define PLUGIN_VALUENAME4_098 "sampleIndex"

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
   ~Sample_t(){
     delete timestamp;
     delete &controller_idx;
     delete &TaskIndex;
     delete &sensorType;
     delete &valueCount;
     delete[] val1;
     delete[] val2;
     delete[] val3;
     delete[] val4;
     delete[] zerofloat;
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
    return true;
  }
  bool sendSample(struct EventStruct *event){

    char *send_buffer = new char[10];
    // TODO: Does buffer need to be 64 characters?
    char *topic_buffer = new char[64];
    for (int i = 0 ; i<64; i++){
      topic_buffer[i] = '\0';
    }

    for (int i=1 ; i<5 ;i++){
      for (int i=0 ; i< 10 ; i++){
        send_buffer[i] = '\0';
      }
      switch (i) {
        case 1:
          std::sprintf(send_buffer, "%f", *this->val1);
          break;
        case 2:
          std::sprintf(send_buffer, "%f", *this->val2);
          break;
        case 3:
          std::sprintf(send_buffer, "%f", *this->val3);
          break;
        case 4:
          std::sprintf(send_buffer, "%f", *this->val4);
          break;
        default:
          addLog(LOG_LEVEL_INFO, F("Case Default"));
          delete []send_buffer;
          delete []topic_buffer;
          return false;
      }
      std::sprintf(topic_buffer, "tracker/%d/%lu/%d/%d", (int)Settings.Unit,*(this->timestamp),this->controller_idx,i);
      bool publish_success = MQTTpublish(event->ControllerIndex, topic_buffer, send_buffer, false);
        //ControllerSettings.mqtt_retainFlag(); Uncomment and load controllersettings to set retain flag from controller interface
      if (!publish_success){
        delete []send_buffer;
        delete []topic_buffer;
        return false;
      }
    }
    delete []send_buffer;
    delete []topic_buffer;
    return true;
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

  Cache_t(){
    // Create Sample
    sample = new Sample_t();
  }
  ~Cache_t(){
    //delete &cache; TODO: Check how to delete stream object instance
    //delete sample;
    //delete[] buffer;
  }
  void initialize(struct EventStruct *event){
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
    if (*fileNr < 1){
      this->fetchFileNr();
    }
    if (*sampleIndex < 1){
      this->fetchSampleIndex();
    }
    if (*offset < 1){
      this->getOffset();
    }
    this->setData();
  }
  void fetchFileNr(){
    // TODO: fetch file nr from server
    (*this->fileNr) = 1;
  }
  void fetchSampleIndex(){
    // TODO: fetch current sample index from server
  }
  void getOffset(){
    // TODO: Find offset instead of static 16 byte offset
    float staticOffset = 16;
    memcpy(offset, &staticOffset, 4);
  }
  bool setData(){
    int fnr = (int)(*this->fileNr);
    char *filename = new char[24];
    for (int i = 0 ; i<24; i++){
      filename[i] = '\0';
    }
    sprintf(filename,"cache_%d.bin",fnr);
    //const char *filename = "cache_1.bin";
    if (!fileExists(filename)){
      this->nextFile();
      return false;
    }
    cache = tryOpenFile(filename,"r");
    delete[] filename;

    return true;
  }
  void readSample(struct EventStruct *event){
    // Seek to current sample index
    // TODO: Check return value if seek is out of bounds!
    cache.seek(24 * (*sampleIndex) + (*offset));
    // Read sample data into buffer
    size_t read = cache.read(buffer, 24);
    // Check if read was valid and does not contain EOF
    if (read == 24 && buffer[23] != EOF){
      // Parse buffer into Sample_t
      sample->parseSample(buffer);
      // Validate and send sample
      bool validated = sample->validateSample();
      if (validated){
        sample->sendSample(event);
      }
      // Increment to next sample
      this->increment();
    } else {
      this->nextFile();
    }
  }
  void setIndex(int index){
    // TODO:
  }
  void increment(){
    // TODO: Check if last sample in file
    // If so, move to next file
    (*this->sampleIndex)++;
  }
  void nextFile(){
    (*this->fileNr)++;
    int fnr = (int)(*this->fileNr);
    char *fname = new char[24];
    for (int i = 0 ; i<24; i++){
      fname[i] = '\0';
    }
    // Currently maximum 100 bin files
    while (fnr < 100){
      sprintf(fname,"cache_%d.bin",fnr);
      if (fileExists(fname)){
        (*this->sampleIndex) = 0;
        (*this->fileNr) = fnr;
        this->setData();
        break;
      }
      if (fnr == 99){
        (*this->fileNr) = 1;
        (*this->sampleIndex) = 0;
        break;
      }
      fnr++;
    }
    delete[] fname;
  }
  bool deleteFile(){
    // TODO:
    // Move to next file before leaving function
    // Don't delete cache_1.bin, just move to cache_2.bin
    return false;
  }
};

struct P098_data_struct : public PluginTaskData_base {
  Cache_t *cache = nullptr;
  P098_data_struct() {}
  ~P098_data_struct(){
    reset();
  }
  void reset(){
    if (cache != nullptr){
      delete cache;
      cache = nullptr;
    }
  }
  bool init(){
    reset();
    cache = new Cache_t();
    return isInitialized();
  }
  bool isInitialized() const {
    return cache != nullptr;
  }
};

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
        initPluginTaskData(event->TaskIndex, new P098_data_struct());
        P098_data_struct *P098_data =
          static_cast<P098_data_struct *>(getPluginTaskData(event->TaskIndex));
        P098_data->init();
        P098_data->cache->initialize(event);
        success = true;
        break;
      }
    case PLUGIN_EXIT: {
      clearPluginTaskData(event->TaskIndex);
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
        String command = parseString(string,1);
        if(command == F("readcachesingle"))
        {
          P098_data_struct *P098_data =
            static_cast<P098_data_struct *>(getPluginTaskData(event->TaskIndex));
          bool initialized = P098_data->isInitialized();

          if (initialized){
            for (int i = 0 ; i < 2 ; i++){
              P098_data->cache->readSample(event);
            }
          } else {
            P098_data->init();
            P098_data->cache->initialize(event);
          }
          success = true;
        }
        break;
      }
  }
  return success;
}

#endif // USES_P098
