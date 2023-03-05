#pragma once
#include <Arduino.h>
#include <Wire.h>
namespace arduino {
enum struct ip5306_shutdown {
    after_8s=0,
    after_32s=1,
    after_16s=2,
    after_64s=3
};
enum struct ip5306_charging_full_voltage {
    at_4_14v=0,
    at_4_17v=1,
    at_4_185v=2,
    at_4_2v=3,
};
enum struct ip5306_press_time {
    press_2_seconds=0,
    press_3_seconds=1
};
enum struct ip5306_end_charge_current {
    at_200mA = 0,
    at_400mA = 1,
    at_500mA = 2,
    at_600mA = 3
};
enum struct ip5306_voltage_pressure {
    none = 0,
    p_14mV=1,
    p_28mV=2,
    p_42mV=3
};
enum struct ip5306_charge_cutoff_voltage {
    at_4_2v = 0,
    at_4_3v = 1,
    at_4_35v = 2,
    at_4_4v = 3
};
enum struct ip5306_source {
    battery = 0,
    vin = 1
};
enum struct ip5306_load {
    heavy = 0,
    light = 1
};
enum struct ip5306_presses {
    short_press_twice = 0,
    long_press_once = 1
};
class ip5306 {
    TwoWire& m_i2c;
public:
    constexpr static const uint8_t address = 0x75;
    inline ip5306(TwoWire& i2c=Wire) : m_i2c(i2c) {

    }
    bool key_off() const;
    void key_off(bool value);
    bool boost_output() const;
    void boost_output(bool value);
    bool power_on_load() const;
    void power_on_load(bool value);
    bool charger() const;
    void charger(bool value);
    bool boost() const;
    void boost(bool value);
    bool low_battery_shutdown() const;
    void low_battery_shutdown(bool value);
    bool boost_after_vin() const;
    void boost_after_vin(bool value);
    bool short_press_boot_switch() const;
    void short_press_boot_switch(bool value);
    ip5306_presses flashlight_clicks() const;
    void flashlight_clicks(ip5306_presses value);
    ip5306_presses boost_off_clicks() const;
    void boost_off_clicks(ip5306_presses value);
    ip5306_shutdown light_load_shutdown_time() const;
    void light_load_shutdown_time(ip5306_shutdown value);
    ip5306_press_time long_press_time() const;
    void long_press_time(ip5306_press_time value);
    ip5306_charging_full_voltage charging_full_stop_voltage() const;
    void charging_full_stop_voltage(ip5306_charging_full_voltage value);
    float charge_under_voltage_loop() const;
    void charge_under_voltage_loop(float value);
    ip5306_end_charge_current end_charge_current_detection() const;
    void end_charge_current_detection(ip5306_end_charge_current value);
    ip5306_voltage_pressure voltage_pressure() const;
    void voltage_pressure(ip5306_voltage_pressure value);
    ip5306_charge_cutoff_voltage charge_cutoff_voltage() const;
    void charge_cutoff_voltage(ip5306_charge_cutoff_voltage value);
    ip5306_source charge_cc_loop() const;
    void charge_cc_loop(ip5306_source value);
    unsigned vin_current() const;
    void vin_current(unsigned value);
    bool short_press_detected() const;
    void clear_short_press_detected();
    bool long_press_detected() const;
    void clear_long_press_detected();
    bool double_click_detected() const;
    void clear_double_click_detected();
    ip5306_source power_source() const;
    bool battery_full() const;
    ip5306_load output_load() const;
    float charge_level() const;
    
};
}