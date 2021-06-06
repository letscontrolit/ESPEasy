#ifndef HELPERS_MISC_H
#define HELPERS_MISC_H

#include <Arduino.h>

#include "../DataStructs/PinMode.h"
#include "../DataTypes/ControllerIndex.h"
#include "../DataTypes/TaskIndex.h"
#include "../Helpers/Scheduler.h"

#include "../../ESPEasy_common.h"

#ifdef FEATURE_SD
#include <SD.h>
#endif

bool remoteConfig(struct EventStruct *event,
                  const String      & string);



/********************************************************************************************\
   delay in milliseconds with background processing
 \*********************************************************************************************/
void delayBackground(unsigned long dsdelay);


/********************************************************************************************\
   Toggle controller enabled state
 \*********************************************************************************************/
bool setControllerEnableStatus(controllerIndex_t controllerIndex,
                               bool              enabled);

/********************************************************************************************\
   Toggle task enabled state
 \*********************************************************************************************/
bool setTaskEnableStatus(struct EventStruct *event,
                         bool        enabled);


/********************************************************************************************\
   Clear task settings for given task
 \*********************************************************************************************/
void taskClear(taskIndex_t taskIndex,
               bool        save);


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
void dump(uint32_t addr);
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
             md5.add((uint8_t *)&calcBuffer[0],(*ptrEnd-i)<sizeof(calcBuffer) ? (*ptrEnd-i):sizeof(calcBuffer) );     // add buffer to md5.
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
String getTaskDeviceName(taskIndex_t TaskIndex);

/********************************************************************************************\
   Handler for getting Value Names from TaskIndex

   value names can be accessed with variable index; maxium number of variables == VARS_PER_TASK
 \*********************************************************************************************/
 String getTaskValueName(taskIndex_t TaskIndex, uint8_t TaskValueIndex);

/********************************************************************************************\
   If RX and TX tied together, perform emergency reset to get the system out of boot loops
 \*********************************************************************************************/

void emergencyReset();


/********************************************************************************************\
   Delayed reboot, in case of issues, do not reboot with high frequency as it might not help...
 \*********************************************************************************************/
void delayedReboot(int rebootDelay, ESPEasy_Scheduler::IntendedRebootReason_e reason = ESPEasy_Scheduler::IntendedRebootReason_e::DelayedReboot);

void reboot(ESPEasy_Scheduler::IntendedRebootReason_e reason);

void FeedSW_watchdog();

void SendValueLogger(taskIndex_t TaskIndex);




// #######################################################################################################
// ############################ quite acurate but slow color converter####################################
// #######################################################################################################
// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white

void HSV2RGB(float H,
             float S,
             float I,
             int   rgb[3]);

// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white

void HSV2RGBW(float H,
              float S,
              float I,
              int   rgbw[4]);

// Simple bitwise get/set functions

uint8_t get8BitFromUL(uint32_t number,
                      byte     bitnr);

void    set8BitToUL(uint32_t& number,
                    byte      bitnr,
                    uint8_t   value);

uint8_t get4BitFromUL(uint32_t number,
                      byte     bitnr);

void    set4BitToUL(uint32_t& number,
                    byte      bitnr,
                    uint8_t   value);


float getCPUload();

int getLoopCountPerSec();

int getUptimeMinutes();

#ifndef BUILD_NO_RAM_TRACKER
void logMemUsageAfter(const __FlashStringHelper * function, int value = -1);
#endif


#endif // ifndef HELPERS_MISC_H
