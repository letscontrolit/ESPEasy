#ifndef HELPERS_MISC_H
#define HELPERS_MISC_H

#include <Arduino.h>

#include "../DataStructs/PinMode.h"
#include "../../ESPEasy-Globals.h"


#define HTML_SYMBOL_WARNING "&#9888;"
#define HTML_SYMBOL_INPUT   "&#8656;"
#define HTML_SYMBOL_OUTPUT  "&#8658;"
#define HTML_SYMBOL_I_O     "&#8660;"


#ifdef ESP32

// MFD: adding tone support here while waiting for the Arduino Espressif implementation to catch up
// As recomandation is not to use external libraries the following code was taken from: https://github.com/lbernstone/Tone Thanks
  # define TONE_CHANNEL 15

void noToneESP32(uint8_t pin,
                 uint8_t channel = TONE_CHANNEL);

void toneESP32(uint8_t       pin,
               unsigned int  frequency,
               unsigned long duration,
               uint8_t       channel = TONE_CHANNEL);

#endif // ifdef ESP32

/*********************************************************************************************\
   ESPEasy specific strings
\*********************************************************************************************/


String getNodeTypeDisplayString(byte nodeType);


#ifdef USES_MQTT
String getMQTT_state();
#endif // USES_MQTT

/********************************************************************************************\
   Get system information
 \*********************************************************************************************/
String getLastBootCauseString();

#ifdef ESP32

// See https://github.com/espressif/esp-idf/blob/master/components/esp32/include/rom/rtc.h
String  getResetReasonString(byte icore);
#endif // ifdef ESP32

String  getResetReasonString();

String  getSystemBuildString();

String  getPluginDescriptionString();

String  getSystemLibraryString();

#ifdef ESP8266
String  getLWIPversion();
#endif // ifdef ESP8266

bool    puyaSupport();

uint8_t getFlashChipVendorId();

bool    flashChipVendorPuya();


/*********************************************************************************************\
   Memory management
\*********************************************************************************************/


// For keeping track of 'cont' stack
// See: https://github.com/esp8266/Arduino/issues/2557
//      https://github.com/esp8266/Arduino/issues/5148#issuecomment-424329183
//      https://github.com/letscontrolit/ESPEasy/issues/1824
#ifdef ESP32

// FIXME TD-er: For ESP32 you need to provide the task number, or NULL to get from the calling task.
uint32_t getCurrentFreeStack();

uint32_t getFreeStackWatermark();

// FIXME TD-er: Must check if these functions are also needed for ESP32.
bool     canYield();

#else // ifdef ESP32

extern "C" {
# include <cont.h>
extern cont_t *g_pcont;
}

uint32_t getCurrentFreeStack();

uint32_t getFreeStackWatermark();

bool     canYield();

bool     allocatedOnStack(const void *address);

#endif // ESP32


bool remoteConfig(struct EventStruct *event,
                  const String      & string);

/*********************************************************************************************\
   Collect the stored preference for factory default
\*********************************************************************************************/
void applyFactoryDefaultPref();


/*********************************************************************************************\
   Device GPIO name functions to share flash strings
\*********************************************************************************************/
String formatGpioDirection(gpio_direction direction);

String formatGpioLabel(int  gpio,
                       bool includeWarning);

String formatGpioName(const String & label,
                      gpio_direction direction,
                      bool           optional);

String formatGpioName(const String & label,
                      gpio_direction direction);

String formatGpioName_input(const String& label);
String formatGpioName_output(const String& label);
String formatGpioName_bidirectional(const String& label);
String formatGpioName_input_optional(const String& label);

String formatGpioName_output_optional(const String& label);

// RX/TX are the only signals which are crossed, so they must be labelled like this:
// "GPIO <-- TX" and "GPIO --> RX"
String formatGpioName_TX(bool optional);

String formatGpioName_RX(bool optional);

String formatGpioName_TX_HW(bool optional);

String formatGpioName_RX_HW(bool optional);

#ifdef ESP32

String formatGpioName_ADC(int gpio_pin);

#endif // ifdef ESP32

String createGPIO_label(int  gpio,
                        int  pinnr,
                        bool input,
                        bool output,
                        bool warning);

/*********************************************************************************************\
   set pin mode & state (info table)
\*********************************************************************************************/
/*
   void setPinState(byte plugin, byte index, byte mode, uint16_t value);
 */

/*********************************************************************************************\
   get pin mode & state (info table)
\*********************************************************************************************/

/*
   bool getPinState(byte plugin, byte index, byte *mode, uint16_t *value);

 */
/*********************************************************************************************\
   check if pin mode & state is known (info table)
\*********************************************************************************************/
/*
   bool hasPinState(byte plugin, byte index);

 */


/*********************************************************************************************\
   report pin mode & state (info table) using json
\*********************************************************************************************/
String getPinStateJSON(bool          search,
                       uint32_t      key,
                       const String& log,
                       int16_t       noSearchValue);

String getPinModeString(byte mode);

#if defined(ESP32)
void   analogWriteESP32(int pin,
                        int value);
#endif // if defined(ESP32)


/********************************************************************************************\
   Status LED
 \*********************************************************************************************/
#define PWMRANGE_FULL 1023
#define STATUS_PWM_NORMALVALUE (PWMRANGE_FULL >> 2)
#define STATUS_PWM_NORMALFADE (PWMRANGE_FULL >> 8)
#define STATUS_PWM_TRAFFICRISE (PWMRANGE_FULL >> 1)

void statusLED(bool traffic);

/********************************************************************************************\
   delay in milliseconds with background processing
 \*********************************************************************************************/
void delayBackground(unsigned long dsdelay);


/********************************************************************************************\
   Check to see if a given argument is a valid taskIndex (argc = 0 => command)
 \*********************************************************************************************/
taskIndex_t parseCommandArgumentTaskIndex(const String& string,
                                          unsigned int  argc);


/********************************************************************************************\
   Get int from command argument (argc = 0 => command)
 \*********************************************************************************************/
int parseCommandArgumentInt(const String& string,
                            unsigned int  argc);

/********************************************************************************************\
   Parse a command string to event struct
 \*********************************************************************************************/
void parseCommandString(struct EventStruct *event,
                        const String      & string);


/********************************************************************************************\
   Toggle controller enabled state
 \*********************************************************************************************/
bool setControllerEnableStatus(controllerIndex_t controllerIndex,
                               bool              enabled);

/********************************************************************************************\
   Toggle task enabled state
 \*********************************************************************************************/
bool setTaskEnableStatus(taskIndex_t taskIndex,
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
   If RX and TX tied together, perform emergency reset to get the system out of boot loops
 \*********************************************************************************************/

void emergencyReset();


/********************************************************************************************\
   Get free system mem
 \*********************************************************************************************/
unsigned long FreeMem(void);

unsigned long getMaxFreeBlock();

void          logtimeStringToSeconds(const String& tBuf,
                                     int           hours,
                                     int           minutes,
                                     int           seconds);

// convert old and new time string to nr of seconds
// return whether it should be considered a time string.
bool timeStringToSeconds(const String& tBuf,
                         int         & time_seconds);


/********************************************************************************************\
   Delayed reboot, in case of issues, do not reboot with high frequency as it might not help...
 \*********************************************************************************************/
void   delayedReboot(int rebootDelay);

void   reboot();

/********************************************************************************************\
   Parse string template
 \*********************************************************************************************/
String parseTemplate(String& tmpString);

String parseTemplate(String& tmpString,
                     bool    useURLencode);

String parseTemplate_padded(String& tmpString,
                            byte    minimal_lineSize);

String parseTemplate_padded(String& tmpString,
                            byte    minimal_lineSize,
                            bool    useURLencode);

// Find the first (enabled) task with given name
// Return INVALID_TASK_INDEX when not found, else return taskIndex
taskIndex_t findTaskIndexByName(const String& deviceName);

// Find the first device value index of a taskIndex.
// Return VARS_PER_TASK if none found.
byte findDeviceValueIndexByName(const String& valueName,
                                taskIndex_t   taskIndex);

// Find positions of [...#...] in the given string.
// Only update pos values on success.
// Return true when found.
bool findNextValMarkInString(const String& input,
                             int         & startpos,
                             int         & hashpos,
                             int         & endpos);

// Find [deviceName#valueName] or [deviceName#valueName#format]
// DeviceName and valueName will be returned in lower case.
// Format may contain case sensitive formatting syntax.
bool findNextDevValNameInString(const String& input,
                                int         & startpos,
                                int         & endpos,
                                String      & deviceName,
                                String      & valueName,
                                String      & format);

/********************************************************************************************\
   Transform values
 \*********************************************************************************************/

// Syntax: [task#value#transformation#justification]
// valueFormat="transformation#justification"
void transformValue(
  String      & newString,
  byte          lineSize,
  String        value,
  String      & valueFormat,
  const String& tmpString);

/********************************************************************************************\
   Calculate function for simple expressions
 \*********************************************************************************************/
#define CALCULATE_OK                            0
#define CALCULATE_ERROR_STACK_OVERFLOW          1
#define CALCULATE_ERROR_BAD_OPERATOR            2
#define CALCULATE_ERROR_PARENTHESES_MISMATCHED  3
#define CALCULATE_ERROR_UNKNOWN_TOKEN           4
#define STACK_SIZE 10 // was 50
#define TOKEN_MAX 20

extern float  globalstack[STACK_SIZE];
extern float *sp;
extern float *sp_max;

int   push(float value);

float pop();

float apply_operator(char  op,
                     float first,
                     float second);

float apply_unary_operator(char  op,
                           float first);

char* next_token(char *linep);

int   RPNCalculate(char *token);

// operators
// precedence   operators         associativity
// 3            !                 right to left
// 2            * / %             left to right
// 1            + - ^             left to right
int          op_preced(const char c);

bool         op_left_assoc(const char c);

unsigned int op_arg_count(const char c);

int          Calculate(const char *input,
                       float      *result);

int          CalculateParam(const char *TmpStr);

void         SendValueLogger(taskIndex_t TaskIndex);


// #ifdef PLUGIN_BUILD_TESTING

// #define isdigit(n) (n >= '0' && n <= '9') //Conflicts with ArduJson 6+, when this lib is used there is no need for this macro

/********************************************************************************************\
   Generate a tone of specified frequency on pin
 \*********************************************************************************************/
void tone_espEasy(uint8_t       _pin,
                  unsigned int  frequency,
                  unsigned long duration);

/********************************************************************************************\
   Play RTTTL string on specified pin
 \*********************************************************************************************/
void play_rtttl(uint8_t     _pin,
                const char *p);

// #endif

bool OTA_possible(uint32_t& maxSketchSize,
                  bool    & use2step);

#ifdef FEATURE_ARDUINO_OTA

/********************************************************************************************\
   Allow updating via the Arduino OTA-protocol. (this allows you to upload directly from platformio)
 \*********************************************************************************************/

void ArduinoOTAInit();

#endif // ifdef FEATURE_ARDUINO_OTA


/**********************************************************
*                                                         *
* Helper Functions for managing the status data structure *
*                                                         *
**********************************************************/

void       savePortStatus(uint32_t                 key,
                          struct portStatusStruct& tempStatus);

bool       existPortStatus(uint32_t key);

void       removeTaskFromPort(uint32_t key);

void       removeMonitorFromPort(uint32_t key);

void       addMonitorToPort(uint32_t key);

uint32_t   createKey(uint16_t pluginNumber,
                     uint16_t portNumber);

pluginID_t getPluginFromKey(uint32_t key);

uint16_t   getPortFromKey(uint32_t key);

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


#endif // ifndef HELPERS_MISC_H
