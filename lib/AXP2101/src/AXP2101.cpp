#include "AXP2101.h"

#ifdef ESP32

// To check if we have implemented all cases of the enums
# pragma GCC diagnostic push
# pragma GCC diagnostic warning "-Wswitch-enum"

/**
 * Utility functions
 */
const __FlashStringHelper* toString(AXP2101_device_model_e device,
                                    bool                   displayString) {
  switch (device) {
    case AXP2101_device_model_e::Unselected: return displayString ? F("Select an option to set default values") : F("Unselected");
    case AXP2101_device_model_e::M5Stack_Core2_v1_1: return displayString ? F("M5Stack Core2 v1.1") : F("M5Core2v11");
    case AXP2101_device_model_e::M5Stack_CoreS3: return displayString ? F("M5Stack CoreS3") : F("M5CoreS3");
    case AXP2101_device_model_e::LilyGO_TBeam_v1_2: return displayString ? F("LilyGo TBeam v1.2") : F("TBeamv12");
    case AXP2101_device_model_e::LilyGO_TBeamS3_v3: return displayString ? F("LilyGo TBeam S3 v3") : F("TBeamS3v3");
    case AXP2101_device_model_e::LilyGO_TPCie_v1_2: return displayString ? F("LilyGo TPCie v1.2") : F("TPCiev12");
    case AXP2101_device_model_e::UserDefined: return displayString ? F("User defined") : F("Userdefined");
    case AXP2101_device_model_e::MAX: break;
  }
  return F("");
}

const __FlashStringHelper* toString(AXP2101_registers_e reg,
                                    bool                displayString) {
  switch (reg) {
    case AXP2101_registers_e::dcdc1: return displayString ? F("DCDC1") : F("dcdc1");
    case AXP2101_registers_e::dcdc2: return displayString ? F("DCDC2") : F("dcdc2");
    case AXP2101_registers_e::dcdc3: return displayString ? F("DCDC3") : F("dcdc3");
    case AXP2101_registers_e::dcdc4: return displayString ? F("DCDC4") : F("dcdc4");
    case AXP2101_registers_e::dcdc5: return displayString ? F("DCDC5") : F("dcdc5");
    case AXP2101_registers_e::aldo1: return displayString ? F("ALDO1") : F("aldo1");
    case AXP2101_registers_e::aldo2: return displayString ? F("ALDO2") : F("aldo2");
    case AXP2101_registers_e::aldo3: return displayString ? F("ALDO3") : F("aldo3");
    case AXP2101_registers_e::aldo4: return displayString ? F("ALDO4") : F("aldo4");
    case AXP2101_registers_e::bldo1: return displayString ? F("BLDO1") : F("bldo1");
    case AXP2101_registers_e::bldo2: return displayString ? F("BLDO2") : F("bldo2");
    case AXP2101_registers_e::dldo1: return displayString ? F("DLDO1") : F("dldo1");
    case AXP2101_registers_e::dldo2: return displayString ? F("DLDO2") : F("dldo2");
    case AXP2101_registers_e::cpuldos: return displayString ? F("CPULDOS") : F("cpuldos");
    case AXP2101_registers_e::chargeled: return displayString ? F("ChargeLed") : F("chargeled");
    case AXP2101_registers_e::batcharge: return displayString ? F("BatCharge") : F("batcharge");
    case AXP2101_registers_e::charging: return displayString ? F("ChargingState") : F("chargingstate");
    case AXP2101_registers_e::batpresent: return displayString ? F("BatPresent") : F("batpresent");
  }
  return F("");
}

const __FlashStringHelper* toString(AXP_pin_s pin) {
  switch (pin) {
    case AXP_pin_s::Off: return F("Off");
    case AXP_pin_s::On: return F("On");
    case AXP_pin_s::Default: return F("Default");
    case AXP_pin_s::Disabled: return F("Disabled");
    case AXP_pin_s::Protected: return F("Protected");
  }
  return F("");
}

const __FlashStringHelper* toString(AXP2101_chargeled_d led) {
  switch (led) {
    case AXP2101_chargeled_d::Off: return F("Off");
    case AXP2101_chargeled_d::Flash_1Hz: return F("Flash 1Hz");
    case AXP2101_chargeled_d::Flash_4Hz: return F("Flash 4Hz");
    case AXP2101_chargeled_d::Steady_On: return F("Steady On");
    case AXP2101_chargeled_d::Protected: return F("Protected");
  }
  return F("");
}

const __FlashStringHelper* toString(AXP2101_chargingState_e state) {
  switch (state) {
    case AXP2101_chargingState_e::Discharging: return F("Discharging");
    case AXP2101_chargingState_e::Standby: return F("Standby");
    case AXP2101_chargingState_e::Charging: return F("Charging");
  }
  return F("");
}

AXP2101_registers_e AXP2101_intToRegister(int reg) {
  switch (reg) {
    case 0: return AXP2101_registers_e::dcdc1;
    case 1: return AXP2101_registers_e::dcdc2;
    case 2: return AXP2101_registers_e::dcdc3;
    case 3: return AXP2101_registers_e::dcdc4;
    case 4: return AXP2101_registers_e::dcdc5;
    case 5: return AXP2101_registers_e::aldo1;
    case 6: return AXP2101_registers_e::aldo2;
    case 7: return AXP2101_registers_e::aldo3;
    case 8: return AXP2101_registers_e::aldo4;
    case 9: return AXP2101_registers_e::bldo1;
    case 10: return AXP2101_registers_e::bldo2;
    case 11: return AXP2101_registers_e::dldo1;
    case 12: return AXP2101_registers_e::dldo2;
    case 13: return AXP2101_registers_e::cpuldos;
    case 14: return AXP2101_registers_e::chargeled;
    case 15: return AXP2101_registers_e::batcharge;
    case 16: return AXP2101_registers_e::charging;
    case 17: return AXP2101_registers_e::batpresent;
  }
  return AXP2101_registers_e::dcdc1; // we shouldn't get here, just defaulting to the first value
}

uint16_t AXP2101_minVoltage(AXP2101_registers_e reg) {
  switch (reg) {
    case AXP2101_registers_e::dcdc1: return AXP2101_DCDC1_MIN;
    case AXP2101_registers_e::dcdc5: return AXP2101_DCDC5_MIN;
    case AXP2101_registers_e::dcdc2:
    case AXP2101_registers_e::dcdc3:
    case AXP2101_registers_e::dcdc4:
    case AXP2101_registers_e::aldo1:
    case AXP2101_registers_e::aldo2:
    case AXP2101_registers_e::aldo3:
    case AXP2101_registers_e::aldo4:
    case AXP2101_registers_e::bldo1:
    case AXP2101_registers_e::bldo2:
    case AXP2101_registers_e::dldo1:
    case AXP2101_registers_e::dldo2:
    case AXP2101_registers_e::cpuldos: return AXP2101_CPUSLDO_MIN;

    // not a voltage register
    case AXP2101_registers_e::chargeled:
    case AXP2101_registers_e::batcharge:
    case AXP2101_registers_e::charging:
    case AXP2101_registers_e::batpresent:
      break;
  }
  return 0u;
}

uint16_t AXP2101_maxVoltage(AXP2101_registers_e reg) {
  switch (reg) {
    case AXP2101_registers_e::dcdc1:
    case AXP2101_registers_e::dcdc3: return AXP2101_DCDC3_MAX;
    case AXP2101_registers_e::dcdc2: return AXP2101_DCDC2_MAX;
    case AXP2101_registers_e::dcdc4: return AXP2101_DCDC4_MAX;
    case AXP2101_registers_e::dcdc5: return AXP2101_DCDC5_MAX;
    case AXP2101_registers_e::aldo1:
    case AXP2101_registers_e::aldo2:
    case AXP2101_registers_e::aldo3:
    case AXP2101_registers_e::aldo4:
    case AXP2101_registers_e::bldo1:
    case AXP2101_registers_e::bldo2:
    case AXP2101_registers_e::dldo1: return AXP2101_DLDO1_MAX;
    case AXP2101_registers_e::dldo2:
    case AXP2101_registers_e::cpuldos: return AXP2101_CPUSLDO_MAX;

    // not a voltage register
    case AXP2101_registers_e::chargeled:
    case AXP2101_registers_e::batcharge:
    case AXP2101_registers_e::charging:
    case AXP2101_registers_e::batpresent:
      break;
  }
  return 0u;
}

/**
 * Is the pin Default, Disabled or Protected, then don't initialize
 */
bool AXP2101_isPinDefault(AXP_pin_s pin) {
  return AXP_pin_s::Protected == pin || AXP_pin_s::Disabled == pin || AXP_pin_s::Default == pin;
}

/**
 * Is the pin Disabled or Protected, then don't change
 */
bool AXP2101_isPinProtected(AXP_pin_s pin) {
  return AXP_pin_s::Protected == pin || AXP_pin_s::Disabled == pin;
}

/**
 * AXP2101_settings struct
 */

// constructor
AXP2101_settings::AXP2101_settings() {}

// constructor
AXP2101_settings::AXP2101_settings(uint16_t _dcdc1, uint16_t _dcdc2, uint16_t _dcdc3, uint16_t _dcdc4, uint16_t _dcdc5,
                                   uint16_t _aldo1, uint16_t _aldo2, uint16_t _aldo3, uint16_t _aldo4,
                                   uint16_t _bldo1, uint16_t _bldo2, uint16_t _dldo1, uint16_t _dldo2, uint16_t _cpuldos,
                                   AXP_pin_s _en_dcdc1, AXP_pin_s _en_dcdc2, AXP_pin_s _en_dcdc3, AXP_pin_s _en_dcdc4, AXP_pin_s _en_dcdc5,
                                   AXP_pin_s _en_aldo1, AXP_pin_s _en_aldo2, AXP_pin_s _en_aldo3, AXP_pin_s _en_aldo4,
                                   AXP_pin_s _en_bldo1, AXP_pin_s _en_bldo2, AXP_pin_s _en_dldo1, AXP_pin_s _en_dldo2, AXP_pin_s _en_cpuldos,
                                   AXP2101_chargeled_d _chargeled)
{
  registers.dcdc1 = _dcdc1; registers.dcdc2 = _dcdc2; registers.dcdc3 = _dcdc3; registers.dcdc4 = _dcdc4; registers.dcdc5 = _dcdc5;
  registers.aldo1 = _aldo1; registers.aldo2 = _aldo2; registers.aldo3 = _aldo3; registers.aldo4 = _aldo4;
  registers.bldo1 = _bldo1; registers.bldo1 = _bldo2; registers.dldo1 = _dldo1; registers.dldo2 = _dldo2; registers.cpuldos = _cpuldos;

  pinStates.en_dcdc1  = static_cast<uint8_t>(_en_dcdc1); pinStates.en_dcdc2 = static_cast<uint8_t>(_en_dcdc2);
  pinStates.en_dcdc3  = static_cast<uint8_t>(_en_dcdc3); pinStates.en_dcdc4 = static_cast<uint8_t>(_en_dcdc4);
  pinStates.en_dcdc5  = static_cast<uint8_t>(_en_dcdc5); pinStates.en_aldo1 = static_cast<uint8_t>(_en_aldo1);
  pinStates.en_aldo2  = static_cast<uint8_t>(_en_aldo2); pinStates.en_aldo3 = static_cast<uint8_t>(_en_aldo3);
  pinStates.en_aldo4  = static_cast<uint8_t>(_en_aldo4); pinStates.en_bldo1 = static_cast<uint8_t>(_en_bldo1);
  pinStates.en_bldo2  = static_cast<uint8_t>(_en_bldo2); pinStates.en_dldo1 = static_cast<uint8_t>(_en_dldo1);
  pinStates.en_dldo2  = static_cast<uint8_t>(_en_dldo2); pinStates.en_cpuldos = static_cast<uint8_t>(_en_cpuldos);
  pinStates.chargeled = static_cast<uint8_t>(_chargeled);
}

void AXP2101_settings::setVoltage(AXP2101_registers_e reg,
                                  int                 voltage) {
  if (-1 == voltage) { voltage = 0xFFFF; }

  switch (reg) {
    case AXP2101_registers_e::dcdc1: registers.dcdc1     = voltage; break;
    case AXP2101_registers_e::dcdc2: registers.dcdc2     = voltage; break;
    case AXP2101_registers_e::dcdc3: registers.dcdc3     = voltage; break;
    case AXP2101_registers_e::dcdc4: registers.dcdc4     = voltage; break;
    case AXP2101_registers_e::dcdc5: registers.dcdc5     = voltage; break;
    case AXP2101_registers_e::aldo1: registers.aldo1     = voltage; break;
    case AXP2101_registers_e::aldo2: registers.aldo2     = voltage; break;
    case AXP2101_registers_e::aldo3: registers.aldo3     = voltage; break;
    case AXP2101_registers_e::aldo4: registers.aldo4     = voltage; break;
    case AXP2101_registers_e::bldo1: registers.bldo1     = voltage; break;
    case AXP2101_registers_e::bldo2: registers.bldo2     = voltage; break;
    case AXP2101_registers_e::dldo1: registers.dldo1     = voltage; break;
    case AXP2101_registers_e::dldo2: registers.dldo2     = voltage; break;
    case AXP2101_registers_e::cpuldos: registers.cpuldos = voltage; break;
    case AXP2101_registers_e::chargeled:
    case AXP2101_registers_e::batcharge:
    case AXP2101_registers_e::charging:
    case AXP2101_registers_e::batpresent:
      break;
  }
}

int AXP2101_settings::getVoltage(AXP2101_registers_e reg,
                                 bool                realValue) {
  int result = -1;

  switch (reg) {
    case AXP2101_registers_e::dcdc1: result   = registers.dcdc1; break;
    case AXP2101_registers_e::dcdc2: result   = registers.dcdc2; break;
    case AXP2101_registers_e::dcdc3: result   = registers.dcdc3; break;
    case AXP2101_registers_e::dcdc4: result   = registers.dcdc4; break;
    case AXP2101_registers_e::dcdc5: result   = registers.dcdc5; break;
    case AXP2101_registers_e::aldo1: result   = registers.aldo1; break;
    case AXP2101_registers_e::aldo2: result   = registers.aldo2; break;
    case AXP2101_registers_e::aldo3: result   = registers.aldo3; break;
    case AXP2101_registers_e::aldo4: result   = registers.aldo4; break;
    case AXP2101_registers_e::bldo1: result   = registers.bldo1; break;
    case AXP2101_registers_e::bldo2: result   = registers.bldo2; break;
    case AXP2101_registers_e::dldo1: result   = registers.dldo1; break;
    case AXP2101_registers_e::dldo2: result   = registers.dldo2; break;
    case AXP2101_registers_e::cpuldos: result = registers.cpuldos; break;
    case AXP2101_registers_e::chargeled:
    case AXP2101_registers_e::batcharge:
    case AXP2101_registers_e::charging:
    case AXP2101_registers_e::batpresent:
      return 0;
  }
  return 0xFFFFF == result ? (realValue ? 0 : -1) : result;
}

void AXP2101_settings::setState(AXP2101_registers_e reg,
                                AXP_pin_s           state) {
  const uint8_t value = static_cast<uint8_t>(state) & 0x03;

  switch (reg) {
    case AXP2101_registers_e::dcdc1: pinStates.en_dcdc1     = value; break;
    case AXP2101_registers_e::dcdc2: pinStates.en_dcdc2     = value; break;
    case AXP2101_registers_e::dcdc3: pinStates.en_dcdc3     = value; break;
    case AXP2101_registers_e::dcdc4: pinStates.en_dcdc4     = value; break;
    case AXP2101_registers_e::dcdc5: pinStates.en_dcdc5     = value; break;
    case AXP2101_registers_e::aldo1: pinStates.en_aldo1     = value; break;
    case AXP2101_registers_e::aldo2: pinStates.en_aldo2     = value; break;
    case AXP2101_registers_e::aldo3: pinStates.en_aldo3     = value; break;
    case AXP2101_registers_e::aldo4: pinStates.en_aldo4     = value; break;
    case AXP2101_registers_e::bldo1: pinStates.en_bldo1     = value; break;
    case AXP2101_registers_e::bldo2: pinStates.en_bldo2     = value; break;
    case AXP2101_registers_e::dldo1: pinStates.en_dldo1     = value; break;
    case AXP2101_registers_e::dldo2: pinStates.en_dldo2     = value; break;
    case AXP2101_registers_e::cpuldos: pinStates.en_cpuldos = value; break;
    case AXP2101_registers_e::chargeled:
    case AXP2101_registers_e::batcharge:
    case AXP2101_registers_e::charging:
    case AXP2101_registers_e::batpresent:
      break;
  }
}

AXP_pin_s AXP2101_settings::getState(AXP2101_registers_e reg) {
  switch (reg) {
    case AXP2101_registers_e::dcdc1: return static_cast<AXP_pin_s>(pinStates.en_dcdc1);
    case AXP2101_registers_e::dcdc2: return static_cast<AXP_pin_s>(pinStates.en_dcdc2);
    case AXP2101_registers_e::dcdc3: return static_cast<AXP_pin_s>(pinStates.en_dcdc3);
    case AXP2101_registers_e::dcdc4: return static_cast<AXP_pin_s>(pinStates.en_dcdc4);
    case AXP2101_registers_e::dcdc5: return static_cast<AXP_pin_s>(pinStates.en_dcdc5);
    case AXP2101_registers_e::aldo1: return static_cast<AXP_pin_s>(pinStates.en_aldo1);
    case AXP2101_registers_e::aldo2: return static_cast<AXP_pin_s>(pinStates.en_aldo2);
    case AXP2101_registers_e::aldo3: return static_cast<AXP_pin_s>(pinStates.en_aldo3);
    case AXP2101_registers_e::aldo4: return static_cast<AXP_pin_s>(pinStates.en_aldo4);
    case AXP2101_registers_e::bldo1: return static_cast<AXP_pin_s>(pinStates.en_bldo1);
    case AXP2101_registers_e::bldo2: return static_cast<AXP_pin_s>(pinStates.en_bldo2);
    case AXP2101_registers_e::dldo1: return static_cast<AXP_pin_s>(pinStates.en_dldo1);
    case AXP2101_registers_e::dldo2: return static_cast<AXP_pin_s>(pinStates.en_dldo2);
    case AXP2101_registers_e::cpuldos: return static_cast<AXP_pin_s>(pinStates.en_cpuldos);
    case AXP2101_registers_e::chargeled:
    case AXP2101_registers_e::batcharge:
    case AXP2101_registers_e::charging:
    case AXP2101_registers_e::batpresent:
      return AXP_pin_s::Protected;
  }
  return AXP_pin_s::Default;
}

void AXP2101_settings::setChargeLed(AXP2101_chargeled_d led) {
  pinStates.chargeled = static_cast<uint8_t>(led);
}

AXP2101_chargeled_d AXP2101_settings::getChargeLed() {
  return static_cast<AXP2101_chargeled_d>(pinStates.chargeled);
}

/**
 * AXP2101 device class
 */

// *INDENT-OFF*
AXP2101_settings AXP2101_deviceSettingsArray[] =
{             // voltages: dcdc1 | dcdc2 | dcdc3 | dcdc4 | dcdc5 | aldo1 | aldo2 | aldo3 | aldo4| bldo1 | bldo2 | dldo1 | dldo2 | cpuldos | en_dcdc1            | en_dcdc2           | en_dcdc3            | en_dcdc4           | en_dcdc5           | en_aldo1            | en_aldo2            | aldo3              | aldo4               | en_bldo1           | en_bldo2           | en_dldo1           | en_dldo2           | en_cpuldos         | chargeled
/* Unselected         */ { 0,      0,      0,      0,      0,      0,      0,      0,      0,     0,      0,      0,      0,      0,       AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP2101_chargeled_d::Off },
/* M5Stack Core2 v1.1 */ { 3300,   0,      3300,   0,      0,      0,      0,      0,      0,     0,      0,      0,      0,      0,       AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Disabled,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP2101_chargeled_d::Off },
/* M5Stack CoreS3     */ { 3300,   0,      3300,   0,      0,      1800,   3300,   3300,   3300,  0,      0,      0,      0,      0,       AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Protected, AXP_pin_s::Protected, AXP_pin_s::Default,  AXP_pin_s::Protected, AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP2101_chargeled_d::Off },
/* LilyGo TBeam v1.2  */ { 3300,   0,      2500,   0,      0,      0,      3300,   3300,   0,     0,      0,      0,      0,      0,       AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Disabled,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Disabled,  AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Default,  AXP2101_chargeled_d::Off },
/* LilyGo TBeamS3     */ { 3300,   0,      0,      0,      0,      0,      0,      0,      0,     0,      0,      0,      0,      0,       AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP2101_chargeled_d::Off },
/* LilyGo TPCie v1.2  */ { 3300,   0,      3300,   0,      0,      0,      0,      0,      0,     0,      0,      0,      0,      0,       AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Default,   AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP2101_chargeled_d::Off },
/* Userdefined        */ { 3300,   0,      0,      0,      0,      0,      0,      0,      0,     0,      0,      0,      0,      0,       AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP2101_chargeled_d::Off },
};
// *INDENT-ON*

bool AXP2101::begin(TwoWire               *wire,
                    uint8_t                addr,
                    AXP2101_device_model_e device) {
  _wire   = wire;
  _addr   = addr;
  _device = device;
  _wire->beginTransmission(_addr);
  return 0 == _wire->endTransmission();
}

void AXP2101::setDevice(AXP2101_device_model_e device) {
  _device = device;
}

bool AXP2101::readRegister(uint8_t  addr,
                           uint8_t  reg,
                           uint8_t *result,
                           uint16_t length) {
  uint8_t index = 0;

  _wire->beginTransmission(addr);
  _wire->write(reg);
  const uint8_t err = _wire->endTransmission();

  _wire->requestFrom(addr, length);

  for (int i = 0; i < length; ++i) {
    result[index++] = _wire->read();
  }

  return err == 0;
}

uint8_t AXP2101::readRegister8(uint8_t addr,
                               uint8_t reg) {
  _wire->beginTransmission(addr);
  _wire->write(reg);
  _wire->endTransmission();
  _wire->requestFrom(addr, 1);
  return _wire->read();
}

bool AXP2101::writeRegister8(uint8_t addr,
                             uint8_t reg,
                             uint8_t data) {
  _wire->beginTransmission(addr);
  _wire->write(reg);
  _wire->write(data);
  return 0 == _wire->endTransmission();
}

bool AXP2101::bitOn(uint8_t addr,
                    uint8_t reg,
                    uint8_t data) {
  const uint8_t temp       = readRegister8(addr, reg);
  const uint8_t write_back = (temp | data);

  return writeRegister8(addr, reg, write_back);
}

bool AXP2101::bitOff(uint8_t addr,
                     uint8_t reg,
                     uint8_t data) {
  const uint8_t temp       = readRegister8(addr, reg);
  const uint8_t write_back = (temp & (~data));

  return writeRegister8(addr, reg, write_back);
}

bool AXP2101::bitGet(uint8_t reg,
                     uint8_t data) {
  const uint8_t temp = readRegister8(AXP2101_ADDR, reg);

  return (temp & data) == data;
}

bool AXP2101::bitOnOff(bool    sw,
                       uint8_t creg,
                       uint8_t mask) {
  bool result = false;

  if (sw) {
    result = bitOn(AXP2101_ADDR, creg, mask);
  } else {
    result = bitOff(AXP2101_ADDR, creg, mask);
  }

  return result;
}

/**
 * Convert a voltage to the indicated register-data, using the matching offset and range(s)
 */
uint8_t voltageToRegister(uint16_t            voltage,
                          AXP2101_registers_e reg) {
  uint16_t min = 500;
  uint16_t max = 0;

  switch (reg) {
    case AXP2101_registers_e::dcdc2:

      if (0 == max) { max = 1540; }
    case AXP2101_registers_e::dcdc3:

      if (0 == max) { max = 3400; }

      if (voltage <= min) {
        return 0u;
      }
      else if (voltage > max) {
        voltage = max;
      }
      else if ((voltage > 1540) && (voltage < 1600)) {
        voltage = 1540u;
      }

      if (voltage <= 1220) {
        return (voltage - 500) / 10;
      }
      else if (voltage <= 1540) {
        return (voltage - 1220) / 20 + (uint8_t)0b01000111;
      }
      return (voltage - 1600) / 100 + (uint8_t)0b01011000;

    case AXP2101_registers_e::dcdc4:

      if (voltage <= min) {
        return 0;
      }
      else if (voltage > 1840) {
        voltage = 1840u;
      }

      if (voltage <= 1220) {
        return (voltage - 500) / 10;
      }
      return (voltage - 1220) / 20 + (uint8_t)0b01000111;

    case AXP2101_registers_e::dcdc1:

      if (0 == max) {
        min = 1500;
        max = 3400;
      }
    case AXP2101_registers_e::dcdc5:

      if (0 == max) {
        min = 1400;
        max = 3700;
      }
    case AXP2101_registers_e::aldo1:
    case AXP2101_registers_e::aldo2:
    case AXP2101_registers_e::aldo3:
    case AXP2101_registers_e::aldo4:
    case AXP2101_registers_e::bldo1:
    case AXP2101_registers_e::bldo2:
    case AXP2101_registers_e::dldo1:

      if (0 == max) { max = 3400; }
    case AXP2101_registers_e::dldo2:
    case AXP2101_registers_e::cpuldos:

      if (0 == max) { max = 1400; }

      if (voltage <= min) { return 0u; }

      if (voltage > max) { voltage = max; }

      return (voltage - min) / 100;
    case AXP2101_registers_e::chargeled:
    case AXP2101_registers_e::batcharge:
    case AXP2101_registers_e::charging:
    case AXP2101_registers_e::batpresent:
      break;
  }
  return 0u;
}

/**
 * Convert read data from a register to a voltage for the indicated output
 */
uint16_t AXP2101::registerToVoltage(uint8_t             data,
                                    AXP2101_registers_e reg) {
  uint16_t off  = 0;
  uint8_t  mask = 0;

  switch (reg) {
    case AXP2101_registers_e::dcdc2:
    case AXP2101_registers_e::dcdc3:
      data &= 0x7F;

      if (data < 0b01000111) {
        return static_cast<uint16_t>(data * 10) + 500;
      }
      else if (data < 0b01011000) {
        return static_cast<uint16_t>(data * 20) - 200;
      }
      return static_cast<uint16_t>(data * 100) - 7200;

    case AXP2101_registers_e::dcdc4:

      if (data < 0b01000111) {
        return static_cast<uint16_t>(data * 10) + 500;
      }
      return static_cast<uint16_t>(data * 20) - 200;

    case AXP2101_registers_e::dcdc1:

      if (0 == off) { off = 1500; }
    case AXP2101_registers_e::dcdc5:

      if (0 == off) { off = 1400; }
    case AXP2101_registers_e::aldo1:
    case AXP2101_registers_e::aldo2:
    case AXP2101_registers_e::aldo3:
    case AXP2101_registers_e::aldo4:
    case AXP2101_registers_e::bldo1:
    case AXP2101_registers_e::bldo2:
    case AXP2101_registers_e::dldo1:
    case AXP2101_registers_e::dldo2:
    case AXP2101_registers_e::cpuldos:

      if (0 == off) { off = 500; }
      return off + (data * 100);
    case AXP2101_registers_e::chargeled:
    case AXP2101_registers_e::batcharge:
    case AXP2101_registers_e::charging:
    case AXP2101_registers_e::batpresent:
      break;
  }
  return 0u;
}

/**
 * Set a voltage to a port (output pin) of the AXP2101
 */
bool AXP2101::setPortVoltage(uint16_t            voltage,
                             AXP2101_registers_e reg) {
  const uint8_t data = voltageToRegister(voltage, reg);
  const uint8_t creg = static_cast<uint8_t>(reg);

  return writeRegister8(AXP2101_ADDR, creg, data);
}

/**
 * Get the voltage of a port (output pin) of the AXP2101
 */
uint16_t AXP2101::getPortVoltage(AXP2101_registers_e reg) {
  const uint8_t creg = static_cast<uint8_t>(reg);
  const uint8_t data = readRegister8(AXP2101_ADDR,
                                     creg);

  return registerToVoltage(data,
                           reg);
}

/**
 * Set the on/off state of a port (output pin) of the AXP2101
 */
bool AXP2101::setPortState(bool                sw,
                           AXP2101_registers_e reg) {
  uint8_t ctrl   = 0;
  uint8_t mask   = 0;
  bool    result = false;

  getControlRegisterMask(reg, ctrl, mask);

  if (ctrl) {
    result = bitOnOff(sw, ctrl, mask);
  }
  return result;
}

/**
 * Get the on/off state of a port (output pin) of the AXP2101
 */
bool AXP2101::getPortState(AXP2101_registers_e reg) {
  uint8_t ctrl   = 0;
  uint8_t mask   = 0;
  bool    result = false;

  getControlRegisterMask(reg, ctrl, mask);

  if (ctrl) {
    result = bitGet(ctrl, mask);
  }
  return result;
}

/**
 * Compound functions, device model dependent
 */

// TODO Enable/disable these specific per device/models
void AXP2101::set_bus_3v3(uint16_t voltage) {
  if (AXP2101_device_model_e::M5Stack_Core2_v1_1 == _device) {
    if (!voltage) {
      set_dcdc1_on_off(false);
      set_dcdc3_on_off(false);
    } else {
      set_dcdc1_on_off(true);
      set_dcdc3_on_off(true);
      set_dcdc1_voltage(voltage);
      set_dcdc3_voltage(voltage);
    }
  } // else...
}

void AXP2101::set_lcd_back_light_voltage(uint16_t voltage) {
  if (AXP2101_device_model_e::M5Stack_Core2_v1_1 == _device) {
    if (!voltage) {
      set_bldo1_on_off(false);
    } else {
      set_bldo1_on_off(true);
      set_bldo1_voltage(voltage);
    }
  } // else...
}

void AXP2101::set_bus_5v(uint8_t sw) {
  if (sw) {
    set_bldo2_on_off(true);
    set_bldo2_voltage(3300);
  } else {
    set_bldo2_on_off(false);
  }
}

void AXP2101::set_spk(bool sw) {
  if (AXP2101_device_model_e::M5Stack_Core2_v1_1 == _device) {
    if (sw) {
      set_aldo3_on_off(true);
      set_aldo3_voltage(3300);
    } else {
      set_aldo3_on_off(false);
    }
  } // else...
}

void AXP2101::set_lcd_rst(bool sw) {
  if (AXP2101_device_model_e::M5Stack_Core2_v1_1 == _device) {
    if (sw) {
      set_aldo2_on_off(true);
      set_aldo2_voltage(3300);
    } else {
      set_aldo2_on_off(false);
    }
  } // else...
}

void AXP2101::set_lcd_and_tf_voltage(uint16_t voltage) {
  if (AXP2101_device_model_e::M5Stack_Core2_v1_1 == _device) {
    if (!voltage) {
      set_aldo4_on_off(false);
    } else {
      set_aldo4_on_off(true);
      set_aldo4_voltage(voltage);
    }
  } // else...
}

void AXP2101::set_vib_motor_voltage(uint16_t voltage) {
  if (AXP2101_device_model_e::M5Stack_Core2_v1_1 == _device) {
    if (!voltage) {
      set_dldo1_on_off(false);
    } else {
      set_dldo1_on_off(true);
      set_dldo1_voltage(voltage);
    }
  } // else...
}

/**
 * Universal functions
 */
bool AXP2101::set_sys_led(bool sw) {
  return bitOnOff(sw, AXP2101_CHGLED_REG, 0b00110000);
}

bool AXP2101::setChargeLed(AXP2101_chargeled_d led) {
  if (AXP2101_chargeled_d::Protected != led) {
    const uint8_t temp       = readRegister8(_addr, AXP2101_CHGLED_REG);
    const uint8_t data       = (static_cast<uint8_t>(led) & 0x03) << 4;
    const uint8_t write_back = ((temp & 0b11001111) | data);

    return writeRegister8(_addr, AXP2101_CHGLED_REG, write_back);
  }
  return false;
}

AXP2101_chargeled_d AXP2101::getChargeLed() {
  return static_cast<AXP2101_chargeled_d>((readRegister8(_addr, AXP2101_CHGLED_REG) >> 4) & 0x07);
}

uint8_t AXP2101::getBatCharge() {
  return readRegister8(_addr, AXP2101_BAT_CHARGE_REG);
}

AXP2101_chargingState_e AXP2101::getChargingState() {
  const uint8_t level = (readRegister8(_addr, AXP2101_COM_STAT1_REG) >> 5) & 0x03;

  return static_cast<AXP2101_chargingState_e>(0x01 == level ? 1 : (0x02 == level ? -1 : 0));
}

bool AXP2101::isBatteryDetected() {
  return (readRegister8(_addr, AXP2101_COM_STAT0_REG) >> 3) & 0x01;
}

bool AXP2101::set_charger_term_current_to_zero(void) {
  return bitOff(AXP2101_ADDR, AXP2101_CHARGER_SETTING_REG, 0b00001111);
}

bool AXP2101::set_charger_constant_current_to_50mA(void) {
  return writeRegister8(AXP2101_ADDR, AXP2101_ICC_CHARGER_SETTING_REG, 2);
}

void AXP2101::set_bat_charge(bool enable) {
  uint8_t val = 0;

  if (readRegister(AXP2101_ADDR, AXP2101_CHARG_FGAUG_WDOG_REG, &val, 1)) {
    writeRegister8(AXP2101_ADDR, AXP2101_CHARG_FGAUG_WDOG_REG, (val & 0xFD) + (enable << 1));
  }
}

bool AXP2101::enable_pwrok_resets(void) {
  return bitOn(AXP2101_ADDR,
               AXP2101_PMU_CONFIG_REG,
               1 << 3);
}

void AXP2101::power_off(void) {
  // 1. AXP2101 Power off
  bitOn(AXP2101_ADDR, AXP2101_IRQ_EN_1_REG, 1 << 1);                  // POWERON Negative Edge IRQ(ponne_irq_en) enable
  writeRegister8(AXP2101_ADDR, AXP2101_PWROK_PWROFF_REG, 0b00011011); // sleep and wait for wakeup
  delay(100);
  writeRegister8(AXP2101_ADDR, AXP2101_PMU_CONFIG_REG, 0b00110001);   // power off
}

uint8_t AXP2101::get_dcdc_status(void) {
  return readRegister8(_addr,
                       AXP2101_DCDC_CTRL_REG);
}

void AXP2101::getControlRegisterMask(AXP2101_registers_e reg,
                                     uint8_t           & ctrl,
                                     uint8_t           & mask) {
  switch (reg) {
    case AXP2101_registers_e::dcdc1:
      ctrl = AXP2101_DCDC_CTRL_REG;
      mask = AXP2101_DCDC1_CTRL_MASK;
      break;
    case AXP2101_registers_e::dcdc2:
      ctrl = AXP2101_DCDC_CTRL_REG;
      mask = AXP2101_DCDC2_CTRL_MASK;
      break;
    case AXP2101_registers_e::dcdc3:
      ctrl = AXP2101_DCDC_CTRL_REG;
      mask = AXP2101_DCDC3_CTRL_MASK;
      break;
    case AXP2101_registers_e::dcdc4:
      ctrl = AXP2101_DCDC_CTRL_REG;
      mask = AXP2101_DCDC4_CTRL_MASK;
      break;
    case AXP2101_registers_e::dcdc5:
      ctrl = AXP2101_DCDC_CTRL_REG;
      mask = AXP2101_DCDC5_CTRL_MASK;
      break;
    case AXP2101_registers_e::aldo1:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_ALDO1_CTRL_MASK;
      break;
    case AXP2101_registers_e::aldo2:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_ALDO2_CTRL_MASK;
      break;
    case AXP2101_registers_e::aldo3:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_ALDO3_CTRL_MASK;
      break;
    case AXP2101_registers_e::aldo4:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_ALDO4_CTRL_MASK;
      break;
    case AXP2101_registers_e::bldo1:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_BLDO1_CTRL_MASK;
      break;
    case AXP2101_registers_e::bldo2:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_BLDO2_CTRL_MASK;
      break;
    case AXP2101_registers_e::dldo1:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_DLDO1_CTRL_MASK;
      break;
    case AXP2101_registers_e::dldo2:
      ctrl = AXP2101_LDO_CTRL_REG1;
      mask = AXP2101_DLDO2_CTRL_MASK;
      break;
    case AXP2101_registers_e::cpuldos:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_CPUSLDO_CTRL_MASK;
      break;
    case AXP2101_registers_e::chargeled:
      ctrl = AXP2101_CHGLED_REG;
      mask = AXP2101_CHGLED_CTRL_MASK;
      break;
    case AXP2101_registers_e::batcharge:
      ctrl = AXP2101_BAT_CHARGE_REG;
      mask = 0xFF;
      break;
    case AXP2101_registers_e::charging:
      ctrl = AXP2101_COM_STAT1_REG;
      mask = 0b01100000;
      break;
    case AXP2101_registers_e::batpresent:
      ctrl = AXP2101_COM_STAT0_REG;
      mask = 0b00001000;
      break;
  }
}

# pragma GCC diagnostic pop

#endif // ifdef ESP32
