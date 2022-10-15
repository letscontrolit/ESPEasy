#include "../Helpers/Misc.h"

#include "../../ESPEasy-Globals.h"
#include "../../ESPEasy_common.h"
#include "../../_Plugin_Helper.h"
#include "../ESPEasyCore/ESPEasy_backgroundtasks.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/Statistics.h"
#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/PeriodicalActions.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"

#if FEATURE_SD
#include <SD.h>
#endif


bool remoteConfig(struct EventStruct *event, const String& string)
{
  // FIXME TD-er: Why have an event here as argument? It is not used.
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("remoteConfig"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  bool   success = false;
  String command = parseString(string, 1);

  if (command.equals(F("config")))
  {
    // Command: "config,task,<taskname>,<actual Set Config command>"
    if (parseString(string, 2).equals(F("task")))
    {
      String configTaskName = parseStringKeepCase(string, 3);

      // FIXME TD-er: This command is not using the tolerance setting
      // tolerantParseStringKeepCase(Line, 4);
      String configCommand = parseStringToEndKeepCase(string, 4);

      if ((configTaskName.isEmpty()) || (configCommand.isEmpty())) {
        return success;
      }
      taskIndex_t index = findTaskIndexByName(configTaskName);

      if (validTaskIndex(index))
      {
        event->setTaskIndex(index);
        success = PluginCall(PLUGIN_SET_CONFIG, event, configCommand);
      }
    } else {
      addLog(LOG_LEVEL_ERROR, F("Expected syntax: config,task,<taskname>,<config command>"));
    }
  }
  return success;
}

/********************************************************************************************\
   delay in milliseconds with background processing
 \*********************************************************************************************/
void delayBackground(unsigned long dsdelay)
{
  unsigned long timer = millis() + dsdelay;

  while (!timeOutReached(timer)) {
    backgroundtasks();
  }
}

/********************************************************************************************\
   Toggle controller enabled state
 \*********************************************************************************************/
bool setControllerEnableStatus(controllerIndex_t controllerIndex, bool enabled)
{
  if (!validControllerIndex(controllerIndex)) { return false; }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("setControllerEnableStatus"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  // Only enable controller if it has a protocol configured
  if ((Settings.Protocol[controllerIndex] != 0) || !enabled) {
    Settings.ControllerEnabled[controllerIndex] = enabled;
    return true;
  }
  return false;
}

/********************************************************************************************\
   Toggle task enabled state
 \*********************************************************************************************/
bool setTaskEnableStatus(struct EventStruct *event, bool enabled)
{
  if (!validTaskIndex(event->TaskIndex)) { return false; }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("setTaskEnableStatus"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  // Only enable task if it has a Plugin configured
  if (validPluginID(Settings.TaskDeviceNumber[event->TaskIndex]) || !enabled) {
    String dummy;

    if (!enabled) {
      PluginCall(PLUGIN_EXIT, event, dummy);
    }
    Settings.TaskDeviceEnabled[event->TaskIndex] = enabled;

    if (enabled) {
      if (!PluginCall(PLUGIN_INIT, event, dummy)) {
        return false;
      }

      // Schedule the task to be executed almost immediately
      Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
    }
    return true;
  }
  return false;
}

/********************************************************************************************\
   Clear task settings for given task
 \*********************************************************************************************/
void taskClear(taskIndex_t taskIndex, bool save)
{
  if (!validTaskIndex(taskIndex)) { return; }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("taskClear"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  Settings.clearTask(taskIndex);
  Cache.clearTaskCaches();
  ExtraTaskSettings.clear(); // Invalidate any cached values.
  ExtraTaskSettings.TaskIndex = taskIndex;

  if (save) {
    SaveTaskSettings(taskIndex);
    SaveSettings();
  }
}

/********************************************************************************************\
   check the program memory hash
   The const MD5_MD5_MD5_MD5_BoundariesOfTheSegmentsGoHere... needs to remain unchanged as it will be replaced by
   - 16 bytes md5 hash, followed by
   - 4 * uint32_t start of memory segment 1-4
   - 4 * uint32_t end of memory segment 1-4
   currently there are only two segemts included in the hash. Unused segments have start adress 0.
   Execution time 520kb @80Mhz: 236ms
   Returns: 0 if hash compare fails, number of checked bytes otherwise.
   The reference hash is calculated by a .py file and injected into the binary.
   Caution: currently the hash sits in an unchecked segment. If it ever moves to a checked segment, make sure
   it is excluded from the calculation !
 \*********************************************************************************************/
#if defined(ARDUINO_ESP8266_RELEASE_2_3_0)
void dump(uint32_t addr) { // Seems already included in core 2.4 ...
  serialPrint(String(addr, HEX));
  serialPrint(": ");

  for (uint32_t a = addr; a < addr + 16; a++)
  {
    serialPrint(String(pgm_read_byte(a), HEX));
    serialPrint(" ");
  }
  serialPrintln();
}

#endif // if defined(ARDUINO_ESP8266_RELEASE_2_3_0)

/*
   uint32_t progMemMD5check(){
    checkRAM(F("progMemMD5check"));
 #define BufSize 10
    uint32_t calcBuffer[BufSize];
    CRCValues.numberOfCRCBytes = 0;
    memcpy (calcBuffer,CRCValues.compileTimeMD5,16);                                                  // is there still the dummy in memory
       ? - the dummy needs to be replaced by the real md5 after linking.
    if( memcmp (calcBuffer, "MD5_MD5_MD5_",12)==0){                                                   // do not memcmp with CRCdummy
       directly or it will get optimized away.
        addLog(LOG_LEVEL_INFO, F("CRC  : No program memory checksum found. Check output of crc2.py"));
        return 0;
    }
    MD5Builder md5;
    md5.begin();
    for (int l = 0; l<4; l++){                                                                            // check max segments,  if the
       pointer is not 0
        uint32_t *ptrStart = (uint32_t *)&CRCValues.compileTimeMD5[16+l*4];
        uint32_t *ptrEnd =   (uint32_t *)&CRCValues.compileTimeMD5[16+4*4+l*4];
        if ((*ptrStart) == 0) break;                                                                      // segment not used.
        for (uint32_t i = *ptrStart; i< (*ptrEnd) ; i=i+sizeof(calcBuffer)){                              // "<" includes last byte
             for (int buf = 0; buf < BufSize; buf ++){
                calcBuffer[buf] = pgm_read_dword((uint32_t*)i+buf);                                       // read 4 bytes
                CRCValues.numberOfCRCBytes+=sizeof(calcBuffer[0]);
             }
             md5.add(reinterpret_cast<const uint8_t *>(&calcBuffer[0]),(*ptrEnd-i)<sizeof(calcBuffer) ? (*ptrEnd-i):sizeof(calcBuffer) );
                    // add buffer to md5.
                At the end not the whole buffer. md5 ptr to data in ram.
        }
   }
   md5.calculate();
   md5.getBytes(CRCValues.runTimeMD5);
   if ( CRCValues.checkPassed())  {
      addLog(LOG_LEVEL_INFO, F("CRC  : program checksum       ...OK"));
      return CRCValues.numberOfCRCBytes;
   }
   addLog(LOG_LEVEL_INFO, F("CRC  : program checksum       ...FAIL"));
   return 0;
   }
 */

/********************************************************************************************\
   Handler for keeping ExtraTaskSettings up to date using cache
 \*********************************************************************************************/
String getTaskDeviceName(taskIndex_t TaskIndex) {
  return Cache.getTaskDeviceName(TaskIndex);
}

/********************************************************************************************\
   Handler for getting Value Names from TaskIndex

   - value names can be accessed with task variable index
   - maximum number of variables <= defined number of variables in plugin
 \*********************************************************************************************/
String getTaskValueName(taskIndex_t TaskIndex, uint8_t TaskValueIndex) {
  const int valueCount = getValueCountForTask(TaskIndex);
  if (TaskValueIndex < valueCount) {
    return Cache.getTaskDeviceValueName(TaskIndex, TaskValueIndex);
  }
  return EMPTY_STRING;
}

/********************************************************************************************\
   If RX and TX tied together, perform emergency reset to get the system out of boot loops
 \*********************************************************************************************/
void emergencyReset()
{
  // Direct Serial is allowed here, since this is only an emergency task.
  Serial.begin(115200);
  Serial.write(0xAA);
  Serial.write(0x55);
  delay(1);

  if (Serial.available() == 2) {
    if ((Serial.read() == 0xAA) && (Serial.read() == 0x55))
    {
      serialPrintln(F("\n\n\rSystem will reset to factory defaults in 10 seconds..."));
      delay(10000);
      ResetFactory();
    }
  }
}

/********************************************************************************************\
   Delayed reboot, in case of issues, do not reboot with high frequency as it might not help...
 \*********************************************************************************************/
void delayedReboot(int rebootDelay, ESPEasy_Scheduler::IntendedRebootReason_e reason)
{
  // Direct Serial is allowed here, since this is only an emergency task.
  while (rebootDelay != 0)
  {
    serialPrint(F("Delayed Reset "));
    serialPrintln(String(rebootDelay));
    rebootDelay--;
    delay(1000);
  }
  reboot(reason);
}

void reboot(ESPEasy_Scheduler::IntendedRebootReason_e reason) {
  prepareShutdown(reason);
  #if defined(ESP32)
  ESP.restart();
  #else // if defined(ESP32)
  ESP.reset();
  #endif // if defined(ESP32)
}

void FeedSW_watchdog()
{
  #ifdef ESP8266
  ESP.wdtFeed();
  #endif // ifdef ESP8266
}

void SendValueLogger(taskIndex_t TaskIndex)
{
#if !defined(BUILD_NO_DEBUG) || FEATURE_SD
  bool   featureSD = false;
  String logger;
  # if FEATURE_SD
  featureSD = true;
  # endif // if FEATURE_SD

  if (featureSD 
      # ifndef BUILD_NO_DEBUG
      || loglevelActiveFor(LOG_LEVEL_DEBUG)
      #endif
  ) {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(TaskIndex);

    if (validDeviceIndex(DeviceIndex)) {
      const uint8_t valueCount = getValueCountForTask(TaskIndex);

      for (uint8_t varNr = 0; varNr < valueCount; varNr++)
      {
        logger += node_time.getDateString('-');
        logger += ' ';
        logger += node_time.getTimeString(':');
        logger += ',';
        logger += Settings.Unit;
        logger += ',';
        logger += getTaskDeviceName(TaskIndex);
        logger += ',';
        logger += getTaskValueName(TaskIndex, varNr);
        logger += ',';
        logger += formatUserVarNoCheck(TaskIndex, varNr);
        logger += F("\r\n");
      }
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, logger);
      #endif
    }
  }
#endif // if !defined(BUILD_NO_DEBUG) || FEATURE_SD

#if FEATURE_SD
  String filename = F("VALUES.CSV");
  fs::File   logFile  = SD.open(filename, FILE_WRITE);

  if (logFile) {
    logFile.print(logger);
  }
  logFile.close();
#endif // if FEATURE_SD
}

// #######################################################################################################
// ############################ quite acurate but slow color converter####################################
// #######################################################################################################
// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white

void HSV2RGB(float H, float S, float I, int rgb[3]) {
  int r, g, b;

  H = fmod(H, 360);                           // cycle H around to 0-360 degrees
  H = 3.14159f * H / static_cast<float>(180); // Convert to radians.
  S = S / 100;
  S = S > 0 ? (S < 1 ? S : 1) : 0;            // clamp S and I to interval [0,1]
  I = I / 100;
  I = I > 0 ? (I < 1 ? I : 1) : 0;

  // Math! Thanks in part to Kyle Miller.
  if (H < 2.09439f) {
    r = 255 * I / 3 * (1 + S * cosf(H) / cosf(1.047196667f - H));
    g = 255 * I / 3 * (1 + S * (1 - cosf(H) / cosf(1.047196667f - H)));
    b = 255 * I / 3 * (1 - S);
  } else if (H < 4.188787f) {
    H = H - 2.09439f;
    g = 255 * I / 3 * (1 + S * cosf(H) / cosf(1.047196667f - H));
    b = 255 * I / 3 * (1 + S * (1 - cosf(H) / cosf(1.047196667f - H)));
    r = 255 * I / 3 * (1 - S);
  } else {
    H = H - 4.188787f;
    b = 255 * I / 3 * (1 + S * cosf(H) / cosf(1.047196667f - H));
    r = 255 * I / 3 * (1 + S * (1 - cosf(H) / cosf(1.047196667f - H)));
    g = 255 * I / 3 * (1 - S);
  }
  rgb[0] = r;
  rgb[1] = g;
  rgb[2] = b;
}

// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white

void HSV2RGBW(float H, float S, float I, int rgbw[4]) {
  int   r, g, b, w;
  float cos_h, cos_1047_h;

  H = fmod(H, 360);                           // cycle H around to 0-360 degrees
  H = 3.14159f * H / static_cast<float>(180); // Convert to radians.
  S = S / 100;
  S = S > 0 ? (S < 1 ? S : 1) : 0;            // clamp S and I to interval [0,1]
  I = I / 100;
  I = I > 0 ? (I < 1 ? I : 1) : 0;

  if (H < 2.09439f) {
    cos_h      = cosf(H);
    cos_1047_h = cosf(1.047196667f - H);
    r          = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
    g          = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
    b          = 0;
    w          = 255 * (1 - S) * I;
  } else if (H < 4.188787f) {
    H          = H - 2.09439f;
    cos_h      = cosf(H);
    cos_1047_h = cosf(1.047196667f - H);
    g          = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
    b          = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
    r          = 0;
    w          = 255 * (1 - S) * I;
  } else {
    H          = H - 4.188787f;
    cos_h      = cosf(H);
    cos_1047_h = cosf(1.047196667f - H);
    b          = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
    r          = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
    g          = 0;
    w          = 255 * (1 - S) * I;
  }

  rgbw[0] = r;
  rgbw[1] = g;
  rgbw[2] = b;
  rgbw[3] = w;
}

// Simple bitwise get/set functions

uint8_t get8BitFromUL(uint32_t number, uint8_t bitnr) {
  return (number >> bitnr) & 0xFF;
}

void set8BitToUL(uint32_t& number, uint8_t bitnr, uint8_t value) {
  uint32_t mask     = (0xFFUL << bitnr);
  uint32_t newvalue = ((value << bitnr) & mask);

  number = (number & ~mask) | newvalue;
}

uint8_t get4BitFromUL(uint32_t number, uint8_t bitnr) {
  return (number >> bitnr) &  0x0F;
}

void set4BitToUL(uint32_t& number, uint8_t bitnr, uint8_t value) {
  uint32_t mask     = (0x0FUL << bitnr);
  uint32_t newvalue = ((value << bitnr) & mask);

  number = (number & ~mask) | newvalue;
}

uint8_t get3BitFromUL(uint32_t number, uint8_t bitnr) {
  return (number >> bitnr) &  0x07;
}

void set3BitToUL(uint32_t& number, uint8_t bitnr, uint8_t value) {
  uint32_t mask     = (0x07UL << bitnr);
  uint32_t newvalue = ((value << bitnr) & mask);

  number = (number & ~mask) | newvalue;
}

uint8_t get2BitFromUL(uint32_t number, uint8_t bitnr) {
  return (number >> bitnr) &  0x03;
}

void set2BitToUL(uint32_t& number, uint8_t bitnr, uint8_t value) {
  uint32_t mask     = (0x03UL << bitnr);
  uint32_t newvalue = ((value << bitnr) & mask);

  number = (number & ~mask) | newvalue;
}

float getCPUload() {
  return 100.0f - Scheduler.getIdleTimePct();
}

int getLoopCountPerSec() {
  return loopCounterLast / 30;
}

int getUptimeMinutes() {
  return wdcounter / 2;
}

/******************************************************************************
 * scan an int array of specified size for a value
 *****************************************************************************/
bool intArrayContains(const int arraySize, const int array[], const int& value) {
  for (int i = 0; i < arraySize; i++) {
    if (array[i] == value) { return true; }
  }
  return false;
}

bool intArrayContains(const int arraySize, const uint8_t array[], const uint8_t& value) {
  for (int i = 0; i < arraySize; i++) {
    if (array[i] == value) { return true; }
  }
  return false;
}

#ifndef BUILD_NO_RAM_TRACKER
void logMemUsageAfter(const __FlashStringHelper *function, int value) {
  // Store free memory in an int, as subtracting may sometimes result in negative value.
  // The recorded used memory is not an exact value, as background (or interrupt) tasks may also allocate or free heap memory.
  static int last_freemem = ESP.getFreeHeap();
  const int  freemem_end  = ESP.getFreeHeap();

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    if (log.reserve(128)) {
      log  = F("After ");
      log += function;

      if (value >= 0) {
        log += value;
      }

      while (log.length() < 30) { log += ' '; }
      log += F("Free mem after: ");
      log += freemem_end;

      while (log.length() < 55) { log += ' '; }
      log += F("diff: ");
      log += last_freemem - freemem_end;
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  }

  last_freemem = freemem_end;
}

#endif // ifndef BUILD_NO_RAM_TRACKER
