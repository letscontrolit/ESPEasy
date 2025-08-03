#ifndef __AXP2101_H
#define __AXP2101_H

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
#include "AXP2101_settings.h"

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
  uint8_t                  voltageToRegister(uint16_t            voltage,
                                             AXP2101_registers_e reg);
  uint16_t                 registerToVoltage(uint8_t             data,
                                             AXP2101_registers_e reg);

  // Convertion between NTC temperature sensor raw value and temperature
  uint16_t                 TS_tempToRegister(float temp_C);
  float                    TS_registerToTemp(uint16_t regValue);

  uint8_t                  get_dcdc_status(void);
  bool                     setPortVoltage(uint16_t            voltage,
                                          AXP2101_registers_e reg);
  uint16_t                 getPortVoltage(AXP2101_registers_e reg);
  bool                     setPortState(bool                sw,
                                        AXP2101_registers_e reg);
  bool                     getPortState(AXP2101_registers_e reg);

  bool                     enableADC(AXP2101_registers_e reg, bool enable);
  uint16_t                 getADCVoltage(AXP2101_registers_e reg);

  bool                     setChargeLed(AXP2101_chargeled_d led);
  AXP2101_chargeled_d      getChargeLed();

  bool                     getTS_disabled();
  void                     setTS_disabled(bool val);

    // Reg 61: Iprechg Charger Settings
  uint16_t getPreChargeCurrentLimit() ;
  void setPreChargeCurrentLimit(uint16_t current_mA);

  // Reg 62: ICC Charger Settings
  uint16_t getConstChargeCurrentLimit() ;
  void setConstChargeCurrentLimit(uint16_t current_mA);

  // Reg 63: Iterm Charger Settings and Control
  // Enable/Disable via chargeStates.term_cur_lim_en
  uint16_t getTerminationChargeCurrentLimit() ;
  void setTerminationChargeCurrentLimit(uint16_t current_mA);
  void setTerminationChargeCurrentLimit(uint16_t current_mA, bool enable);

  // Reg 64: CV Charger Voltage Settings
  AXP2101_CV_charger_voltage_e getCV_chargeVoltage() ;
  void setCV_chargeVoltage(AXP2101_CV_charger_voltage_e voltage_mV);

  // Reg 14: Minimum System Voltage Control
  AXP2101_Linear_Charger_Vsys_dpm_e getLinear_Charger_Vsys_dpm() ;
  void setLinear_Charger_Vsys_dpm(AXP2101_Linear_Charger_Vsys_dpm_e voltage);

  // Reg 15: Input Voltage Limit
  AXP2101_VINDPM_e getVin_DPM() ;
  void setVin_DPM(AXP2101_VINDPM_e voltage);

  // Reg 16: Input Current Limit
  AXP2101_InputCurrentLimit_e getInputCurrentLimit() ;
  void setInputCurrentLimit(AXP2101_InputCurrentLimit_e current);


  uint8_t                  getBatCharge();
  AXP2101_chargingState_e  getChargingState();
  bool                     isBatteryDetected();
  bool                     isVbusGood();
  bool                     isVbusIn();
  AXP2101_chargingDetail_e getChargingDetail();
  uint8_t                  getChipIDRaw();
  AXP2101_chipid_e         getChipID();

  // Device common functions
  void                     set_bus_3v3(uint16_t voltage);
  void                     set_lcd_back_light_voltage(uint16_t voltage);
  void                     set_bus_5v(uint8_t sw);
  bool                     set_sys_led(bool sw);
  void                     set_spk(bool sw);
  void                     set_lcd_rst(bool sw);
  void                     set_lcd_and_tf_voltage(uint16_t voltage);
  void                     set_vib_motor_voltage(uint16_t voltage);
  void                     set_bat_charge(bool enable);
  void                     power_off(void);
  bool                     set_charger_term_current_to_zero(void);
  bool                     setConstChargeCurrentLimit_to_50mA(void);
  bool                     enable_pwrok_resets(void);

  void                     set_IRQ_enable_0(uint8_t val);


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
