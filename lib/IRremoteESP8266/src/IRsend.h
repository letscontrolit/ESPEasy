// Copyright 2009 Ken Shirriff
// Copyright 2015 Mark Szabo
// Copyright 2017 David Conran
#ifndef IRSEND_H_
#define IRSEND_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "IRremoteESP8266.h"

// Originally from https://github.com/shirriff/Arduino-IRremote/
// Updated by markszabo (https://github.com/crankyoldgit/IRremoteESP8266) for
// sending IR code on ESP8266

#if TEST || UNIT_TEST
#define VIRTUAL virtual
#else
#define VIRTUAL
#endif

// Constants
// Offset (in microseconds) to use in Period time calculations to account for
// code excution time in producing the software PWM signal.
#if defined(ESP32)
// Calculated on a generic ESP-WROOM-32 board with v3.2-18 SDK @ 240MHz
const int8_t kPeriodOffset = -2;
#elif (defined(ESP8266) && F_CPU == 160000000L)  // NOLINT(whitespace/parens)
// Calculated on an ESP8266 NodeMCU v2 board using:
// v2.6.0 with v2.5.2 ESP core @ 160MHz
const int8_t kPeriodOffset = -2;
#else  // (defined(ESP8266) && F_CPU == 160000000L)
// Calculated on ESP8266 Wemos D1 mini using v2.4.1 with v2.4.0 ESP core @ 40MHz
const int8_t kPeriodOffset = -5;
#endif  // (defined(ESP8266) && F_CPU == 160000000L)
const uint8_t kDutyDefault = 50;  // Percentage
const uint8_t kDutyMax = 100;     // Percentage
// delayMicroseconds() is only accurate to 16383us.
// Ref: https://www.arduino.cc/en/Reference/delayMicroseconds
const uint16_t kMaxAccurateUsecDelay = 16383;
//  Usecs to wait between messages we don't know the proper gap time.
const uint32_t kDefaultMessageGap = 100000;
/// Placeholder for missing sensor temp value
/// @note Not using "-1" as it may be a valid external temp
const float kNoTempValue = -100.0;

/// Enumerators and Structures for the Common A/C API.
namespace stdAc {
/// Common A/C settings for A/C operating modes.
enum class opmode_t {
  kOff  = -1,
  kAuto =  0,
  kCool =  1,
  kHeat =  2,
  kDry  =  3,
  kFan  =  4,
  // Add new entries before this one, and update it to point to the last entry
  kLastOpmodeEnum = kFan,
};

/// Common A/C settings for Fan Speeds.
enum class fanspeed_t {
  kAuto =       0,
  kMin =        1,
  kLow =        2,
  kMedium =     3,
  kHigh =       4,
  kMax =        5,
  kMediumHigh = 6,
  // Add new entries before this one, and update it to point to the last entry
  kLastFanspeedEnum = kMediumHigh,
};

/// Common A/C settings for Vertical Swing.
enum class swingv_t {
  kOff =    -1,
  kAuto =    0,
  kHighest = 1,
  kHigh =    2,
  kMiddle =  3,
  kLow =     4,
  kLowest =  5,
  kUpperMiddle = 6,
  // Add new entries before this one, and update it to point to the last entry
  kLastSwingvEnum = kUpperMiddle,
};

/// @brief Tyoe of A/C command (if the remote uses different codes for each)
/// @note Most remotes support only a single command or aggregate multiple
///       into one (e.g. control+timer). Use @c kControlCommand in such case
enum class ac_command_t {
  kControlCommand = 0,
  kSensorTempReport = 1,
  kTimerCommand = 2,
  kConfigCommand = 3,
  // Add new entries before this one, and update it to point to the last entry
  kLastAcCommandEnum = kConfigCommand,
};

/// Common A/C settings for Horizontal Swing.
enum class swingh_t {
  kOff =     -1,
  kAuto =     0,  // a.k.a. On.
  kLeftMax =  1,
  kLeft =     2,
  kMiddle =   3,
  kRight =    4,
  kRightMax = 5,
  kWide =     6,  // a.k.a. left & right at the same time.
  // Add new entries before this one, and update it to point to the last entry
  kLastSwinghEnum = kWide,
};

/// Structure to hold a common A/C state.
struct state_t {
  decode_type_t protocol = decode_type_t::UNKNOWN;
  int16_t model = -1;  // `-1` means unused.
  bool power = false;
  stdAc::opmode_t mode = stdAc::opmode_t::kOff;
  float degrees = 25;
  bool celsius = true;
  stdAc::fanspeed_t fanspeed = stdAc::fanspeed_t::kAuto;
  stdAc::swingv_t swingv = stdAc::swingv_t::kOff;
  stdAc::swingh_t swingh = stdAc::swingh_t::kOff;
  bool quiet = false;
  bool turbo = false;
  bool econo = false;
  bool light = false;
  bool filter = false;
  bool clean = false;
  bool beep = false;
  int16_t sleep = -1;  // `-1` means off.
  int16_t clock = -1;  // `-1` means not set.
  stdAc::ac_command_t command = stdAc::ac_command_t::kControlCommand;
  bool iFeel = false;
  float sensorTemperature = kNoTempValue;  // `kNoTempValue` means not set.
};
};  // namespace stdAc

/// Fujitsu A/C model numbers
enum fujitsu_ac_remote_model_t {
  ARRAH2E = 1,  ///< (1) AR-RAH2E, AR-RAC1E, AR-RAE1E, AR-RCE1E, AR-RAH2U,
                ///<     AR-REG1U (Default)
                ///< Warning: Use on incorrect models can cause the A/C to lock
                ///< up, requring the A/C to be physically powered off to fix.
                ///< e.g. AR-RAH1U may lock up with a Swing command.
  ARDB1,        ///< (2) AR-DB1, AR-DL10 (AR-DL10 swing doesn't work)
  ARREB1E,      ///< (3) AR-REB1E, AR-RAH1U (Similar to ARRAH2E but no horiz
                ///<     control)
  ARJW2,        ///< (4) AR-JW2  (Same as ARDB1 but with horiz control)
  ARRY4,        ///< (5) AR-RY4 (Same as AR-RAH2E but with clean & filter)
  ARREW4E,      ///< (6) Similar to ARRAH2E, but with different temp config.
};

/// Gree A/C model numbers
enum gree_ac_remote_model_t {
  YAW1F = 1,  // (1) Ultimate, EKOKAI, RusClimate (Default)
  YBOFB,      // (2) Green, YBOFB2, YAPOF3
  YX1FSF,     // (3) Soleus Air window unit (Similar to YAW1F, but with an
              //     Operation mode of Energy Saver (Econo))
};

/// HAIER_AC176 A/C model numbers
enum haier_ac176_remote_model_t {
  V9014557_A = 1,  // (1) V9014557 Remote in "A" setting. (Default)
  V9014557_B,      // (2) V9014557 Remote in "B" setting.
};

/// HITACHI_AC1 A/C model numbers
enum hitachi_ac1_remote_model_t {
  R_LT0541_HTA_A = 1,  // (1) R-LT0541-HTA Remote in "A" setting. (Default)
  R_LT0541_HTA_B,      // (2) R-LT0541-HTA Remote in "B" setting.
};

/// MIRAGE A/C model numbers
enum mirage_ac_remote_model_t {
  KKG9AC1 = 1,  // (1) KKG9A-C1 Remote. (Default)
  KKG29AC1,     // (2) KKG29A-C1 Remote.
};

/// Panasonic A/C model numbers
enum panasonic_ac_remote_model_t {
  kPanasonicUnknown = 0,
  kPanasonicLke = 1,
  kPanasonicNke = 2,
  kPanasonicDke = 3,  // PKR too.
  kPanasonicJke = 4,
  kPanasonicCkp = 5,
  kPanasonicRkr = 6,
};

/// Sharp A/C model numbers
enum sharp_ac_remote_model_t {
  A907 = 1,
  A705 = 2,
  A903 = 3,  // 820 too
};

/// TCL (& Teknopoint) A/C model numbers
enum tcl_ac_remote_model_t {
  TAC09CHSD = 1,
  GZ055BE1 = 2,  // Also Teknopoint GZ01-BEJ0-000
};

/// Voltas A/C model numbers
enum voltas_ac_remote_model_t {
  kVoltasUnknown = 0,  // Full Function
  kVoltas122LZF = 1,   // (1) 122LZF (No SwingH support) (Default)
};

/// Whirlpool A/C model numbers
enum whirlpool_ac_remote_model_t {
  DG11J13A = 1,  // DG11J1-04 too
  DG11J191,
};

/// LG A/C model numbers
enum lg_ac_remote_model_t {
  GE6711AR2853M = 1,  // (1) LG 28-bit Protocol (default)
  AKB75215403,        // (2) LG2 28-bit Protocol
  AKB74955603,        // (3) LG2 28-bit Protocol variant
  AKB73757604,        // (4) LG2 Variant of AKB74955603
  LG6711A20083V,      // (5) Same as GE6711AR2853M, but only SwingV toggle.
};

/// Argo A/C model numbers
enum argo_ac_remote_model_t {
  SAC_WREM2 = 1,   // (1) ARGO WREM2 remote (default)
  SAC_WREM3        // (2) ARGO WREM3 remote (touch buttons), bit-len vary by cmd
};

// Classes

/// Class for sending all basic IR protocols.
/// @note Originally from https://github.com/shirriff/Arduino-IRremote/
///  Updated by markszabo (https://github.com/crankyoldgit/IRremoteESP8266) for
///  sending IR code on ESP8266
class IRsend {
 public:
  explicit IRsend(uint16_t IRsendPin, bool inverted = false,
                  bool use_modulation = true);
  void begin();
  void enableIROut(uint32_t freq, uint8_t duty = kDutyDefault);
  VIRTUAL void _delayMicroseconds(uint32_t usec);
  VIRTUAL uint16_t mark(uint16_t usec);
  VIRTUAL void space(uint32_t usec);
  int8_t calibrate(uint16_t hz = 38000U);
  void sendRaw(const uint16_t buf[], const uint16_t len, const uint16_t hz);
  void sendData(uint16_t onemark, uint32_t onespace, uint16_t zeromark,
                uint32_t zerospace, uint64_t data, uint16_t nbits,
                bool MSBfirst = true);
  void sendManchesterData(const uint16_t half_period, const uint64_t data,
                          const uint16_t nbits, const bool MSBfirst = true,
                          const bool GEThomas = true);
  void sendManchester(const uint16_t headermark, const uint32_t headerspace,
                      const uint16_t half_period, const uint16_t footermark,
                      const uint32_t gap, const uint64_t data,
                      const uint16_t nbits, const uint16_t frequency = 38,
                      const bool MSBfirst = true,
                      const uint16_t repeat = kNoRepeat,
                      const uint8_t dutycycle = kDutyDefault,
                      const bool GEThomas = true);
  void sendGeneric(const uint16_t headermark, const uint32_t headerspace,
                   const uint16_t onemark, const uint32_t onespace,
                   const uint16_t zeromark, const uint32_t zerospace,
                   const uint16_t footermark, const uint32_t gap,
                   const uint64_t data, const uint16_t nbits,
                   const uint16_t frequency, const bool MSBfirst,
                   const uint16_t repeat, const uint8_t dutycycle);
  void sendGeneric(const uint16_t headermark, const uint32_t headerspace,
                   const uint16_t onemark, const uint32_t onespace,
                   const uint16_t zeromark, const uint32_t zerospace,
                   const uint16_t footermark, const uint32_t gap,
                   const uint32_t mesgtime, const uint64_t data,
                   const uint16_t nbits, const uint16_t frequency,
                   const bool MSBfirst, const uint16_t repeat,
                   const uint8_t dutycycle);
  void sendGeneric(const uint16_t headermark, const uint32_t headerspace,
                   const uint16_t onemark, const uint32_t onespace,
                   const uint16_t zeromark, const uint32_t zerospace,
                   const uint16_t footermark, const uint32_t gap,
                   const uint8_t *dataptr, const uint16_t nbytes,
                   const uint16_t frequency, const bool MSBfirst,
                   const uint16_t repeat, const uint8_t dutycycle);
  static uint16_t minRepeats(const decode_type_t protocol);
  static uint16_t defaultBits(const decode_type_t protocol);
  bool send(const decode_type_t type, const uint64_t data,
            const uint16_t nbits, const uint16_t repeat = kNoRepeat);
  bool send(const decode_type_t type, const uint8_t *state,
            const uint16_t nbytes);
#if (SEND_NEC || SEND_SHERWOOD || SEND_AIWA_RC_T501 || SEND_SANYO || \
     SEND_MIDEA24)
  void sendNEC(uint64_t data, uint16_t nbits = kNECBits,
               uint16_t repeat = kNoRepeat);
  uint32_t encodeNEC(uint16_t address, uint16_t command);
#endif
#if SEND_SONY
  // sendSony() should typically be called with repeat=2 as Sony devices
  // expect the code to be sent at least 3 times. (code + 2 repeats = 3 codes)
  // Legacy use of this procedure was to only send a single code so call it with
  // repeat=0 for backward compatibility. As of v2.0 it defaults to sending
  // a Sony command that will be accepted be a device.
  void sendSony(const uint64_t data, const uint16_t nbits = kSony20Bits,
                const uint16_t repeat = kSonyMinRepeat);
  void sendSony38(const uint64_t data, const uint16_t nbits = kSony20Bits,
                  const uint16_t repeat = kSonyMinRepeat + 1);
  uint32_t encodeSony(const uint16_t nbits, const uint16_t command,
                      const uint16_t address, const uint16_t extended = 0);
#endif  // SEND_SONY
#if SEND_SHERWOOD
  void sendSherwood(uint64_t data, uint16_t nbits = kSherwoodBits,
                    uint16_t repeat = kSherwoodMinRepeat);
#endif
  // `sendSAMSUNG()` is required by `sendLG()`
#if (SEND_SAMSUNG || SEND_LG)
  void sendSAMSUNG(const uint64_t data, const uint16_t nbits = kSamsungBits,
                   const uint16_t repeat = kNoRepeat);
  uint32_t encodeSAMSUNG(const uint8_t customer, const uint8_t command);
#endif  // (SEND_SAMSUNG || SEND_LG)
#if SEND_SAMSUNG36
  void sendSamsung36(const uint64_t data, const uint16_t nbits = kSamsung36Bits,
                     const uint16_t repeat = kNoRepeat);
#endif
#if SEND_SAMSUNG_AC
  void sendSamsungAC(const unsigned char data[],
                     const uint16_t nbytes = kSamsungAcStateLength,
                     const uint16_t repeat = kSamsungAcDefaultRepeat);
#endif
#if SEND_LG
  void sendLG(uint64_t data, uint16_t nbits = kLgBits,
              uint16_t repeat = kNoRepeat);
  void sendLG2(uint64_t data, uint16_t nbits = kLgBits,
               uint16_t repeat = kNoRepeat);
  uint32_t encodeLG(uint16_t address, uint16_t command);
#endif
#if (SEND_SHARP || SEND_DENON)
  uint32_t encodeSharp(const uint16_t address, const uint16_t command,
                       const uint16_t expansion = 1, const uint16_t check = 0,
                       const bool MSBfirst = false);
  void sendSharp(const uint16_t address, const uint16_t command,
                 const uint16_t nbits = kSharpBits,
                 const uint16_t repeat = kNoRepeat);
  void sendSharpRaw(const uint64_t data, const uint16_t nbits = kSharpBits,
                    const uint16_t repeat = kNoRepeat);
#endif
#if SEND_SHARP_AC
  void sendSharpAc(const unsigned char data[],
                   const uint16_t nbytes = kSharpAcStateLength,
                   const uint16_t repeat = kSharpAcDefaultRepeat);
#endif  // SEND_SHARP_AC
#if SEND_JVC
  void sendJVC(uint64_t data, uint16_t nbits = kJvcBits,
               uint16_t repeat = kNoRepeat);
  uint16_t encodeJVC(uint8_t address, uint8_t command);
#endif
#if SEND_DENON
  void sendDenon(uint64_t data, uint16_t nbits = kDenonBits,
                 uint16_t repeat = kNoRepeat);
#endif
#if SEND_SANYO
  uint64_t encodeSanyoLC7461(uint16_t address, uint8_t command);
  void sendSanyoLC7461(const uint64_t data,
                       const uint16_t nbits = kSanyoLC7461Bits,
                       const uint16_t repeat = kNoRepeat);
#endif
#if SEND_SANYO_AC
  void sendSanyoAc(const uint8_t *data,
                   const uint16_t nbytes = kSanyoAcStateLength,
                   const uint16_t repeat = kNoRepeat);
#endif  // SEND_SANYO_AC
#if SEND_SANYO_AC88
  void sendSanyoAc88(const uint8_t *data,
                     const uint16_t nbytes = kSanyoAc88StateLength,
                     const uint16_t repeat = kSanyoAc88MinRepeat);
#endif  // SEND_SANYO_AC88
#if SEND_SANYO_AC152
  void sendSanyoAc152(const uint8_t *data,
                     const uint16_t nbytes = kSanyoAc152StateLength,
                     const uint16_t repeat = kSanyoAc152MinRepeat);
#endif  // SEND_SANYO_AC152
#if SEND_DISH
  // sendDISH() should typically be called with repeat=3 as DISH devices
  // expect the code to be sent at least 4 times. (code + 3 repeats = 4 codes)
  // Legacy use of this procedure was only to send a single code
  // so use repeat=0 for backward compatibility.
  void sendDISH(uint64_t data, uint16_t nbits = kDishBits,
                uint16_t repeat = kDishMinRepeat);
#endif
#if (SEND_PANASONIC || SEND_DENON)
  void sendPanasonic64(const uint64_t data,
                       const uint16_t nbits = kPanasonicBits,
                       const uint16_t repeat = kNoRepeat);
  void sendPanasonic(const uint16_t address, const uint32_t data,
                     const uint16_t nbits = kPanasonicBits,
                     const uint16_t repeat = kNoRepeat);
  uint64_t encodePanasonic(const uint16_t manufacturer, const uint8_t device,
                           const uint8_t subdevice, const uint8_t function);
#endif
#if SEND_RC5
  void sendRC5(const uint64_t data, uint16_t nbits = kRC5XBits,
               const uint16_t repeat = kNoRepeat);
  uint16_t encodeRC5(const uint8_t address, const uint8_t command,
                     const bool key_released = false);
  uint16_t encodeRC5X(const uint8_t address, const uint8_t command,
                      const bool key_released = false);
  uint64_t toggleRC5(const uint64_t data);
#endif
#if SEND_RC6
  void sendRC6(const uint64_t data, const uint16_t nbits = kRC6Mode0Bits,
               const uint16_t repeat = kNoRepeat);
  uint64_t encodeRC6(const uint32_t address, const uint8_t command,
                     const uint16_t mode = kRC6Mode0Bits);
  uint64_t toggleRC6(const uint64_t data, const uint16_t nbits = kRC6Mode0Bits);
#endif
#if SEND_RCMM
  void sendRCMM(uint64_t data, uint16_t nbits = kRCMMBits,
                uint16_t repeat = kNoRepeat);
#endif
#if SEND_COOLIX
  void sendCOOLIX(const uint64_t data, const uint16_t nbits = kCoolixBits,
                  const uint16_t repeat = kCoolixDefaultRepeat);
#endif  // SEND_COOLIX
#if SEND_COOLIX48
  void sendCoolix48(const uint64_t data, const uint16_t nbits = kCoolix48Bits,
                    const uint16_t repeat = kCoolixDefaultRepeat);
#endif  // SEND_COOLIX48
#if SEND_WHYNTER
  void sendWhynter(const uint64_t data, const uint16_t nbits = kWhynterBits,
                   const uint16_t repeat = kNoRepeat);
#endif
#if SEND_MIRAGE
  void sendMirage(const unsigned char data[],
                  const uint16_t nbytes = kMirageStateLength,
                  const uint16_t repeat = kMirageMinRepeat);
#endif  // SEND_MIRAGE
#if SEND_MITSUBISHI
  void sendMitsubishi(uint64_t data, uint16_t nbits = kMitsubishiBits,
                      uint16_t repeat = kMitsubishiMinRepeat);
#endif
#if SEND_MITSUBISHI136
  void sendMitsubishi136(const unsigned char data[],
                         const uint16_t nbytes = kMitsubishi136StateLength,
                         const uint16_t repeat = kMitsubishi136MinRepeat);
#endif
#if SEND_MITSUBISHI112
  void sendMitsubishi112(const unsigned char data[],
                         const uint16_t nbytes = kMitsubishi112StateLength,
                         const uint16_t repeat = kMitsubishi112MinRepeat);
#endif
#if SEND_MITSUBISHI2
  void sendMitsubishi2(uint64_t data, uint16_t nbits = kMitsubishiBits,
                       uint16_t repeat = kMitsubishiMinRepeat);
#endif
#if SEND_MITSUBISHI_AC
  void sendMitsubishiAC(const unsigned char data[],
                        const uint16_t nbytes = kMitsubishiACStateLength,
                        const uint16_t repeat = kMitsubishiACMinRepeat);
#endif
#if SEND_MITSUBISHIHEAVY
  void sendMitsubishiHeavy88(
      const unsigned char data[],
      const uint16_t nbytes = kMitsubishiHeavy88StateLength,
      const uint16_t repeat = kMitsubishiHeavy88MinRepeat);
  void sendMitsubishiHeavy152(
      const unsigned char data[],
      const uint16_t nbytes = kMitsubishiHeavy152StateLength,
      const uint16_t repeat = kMitsubishiHeavy152MinRepeat);
#endif
#if SEND_FUJITSU_AC
  void sendFujitsuAC(const unsigned char data[], const uint16_t nbytes,
                     const uint16_t repeat = kFujitsuAcMinRepeat);
#endif
#if SEND_INAX
  void sendInax(const uint64_t data, const uint16_t nbits = kInaxBits,
                const uint16_t repeat = kInaxMinRepeat);
#endif  // SEND_INAX
#if SEND_GLOBALCACHE
  void sendGC(uint16_t buf[], uint16_t len);
#endif
#if SEND_KELVINATOR
  void sendKelvinator(const unsigned char data[],
                      const uint16_t nbytes = kKelvinatorStateLength,
                      const uint16_t repeat = kKelvinatorDefaultRepeat);
#endif
#if SEND_DAIKIN
  void sendDaikin(const unsigned char data[],
                  const uint16_t nbytes = kDaikinStateLength,
                  const uint16_t repeat = kDaikinDefaultRepeat);
#endif
#if SEND_DAIKIN64
  void sendDaikin64(const uint64_t data, const uint16_t nbits = kDaikin64Bits,
                    const uint16_t repeat = kDaikin64DefaultRepeat);
#endif  // SEND_DAIKIN64
#if SEND_DAIKIN128
  void sendDaikin128(const unsigned char data[],
                     const uint16_t nbytes = kDaikin128StateLength,
                     const uint16_t repeat = kDaikin128DefaultRepeat);
#endif  // SEND_DAIKIN128
#if SEND_DAIKIN152
  void sendDaikin152(const unsigned char data[],
                     const uint16_t nbytes = kDaikin152StateLength,
                     const uint16_t repeat = kDaikin152DefaultRepeat);
#endif  // SEND_DAIKIN152
#if SEND_DAIKIN160
  void sendDaikin160(const unsigned char data[],
                     const uint16_t nbytes = kDaikin160StateLength,
                     const uint16_t repeat = kDaikin160DefaultRepeat);
#endif  // SEND_DAIKIN160
#if SEND_DAIKIN176
  void sendDaikin176(const unsigned char data[],
                     const uint16_t nbytes = kDaikin176StateLength,
                     const uint16_t repeat = kDaikin176DefaultRepeat);
#endif  // SEND_DAIKIN176
#if SEND_DAIKIN2
  void sendDaikin2(const unsigned char data[],
                   const uint16_t nbytes = kDaikin2StateLength,
                   const uint16_t repeat = kDaikin2DefaultRepeat);
#endif
#if SEND_DAIKIN200
  void sendDaikin200(const unsigned char data[],
                     const uint16_t nbytes = kDaikin200StateLength,
                     const uint16_t repeat = kDaikin200DefaultRepeat);
#endif  // SEND_DAIKIN200
#if SEND_DAIKIN216
  void sendDaikin216(const unsigned char data[],
                     const uint16_t nbytes = kDaikin216StateLength,
                     const uint16_t repeat = kDaikin216DefaultRepeat);
#endif  // SEND_DAIKIN216
#if SEND_DAIKIN312
  void sendDaikin312(const unsigned char data[],
                     const uint16_t nbytes = kDaikin312StateLength,
                     const uint16_t repeat = kDaikin312DefaultRepeat);
#endif  // SEND_DAIKIN312
#if SEND_AIWA_RC_T501
  void sendAiwaRCT501(uint64_t data, uint16_t nbits = kAiwaRcT501Bits,
                      uint16_t repeat = kAiwaRcT501MinRepeats);
#endif
#if SEND_GREE
  void sendGree(const uint64_t data, const uint16_t nbits = kGreeBits,
                const uint16_t repeat = kGreeDefaultRepeat);
  void sendGree(const uint8_t data[], const uint16_t nbytes = kGreeStateLength,
                const uint16_t repeat = kGreeDefaultRepeat);
#endif
#if SEND_GOODWEATHER
  void sendGoodweather(const uint64_t data,
                       const uint16_t nbits = kGoodweatherBits,
                       const uint16_t repeat = kGoodweatherMinRepeat);
#endif  // SEND_GOODWEATHER
#if SEND_GORENJE
  void sendGorenje(const uint64_t data, const uint16_t nbits = kGorenjeBits,
                   const uint16_t repeat = kNoRepeat);
#endif  // SEND_GORENJE
#if SEND_PRONTO
  void sendPronto(uint16_t data[], uint16_t len, uint16_t repeat = kNoRepeat);
#endif
#if SEND_ARGO
  void sendArgo(const unsigned char data[],
                const uint16_t nbytes = kArgoStateLength,
                const uint16_t repeat = kArgoDefaultRepeat,
                bool sendFooter = false);
  void sendArgoWREM3(const unsigned char data[],
                const uint16_t nbytes = kArgoStateLength,
                const uint16_t repeat = kArgoDefaultRepeat);
#endif  // SEND_ARGO
#if SEND_TROTEC
  void sendTrotec(const unsigned char data[],
                  const uint16_t nbytes = kTrotecStateLength,
                  const uint16_t repeat = kTrotecDefaultRepeat);
#endif  // SEND_TROTEC
#if SEND_TROTEC_3550
  void sendTrotec3550(const unsigned char data[],
                      const uint16_t nbytes = kTrotecStateLength,
                      const uint16_t repeat = kTrotecDefaultRepeat);
#endif  // SEND_TROTEC_3550
#if SEND_NIKAI
  void sendNikai(uint64_t data, uint16_t nbits = kNikaiBits,
                 uint16_t repeat = kNoRepeat);
#endif
#if SEND_TOSHIBA_AC
  void sendToshibaAC(const uint8_t data[],
                     const uint16_t nbytes = kToshibaACStateLength,
                     const uint16_t repeat = kToshibaACMinRepeat);
#endif
#if SEND_MIDEA
  void sendMidea(uint64_t data, uint16_t nbits = kMideaBits,
                 uint16_t repeat = kMideaMinRepeat);
#endif  // SEND_MIDEA
#if SEND_MIDEA24
  void sendMidea24(const uint64_t data, const uint16_t nbits = kMidea24Bits,
                   const uint16_t repeat = kMidea24MinRepeat);
#endif  // SEND_MIDEA24
#if SEND_MAGIQUEST
  void sendMagiQuest(const uint64_t data, const uint16_t nbits = kMagiquestBits,
                     const uint16_t repeat = kNoRepeat);
  uint64_t encodeMagiQuest(const uint32_t wand_id, const uint16_t magnitude);
#endif
#if SEND_LASERTAG
  void sendLasertag(uint64_t data, uint16_t nbits = kLasertagBits,
                    uint16_t repeat = kLasertagMinRepeat);
#endif
#if SEND_CARRIER_AC
  void sendCarrierAC(uint64_t data, uint16_t nbits = kCarrierAcBits,
                     uint16_t repeat = kCarrierAcMinRepeat);
#endif
#if SEND_CARRIER_AC40
  void sendCarrierAC40(uint64_t data, uint16_t nbits = kCarrierAc40Bits,
                       uint16_t repeat = kCarrierAc40MinRepeat);
#endif
#if SEND_CARRIER_AC64
  void sendCarrierAC64(uint64_t data, uint16_t nbits = kCarrierAc64Bits,
                       uint16_t repeat = kCarrierAc64MinRepeat);
#endif
#if SEND_CARRIER_AC84
  void sendCarrierAC84(const uint8_t data[],
                       const uint16_t nbytes = kCarrierAc84StateLength,
                       const uint16_t repeat = kNoRepeat);
#endif  // SEND_CARRIER_AC84
#if SEND_CARRIER_AC128
  void sendCarrierAC128(const uint8_t data[],
                        uint16_t nbytes = kCarrierAc128StateLength,
                        uint16_t repeat = kCarrierAc128MinRepeat);
#endif  // SEND_CARRIER_AC128
#if (SEND_HAIER_AC || SEND_HAIER_AC_YRW02 || SEND_HAIER_AC176)
  void sendHaierAC(const unsigned char data[],
                   const uint16_t nbytes = kHaierACStateLength,
                   const uint16_t repeat = kHaierAcDefaultRepeat);
#endif  // (SEND_HAIER_AC || SEND_HAIER_AC_YRW02 || SEND_HAIER_AC176)
#if SEND_HAIER_AC_YRW02
  void sendHaierACYRW02(const unsigned char data[],
                        const uint16_t nbytes = kHaierACYRW02StateLength,
                        const uint16_t repeat = kHaierAcYrw02DefaultRepeat);
#endif  // SEND_HAIER_AC_YRW02
#if SEND_HAIER_AC160
  void sendHaierAC160(const unsigned char data[],
                      const uint16_t nbytes = kHaierAC160StateLength,
                      const uint16_t repeat = kHaierAc160DefaultRepeat);
#endif  // SEND_HAIER_AC160
#if SEND_HAIER_AC176
  void sendHaierAC176(const unsigned char data[],
                      const uint16_t nbytes = kHaierAC176StateLength,
                      const uint16_t repeat = kHaierAc176DefaultRepeat);
#endif  // SEND_HAIER_AC176
#if SEND_HITACHI_AC
  void sendHitachiAC(const unsigned char data[],
                     const uint16_t nbytes = kHitachiAcStateLength,
                     const uint16_t repeat = kHitachiAcDefaultRepeat);
#endif
#if SEND_HITACHI_AC1
  void sendHitachiAC1(const unsigned char data[],
                      const uint16_t nbytes = kHitachiAc1StateLength,
                      const uint16_t repeat = kHitachiAcDefaultRepeat);
#endif
#if SEND_HITACHI_AC2
  void sendHitachiAC2(const unsigned char data[],
                      const uint16_t nbytes = kHitachiAc2StateLength,
                      const uint16_t repeat = kHitachiAcDefaultRepeat);
#endif
#if SEND_HITACHI_AC3
  void sendHitachiAc3(const unsigned char data[],
                      const uint16_t nbytes,  // No default as there as so many
                                              // different sizes
                      const uint16_t repeat = kHitachiAcDefaultRepeat);
#endif  // SEND_HITACHI_AC3
#if SEND_HITACHI_AC264
  void sendHitachiAc264(const unsigned char data[],
                        const uint16_t nbytes = kHitachiAc264StateLength,
                        const uint16_t repeat = kHitachiAcDefaultRepeat);
#endif  // SEND_HITACHI_AC264
#if SEND_HITACHI_AC296
  void sendHitachiAc296(const unsigned char data[],
                        const uint16_t nbytes = kHitachiAc296StateLength,
                        const uint16_t repeat = kHitachiAcDefaultRepeat);
#endif  // SEND_HITACHI_AC296
#if SEND_HITACHI_AC344
  void sendHitachiAc344(const unsigned char data[],
                        const uint16_t nbytes = kHitachiAc344StateLength,
                        const uint16_t repeat = kHitachiAcDefaultRepeat);
#endif  // SEND_HITACHI_AC344
#if SEND_HITACHI_AC424
  void sendHitachiAc424(const unsigned char data[],
                        const uint16_t nbytes = kHitachiAc424StateLength,
                        const uint16_t repeat = kHitachiAcDefaultRepeat);
#endif  // SEND_HITACHI_AC424
#if SEND_GICABLE
  void sendGICable(uint64_t data, uint16_t nbits = kGicableBits,
                   uint16_t repeat = kGicableMinRepeat);
#endif
#if SEND_WHIRLPOOL_AC
  void sendWhirlpoolAC(const unsigned char data[],
                       const uint16_t nbytes = kWhirlpoolAcStateLength,
                       const uint16_t repeat = kWhirlpoolAcDefaultRepeat);
#endif
#if SEND_LUTRON
  void sendLutron(uint64_t data, uint16_t nbits = kLutronBits,
                  uint16_t repeat = kNoRepeat);
#endif
#if SEND_ELECTRA_AC
  void sendElectraAC(const unsigned char data[],
                     const uint16_t nbytes = kElectraAcStateLength,
                     const uint16_t repeat = kNoRepeat);
#endif
#if SEND_PANASONIC_AC
  void sendPanasonicAC(const unsigned char data[],
                       const uint16_t nbytes = kPanasonicAcStateLength,
                       const uint16_t repeat = kPanasonicAcDefaultRepeat);
#endif  // SEND_PANASONIC_AC
#if SEND_PANASONIC_AC32
  void sendPanasonicAC32(const uint64_t data,
                         const uint16_t nbits = kPanasonicAc32Bits,
                         const uint16_t repeat = kPanasonicAcDefaultRepeat);
#endif  // SEND_PANASONIC_AC32
#if SEND_PIONEER
  void sendPioneer(const uint64_t data, const uint16_t nbits = kPioneerBits,
                   const uint16_t repeat = kNoRepeat);
  uint64_t encodePioneer(uint16_t address, uint16_t command);
#endif
#if SEND_MWM
  void sendMWM(const unsigned char data[], const uint16_t nbytes,
               const uint16_t repeat = kNoRepeat);
#endif
#if SEND_VESTEL_AC
  void sendVestelAc(const uint64_t data, const uint16_t nbits = kVestelAcBits,
                    const uint16_t repeat = kNoRepeat);
#endif
#if SEND_TCL96AC
  void sendTcl96Ac(const unsigned char data[],
                    const uint16_t nbytes = kTcl96AcStateLength,
                    const uint16_t repeat = kTcl96AcDefaultRepeat);
#endif  // SEND_TCL96AC
#if SEND_TCL112AC
  void sendTcl112Ac(const unsigned char data[],
                    const uint16_t nbytes = kTcl112AcStateLength,
                    const uint16_t repeat = kTcl112AcDefaultRepeat);
#endif  // SEND_TCL112AC
#if SEND_TECO
  void sendTeco(const uint64_t data, const uint16_t nbits = kTecoBits,
                const uint16_t repeat = kNoRepeat);
#endif
#if SEND_LEGOPF
  void sendLegoPf(const uint64_t data, const uint16_t nbits = kLegoPfBits,
                  const uint16_t repeat = kLegoPfMinRepeat);
#endif
#if SEND_NEOCLIMA
  void sendNeoclima(const unsigned char data[],
                    const uint16_t nbytes = kNeoclimaStateLength,
                    const uint16_t repeat = kNeoclimaMinRepeat);
#endif  // SEND_NEOCLIMA
#if SEND_AMCOR
  void sendAmcor(const unsigned char data[],
                 const uint16_t nbytes = kAmcorStateLength,
                 const uint16_t repeat = kAmcorDefaultRepeat);
#endif  // SEND_AMCOR
#if SEND_EPSON
  void sendEpson(uint64_t data, uint16_t nbits = kEpsonBits,
                 uint16_t repeat = kEpsonMinRepeat);
#endif
#if SEND_SYMPHONY
  void sendSymphony(uint64_t data, uint16_t nbits = kSymphonyBits,
                    uint16_t repeat = kSymphonyDefaultRepeat);
#endif
#if SEND_AIRWELL
  void sendAirwell(uint64_t data, uint16_t nbits = kAirwellBits,
                   uint16_t repeat = kAirwellMinRepeats);
#endif
#if SEND_DELONGHI_AC
  void sendDelonghiAc(uint64_t data, uint16_t nbits = kDelonghiAcBits,
                      uint16_t repeat = kDelonghiAcDefaultRepeat);
#endif
#if SEND_DOSHISHA
  void sendDoshisha(const uint64_t data, uint16_t nbits = kDoshishaBits,
                    const uint16_t repeat = kNoRepeat);
  uint64_t encodeDoshisha(const uint8_t command, const uint8_t channel = 0);
#endif  // SEND_DOSHISHA
#if SEND_MULTIBRACKETS
  void sendMultibrackets(const uint64_t data,
                         const uint16_t nbits = kMultibracketsBits,
                         const uint16_t repeat = kMultibracketsDefaultRepeat);
#endif
#if SEND_TECHNIBEL_AC
  void sendTechnibelAc(uint64_t data, uint16_t nbits = kTechnibelAcBits,
                       uint16_t repeat = kTechnibelAcDefaultRepeat);
#endif
#if SEND_CORONA_AC
  void sendCoronaAc(const uint8_t data[],
                    const uint16_t nbytes = kCoronaAcStateLength,
                    const uint16_t repeat = kNoRepeat);
#endif  // SEND_CORONA_AC
#if SEND_ZEPEAL
  void sendZepeal(const uint64_t data,
                  const uint16_t nbits = kZepealBits,
                  const uint16_t repeat = kZepealMinRepeat);
#endif  // SEND_ZEPEAL
#if SEND_VOLTAS
  void sendVoltas(const unsigned char data[],
                  const uint16_t nbytes = kVoltasStateLength,
                  const uint16_t repeat = kNoRepeat);
#endif  // SEND_VOLTAS
#if SEND_METZ
  void sendMetz(const uint64_t data,
                const uint16_t nbits = kMetzBits,
                const uint16_t repeat = kMetzMinRepeat);
  static uint32_t encodeMetz(const uint8_t address, const uint8_t command,
                             const bool toggle = false);
#endif  // SEND_METZ
#if SEND_TRANSCOLD
  void sendTranscold(const uint64_t data, const uint16_t nbits = kTranscoldBits,
                     const uint16_t repeat = kTranscoldDefaultRepeat);
#endif  // SEND_TRANSCOLD
#if SEND_ELITESCREENS
  void sendElitescreens(const uint64_t data,
                        const uint16_t nbits = kEliteScreensBits,
                        const uint16_t repeat = kEliteScreensDefaultRepeat);
#endif  // SEND_ELITESCREENS
#if SEND_MILESTAG2
  // Since There 2 types of transmissions
  // (14bits for Shooting by default, you can set 24 bit for msg delivery)
  void sendMilestag2(const uint64_t data,
                     const uint16_t nbits = kMilesTag2ShotBits,
                     const uint16_t repeat = kMilesMinRepeat);
#endif  // SEND_MILESTAG2
#if SEND_ECOCLIM
  void sendEcoclim(const uint64_t data, const uint16_t nbits = kEcoclimBits,
                   const uint16_t repeat = kNoRepeat);
#endif  // SEND_ECOCLIM
#if SEND_XMP
  void sendXmp(const uint64_t data, const uint16_t nbits = kXmpBits,
               const uint16_t repeat = kNoRepeat);
#endif  // SEND_XMP
#if SEND_TRUMA
  void sendTruma(const uint64_t data, const uint16_t nbits = kTrumaBits,
                 const uint16_t repeat = kNoRepeat);
#endif  // SEND_TRUMA
#if SEND_TEKNOPOINT
  void sendTeknopoint(const unsigned char data[],
                      const uint16_t nbytes = kTeknopointStateLength,
                      const uint16_t repeat = kNoRepeat);
#endif  // SEND_TEKNOPOINT
#if SEND_KELON
  void sendKelon(const uint64_t data, const uint16_t nbits = kKelonBits,
                 const uint16_t repeat = kNoRepeat);
#endif  // SEND_KELON
#if SEND_KELON168
  void sendKelon168(const unsigned char data[],
                    const uint16_t nbytes = kKelon168StateLength,
                    const uint16_t repeat = kNoRepeat);
#endif  // SEND_KELON168
#if SEND_BOSE
  void sendBose(const uint64_t data, const uint16_t nbits = kBoseBits,
                const uint16_t repeat = kNoRepeat);
#endif  // SEND_BOSE
#if SEND_ARRIS
  void sendArris(const uint64_t data, const uint16_t nbits = kArrisBits,
                 const uint16_t repeat = kNoRepeat);
  static uint32_t toggleArrisRelease(const uint32_t data);
  static uint32_t encodeArris(const uint32_t command, const bool release);
#endif  // SEND_ARRIS
#if SEND_RHOSS
  void sendRhoss(const unsigned char data[],
                 const uint16_t nbytes = kRhossStateLength,
                 const uint16_t repeat = kRhossDefaultRepeat);
#endif  // SEND_RHOSS
#if SEND_AIRTON
  void sendAirton(const uint64_t data, const uint16_t nbits = kAirtonBits,
                  const uint16_t repeat = kAirtonDefaultRepeat);
#endif  // SEND_AIRTON
#if SEND_TOTO
  void sendToto(const uint64_t data, const uint16_t nbits = kTotoBits,
                const uint16_t repeat = kTotoDefaultRepeat);
#endif  // SEND_TOTO
#if SEND_CLIMABUTLER
  void sendClimaButler(const uint64_t data,
                       const uint16_t nbits = kClimaButlerBits,
                       const uint16_t repeat = kNoRepeat);
#endif  // SEND_CLIMABUTLER
#if SEND_BOSCH144
  void sendBosch144(const unsigned char data[],
                    const uint16_t nbytes = kBosch144StateLength,
                    const uint16_t repeat = kNoRepeat);
#endif  // SEND_BOSCH144
#if SEND_WOWWEE
  void sendWowwee(const uint64_t data, const uint16_t nbits = kWowweeBits,
                  const uint16_t repeat = kWowweeDefaultRepeat);
#endif  // SEND_WOWWEE
#if SEND_YORK
  void sendYork(const unsigned char data[],
                    const uint16_t nbytes = kYorkStateLength,
                    const uint16_t repeat = kNoRepeat);
#endif  // SEND_YORK

 protected:
#ifdef UNIT_TEST
#ifndef HIGH
#define HIGH 0x1
#endif
#ifndef LOW
#define LOW 0x0
#endif
#endif  // UNIT_TEST
  uint8_t outputOn;
  uint8_t outputOff;
  VIRTUAL void ledOff();
  VIRTUAL void ledOn();
#ifndef UNIT_TEST

 private:
#else
  uint32_t _freq_unittest;
#endif  // UNIT_TEST
  uint16_t onTimePeriod;
  uint16_t offTimePeriod;
  uint16_t IRpin;
  int8_t periodOffset;
  uint8_t _dutycycle;
  bool modulation;
  uint32_t calcUSecPeriod(uint32_t hz, bool use_offset = true);
#if SEND_SONY
  void _sendSony(const uint64_t data, const uint16_t nbits,
                 const uint16_t repeat, const uint16_t freq);
#endif  // SEND_SONY
};

#endif  // IRSEND_H_
