#ifndef __AXP2101_SETTINGS_H
#define __AXP2101_SETTINGS_H

/**
 * AXP2101 library adjusted for ESPEasy
 * 2024-02-04 tonhuisman: Start.
 *
 * Based on the AXP2101 driver included in https://github.com/m5stack/M5Core2
 */

/** Changelog:
 * 2024-02-21 tonhuisman: Add support for ChipId and ChargingDetail state
 * 2024-02-18 tonhuisman: Add support for ChargingState, isBatteryDetected
 * 2024-02-17 tonhuisman: Add support for Charge-led and battery charge level, limit to ESP32, as this chip is only available
 *                        on ESP32 based units
 * 2024-02-16 tonhuisman: Initial 'release' with AXP2101 plugin for ESPEasy, implementing all output pins
 *                        Including predefined settings for M5Stack Core2 v1.1, M5Stack CoreS3, LilyGO_TBeam_v1_2,
 *                        LilyGO_TBeamS3_v3, LilyGO_TPCie_v1_2.
 * 2024-02-04 tonhuisman: Start development of the library
 */

#include <Arduino.h>
#include <Wire.h>

#define AXP2101_ADDR                    (0x34)

#define AXP2101_DCDC_CTRL_REG           (0x80)
#define AXP2101_LDO_CTRL_REG            (0x90)
#define AXP2101_LDO_CTRL_REG1           (0x91)

#define AXP2101_DCDC1_VOLTAGE_REG       (0x82)
#define AXP2101_DCDC2_VOLTAGE_REG       (0x83)
#define AXP2101_DCDC3_VOLTAGE_REG       (0x84)
#define AXP2101_DCDC4_VOLTAGE_REG       (0x85)
#define AXP2101_DCDC5_VOLTAGE_REG       (0x86)
#define AXP2101_ALDO1_VOLTAGE_REG       (0x92)
#define AXP2101_ALDO2_VOLTAGE_REG       (0x93)
#define AXP2101_ALDO3_VOLTAGE_REG       (0x94)
#define AXP2101_ALDO4_VOLTAGE_REG       (0x95)
#define AXP2101_BLDO1_VOLTAGE_REG       (0x96)
#define AXP2101_BLDO2_VOLTAGE_REG       (0x97)
#define AXP2101_DLDO1_VOLTAGE_REG       (0x99)
#define AXP2101_DLDO2_VOLTAGE_REG       (0x9A)
#define AXP2101_CPUSLDO_VOLTAGE_REG     (0x98)

// Measure V battery
#define AXP2101_VBAT_H_ADC_REG          (0x34)
#define AXP2101_VBAT_L_ADC_REG          (0x35)

// Measure (optional) temperature sensor
#define AXP2101_TS_H_ADC_REG            (0x36)
#define AXP2101_TS_L_ADC_REG            (0x37)

// Measure Vbus
#define AXP2101_VBUS_H_ADC_REG          (0x38)
#define AXP2101_VBUS_L_ADC_REG          (0x39)

// Measure Vsys
#define AXP2101_VSYS_H_ADC_REG          (0x3A)
#define AXP2101_VSYS_L_ADC_REG          (0x3B)

// Measure chip die temperature
#define AXP2101_TDIE_H_ADC_REG          (0x3C)
#define AXP2101_TDIE_L_ADC_REG          (0x3D)


#define AXP2101_VBAT_CTRL_MASK          (1 << 0)
#define AXP2101_BATTEMP_CTRL_MASK       (1 << 1)
#define AXP2101_VBUS_CTRL_MASK          (1 << 2)
#define AXP2101_VSYS_CTRL_MASK          (1 << 3)
#define AXP2101_TDIE_CTRL_MASK          (1 << 4)

// What to do with bit 5: "general purpose ADC channel enable"?


#define AXP2101_COM_STAT0_REG           (0x00)
#define AXP2101_COM_STAT1_REG           (0x01)
#define AXP2101_CHIP_ID_REG             (0x03)
#define AXP2101_CHARGE_DET_REG          (0x04) // Fake, not a usable register!
#define AXP2101_PMU_CONFIG_REG          (0x10)
#define AXP2101_MIN_VSYS_REG            (0x14)
#define AXP2101_VIN_DPM_REG             (0x15)
#define AXP2101_IN_CURRENT_LIMIT_REG    (0x16)
#define AXP2101_CHARG_FGAUG_WDOG_REG    (0x18)
#define AXP2101_PWROK_PWROFF_REG        (0x25)
#define AXP2101_ADC_ENABLE_REG          (0x30)
#define AXP2101_IRQ_EN_0_REG            (0x40)
#define AXP2101_IRQ_EN_1_REG            (0x41)
#define AXP2101_IRQ_EN_2_REG            (0x42)
#define AXP2101_IRQ_STATUS_0_REG        (0x48)
#define AXP2101_IRQ_STATUS_1_REG        (0x49)
#define AXP2101_IRQ_STATUS_2_REG        (0x4A)
#define AXP2101_TS_PIN_CTRL_REG         (0x50)
#define AXP2101_IPRECHG_REG             (0x61)
#define AXP2101_ICC_CHARGER_SETTING_REG (0x62)
#define AXP2101_CHARGER_SETTING_REG     (0x63)
#define AXP2101_CV_CHARGER_SETTING_REG  (0x64)
#define AXP2101_CHGLED_REG              (0x69)
#define AXP2101_BAT_CHARGE_REG          (0xA4) /* pdf has a duplicate listed for register 0x04, should be 0xA4 */

#define AXP2101_DCDC1_CTRL_MASK         (1 << 0)
#define AXP2101_DCDC2_CTRL_MASK         (1 << 1)
#define AXP2101_DCDC3_CTRL_MASK         (1 << 2)
#define AXP2101_DCDC4_CTRL_MASK         (1 << 3)
#define AXP2101_DCDC5_CTRL_MASK         (1 << 4)
#define AXP2101_ALDO1_CTRL_MASK         (1 << 0)
#define AXP2101_ALDO2_CTRL_MASK         (1 << 1)
#define AXP2101_ALDO3_CTRL_MASK         (1 << 2)
#define AXP2101_ALDO4_CTRL_MASK         (1 << 3)
#define AXP2101_BLDO1_CTRL_MASK         (1 << 4)
#define AXP2101_BLDO2_CTRL_MASK         (1 << 5)
#define AXP2101_DLDO1_CTRL_MASK         (1 << 7)
#define AXP2101_DLDO2_CTRL_MASK         (1 << 0)
#define AXP2101_CPUSLDO_CTRL_MASK       (1 << 6)
#define AXP2101_CHGLED_CTRL_MASK        (0x03 << 4)

#define AXP2101_DCDC1_MIN               (1500)
#define AXP2101_DCDC2_MIN               (500)
#define AXP2101_DCDC3_MIN               (500)
#define AXP2101_DCDC4_MIN               (500)
#define AXP2101_DCDC5_MIN               (1400)
#define AXP2101_ALDO1_MIN               (500)
#define AXP2101_ALDO2_MIN               (500)
#define AXP2101_ALDO3_MIN               (500)
#define AXP2101_ALDO4_MIN               (500)
#define AXP2101_BLDO1_MIN               (500)
#define AXP2101_BLDO2_MIN               (500)
#define AXP2101_DLDO1_MIN               (500)
#define AXP2101_DLDO2_MIN               (500)
#define AXP2101_CPUSLDO_MIN             (500)

#define AXP2101_DCDC1_MAX               (3400)
#define AXP2101_DCDC2_MAX               (1540)
#define AXP2101_DCDC3_MAX               (3400)
#define AXP2101_DCDC4_MAX               (1840)
#define AXP2101_DCDC5_MAX               (3700)
#define AXP2101_ALDO1_MAX               (3500)
#define AXP2101_ALDO2_MAX               (3500)
#define AXP2101_ALDO3_MAX               (3500)
#define AXP2101_ALDO4_MAX               (3500)
#define AXP2101_BLDO1_MAX               (3500)
#define AXP2101_BLDO2_MAX               (3500)
#define AXP2101_DLDO1_MAX               (3500)
#define AXP2101_DLDO2_MAX               (1400)
#define AXP2101_CPUSLDO_MAX             (1400)

enum class AXP2101_device_model_e : uint8_t {
  Unselected         = 0u, // >>>>> Don't change these values, they are probably stored in user-settings <<<<<
  M5Stack_Core2_v1_1 = 1u, // https://docs.m5stack.com/en/core/Core2%20v1.1
  M5Stack_CoreS3     = 2u, // https://docs.m5stack.com/en/core/CoreS3
  LilyGO_TBeam_v1_2  = 3u, // https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/schematic/LilyGo_TBeam_V1.2.pdf
  LilyGO_TBeamS3_v3  = 4u, // https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/schematic/LilyGo_TBeam_S3_Core_V3.0.pdf
  LilyGO_TPCie_v1_2  = 5u, // https://github.com/Xinyuan-LilyGO/LilyGo-T-PCIE/blob/master/schematic/T-PCIE-V1.2.pdf

  MAX,                     // Keep MAX as first after last device, devices must be sequentially numbered
  UserDefined = 99u,       // Keep UserDefined as last!!!
};

// The voltage registers mapped into an enum, don't change order without also changing AXP2101_intToRegister()
enum class AXP2101_registers_e : uint8_t {
  dcdc1   = AXP2101_DCDC1_VOLTAGE_REG,
  dcdc2   = AXP2101_DCDC2_VOLTAGE_REG,
  dcdc3   = AXP2101_DCDC3_VOLTAGE_REG,
  dcdc4   = AXP2101_DCDC4_VOLTAGE_REG,
  dcdc5   = AXP2101_DCDC5_VOLTAGE_REG,
  aldo1   = AXP2101_ALDO1_VOLTAGE_REG,
  aldo2   = AXP2101_ALDO2_VOLTAGE_REG,
  aldo3   = AXP2101_ALDO3_VOLTAGE_REG,
  aldo4   = AXP2101_ALDO4_VOLTAGE_REG,
  bldo1   = AXP2101_BLDO1_VOLTAGE_REG,
  bldo2   = AXP2101_BLDO2_VOLTAGE_REG,
  dldo1   = AXP2101_DLDO1_VOLTAGE_REG,
  dldo2   = AXP2101_DLDO2_VOLTAGE_REG,
  cpuldos = AXP2101_CPUSLDO_VOLTAGE_REG,

  // ADC inputs
  vbat     = AXP2101_VBAT_H_ADC_REG,
  battemp  = AXP2101_TS_H_ADC_REG,
  vbus     = AXP2101_VBUS_H_ADC_REG,
  vsys     = AXP2101_VSYS_H_ADC_REG,
  chiptemp = AXP2101_TDIE_H_ADC_REG,

  // Above are settable pinstates/voltages of the AXP2101
  // Below are non-voltage and read-only values of the AXP2101, also update AXP2101_register_count when adding values
  chargeled  = AXP2101_CHGLED_REG,
  batcharge  = AXP2101_BAT_CHARGE_REG,
  charging   = AXP2101_COM_STAT1_REG,
  batpresent = AXP2101_COM_STAT0_REG,
  chipid     = AXP2101_CHIP_ID_REG,
  chargedet  = AXP2101_CHARGE_DET_REG,
};
constexpr int AXP2101_settings_count = 14; // Changeable settings
constexpr int AXP2101_register_count = 25; // All registers

enum class AXP_pin_s : uint8_t {
  Off       = 0x00,                        // Max. 3 bits can be stored in settings!
  On        = 0x01,
  Default   = 0x02,                        // Don't update value or state on boot
  Disabled  = 0x03,                        // Port not connected, don't use
  Protected = 0x07                         // Don't try to change port value, can make the unit fail!
};

enum class AXP2101_chargeled_d : uint8_t {
  Off       = 0x00,
  Flash_1Hz = 0x01,
  Flash_4Hz = 0x02,
  Steady_On = 0x03,
  Protected = 0x07 // Don't try to change or not connected
};

enum class AXP2101_chargingState_e : int8_t {
  Discharging = -1,
  Standby     = 0,
  Charging    = 1,
};

enum class AXP2101_chipid_e : uint8_t {
  axp2101 = 0b01000111, // AXP2101
};

enum class AXP2101_chargingDetail_e : uint8_t {
  tricharge    = 0b000,
  precharge    = 0b001,
  constcharge  = 0b010,
  constvoltage = 0b011,
  done         = 0b100,
  notcharging  = 0b101,
};

enum class AXP2101_CV_charger_voltage_e : uint8_t {
  reserved    = 0,
  limit_4_00V = 0b001,
  limit_4_10V = 0b010,
  limit_4_20V = 0b011, // default
  limit_4_35V = 0b100,
  limit_4_40V = 0b101,
  MAX
};

enum class AXP2101_Linear_Charger_Vsys_dpm_e : uint8_t {
  vsys_4_1V = 0,
  vsys_4_2V,
  vsys_4_3V,
  vsys_4_4V,
  vsys_4_5V,
  vsys_4_6V,
  vsys_4_7V,
  vsys_4_8V,
  MAX
};

enum class AXP2101_VINDPM_e : uint8_t {
  Vin_3_88V = 0,
  Vin_3_96V,
  Vin_4_04V,
  Vin_4_12V,
  Vin_4_20V,
  Vin_4_28V,
  Vin_4_36V,
  Vin_4_44V,
  Vin_4_52V,
  Vin_4_60V,
  Vin_4_68V,
  Vin_4_76V,
  Vin_4_84V,
  Vin_4_92V,
  Vin_5_00V,
  Vin_5_08V,
  MAX
};

enum class AXP2101_InputCurrentLimit_e : uint8_t {
  limit_100mA = 0,
  limit_500mA,
  limit_900mA,
  limit_1000mA,
  limit_1500mA,
  limit_2000mA,
  MAX
};


AXP2101_registers_e        AXP2101_intToRegister(int reg);
uint16_t                   AXP2101_maxVoltage(AXP2101_registers_e reg);
uint16_t                   AXP2101_minVoltage(AXP2101_registers_e reg);
bool                       AXP2101_isPinDefault(AXP_pin_s pin);   // Default, Disabled or Protected
bool                       AXP2101_isPinProtected(AXP_pin_s pin); // Disabled or Protected

const __FlashStringHelper* toString(AXP2101_registers_e reg,
                                    bool                displayString = true);
const __FlashStringHelper* toString(AXP2101_device_model_e device,
                                    bool                   displayString = true);
const __FlashStringHelper* toString(AXP_pin_s pin);
const __FlashStringHelper* toString(AXP2101_chargeled_d led);
const __FlashStringHelper* toString(AXP2101_chargingState_e state);
const __FlashStringHelper* toString(AXP2101_chipid_e chip);
const __FlashStringHelper* toString(AXP2101_chargingDetail_e charge);

class AXP2101_settings { // Voltages in mV, range 0..3700, max. depending on the AXP2101 pin/port used.
public:

  AXP2101_settings();
  AXP2101_settings(uint16_t            _dcdc1,
                   uint16_t            _dcdc2,
                   uint16_t            _dcdc3,
                   uint16_t            _dcdc4,
                   uint16_t            _dcdc5,
                   uint16_t            _aldo1,
                   uint16_t            _aldo2,
                   uint16_t            _aldo3,
                   uint16_t            _aldo4,
                   uint16_t            _bldo1,
                   uint16_t            _bldo2,
                   uint16_t            _dldo1,
                   uint16_t            _dldo2,
                   uint16_t            _cpuldos,
                   AXP_pin_s           _en_dcdc1,
                   AXP_pin_s           _en_dcdc2,
                   AXP_pin_s           _en_dcdc3,
                   AXP_pin_s           _en_dcdc4,
                   AXP_pin_s           _en_dcdc5,
                   AXP_pin_s           _en_aldo1,
                   AXP_pin_s           _en_aldo2,
                   AXP_pin_s           _en_aldo3,
                   AXP_pin_s           _en_aldo4,
                   AXP_pin_s           _en_bldo1,
                   AXP_pin_s           _en_bldo2,
                   AXP_pin_s           _en_dldo1,
                   AXP_pin_s           _en_dldo2,
                   AXP_pin_s           _en_cpuldos,
                   AXP2101_chargeled_d _chargeled);
  void                setVoltage(AXP2101_registers_e reg,
                                 int                 voltage);
  int                 getVoltage(AXP2101_registers_e reg,
                                 bool                realValue = true);
  void                setState(AXP2101_registers_e reg,
                               AXP_pin_s           state);
  AXP_pin_s           getState(AXP2101_registers_e reg);
  void                setChargeLed(AXP2101_chargeled_d led);
  AXP2101_chargeled_d getChargeLed();

  bool                getTS_disabled();
  void                setTS_disabled(bool val);


  // Reg 61: Iprechg Charger Settings
  uint16_t getPreChargeCurrentLimit() const;
  void     setPreChargeCurrentLimit(uint16_t current_mA);

  // Reg 62: ICC Charger Settings
  uint16_t getConstChargeCurrentLimit() const;
  void     setConstChargeCurrentLimit(uint16_t current_mA);

  // Reg 63: Iterm Charger Settings and Control
  // Enable/Disable via chargeStates.term_cur_lim_en
  uint16_t getTerminationChargeCurrentLimit() const;
  bool     getTerminationChargeCurrentLimitEnable() const;
  void     setTerminationChargeCurrentLimit(uint16_t current_mA,
                                            bool     enable);

  // Reg 64: CV Charger Voltage Settings
  AXP2101_CV_charger_voltage_e      getCV_chargeVoltage() const;
  void                              setCV_chargeVoltage(AXP2101_CV_charger_voltage_e voltage_mV);

  // Reg 14: Minimum System Voltage Control
  AXP2101_Linear_Charger_Vsys_dpm_e getLinear_Charger_Vsys_dpm() const;
  void                              setLinear_Charger_Vsys_dpm(AXP2101_Linear_Charger_Vsys_dpm_e voltage);

  // Reg 15: Input Voltage Limit
  AXP2101_VINDPM_e                  getVin_DPM() const;
  void                              setVin_DPM(AXP2101_VINDPM_e voltage);

  // Reg 16: Input Current Limit
  AXP2101_InputCurrentLimit_e       getInputCurrentLimit() const;
  void                              setInputCurrentLimit(AXP2101_InputCurrentLimit_e current);

private:

  union {
    struct {
      uint16_t dcdc1;
      uint16_t dcdc2;
      uint16_t dcdc3;
      uint16_t dcdc4;
      uint16_t dcdc5;
      uint16_t aldo1;
      uint16_t aldo2;
      uint16_t aldo3;
      uint16_t aldo4;
      uint16_t bldo1;
      uint16_t bldo2;
      uint16_t dldo1;
      uint16_t dldo2;
      uint16_t cpuldos;
    }        registers;
    uint16_t registers_[AXP2101_settings_count]{};
  };
  union {
    struct {                   // AXP_pin_s: Off / On / default / disabled / unavailable? / unused? / Protected
      uint64_t en_dcdc1   : 3; // bit 0/1/2
      uint64_t en_dcdc2   : 3; // bit 3/4/5
      uint64_t en_dcdc3   : 3; // bit 6/7/8
      uint64_t en_dcdc4   : 3; // bit 9/10/11
      uint64_t en_dcdc5   : 3; // bit 12/13/14
      uint64_t en_aldo1   : 3; // bit 15/16/17
      uint64_t en_aldo2   : 3; // bit 18/19/20
      uint64_t en_aldo3   : 3; // bit 21/22/23
      uint64_t en_aldo4   : 3; // bit 24/25/26
      uint64_t en_bldo1   : 3; // bit 27/28/29
      uint64_t en_bldo2   : 3; // bit 30/31/32
      uint64_t en_dldo1   : 3; // bit 33/34/35
      uint64_t en_dldo2   : 3; // bit 36/37/38
      uint64_t en_cpuldos : 3; // bit 39/40/41
      uint64_t chargeled  : 3; // bit 42/43/44

      // Settings for external temperature sensor (TS)
      uint64_t dis_TS_pin : 1; // bit 45, reg50 bit 4
      uint64_t TS_cur_src : 2; // bit 46/47, reg50 bit 3:2
      uint64_t TS_current : 2; // bit 48/49, reg50 bit 1:0

      uint64_t en_unused : 13; // bit 50..63 // All bits defined
    }        pinStates;
    uint64_t pinStates_{};     // 8 bytes
  };

  union {
    struct {
      uint64_t pre_chg_cur        : 4; // reg 0x61: 25* N mA
      uint64_t const_cur_lim      : 5; // reg 0x62: 25* N mA if N <= 8, 200+100*(N-8) mA if N > 8
      uint64_t term_cur_lim_en    : 1; // reg 0x63: Charging termination of current enable
      uint64_t term_cur_lim       : 4; // reg 0x63: 25* N mA
      uint64_t chg_volt_lim       : 3; // reg 0x64:
      uint64_t thermal_thresh     : 2; // reg 0x65: 00: 60deg, 01: 80deg, 10: 100deg, 11:120deg
      uint64_t chg_timeout_ctrl   : 8; // reg 0x67:
      uint64_t bat_detection      : 1; // reg 0x68:
      uint64_t coincell_term_volt : 3; // reg 0x6A: 2.6~3.3V, 100mV/step, 8 steps

      uint64_t min_sys_voltage : 3;    // reg 0x14: 4.1 + N*0.1V  Linear Charger Vsys Voltage dpm
      uint64_t inp_volt_limit  : 4;    // reg 0x15: Vindpm 3.88+N*0.08V
      uint64_t inp_cur_limit   : 3;    // reg 0x16:


      uint64_t en_unused : 23;
    }        chargeStates;
    uint64_t chargeStates_{}; // 8 bytes
  };
};

extern AXP2101_settings AXP2101_deviceSettingsArray[];


#endif // ifndef __AXP2101_SETTINGS_H
