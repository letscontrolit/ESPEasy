#include <ip5306.hpp>
namespace arduino {
// based on
// https://gist.github.com/me-no-dev/7702f08dd578de5efa47caf322250b57

/*
** IP5306 Power Module
*/

/* M5 Defaults
KeyOff: Enabled
BoostOutput: Disabled
PowerOnLoad: Enabled
Charger: Enabled
Boost: Enabled
LowBatShutdown: Enabled
ShortPressBoostSwitch: Disabled
FlashlightClicks: Double Press
BoostOffClicks: Long Press
BoostAfterVin: Open
LongPressTime: 2s
ChargeUnderVoltageLoop: 4.55V
ChargeCCLoop: Vin
VinCurrent: 2250mA
VoltagePressure: 28mV
ChargingFullStopVoltage: 4.17V
LightLoadShutdownTime: 32s
EndChargeCurrentDetection: 500mA
ChargeCutoffVoltage: 4.2V
*/

#define IP5306_REG_SYS_0    0x00
#define IP5306_REG_SYS_1    0x01
#define IP5306_REG_SYS_2    0x02
#define IP5306_REG_CHG_0    0x20
#define IP5306_REG_CHG_1    0x21
#define IP5306_REG_CHG_2    0x22
#define IP5306_REG_CHG_3    0x23
#define IP5306_REG_CHG_4    0x24
#define IP5306_REG_READ_0   0x70
#define IP5306_REG_READ_1   0x71
#define IP5306_REG_READ_2   0x72
#define IP5306_REG_READ_3   0x77
#define IP5306_REG_READ_4   0x78

#define IP5306_LEDS2PCT(byte)  \
  ((byte & 0x01 ? 25 : 0) + \
  (byte & 0x02 ? 25 : 0) + \
  (byte & 0x04 ? 25 : 0) + \
  (byte & 0x08 ? 25 : 0))

int ip5306_get_reg(TwoWire& i2c, uint8_t reg){
    i2c.beginTransmission(ip5306::address);
    i2c.write(reg);
    if(i2c.endTransmission(false) == 0 && i2c.requestFrom(0x75, 1)){
        return i2c.read();
    }
    return -1;
}

int ip5306_set_reg(TwoWire& i2c, uint8_t reg, uint8_t value){
    i2c.beginTransmission(ip5306::address);
    i2c.write(reg);
    i2c.write(value);
    if(i2c.endTransmission(true) == 0){
        return 0;
    }
    return -1;
}

uint8_t ip5306_get_bits(TwoWire& i2c, uint8_t reg, uint8_t index, uint8_t bits){
    int value = ip5306_get_reg(i2c,reg);
    if(value < 0){
        Serial.printf("ip5306_get_bits fail: 0x%02x\n", reg);
        return 0;
    }
    return (value >> index) & ((1 << bits)-1);
}

void ip5306_set_bits(TwoWire& i2c, uint8_t reg, uint8_t index, uint8_t bits, uint8_t value){
    uint8_t mask = (1 << bits) - 1;
    int v = ip5306_get_reg(i2c,reg);
    if(v < 0){
        Serial.printf("ip5306_get_reg fail: 0x%02x\n", reg);
        return;
    }
    v &= ~(mask << index);
    v |= ((value & mask) << index);
    if(ip5306_set_reg(i2c,reg, v)){
        Serial.printf("ip5306_set_bits fail: 0x%02x\n", reg);
    }
}

bool ip5306::key_off() const {
    return ip5306_get_bits(m_i2c,IP5306_REG_SYS_0, 0, 1);
}
void ip5306::key_off(bool value) {
    ip5306_set_bits(m_i2c,IP5306_REG_SYS_0, 0, 1, value);
}
bool ip5306::boost_output() const {
    return ip5306_get_bits(m_i2c,IP5306_REG_SYS_0, 1, 1);
}
void ip5306::boost_output(bool value) {
    ip5306_set_bits(m_i2c,IP5306_REG_SYS_0, 1, 1, value);
}

bool ip5306::power_on_load() const {
    return ip5306_get_bits(m_i2c,IP5306_REG_SYS_0, 2, 1);
}
void ip5306::power_on_load(bool value) {
    ip5306_set_bits(m_i2c,IP5306_REG_SYS_0, 2, 1, value);
}
bool ip5306::charger() const {
    return ip5306_get_bits(m_i2c,IP5306_REG_SYS_0, 4, 1);

}
void ip5306::charger(bool value) {
    ip5306_set_bits(m_i2c,IP5306_REG_SYS_0, 4, 1, value);
}
bool ip5306::boost() const {
    return ip5306_get_bits(m_i2c,IP5306_REG_SYS_0, 5, 1);
}
void ip5306::boost(bool value) {
    ip5306_set_bits(m_i2c,IP5306_REG_SYS_0, 5, 1, value);
}
bool ip5306::low_battery_shutdown() const {
    return ip5306_get_bits(m_i2c,IP5306_REG_SYS_1, 0, 1);
}
void ip5306::low_battery_shutdown(bool value) {
    ip5306_set_bits(m_i2c,IP5306_REG_SYS_1, 0, 1, value);
}
bool ip5306::boost_after_vin() const {
    return ip5306_get_bits(m_i2c,IP5306_REG_SYS_1, 2, 1);
}
void ip5306::boost_after_vin(bool value) {
    ip5306_set_bits(m_i2c,IP5306_REG_SYS_1, 2, 1, value);
}
bool ip5306::short_press_boot_switch() const {
    return ip5306_get_bits(m_i2c,IP5306_REG_SYS_1, 5, 1);
}
void ip5306::short_press_boot_switch(bool value) {
    return ip5306_set_bits(m_i2c,IP5306_REG_SYS_1, 5, 1, value);
}
ip5306_presses ip5306::flashlight_clicks() const {
    return (ip5306_presses)ip5306_get_bits(m_i2c,IP5306_REG_SYS_1, 6, 1);
}
void ip5306::flashlight_clicks(ip5306_presses value) {
    ip5306_set_bits(m_i2c,IP5306_REG_SYS_1, 6, 1, (int)value);
}
ip5306_presses ip5306::boost_off_clicks() const {
    return (ip5306_presses)(1-ip5306_get_bits(m_i2c,IP5306_REG_SYS_1, 7, 1));
}
void ip5306::boost_off_clicks(ip5306_presses value) {
    ip5306_set_bits(m_i2c,IP5306_REG_SYS_1, 7, 1, 1-((int)value));
}
ip5306_shutdown ip5306::light_load_shutdown_time() const {
    return (ip5306_shutdown)ip5306_get_bits(m_i2c,IP5306_REG_SYS_2, 2, 2);
}
void ip5306::light_load_shutdown_time(ip5306_shutdown value) {
    ip5306_set_bits(m_i2c,IP5306_REG_SYS_2, 2, 2, (int)value);
}
ip5306_press_time ip5306::long_press_time() const {
    return (ip5306_press_time)ip5306_get_bits(m_i2c,IP5306_REG_SYS_2, 4, 1);
}
void ip5306::long_press_time(ip5306_press_time value) {
    ip5306_set_bits(m_i2c,IP5306_REG_SYS_2, 4, 1, (int)value);
}
ip5306_charging_full_voltage ip5306::charging_full_stop_voltage() const {
    return (ip5306_charging_full_voltage)ip5306_get_bits(m_i2c,IP5306_REG_CHG_0, 0, 2);
}
void ip5306::charging_full_stop_voltage(ip5306_charging_full_voltage value) {
    ip5306_set_bits(m_i2c,IP5306_REG_CHG_0, 0, 2, (int)value);
}
float ip5306::charge_under_voltage_loop() const {
    //Automatically adjust the charging current when the voltage of VOUT is greater than the set value
    //Vout=4.45V + (v * 0.05V) (default 4.55V) 
    //When charging at the maximum current, the charge is less than the set value. Slowly reducing the charging current to maintain this voltage
    int v = ip5306_get_bits(m_i2c,IP5306_REG_CHG_1, 2, 3);
    return 4.45+(v*.05);
}
void ip5306::charge_under_voltage_loop(float value) {
    value-=4.45;
    int i = value/.05;
    ip5306_set_bits(m_i2c,IP5306_REG_CHG_1, 2, 3, i);
}
ip5306_end_charge_current ip5306::end_charge_current_detection() const {
    return (ip5306_end_charge_current)ip5306_get_bits(m_i2c,IP5306_REG_CHG_1, 6, 2);
}
void ip5306::end_charge_current_detection(ip5306_end_charge_current value) {
    ip5306_set_bits(m_i2c,IP5306_REG_CHG_1, 6, 2, (int)value);
}
ip5306_voltage_pressure ip5306::voltage_pressure() const {
    return (ip5306_voltage_pressure)ip5306_get_bits(m_i2c,IP5306_REG_CHG_2, 0, 2);
}
void ip5306::voltage_pressure(ip5306_voltage_pressure value) {
    ip5306_set_bits(m_i2c,IP5306_REG_CHG_2, 0, 2, (int)value);
}
ip5306_charge_cutoff_voltage ip5306::charge_cutoff_voltage() const {
    return (ip5306_charge_cutoff_voltage)ip5306_get_bits(m_i2c,IP5306_REG_CHG_2, 2, 2);
}
void ip5306::charge_cutoff_voltage(ip5306_charge_cutoff_voltage value) {
    ip5306_set_bits(m_i2c,IP5306_REG_CHG_2, 2, 2, (int)value);
}
ip5306_source ip5306::charge_cc_loop() const {
    return (ip5306_source)ip5306_get_bits(m_i2c,IP5306_REG_CHG_3, 5, 1);
}
void ip5306::charge_cc_loop(ip5306_source value) {
    ip5306_set_bits(m_i2c,IP5306_REG_CHG_3, 5, 1, (int)value);
}
unsigned ip5306::vin_current() const {
    int v = ip5306_get_bits(m_i2c,IP5306_REG_CHG_4, 0, 5);
    return (v*100)+50;
}
void ip5306::vin_current(unsigned value) {
    //ImA=(v*100)+50 (default 2250mA)
    ip5306_set_bits(m_i2c,IP5306_REG_CHG_4, 0, 5, (value-50)/100);
}
bool ip5306::short_press_detected() const {
    return ip5306_get_bits(m_i2c,IP5306_REG_READ_3, 0, 1);
}
void ip5306::clear_short_press_detected() {
    ip5306_set_bits(m_i2c,IP5306_REG_READ_3, 0, 1, 1);
}
bool ip5306::long_press_detected() const {
    return ip5306_get_bits(m_i2c,IP5306_REG_READ_3, 1, 1);
}
void ip5306::clear_long_press_detected() {
    ip5306_set_bits(m_i2c,IP5306_REG_READ_3, 1, 1, 1);
}
bool ip5306::double_click_detected() const {
    return ip5306_get_bits(m_i2c,IP5306_REG_READ_3, 2, 1);
}
void ip5306::clear_double_click_detected() {
    ip5306_set_bits(m_i2c,IP5306_REG_READ_3, 2, 1, 1);
}
ip5306_source ip5306::power_source() const {
    return (ip5306_source)ip5306_get_bits(m_i2c,IP5306_REG_READ_0, 3, 1);
}
bool ip5306::battery_full() const {
    return ip5306_get_bits(m_i2c,IP5306_REG_READ_1, 3, 1);
}
ip5306_load ip5306::output_load() const {
    return (ip5306_load)ip5306_get_bits(m_i2c,IP5306_REG_READ_2, 2, 1);
}
float ip5306::charge_level() const {
    // LED[0-4] State (inverted)
    return IP5306_LEDS2PCT(((~ip5306_get_bits(m_i2c,IP5306_REG_READ_4, 4, 4)) & 0x0F))/100.0;
}

}