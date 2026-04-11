#include "AXP2101_settings.h"

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
    case AXP2101_registers_e::chipid: return displayString ? F("ChipID") : F("chipid");
    case AXP2101_registers_e::chargedet: return displayString ? F("ChargingDetail") : F("chargingdet");

    case AXP2101_registers_e::vbat: return displayString ? F("BatVoltage") : F("vbat");
    case AXP2101_registers_e::battemp: return displayString ? F("BatTemp") : F("battemp");
    case AXP2101_registers_e::vbus: return displayString ? F("BusVoltage") : F("vbus");
    case AXP2101_registers_e::vsys: return displayString ? F("SysVoltage") : F("vsys");
    case AXP2101_registers_e::chiptemp: return displayString ? F("ChipTemp") : F("chiptemp");
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

const __FlashStringHelper* toString(AXP2101_chipid_e chip) {
  switch (chip) {
    case AXP2101_chipid_e::axp2101: return F("AXP2101");
  }
  return F("");
}

const __FlashStringHelper* toString(AXP2101_chargingDetail_e charge) {
  switch (charge) {
    case AXP2101_chargingDetail_e::tricharge: return F("tri-charge");
    case AXP2101_chargingDetail_e::precharge: return F("pre-charge");
    case AXP2101_chargingDetail_e::constcharge: return F("constant charge (CC)");
    case AXP2101_chargingDetail_e::constvoltage: return F("constant voltage (CV)");
    case AXP2101_chargingDetail_e::done: return F("charge done");
    case AXP2101_chargingDetail_e::notcharging: return F("not charging");
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
    case 18: return AXP2101_registers_e::chipid;
    case 19: return AXP2101_registers_e::chargedet;

    case 20: return AXP2101_registers_e::vbat;
    case 21: return AXP2101_registers_e::battemp;
    case 22: return AXP2101_registers_e::vbus;
    case 23: return AXP2101_registers_e::vsys;
    case 24: return AXP2101_registers_e::chiptemp;
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
    case AXP2101_registers_e::chipid:
    case AXP2101_registers_e::chargedet:

    case AXP2101_registers_e::vbat:
    case AXP2101_registers_e::battemp:
    case AXP2101_registers_e::vbus:
    case AXP2101_registers_e::vsys:
    case AXP2101_registers_e::chiptemp:
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
    case AXP2101_registers_e::chipid:
    case AXP2101_registers_e::chargedet:

    case AXP2101_registers_e::vbat:
    case AXP2101_registers_e::battemp:
    case AXP2101_registers_e::vbus:
    case AXP2101_registers_e::vsys:
    case AXP2101_registers_e::chiptemp:
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
AXP2101_settings::AXP2101_settings() {
  chargeStates.pre_chg_cur        = 0b0101;
  chargeStates.const_cur_lim      = 0b01001; // 300 mA, however can be set via EFUSE
  chargeStates.term_cur_lim_en    = 1;
  chargeStates.term_cur_lim       = 0b0101;  // 125 mA
  chargeStates.chg_volt_lim       = 0b011;   // 4.2V
  chargeStates.thermal_thresh     = 0b10;    // 100 deg
  chargeStates.chg_timeout_ctrl   = 0b11100110;
  chargeStates.bat_detection      = 1;
  chargeStates.coincell_term_volt = 0b011;   // 2.9V
  chargeStates.min_sys_voltage    = 0b110;   // 4.7V
  chargeStates.inp_volt_limit     = 0b0110;  // 4.36V
  chargeStates.inp_cur_limit      = 0b100;   // 1500 mA
}

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
  registers.bldo1 = _bldo1; registers.bldo2 = _bldo2; registers.dldo1 = _dldo1; registers.dldo2 = _dldo2; registers.cpuldos = _cpuldos;

  pinStates.en_dcdc1  = static_cast<uint8_t>(_en_dcdc1); pinStates.en_dcdc2 = static_cast<uint8_t>(_en_dcdc2);
  pinStates.en_dcdc3  = static_cast<uint8_t>(_en_dcdc3); pinStates.en_dcdc4 = static_cast<uint8_t>(_en_dcdc4);
  pinStates.en_dcdc5  = static_cast<uint8_t>(_en_dcdc5); pinStates.en_aldo1 = static_cast<uint8_t>(_en_aldo1);
  pinStates.en_aldo2  = static_cast<uint8_t>(_en_aldo2); pinStates.en_aldo3 = static_cast<uint8_t>(_en_aldo3);
  pinStates.en_aldo4  = static_cast<uint8_t>(_en_aldo4); pinStates.en_bldo1 = static_cast<uint8_t>(_en_bldo1);
  pinStates.en_bldo2  = static_cast<uint8_t>(_en_bldo2); pinStates.en_dldo1 = static_cast<uint8_t>(_en_dldo1);
  pinStates.en_dldo2  = static_cast<uint8_t>(_en_dldo2); pinStates.en_cpuldos = static_cast<uint8_t>(_en_cpuldos);
  pinStates.chargeled = static_cast<uint8_t>(_chargeled);

  chargeStates.pre_chg_cur        = 0b0101;
  chargeStates.const_cur_lim      = 0b01001; // 300 mA, however can be set via EFUSE
  chargeStates.term_cur_lim_en    = 1;
  chargeStates.term_cur_lim       = 0b0101;  // 125 mA
  chargeStates.chg_volt_lim       = 0b011;   // 4.2V
  chargeStates.thermal_thresh     = 0b10;    // 100 deg
  chargeStates.chg_timeout_ctrl   = 0b11100110;
  chargeStates.bat_detection      = 1;
  chargeStates.coincell_term_volt = 0b011;   // 2.9V
  chargeStates.min_sys_voltage    = 0b110;   // 4.7V
  chargeStates.inp_volt_limit     = 0b0110;  // 4.36V
  chargeStates.inp_cur_limit      = 0b100;   // 1500 mA
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
    case AXP2101_registers_e::chipid:
    case AXP2101_registers_e::chargedet:
    case AXP2101_registers_e::vbat:
    case AXP2101_registers_e::battemp:
    case AXP2101_registers_e::vbus:
    case AXP2101_registers_e::vsys:
    case AXP2101_registers_e::chiptemp:
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
    case AXP2101_registers_e::chipid:
    case AXP2101_registers_e::chargedet:
    case AXP2101_registers_e::vbat:
    case AXP2101_registers_e::battemp:
    case AXP2101_registers_e::vbus:
    case AXP2101_registers_e::vsys:
    case AXP2101_registers_e::chiptemp:
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
    case AXP2101_registers_e::chipid:
    case AXP2101_registers_e::chargedet:
    case AXP2101_registers_e::vbat:
    case AXP2101_registers_e::battemp:
    case AXP2101_registers_e::vbus:
    case AXP2101_registers_e::vsys:
    case AXP2101_registers_e::chiptemp:
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
    case AXP2101_registers_e::chipid:
    case AXP2101_registers_e::chargedet:
      return AXP_pin_s::Protected;
    case AXP2101_registers_e::vbat:
    case AXP2101_registers_e::battemp:
    case AXP2101_registers_e::vbus:
    case AXP2101_registers_e::vsys:
    case AXP2101_registers_e::chiptemp:
      break;
  }
  return AXP_pin_s::Default;
}

void AXP2101_settings::setChargeLed(AXP2101_chargeled_d led) {
  pinStates.chargeled = static_cast<uint8_t>(led);
}

AXP2101_chargeled_d AXP2101_settings::getChargeLed() {
  return static_cast<AXP2101_chargeled_d>(pinStates.chargeled);
}

bool AXP2101_settings::getTS_disabled() {
  return pinStates.dis_TS_pin;
}

void AXP2101_settings::setTS_disabled(bool val) {
  pinStates.dis_TS_pin = val;
}

uint16_t AXP2101_settings::getPreChargeCurrentLimit() const {
  return chargeStates.pre_chg_cur * 25;
}

void AXP2101_settings::setPreChargeCurrentLimit(uint16_t current_mA) {
  if (current_mA > 200) {
    current_mA = 200;
  }
  chargeStates.pre_chg_cur = current_mA / 25;
}

uint16_t AXP2101_settings::getConstChargeCurrentLimit() const {
  if (chargeStates.const_cur_lim <= 8) {
    return chargeStates.const_cur_lim * 25;
  }
  return (chargeStates.const_cur_lim - 8) * 100 + 200;
}

void AXP2101_settings::setConstChargeCurrentLimit(uint16_t current_mA) {
  if (current_mA > 1000) {
    current_mA = 1000;
  }

  if (current_mA <= 200) {
    chargeStates.const_cur_lim = current_mA / 25;
  } else {
    chargeStates.const_cur_lim = ((current_mA - 200) / 100) + 8;
  }
}

uint16_t AXP2101_settings::getTerminationChargeCurrentLimit() const {
  return chargeStates.term_cur_lim * 25;
}

bool AXP2101_settings::getTerminationChargeCurrentLimitEnable() const {
  return chargeStates.term_cur_lim_en;
}

void AXP2101_settings::setTerminationChargeCurrentLimit(uint16_t current_mA, bool enable) {
  if (current_mA > 200) {
    current_mA = 200;
  }
  chargeStates.term_cur_lim    = current_mA / 25;
  chargeStates.term_cur_lim_en = enable;
}

AXP2101_CV_charger_voltage_e AXP2101_settings::getCV_chargeVoltage() const {
  return static_cast<AXP2101_CV_charger_voltage_e>(chargeStates.chg_volt_lim);
}

void AXP2101_settings::setCV_chargeVoltage(AXP2101_CV_charger_voltage_e voltage_mV) {
  chargeStates.chg_volt_lim = static_cast<uint8_t>(voltage_mV);

  if (chargeStates.chg_volt_lim > static_cast<uint8_t>(AXP2101_CV_charger_voltage_e::limit_4_40V)) {
    // Set to a default safe limit
    chargeStates.chg_volt_lim = static_cast<uint8_t>(AXP2101_CV_charger_voltage_e::limit_4_20V);
  }
}

AXP2101_Linear_Charger_Vsys_dpm_e AXP2101_settings::getLinear_Charger_Vsys_dpm() const {
  return static_cast<AXP2101_Linear_Charger_Vsys_dpm_e>(chargeStates.min_sys_voltage);
}

void AXP2101_settings::setLinear_Charger_Vsys_dpm(AXP2101_Linear_Charger_Vsys_dpm_e voltage) {
  chargeStates.min_sys_voltage = static_cast<uint8_t>(voltage);
}

AXP2101_VINDPM_e AXP2101_settings::getVin_DPM() const {
  return static_cast<AXP2101_VINDPM_e>(chargeStates.inp_volt_limit);
}

void AXP2101_settings::setVin_DPM(AXP2101_VINDPM_e voltage) {
  chargeStates.inp_volt_limit = static_cast<uint8_t>(voltage);
}

AXP2101_InputCurrentLimit_e AXP2101_settings::getInputCurrentLimit() const {
  return static_cast<AXP2101_InputCurrentLimit_e>(chargeStates.inp_cur_limit);
}

void AXP2101_settings::setInputCurrentLimit(AXP2101_InputCurrentLimit_e current) {
  chargeStates.inp_cur_limit = static_cast<uint8_t>(current);
}

/**
 * AXP2101 device class
 */

// *INDENT-OFF*
AXP2101_settings AXP2101_deviceSettingsArray[] =
{             // voltages: dcdc1 | dcdc2 | dcdc3 | dcdc4 | dcdc5 | aldo1 | aldo2 | aldo3 | aldo4| bldo1 | bldo2 | dldo1 | dldo2 | cpuldos | en_dcdc1            | en_dcdc2           | en_dcdc3            | en_dcdc4           | en_dcdc5           | en_aldo1            | en_aldo2            | en_aldo3           | en_aldo4            | en_bldo1           | en_bldo2           | en_dldo1           | en_dldo2           | en_cpuldos         | chargeled
/* Unselected         */ { 0,      0,      0,      0,      0,      0,      0,      0,      0,     0,      0,      0,      0,      0,       AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP2101_chargeled_d::Off },
/* M5Stack Core2 v1.1 */ { 3300,   0,      3300,   0,      0,      0,      0,      0,      0,     0,      0,      0,      0,      0,       AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Disabled,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP2101_chargeled_d::Off },
/* M5Stack CoreS3     */ { 3300,   0,      3300,   0,      0,      1800,   3300,   3300,   3300,  0,      0,      0,      0,      0,       AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Protected, AXP_pin_s::Protected, AXP_pin_s::Default,  AXP_pin_s::Protected, AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP2101_chargeled_d::Off },
/* LilyGo TBeam v1.2  */ { 3300,   0,      2500,   0,      0,      0,      3300,   3300,   0,     0,      0,      0,      0,      0,       AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Disabled,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Disabled,  AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Default,  AXP2101_chargeled_d::Off },
/* LilyGo TBeamS3     */ { 3300,   500,    500,    1800,   3300,   1800,   3300,   3300,   3300,  1800,   3300,   3300,   2300,   2300,    AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP2101_chargeled_d::Off },
/* LilyGo TPCie v1.2  */ { 3300,   900,    900,    1100,   1200,   1800,   2800,   3300,   2900,  1800,   2800,   500,    1900,   1300,    AXP_pin_s::Protected, AXP_pin_s::Disabled, AXP_pin_s::Protected, AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP_pin_s::Disabled, AXP2101_chargeled_d::Off },
/* Userdefined        */ { 3300,   0,      0,      0,      0,      0,      0,      0,      0,     0,      0,      0,      0,      0,       AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,   AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP_pin_s::Default,  AXP2101_chargeled_d::Off },
};
// *INDENT-ON*

# pragma GCC diagnostic pop

#endif // ifdef ESP32
