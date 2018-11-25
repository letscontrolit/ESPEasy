#ifdef USES_P016
//#######################################################################################################
//#################################### Plugin 016: Input IR #############################################
//#######################################################################################################


#ifdef ESP8266  // Needed for precompile issues.
#include <IRremoteESP8266.h>
#endif

#include <IRrecv.h>
#include <IRutils.h>


// The following are only needed for extended decoding of A/C Messages
#include <ir_Daikin.h>
#include <ir_Fujitsu.h>
#include <ir_Gree.h>
#include <ir_Haier.h>
#include <ir_Kelvinator.h>
#include <ir_Midea.h>
#include <ir_Mitsubishi.h>
#include <ir_Panasonic.h>
#include <ir_Samsung.h>
#include <ir_Toshiba.h>
#include <ir_Coolix.h>

#define PLUGIN_016
#define PLUGIN_ID_016         16
#define PLUGIN_NAME_016       "Communication - TSOP4838"
#define PLUGIN_VALUENAME1_016 "IR"

// A lot of the following code has been taken directly (with permission) from the IRrecvDumpV2.ino example code
// of the IRremoteESP8266 library. (https://github.com/markszabo/IRremoteESP8266)

// ==================== start of TUNEABLE PARAMETERS ====================
// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
const uint16_t kCaptureBufferSize = 1024;

// kTimeout is the Nr. of milli-Seconds of no-more-data before we consider a
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
// So, choosing the best kTimeout value for your use particular case is
// quite nuanced. Good luck and happy hunting.
// NOTE: Don't exceed kMaxTimeoutMs. Typically 130ms.
#if DECODE_AC
// Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
// A value this large may swallow repeats of some protocols
const uint8_t P016_TIMEOUT = 50;
#else   // DECODE_AC
// Suits most messages, while not swallowing many repeats.
const uint8_t P016_TIMEOUT = 15;
#endif  // DECODE_AC
// Alternatives:
// const uint8_t kTimeout = 90;
// Suits messages with big gaps like XMP-1 & some aircon units, but can
// accidentally swallow repeated messages in the rawData[] output.
//
// const uint8_t kTimeout = kMaxTimeoutMs;
// This will set it to our currently allowed maximum.
// Values this high are problematic because it is roughly the typical boundary
// where most messages repeat.
// e.g. It will stop decoding a message and start sending it to serial at
//      precisely the time when the next message is likely to be transmitted,
//      and may miss it.

// Set the smallest sized "UNKNOWN" message packets we actually care about.
// This value helps reduce the false-positive detection rate of IR background
// noise as real messages. The chances of background IR noise getting detected
// as a message increases with the length of the kTimeout value. (See above)
// The downside of setting this message too large is you can miss some valid
// short messages for protocols that this library doesn't yet decode.
//
// Set higher if you get lots of random short UNKNOWN messages when nothing
// should be sending a message.
// Set lower if you are sure your setup is working, but it doesn't see messages
// from your device. (e.g. Other IR remotes work.)
// NOTE: Set this value very high to effectively turn off UNKNOWN detection.
const uint16_t kMinUnknownSize = 12;
// ==================== end of TUNEABLE PARAMETERS ====================


IRrecv *irReceiver = NULL;
decode_results results;

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

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_input(F("IR"));
        break;
      }

    case PLUGIN_INIT:
      {
        int irPin = Settings.TaskDevicePin1[event->TaskIndex];
        if (irReceiver == 0 && irPin != -1)
        {
          serialPrintln(F("IR Init"));
          irReceiver = new IRrecv(irPin, kCaptureBufferSize, P016_TIMEOUT, true);
          irReceiver->setUnknownThreshold(kMinUnknownSize); // Ignore messages with less than minimum on or off pulses.
          irReceiver->enableIRIn(); // Start the receiver
        }
        if (irReceiver != 0 && irPin == -1)
        {
          irReceiver->disableIRIn();
          delete irReceiver;
          irReceiver = 0;
        }
        success = true;
        break;
      }
          case PLUGIN_EXIT:
      {
        {
          if (irReceiver != 0)
          { 
          irReceiver->disableIRIn(); // Stop the receiver
          delete irReceiver;
          irReceiver = 0;
          }
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
          String description = "IR: ";
          if (results.overflow)
            description += F("WARNING: IR code is too big for buffer. This result shouldn't be trusted until this is resolved. Edit & increase CAPTURE_BUFFER_SIZE.\n");
          // Display the basic output of what we found.
          description += resultToHumanReadableBasic(&results);
          addLog(LOG_LEVEL_INFO, description);
          displayRawToReadableB32Hex();

          // Display any extra A/C info if we have it.
          // Display the human readable state of an A/C message if we can.
          description = "";
#if DECODE_DAIKIN
  if (results.decode_type == DAIKIN) {
    IRDaikinESP ac(0);
    ac.setRaw(results.state);
    description = ac.toString();
  }
#endif  // DECODE_DAIKIN
#if DECODE_FUJITSU_AC
  if (results.decode_type == FUJITSU_AC) {
    IRFujitsuAC ac(0);
    ac.setRaw(results.state, results.bits / 8);
    description = ac.toString();
  }
#endif  // DECODE_FUJITSU_AC
#if DECODE_KELVINATOR
  if (results.decode_type == KELVINATOR) {
    IRKelvinatorAC ac(0);
    ac.setRaw(results.state);
    description = ac.toString();
  }
#endif  // DECODE_KELVINATOR
#if DECODE_MITSUBISHI_AC
  if (results.decode_type == MITSUBISHI_AC) {
    IRMitsubishiAC ac(0);
    ac.setRaw(results.state);
    description = ac.toString();
  }
#endif  // DECODE_MITSUBISHI_AC
#if DECODE_TOSHIBA_AC
  if (results.decode_type == TOSHIBA_AC) {
    IRToshibaAC ac(0);
    ac.setRaw(results.state);
    description = ac.toString();
  }
#endif  // DECODE_TOSHIBA_AC
#if DECODE_GREE
  if (results.decode_type == GREE) {
    IRGreeAC ac(0);
    ac.setRaw(results.state);
    description = ac.toString();
  }
#endif  // DECODE_GREE
#if DECODE_MIDEA
  if (results.decode_type == MIDEA) {
    IRMideaAC ac(0);
    ac.setRaw(results.value);  // Midea uses value instead of state.
    description = ac.toString();
  }
#endif  // DECODE_MIDEA
#if DECODE_HAIER_AC
  if (results.decode_type == HAIER_AC) {
    IRHaierAC ac(0);
    ac.setRaw(results.state);
    description = ac.toString();
  }
#endif  // DECODE_HAIER_AC
#if DECODE_HAIER_AC_YRW02
  if (results.decode_type == HAIER_AC_YRW02) {
    IRHaierACYRW02 ac(0);
    ac.setRaw(results.state);
    description = ac.toString();
  }
#endif  // DECODE_HAIER_AC_YRW02
#if DECODE_SAMSUNG_AC
  if (results.decode_type == SAMSUNG_AC) {
    IRSamsungAc ac(0);
    ac.setRaw(results.state);
    description = ac.toString();
  }
#endif  // DECODE_SAMSUNG_AC
#if DECODE_COOLIX
  if (results.decode_type == COOLIX) {
    IRCoolixAC ac(0);
    ac.setRaw(results.value);  // Coolix uses value instead of state.
    description = ac.toString();
  }
#endif  // DECODE_COOLIX
#if DECODE_PANASONIC_AC
  if (results.decode_type == PANASONIC_AC &&
      results.bits > kPanasonicAcShortBits) {
    IRPanasonicAc ac(0);
    ac.setRaw(results.state);
    description = ac.toString();
  }
#endif  // DECODE_PANASONIC_AC
  // If we got a human-readable description of the message, display it.
          if (description != "") addLog(LOG_LEVEL_INFO, description);

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


#define PCT_TOLERANCE       7u
#define pct_tolerance(v)    ((v) / (100u / PCT_TOLERANCE))
//#define MIN_TOLERANCE       10u
//#define get_tolerance(v)    (pct_tolerance(v) > MIN_TOLERANCE? pct_tolerance(v) : MIN_TOLERANCE)
#define get_tolerance(v)    (pct_tolerance(v))
#define MIN_VIABLE_DIV      40u
#define to_32hex(c)         ((c) < 10 ? (c) + '0' : (c) + 'A' - 10)

// This function attempts to convert the raw IR timings buffer to a short string that can be sent over as
// an IRSEND HTTP/MQTT command. It analyzes the timings, and searches for a common denominator which can be
// used to compress the values. If found, it then produces a string consisting of B32 Hex digit for each
// timing value, appended by the denominators for Pulse and Blank. This string can then be used in an
// IRSEND command. An important advantage of this string over the current IRSEND RAW B32 format implemented
// by GusPS is that it allows easy inspections and modifications after the code is constructed.
//
// Author: Gilad Raz (jazzgil)  23sep2018

void displayRawToReadableB32Hex() {
    String line;
    uint16_t div[2];

    // print the values: either pulses or blanks
    for (uint16_t i = 1; i < results.rawlen; i++)
        line += uint64ToString(results.rawbuf[i] * RAWTICK, 10) + ",";
    addLog(LOG_LEVEL_DEBUG, line);

    // Find a common denominator divisor for odd indexes (pulses) and then even indexes (blanks).
    for (uint16_t p = 0; p < 2; p++) {
        uint16_t cd = 0xFFFFU;      // current divisor
        // find the lowest value to start the divisor with.
        for (uint16_t i = 1 + p; i < results.rawlen; i += 2) {
            uint16_t val = results.rawbuf[i] * RAWTICK;
            if (cd > val) cd = val;
        }

        uint16_t bstDiv = -1, bstAvg = 0xFFFFU;
        float bstMul = 5000;
        cd += get_tolerance(cd) + 1;
        //serialPrintln(String("p="+ uint64ToString(p, 10) + " start cd=" + uint64ToString(cd, 10)).c_str());
        // find the best divisor based on lowest avg err, within allowed tolerance.
        while (--cd >= MIN_VIABLE_DIV) {
            uint32_t avg = 0;
            uint16_t totTms = 0;
            // calculate average error for current divisor, and verify it's within tolerance for all timings.
            for (uint16_t i = 1 + p; i < results.rawlen; i += 2) {
                uint16_t val = results.rawbuf[i] * RAWTICK;
                uint16_t rmdr = val >= cd ? val % cd : cd - val;
                if (rmdr > get_tolerance(val)) { avg = 0xFFFFU; break; }
                avg += rmdr;
                totTms += val / cd + (cd > val? 1 : 0);
            }
            if (avg == 0xFFFFU) continue;
            avg /= results.rawlen / 2;
            float avgTms = (float)totTms / (results.rawlen / 2);
            if (avgTms <= bstMul && avg < bstAvg) {
                bstMul = avgTms;
                bstAvg = avg;
                bstDiv = cd;
                //serialPrintln(String("p="+ uint64ToString(p, 10) + " cd=" + uint64ToString(cd, 10) +"  avgErr=" + uint64ToString(avg, 10) + " totTms="+ uint64ToString(totTms, 10) + " avgTms="+ uint64ToString((uint16_t)(avgTms*10), 10) ).c_str());
            }
        }
        if (bstDiv == 0xFFFFU) {
            addLog(LOG_LEVEL_INFO, F("IR2:No proper divisor found. Try again..."));
            return;
        }
        div[p] = bstDiv;

        line = String(p? "Blank: " : "Pulse: ") + " divisor=" + uint64ToString(bstDiv, 10)
            +"  avgErr=" + uint64ToString(bstAvg, 10) + " avgMul="+ uint64ToString((uint16_t)bstMul, 10)
            +'.'+ ((char)((bstMul - (uint16_t)bstMul) * 10 )+ '0');
        addLog(LOG_LEVEL_DEBUG, line);
    }

    // Generate the B32 Hex string, per the divisors found.
    uint16_t total = results.rawlen - 1, tmOut[total];
    //line = "Timing muls ("+ uint64ToString(total, 10) + "): ";

    for (unsigned int i = 0; i < total; i++) {
        uint16_t val = results.rawbuf[i+1] * RAWTICK;
        unsigned int dv = div[(i) & 1];
        unsigned int tm = val / dv + (val % dv > dv / 2? 1 : 0);
        tmOut[i] = tm;
        //line += uint64ToString(tm, 10) + ",";
    }
    //serialPrintln(line);

    char out[total];
    unsigned int iOut = 0, s = 2, d = 0;
    for (; s+1 < total; d = s, s += 2) {
        unsigned int vals = 2;
        while (s+1 < total && tmOut[s] == tmOut[d] && tmOut[s+1] == tmOut[d+1]) {
            vals += 2;
            s += 2;
        }
        if (iOut + 5 > sizeof(out) || tmOut[d] >= 32*32 || tmOut[d+1] >= 32*32 || vals >= 64) {
            addLog(LOG_LEVEL_INFO, F("IR2: Raw code too long. Try again..."));
            return;
        }

        if (vals > 4 || (vals == 4 && (tmOut[d] >= 32 || tmOut[d+1] >= 32))) {
            out[iOut++] = '*';
            out[iOut++] = to_32hex(vals / 2);
            vals = 2;
        }
        while (vals--)
            iOut = storeB32Hex(out, iOut, tmOut[d++]);
    }
    while (d < total)
        iOut = storeB32Hex(out, iOut, tmOut[d++]);

    out[iOut] = 0;
    line = "IRSEND,RAW2," + String(out) + ",38," + uint64ToString(div[0], 10) +','+ uint64ToString(div[1], 10);
    addLog(LOG_LEVEL_INFO, line);
}

unsigned int storeB32Hex(char out[], unsigned int iOut, unsigned int val) {
    if (val >= 32) {
        out[iOut++] = '^';
        out[iOut++] = to_32hex(val/32);
        val %= 32;
    }
    out[iOut++] = to_32hex(val);
    return iOut;
}

#endif // USES_P016
