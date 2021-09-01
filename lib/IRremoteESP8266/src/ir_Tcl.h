// Copyright 2019 David Conran

/// @file
/// @brief Support for TCL protocols.

// Supports:
//   Brand: Leberg,  Model: LBS-TOR07 A/C

#ifndef IR_TCL_H_
#define IR_TCL_H_

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRrecv.h"
#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif

/// Native representation of a TCL 112 A/C message.
union Tcl112Protocol{
  uint8_t raw[kTcl112AcStateLength];  ///< The State in IR code form.
  struct {
    // Byte 0~4
    uint8_t pad0[5];
    // Byte 5
    uint8_t       :2;
    uint8_t Power :1;
    uint8_t       :3;
    uint8_t Light :1;
    uint8_t Econo :1;
    // Byte 6
    uint8_t Mode    :4;
    uint8_t Health  :1;
    uint8_t Turbo   :1;
    uint8_t         :2;
    // Byte 7
    uint8_t Temp  :4;
    uint8_t       :4;
    // Byte 8
    uint8_t Fan     :3;
    uint8_t SwingV  :3;
    uint8_t         :2;
    // Byte 9~11
    uint8_t pad1[3];
    // Byte 12
    uint8_t             :3;
    uint8_t SwingH      :1;
    uint8_t             :1;
    uint8_t HalfDegree  :1;
    uint8_t             :2;
    // Byte 13
    uint8_t Sum :8;
  };
};

// Constants
const uint16_t kTcl112AcHdrMark = 3000;
const uint16_t kTcl112AcHdrSpace = 1650;
const uint16_t kTcl112AcBitMark = 500;
const uint16_t kTcl112AcOneSpace = 1050;
const uint16_t kTcl112AcZeroSpace = 325;
const uint32_t kTcl112AcGap = kDefaultMessageGap;  // Just a guess.
// Total tolerance percentage to use for matching the header mark.
const uint8_t kTcl112AcHdrMarkTolerance = 6;
const uint8_t kTcl112AcTolerance = 5;  // Extra Percentage for the rest.

const uint8_t kTcl112AcHeat = 1;
const uint8_t kTcl112AcDry =  2;
const uint8_t kTcl112AcCool = 3;
const uint8_t kTcl112AcFan =  7;
const uint8_t kTcl112AcAuto = 8;

const uint8_t kTcl112AcFanAuto = 0b000;
const uint8_t kTcl112AcFanLow  = 0b010;
const uint8_t kTcl112AcFanMed  = 0b011;
const uint8_t kTcl112AcFanHigh = 0b101;

const float   kTcl112AcTempMax    = 31.0;
const float   kTcl112AcTempMin    = 16.0;

const uint8_t kTcl112AcSwingVOn =    0b111;
const uint8_t kTcl112AcSwingVOff =   0b000;

// Classes
/// Class for handling detailed TCL A/C messages.
class IRTcl112Ac {
 public:
  explicit IRTcl112Ac(const uint16_t pin, const bool inverted = false,
                      const bool use_modulation = true);
#if SEND_TCL112AC
  void send(const uint16_t repeat = kTcl112AcDefaultRepeat);
  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_TCL
  void begin(void);
  void stateReset(void);
  uint8_t* getRaw(void);
  void setRaw(const uint8_t new_code[],
              const uint16_t length = kTcl112AcStateLength);
  void on(void);
  void off(void);
  void setPower(const bool on);
  bool getPower(void) const;
  void setTemp(const float celsius);  // Celsius in 0.5 increments
  float getTemp(void) const;
  void setMode(const uint8_t mode);
  uint8_t getMode(void) const;
  static uint8_t calcChecksum(uint8_t state[],
                              const uint16_t length = kTcl112AcStateLength);
  static bool validChecksum(uint8_t state[],
                            const uint16_t length = kTcl112AcStateLength);
  void setFan(const uint8_t speed);
  uint8_t getFan(void) const;
  void setEcono(const bool on);
  bool getEcono(void) const;
  void setHealth(const bool on);
  bool getHealth(void) const;
  void setLight(const bool on);
  bool getLight(void) const;
  void setSwingHorizontal(const bool on);
  bool getSwingHorizontal(void) const;
  void setSwingVertical(const bool on);
  bool getSwingVertical(void) const;
  void setTurbo(const bool on);
  bool getTurbo(void) const;
  static uint8_t convertMode(const stdAc::opmode_t mode);
  static uint8_t convertFan(const stdAc::fanspeed_t speed);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  stdAc::state_t toCommon(void) const;
  String toString(void) const;
#ifndef UNIT_TEST

 private:
  IRsend _irsend;  ///< Instance of the IR send class
#else  // UNIT_TEST
  /// @cond IGNORE
  IRsendTest _irsend;  ///< Instance of the testing IR send class
  /// @endcond
#endif  // UNIT_TEST
  Tcl112Protocol _;
  void checksum(const uint16_t length = kTcl112AcStateLength);
};

#endif  // IR_TCL_H_
