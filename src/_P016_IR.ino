#ifdef USES_P016
//#######################################################################################################
//#################################### Plugin 016: Input IR #############################################
//#######################################################################################################


#ifdef ESP8266  // Needed for precompile issues.
#include <IRremoteESP8266.h>
#endif

#include <IRrecv.h>
#include <IRutils.h>

#if DECODE_DAIKIN
#include <ir_Daikin.h>
#endif
#if DECODE_FUJITSU_AC
#include <ir_Fujitsu.h>
#endif
#if DECODE_GREE
#include <ir_Gree.h>
#endif
#if DECODE_HAIER_AC
#include <ir_Haier.h>
#endif
#if DECODE_KELVINATOR
#include <ir_Kelvinator.h>
#endif
#if DECODE_MIDEA
#include <ir_Midea.h>
#endif
#if DECODE_TOSHIBA_AC
#include <ir_Toshiba.h>
#endif

IRrecv *irReceiver;
decode_results results;

#define PLUGIN_016
#define PLUGIN_ID_016         16
#define PLUGIN_NAME_016       "Communication - TSOP4838"
#define PLUGIN_VALUENAME1_016 "IR"

// ==================== start of TUNEABLE PARAMETERS ====================
// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
#define CAPTURE_BUFFER_SIZE 1024

// P016_TIMEOUT is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
// This parameter is an interesting trade-off. The longer the timeout, the more
// complex a message it can capture. e.g. Some device protocols will send
// multiple message packets in quick succession, like Air Conditioner remotes.
// Air Coniditioner protocols often have a considerable gap (20-40+ms) between
// packets.
// The downside of a large timeout value is a lot of less complex protocols
// send multiple messages when the remote's button is held down. The gap between
// them is often also around 20+ms. This can result in the raw data be 2-3+
// times larger than needed as it has captured 2-3+ messages in a single
// capture. Setting a low timeout value can resolve this.
// So, choosing the best P016_TIMEOUT value for your use particular case is
// quite nuanced. Good luck and happy hunting.
// NOTE: Don't exceed MAX_P016_TIMEOUT_MS. Typically 130ms.
#if DECODE_AC
#define P016_TIMEOUT 50U  // Some A/C units have gaps in their protocols of ~40ms.
// e.g. Kelvinator
// A value this large may swallow repeats of some protocols
#else  // DECODE_AC
#define P016_TIMEOUT 15U  // Suits most messages, while not swallowing many repeats.
#endif  // DECODE_AC
// Alternatives:
// #define P016_TIMEOUT 90U  // Suits messages with big gaps like XMP-1 & some aircon
// units, but can accidentally swallow repeated messages
// in the rawData[] output.
// #define P016_TIMEOUT MAX_P016_TIMEOUT_MS  // This will set it to our currently allowed
// maximum. Values this high are problematic
// because it is roughly the typical boundary
// where most messages repeat.
// e.g. It will stop decoding a message and
//   start sending it to serial at precisely
//   the time when the next message is likely
//   to be transmitted, and may miss it.

// Set the smallest sized "UNKNOWN" message packets we actually care about.
// This value helps reduce the false-positive detection rate of IR background
// noise as real messages. The chances of background IR noise getting detected
// as a message increases with the length of the P016_TIMEOUT value. (See above)
// The downside of setting this message too large is you can miss some valid
// short messages for protocols that this library doesn't yet decode.
//
// Set higher if you get lots of random short UNKNOWN messages when nothing
// should be sending a message.
// Set lower if you are sure your setup is working, but it doesn't see messages
// from your device. (e.g. Other IR remotes work.)
// NOTE: Set this value very high to effectively turn off UNKNOWN detection.
#define MIN_UNKNOWN_SIZE 12
// ==================== end of TUNEABLE PARAMETERS ====================

boolean Plugin_016(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_016;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_LONG;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = true;
        Device[deviceCount].InverseLogicOption = true;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_016);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_016));
        break;
      }

    case PLUGIN_INIT:
      {
        int irPin = Settings.TaskDevicePin1[event->TaskIndex];
        if (irReceiver == 0 && irPin != -1)
        {
          Serial.println(F("IR Init"));
          irReceiver = new IRrecv(irPin, CAPTURE_BUFFER_SIZE, P016_TIMEOUT, true);
          irReceiver->setUnknownThreshold(MIN_UNKNOWN_SIZE); // Ignore messages with less than minimum on or off pulses.
          irReceiver->enableIRIn(); // Start the receiver
        }
        if (irReceiver != 0 && irPin == -1)
        {
          Serial.println(F("IR Removed"));
          irReceiver->disableIRIn();
          delete irReceiver;
          irReceiver = 0;
        }
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (irReceiver->decode(&results))
        {
          unsigned long IRcode = results.value;
          irReceiver->resume();
          UserVar[event->BaseVarIndex] = (IRcode & 0xFFFF);
          UserVar[event->BaseVarIndex + 1] = ((IRcode >> 16) & 0xFFFF);
          String log = "IR: ";
          if (results.overflow)
            log += F("WARNING: IR code is too big for buffer. This result shouldn't be trusted until this is resolved. Edit & increase CAPTURE_BUFFER_SIZE.\n");
          // Display the basic output of what we found.
          log += resultToHumanReadableBasic(&results);
          addLog(LOG_LEVEL_INFO, log);
          // Display any extra A/C info if we have it.
          // Display the human readable state of an A/C message if we can.
          log = "";
#if DECODE_DAIKIN
          if (results.decode_type == DAIKIN) {
            IRDaikinESP ac(0);
            ac.setRaw(results.state);
            log = ac.toString();
          }
#endif  // DECODE_DAIKIN
#if DECODE_FUJITSU_AC
          if (results.decode_type == FUJITSU_AC) {
            IRFujitsuAC ac(0);
            ac.setRaw(results.state, results.bits / 8);
            log = ac.toString();
          }
#endif  // DECODE_FUJITSU_AC
#if DECODE_KELVINATOR
          if (results.decode_type == KELVINATOR) {
            IRKelvinatorAC ac(0);
            ac.setRaw(results.state);
            log = ac.toString();
          }
#endif  // DECODE_KELVINATOR
#if DECODE_TOSHIBA_AC
          if (results.decode_type == TOSHIBA_AC) {
            IRToshibaAC ac(0);
            ac.setRaw(results.state);
            log = ac.toString();
          }
#endif  // DECODE_TOSHIBA_AC
#if DECODE_GREE
          if (results.decode_type == GREE) {
            IRGreeAC ac(0);
            ac.setRaw(results.state);
            log = ac.toString();
          }
#endif  // DECODE_GREE
#if DECODE_MIDEA
          if (results.decode_type == MIDEA) {
            IRMideaAC ac(0);
            ac.setRaw(results.value);  // Midea uses value instead of state.
            log = ac.toString();
          }
#endif  // DECODE_MIDEA
#if DECODE_HAIER_AC
          if (results.decode_type == HAIER_AC) {
            IRHaierAC ac(0);
            ac.setRaw(results.state);
            log = ac.toString();
          }
#endif  // DECODE_HAIER_AC
          // If we got a human-readable description of the message, display it.
          if (log != "") addLog(LOG_LEVEL_INFO, log);

          // Output RAW timing info of the result.
          //log += resultToTimingInfo(&results);  //not showing up nicely in the web log... Maybe send them to serial?
          // Output the results as source code
          //  log += resultToSourceCode(&results); //not showing up nicely in the web log... Maybe send them to serial?
          sendData(event);
        }
        success = true;
        break;
      }
  }
  return success;
}

#endif // USES_P016
