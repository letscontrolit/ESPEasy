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
# include <SD.h>
#endif // if FEATURE_SD


bool remoteConfig(struct EventStruct *event, const String& string)
{
  // FIXME TD-er: Why have an event here as argument? It is not used.
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("remoteConfig"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  bool   success = false;
  String command = parseString(string, 1);

  if (equals(command, F("config")))
  {
    // Command: "config,task,<taskname>,<actual Set Config command>"
    if (equals(parseString(string, 2), F("task")))
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
  if (validPluginID(Settings.getPluginID_for_task(event->TaskIndex)) || !enabled) {
    String dummy;

    if (!enabled) {
      PluginCall(PLUGIN_EXIT, event, dummy);
    }

    // Toggle enable/disable state via command
    // FIXME TD-er: Should this be a 'runtime' change, or actually change the intended state?
    // Settings.TaskDeviceEnabled[event->TaskIndex].enabled = enabled;
    Settings.TaskDeviceEnabled[event->TaskIndex] = enabled;

    if (enabled) {
      // Schedule the plugin to be read.
      // Do this before actual init, to allow the plugin to schedule a specific first read.
      Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);

      if (!PluginCall(PLUGIN_INIT, event, dummy)) {
        return false;
      }
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

  if (Settings.TaskDeviceEnabled[taskIndex]) {
    struct EventStruct TempEvent(taskIndex);
    String dummy;
    PluginCall(PLUGIN_EXIT, &TempEvent, dummy);
  }
  Settings.clearTask(taskIndex);
  clearTaskCache(taskIndex); // Invalidate any cached values.
  ExtraTaskSettings.clear();
  ExtraTaskSettings.TaskIndex = taskIndex;

  if (save) {
    addLog(LOG_LEVEL_INFO, F("taskClear() save settings"));
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
  ESPEASY_SERIAL_0.begin(115200);
  ESPEASY_SERIAL_0.write(0xAA);
  ESPEASY_SERIAL_0.write(0x55);
  delay(1);

  if (ESPEASY_SERIAL_0.available() == 2) {
    if ((ESPEASY_SERIAL_0.read() == 0xAA) && (ESPEASY_SERIAL_0.read() == 0x55))
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
void delayedReboot(int rebootDelay, IntendedRebootReason_e reason)
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

void reboot(IntendedRebootReason_e reason) {
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
      # endif // ifndef BUILD_NO_DEBUG
      ) {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(TaskIndex);

    if (validDeviceIndex(DeviceIndex)) {
      const uint8_t valueCount = getValueCountForTask(TaskIndex);

      const String logline_prefix =
        strformat(F("%s %s,%d,%s")
                  , node_time.getDateString('-').c_str()
                  , node_time.getTimeString(':').c_str()
                  , Settings.Unit
                  , getTaskDeviceName(TaskIndex).c_str()
                  );

      for (uint8_t varNr = 0; varNr < valueCount; varNr++)
      {
        logger += strformat(F("%s,%s,%s\r\n")
                            , logline_prefix.c_str()
                            , Cache.getTaskDeviceValueName(TaskIndex, varNr).c_str()
                            , formatUserVarNoCheck(TaskIndex, varNr).c_str()
                            );
      }
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, logger);
      # endif // ifndef BUILD_NO_DEBUG
    }
  }
#endif // if !defined(BUILD_NO_DEBUG) || FEATURE_SD

#if FEATURE_SD

  if (!logger.isEmpty()) {
    String   filename = patch_fname(F("VALUES.CSV"));
    fs::File logFile  = SD.open(filename, "a+");

    if (logFile) {
      logFile.print(logger);
    }
    logFile.close();
  }
#endif // if FEATURE_SD
}

// #######################################################################################################
// ############################ quite acurate but slow color converter####################################
// #######################################################################################################
// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white

void HSV2RGB(float H, float S, float I, int rgb[3]) {
  // FIXME TD-er:   Why not just call HSV2RGBW and leave out the W part?

  int rgbw[4]{};

  HSV2RGBW(H, S, I, rgbw);
  memcpy(rgb, rgbw, 3 * sizeof(int));

  /*

     int r, g, b;

     H = fmod(H, 360);                           // cycle H around to 0-360 degrees
     constexpr float deg2rad = 3.14159f / 180.0f;
     H *= deg2rad;                               // Convert to radians.
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
   */
}

// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white
void HSV2RGBW(float H, float S, float I, int rgbw[4]) {
  H = fmod(H, 360);                 // cycle H around to 0-360 degrees
  constexpr float deg2rad = 3.14159f / 180.0f;
  H *= deg2rad;                     // Convert to radians.
  S  = S / 100;
  S  = S > 0 ? (S < 1 ? S : 1) : 0; // clamp S and I to interval [0,1]
  I  = I / 100;
  I  = I > 0 ? (I < 1 ? I : 1) : 0;

  #define RGB_ORDER 0
  #define BRG_ORDER 1
  #define GBR_ORDER 2

  int order = RGB_ORDER;

  constexpr float ANGLE_120_DEG = 120.0f * deg2rad;
  constexpr float ANGLE_240_DEG = 240.0f * deg2rad;
  constexpr float ANGLE_60_DEG  =  60.0f * deg2rad;

  if (H < ANGLE_120_DEG) {
    order = RGB_ORDER;
  } else if (H < ANGLE_240_DEG) {
    H     = H - ANGLE_120_DEG;
    order = BRG_ORDER;
  } else {
    H     = H - ANGLE_240_DEG;
    order = GBR_ORDER;
  }
  const float cos_h      = cosf(H);
  const float cos_1047_h = cosf(ANGLE_60_DEG - H);

  const int r = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
  const int g = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
  const int b = 0;
  rgbw[3] = 255 * (1 - S) * I;

  if (RGB_ORDER == order) {
    rgbw[0] = r;
    rgbw[1] = g;
    rgbw[2] = b;
  } else if (BRG_ORDER == order) {
    rgbw[0] = b;
    rgbw[1] = r;
    rgbw[2] = g;
  } else if (GBR_ORDER == order) {
    rgbw[0] = g;
    rgbw[1] = b;
    rgbw[2] = r;
  }
}

// Convert RGB Color to HSV Color
void RGB2HSV(uint8_t r, uint8_t g, uint8_t b, float hsv[3]) {
  const float rf = static_cast<float>(r) / 255.0f;
  const float gf = static_cast<float>(g) / 255.0f;
  const float bf = static_cast<float>(b) / 255.0f;
  float maxval   = rf;

  if (gf > maxval) { maxval = gf; }

  if (bf > maxval) { maxval = bf; }
  float minval = rf;

  if (gf < minval) { minval = gf; }

  if (bf < minval) { minval = bf; }
  float h = 0.0f, s, v = maxval;
  float f = maxval - minval;

  s = maxval == 0.0f ? 0.0f : f / maxval;

  if (maxval == minval) {
    h = 0.0f; // achromatic
  } else {
    if (maxval == rf) {
      h = (gf - bf) / f + (gf < bf ? 6.0f : 0.0f);
    } else if (maxval == gf) {
      h = (bf - rf) / f + 2.0f;
    } else if (maxval == bf) {
      h = (rf - gf) / f + 4.0f;
    }
    h /= 6.0f;
  }

  hsv[0] = h * 360.0f;
  hsv[1] = s * 255.0f;
  hsv[2] = v * 255.0f;
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
