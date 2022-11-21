/*

HLW8012

Copyright (C) 2016-2018 by Xose Pérez <xose dot perez at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef HLW8012_h
#define HLW8012_h

#include <Arduino.h>

// Internal voltage reference value
#define V_REF               2.43

// The factor of a 1mOhm resistor
// as per recomended circuit in datasheet
// A 1mOhm resistor allows a ~30A max measurement
#define R_CURRENT           0.001

// This is the factor of a voltage divider of 6x 470K upstream and 1k downstream
// as per recomended circuit in datasheet
#define R_VOLTAGE           2821

// Frequency of the HLW8012 internal clock
#define F_OSC               3579000

// Minimum delay between selecting a mode and reading a sample
#define READING_INTERVAL    3000

// Maximum pulse with in microseconds
// If longer than this pulse width is reset to 0
// This value is purely experimental.
// Higher values allow for a better precission but reduce sampling rate
// and response speed to change
// Lower values increase sampling rate but reduce precission
// Values below 0.5s are not recommended since current and voltage output
// will have no time to stabilise
#define PULSE_TIMEOUT       2000000

// Define ICACHE_RAM_ATTR for AVR platforms
#if defined(ARDUINO_ARCH_AVR)
#define ICACHE_RAM_ATTR     
#endif

// CF1 mode
typedef enum {
    MODE_CURRENT,
    MODE_VOLTAGE
} hlw8012_mode_t;

class HLW8012 {

    public:

        void cf_interrupt();
        void cf1_interrupt();

        void begin(
            unsigned char cf_pin,
            unsigned char cf1_pin,
            unsigned char sel_pin,
            unsigned char currentWhen = HIGH,
            bool use_interrupts = true,
            unsigned long pulse_timeout = PULSE_TIMEOUT);

        void setMode(hlw8012_mode_t mode);

        hlw8012_mode_t getMode();
        hlw8012_mode_t toggleMode();

        float getCurrent();
        float getVoltage();
        float getActivePower();
        float getApparentPower();
        float getPowerFactor();
        float getReactivePower();
        float getEnergy(); //in Ws
        void resetEnergy();

        void setResistors(float current, float voltage_upstream, float voltage_downstream);

        void expectedCurrent(float current);
        void expectedVoltage(float current);
        void expectedActivePower(float power);

        float getCurrentMultiplier() { return _current_multiplier; };
        float getVoltageMultiplier() { return _voltage_multiplier; };
        float getPowerMultiplier() { return _power_multiplier; };

        void setCurrentMultiplier(float current_multiplier) { _current_multiplier = current_multiplier; };
        void setVoltageMultiplier(float voltage_multiplier) { _voltage_multiplier = voltage_multiplier; };
        void setPowerMultiplier(float power_multiplier) { _power_multiplier = power_multiplier; };
        void resetMultipliers();

    private:

        unsigned char _cf_pin = 0;
        unsigned char _cf1_pin = 0;
        unsigned char _sel_pin = 0;

        float _current_resistor = R_CURRENT;
        float _voltage_resistor = R_VOLTAGE;

        float _current_multiplier = 0.0; // Unit: us/A
        float _voltage_multiplier = 0.0; // Unit: us/V
        float _power_multiplier = 0.0;   // Unit: us/W

        unsigned long _pulse_timeout = PULSE_TIMEOUT;    //Unit: us
        volatile unsigned long _voltage_pulse_width = 0; //Unit: us
        volatile unsigned long _current_pulse_width = 0; //Unit: us
        volatile unsigned long _power_pulse_width = 0;   //Unit: us

        float _current = 0;
        float _voltage = 0;
        float _power = 0;

        unsigned char _current_mode = HIGH;
        volatile unsigned char _mode = 0;

        bool _use_interrupts = true;
        volatile unsigned long _last_cf_interrupt = 0;
        volatile unsigned long _last_cf1_interrupt = 0;
        volatile unsigned long _first_cf1_interrupt = 0;
        volatile unsigned long _cf_pulse_count = 0;
        volatile unsigned long _cf1_pulse_count = 0;

        void _checkCFSignal();
        void _checkCF1Signal();
        void _calculateDefaultMultipliers();

};

#endif
