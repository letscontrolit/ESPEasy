#ifndef __AXP2101_H
#define __AXP2101_H

/**
 * AXP2102 library adjusted for ESPEasy
 * 2024-02-04 tonhuisman: Start.
 *
 * Based on the AXP2102 driver included in https://github.com/m5stack/M5Core2
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

#define AXP2101_PMU_CONFIG_REG          (0x10)
#define AXP2101_CHARG_FGAUG_WDOG_REG    (0x18)
#define AXP2101_PWROK_PWROFF_REG        (0x25)
#define AXP2101_ADC_ENABLE_REG          (0x30)
#define AXP2101_IRQ_EN_1_REG            (0x41)
#define AXP2101_ICC_CHARGER_SETTING_REG (0x62)
#define AXP2101_CHARGER_SETTING_REG     (0x63)
#define AXP2101_CHGLED_REG              (0x69)

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
  Unselected         = 0u, // >>>>> Don't change these values, they are stored in user-settings <<<<<
  M5Stack_Core2_v1_1 = 1u, // https://docs.m5stack.com/en/core/Core2%20v1.1
  M5Stack_CoreS3     = 2u, // https://docs.m5stack.com/en/core/CoreS3
  LilyGO_TBeam_v1_2  = 3u, // https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/schematic/LilyGo_TBeam_V1.2.pdf
  LilyGO_TBeamS3_v3  = 4u, // https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/schematic/LilyGo_TBeam_S3_Core_V3.0.pdf
  LilyGO_TPCie_v1_2  = 5u, // https://github.com/Xinyuan-LilyGO/LilyGo-T-PCIE/blob/master/schematic/T-PCIE-V1.2.pdf

  MAX,                     // Keep MAX as first after last device, devices must be sequentially numbered
  UserDefined = 99u,       // Keep UserDefined as last!!!
};

// The voltage registers mapped into an enum
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

  // Above are settable pinstates/voltages of the AXP2101
  // TODO Below are read-only values of the AXP2101, also update AXP2101_register_count when adding values
};
constexpr int AXP2101_settings_count = 14; // Changeable settings
constexpr int AXP2101_register_count = 14; // All registers

enum class AXP_pin_s : uint8_t {
  Off       = 0x00,                        // Max. 3 bits can be stored in settings!
  On        = 0x01,
  Default   = 0x02,                        // Don't update value or state on boot
  Disabled  = 0x03,                        // Port not connected, don't use
  Protected = 0x07                         // Don't try to change port value, can make the unit fail!
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

class AXP2101_settings { // Voltages in mV, range 0..3700, max. depending on the AXP2101 pin/port used.
public:

  AXP2101_settings();
  AXP2101_settings(uint16_t  _dcdc1,
                   uint16_t  _dcdc2,
                   uint16_t  _dcdc3,
                   uint16_t  _dcdc4,
                   uint16_t  _dcdc5,
                   uint16_t  _aldo1,
                   uint16_t  _aldo2,
                   uint16_t  _aldo3,
                   uint16_t  _aldo4,
                   uint16_t  _bldo1,
                   uint16_t  _bldo2,
                   uint16_t  _dldo1,
                   uint16_t  _dldo2,
                   uint16_t  _cpuldos,
                   AXP_pin_s _en_dcdc1,
                   AXP_pin_s _en_dcdc2,
                   AXP_pin_s _en_dcdc3,
                   AXP_pin_s _en_dcdc4,
                   AXP_pin_s _en_dcdc5,
                   AXP_pin_s _en_aldo1,
                   AXP_pin_s _en_aldo2,
                   AXP_pin_s _en_aldo3,
                   AXP_pin_s _en_aldo4,
                   AXP_pin_s _en_bldo1,
                   AXP_pin_s _en_bldo2,
                   AXP_pin_s _en_dldo1,
                   AXP_pin_s _en_dldo2,
                   AXP_pin_s _en_cpuldos);
  void      setVoltage(AXP2101_registers_e reg,
                       int                 voltage);
  int       getVoltage(AXP2101_registers_e reg,
                       bool                realValue = true);
  void      setState(AXP2101_registers_e reg,
                     AXP_pin_s           state);
  AXP_pin_s getState(AXP2101_registers_e reg);

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
    struct {                    // AXP_pin_s: Off / On / default / disabled / unavailable? / unused? / Protected
      uint64_t en_dcdc1   : 3;  // bit 0/1/2
      uint64_t en_dcdc2   : 3;  // bit 3/4/5
      uint64_t en_dcdc3   : 3;  // bit 6/7/8
      uint64_t en_dcdc4   : 3;  // bit 9/10/11
      uint64_t en_dcdc5   : 3;  // bit 12/13/14
      uint64_t en_aldo1   : 3;  // bit 15/16/17
      uint64_t en_aldo2   : 3;  // bit 18/19/20
      uint64_t en_aldo3   : 3;  // bit 21/22/23
      uint64_t en_aldo4   : 3;  // bit 24/25/26
      uint64_t en_bldo1   : 3;  // bit 27/28/29
      uint64_t en_bldo2   : 3;  // bit 30/31/32
      uint64_t en_dldo1   : 3;  // bit 33/34/35
      uint64_t en_dldo2   : 3;  // bit 36/37/38
      uint64_t en_cpuldos : 3;  // bit 39/40/41
      uint64_t en_unused  : 21; // bit 42..63 // All bits defined
    }        pinStates;
    uint64_t pinStates_{};      // 8 bytes
  };
};

extern AXP2101_settings AXP2101_deviceSettingsArray[];

class AXP2101 {
private:

  uint8_t _addr;
  TwoWire *_wire;
  AXP2101_device_model_e _device = AXP2101_device_model_e::Unselected;

public:

  AXP2101() {}

  ~AXP2101() {}

  bool begin(TwoWire               *wire   = & Wire,
             uint8_t                addr   = AXP2101_ADDR,
             AXP2101_device_model_e device = AXP2101_device_model_e::Unselected);

  void setDevice(AXP2101_device_model_e device);

private:

  bool readRegister(uint8_t  addr,
                    uint8_t  reg,
                    uint8_t *result,
                    uint16_t length);
  uint8_t readRegister8(uint8_t addr,
                        uint8_t reg);
  bool    writeRegister8(uint8_t addr,
                         uint8_t reg,
                         uint8_t data);
  bool    bitOn(uint8_t addr,
                uint8_t reg,
                uint8_t data);
  bool    bitOff(uint8_t addr,
                 uint8_t reg,
                 uint8_t data);
  bool    bitOnOff(bool    sw,
                   uint8_t creg,
                   uint8_t mask);
  bool    bitGet(uint8_t reg,
                 uint8_t data);
  void    getControlRegisterMask(AXP2101_registers_e reg,
                                 uint8_t           & ctrl,
                                 uint8_t           & mask);

public:

  // Utility
  uint8_t  voltageToRegister(uint16_t            voltage,
                             AXP2101_registers_e reg);
  uint16_t registerToVoltage(uint8_t             data,
                             AXP2101_registers_e reg);
  uint8_t  get_dcdc_status(void);
  bool     setPortVoltage(uint16_t            voltage,
                          AXP2101_registers_e reg);
  uint16_t getPortVoltage(AXP2101_registers_e reg);
  bool     setPortState(bool                sw,
                        AXP2101_registers_e reg);
  bool     getPortState(AXP2101_registers_e reg);

  // Device common functions
  void     set_bus_3v3(uint16_t voltage);
  void     set_lcd_back_light_voltage(uint16_t voltage);
  void     set_bus_5v(uint8_t sw);
  bool     set_sys_led(bool sw);
  void     set_spk(bool sw);
  void     set_lcd_rst(bool sw);
  void     set_lcd_and_tf_voltage(uint16_t voltage);
  void     set_vib_motor_voltage(uint16_t voltage);
  void     set_bat_charge(bool enable);
  void     power_off(void);
  bool     set_charger_term_current_to_zero(void);
  bool     set_charger_constant_current_to_50mA(void);
  bool     enable_pwrok_resets(void);


  // Low-level output functions
  bool set_dcdc1_voltage(uint16_t voltage) { // 1.5 - 3.4V 100mV/step, 20 steps
    return setPortVoltage(voltage, AXP2101_registers_e::dcdc1);
  }

  uint16_t get_dcdc1_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::dcdc1);
  }

  bool set_dcdc2_voltage(uint16_t voltage) { // 0.5 - 1.2V 10mV/step, 71 steps,
                                             // 1.22 - 1.54V 20mV/step, 17 steps,
    return setPortVoltage(voltage, AXP2101_registers_e::dcdc2);
  }

  uint16_t get_dcdc2_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::dcdc2);
  }

  bool set_dcdc3_voltage(uint16_t voltage) { // 0.5 - 1.2V 10mV/step, 71 steps,
                                             // 1.22 - 1.54V 20mV/step, 17 steps,
                                             // 1.6 - 3.4V 100mV/step, 19 steps
    return setPortVoltage(voltage, AXP2101_registers_e::dcdc3);
  }

  uint16_t get_dcdc3_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::dcdc3);
  }

  bool set_dcdc4_voltage(uint16_t voltage) { // 0.5 - 1.2V 10mV/step, 71 steps,
                                             // 1.22 - 1.84V 20mV/step, 32 steps
    return setPortVoltage(voltage, AXP2101_registers_e::dcdc4);
  }

  uint16_t get_dcdc4_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::dcdc4);
  }

  bool set_dcdc5_voltage(uint16_t voltage) { // 1.4 - 3.7V 100mV/step, 24 steps
    return setPortVoltage(voltage, AXP2101_registers_e::dcdc5);
  }

  uint16_t get_dcdc5_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::dcdc5);
  }

  bool set_aldo1_voltage(uint16_t voltage) { // 0.5 - 3.5V 100mV/step, 31 steps
    return setPortVoltage(voltage, AXP2101_registers_e::aldo1);
  }

  uint16_t get_aldo1_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::aldo1);
  }

  bool set_aldo2_voltage(uint16_t voltage) { // 0.5 - 3.5V 100mV/step, 31 steps
    return setPortVoltage(voltage, AXP2101_registers_e::aldo2);
  }

  uint16_t get_aldo2_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::aldo2);
  }

  bool set_aldo3_voltage(uint16_t voltage) { // 0.5 - 3.5V 100mV/step, 31 steps
    return setPortVoltage(voltage, AXP2101_registers_e::aldo3);
  }

  uint16_t get_aldo3_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::aldo3);
  }

  bool set_aldo4_voltage(uint16_t voltage) { // 0.5 - 3.5V 100mV/step, 31 steps
    return setPortVoltage(voltage, AXP2101_registers_e::aldo4);
  }

  uint16_t get_aldo4_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::aldo4);
  }

  bool set_bldo1_voltage(uint16_t voltage) { // 0.5 - 3.5V 100mV/step, 31 steps
    return setPortVoltage(voltage, AXP2101_registers_e::bldo1);
  }

  uint16_t get_bldo1_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::bldo1);
  }

  bool set_bldo2_voltage(uint16_t voltage) { // 0.5 - 3.5V 100mV/step, 31 steps
    return setPortVoltage(voltage, AXP2101_registers_e::bldo2);
  }

  uint16_t get_bldo2_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::bldo2);
  }

  bool set_dldo1_voltage(uint16_t voltage) { // 0.5 - 3.5V 100mV/step, 31 steps
    return setPortVoltage(voltage, AXP2101_registers_e::dldo1);
  }

  uint16_t get_dldo1_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::dldo1);
  }

  bool set_dldo2_voltage(uint16_t voltage) { // 0.5 - 1.4V 50mV/step, 20 steps
    return setPortVoltage(voltage, AXP2101_registers_e::dldo2);
  }

  uint16_t get_dldo2_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::dldo2);
  }

  bool set_cpuldos_voltage(uint16_t voltage) { // 0.5 - 1.4V 50mV/step, 20 steps
    return setPortVoltage(voltage, AXP2101_registers_e::cpuldos);
  }

  uint16_t get_cpuldos_voltage(void) {
    return getPortVoltage(AXP2101_registers_e::cpuldos);
  }

  bool set_dcdc1_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::dcdc1);
  }

  bool set_dcdc2_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::dcdc2);
  }

  bool set_dcdc3_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::dcdc3);
  }

  bool set_dcdc4_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::dcdc4);
  }

  bool set_dcdc5_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::dcdc5);
  }

  bool set_aldo1_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::aldo1);
  }

  bool set_aldo2_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::aldo2);
  }

  bool set_aldo3_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::aldo3);
  }

  bool set_aldo4_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::aldo4);
  }

  bool set_bldo1_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::bldo1);
  }

  bool set_bldo2_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::bldo2);
  }

  bool set_dldo1_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::dldo1);
  }

  bool set_dldo2_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::dldo2);
  }

  bool set_cpuldos_on_off(bool sw) {
    return setPortState(sw, AXP2101_registers_e::cpuldos);
  }

  bool get_dcdc1_on_off() {
    return getPortState(AXP2101_registers_e::dcdc1);
  }

  bool get_dcdc2_on_off() {
    return getPortState(AXP2101_registers_e::dcdc2);
  }

  bool get_dcdc3_on_off() {
    return getPortState(AXP2101_registers_e::dcdc3);
  }

  bool get_dcdc4_on_off() {
    return getPortState(AXP2101_registers_e::dcdc4);
  }

  bool get_dcdc5_on_off() {
    return getPortState(AXP2101_registers_e::dcdc5);
  }

  bool get_aldo1_on_off() {
    return getPortState(AXP2101_registers_e::aldo1);
  }

  bool get_aldo2_on_off() {
    return getPortState(AXP2101_registers_e::aldo2);
  }

  bool get_aldo3_on_off() {
    return getPortState(AXP2101_registers_e::aldo3);
  }

  bool get_aldo4_on_off() {
    return getPortState(AXP2101_registers_e::aldo4);
  }

  bool get_bldo1_on_off() {
    return getPortState(AXP2101_registers_e::bldo1);
  }

  bool get_bldo2_on_off() {
    return getPortState(AXP2101_registers_e::bldo2);
  }

  bool get_dldo1_on_off() {
    return getPortState(AXP2101_registers_e::dldo1);
  }

  bool get_dldo2_on_off() {
    return getPortState(AXP2101_registers_e::dldo2);
  }

  bool get_cpuldos_on_off() {
    return getPortState(AXP2101_registers_e::cpuldos);
  }
};

#endif // ifndef __AXP2101_H
