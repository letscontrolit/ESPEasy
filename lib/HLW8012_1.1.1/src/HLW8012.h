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

#include "HLW8012_sample.h"

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
// Higher values allow for a better precision but reduce sampling rate
// and response speed to change
// Lower values increase sampling rate but reduce precision
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

    #if ESP_IDF_VERSION_MAJOR >= 5
        void cf_interrupt();
        void cf1_interrupt();
    #else
        void cf_interrupt() IRAM_ATTR;
        void cf1_interrupt() IRAM_ATTR;
    #endif

        void begin(
            unsigned char cf_pin,
            unsigned char cf1_pin,
            unsigned char sel_pin,
            unsigned char currentWhen = HIGH,
            unsigned long pulse_timeout = PULSE_TIMEOUT);

        void setMode(hlw8012_mode_t mode);

        hlw8012_mode_t getMode();
        hlw8012_mode_t toggleMode();

        float getCurrent(bool &valid);
    private:
        float getCF1Current(bool &valid);
    public:
        float getVoltage(bool &valid);
        float getActivePower(bool &valid);
        float getApparentPower(bool &valid);
        float getPowerFactor(bool &valid);
        float getReactivePower(bool &valid);
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

        // Perform some IIR filtering
        // new = (old + 3 * new) / 4
        static unsigned long filter(unsigned long oldvalue, unsigned long newvalue) IRAM_ATTR;

        unsigned char _cf_pin{};
        unsigned char _cf1_pin{};
        unsigned char _sel_pin{};

        float _current_resistor = R_CURRENT;
        float _voltage_resistor = R_VOLTAGE;

        float _current_multiplier{}; // Unit: us/A
        float _voltage_multiplier{}; // Unit: us/V
        float _power_multiplier{};   // Unit: us/W

        long _pulse_timeout = PULSE_TIMEOUT;    //Unit: us
        HLW8012_sample _power_sample{}; //Unit: us
        HLW8012_sample _voltage_sample{}; //Unit: us
        HLW8012_sample _current_sample{}; //Unit: us

        float _current{};
        float _cf1_current{};
        float _voltage{};
        float _power{};

        unsigned char _current_mode = HIGH;
        HLW8012_VOLATILE_UCHAR _mode{};
        HLW8012_VOLATILE_UINT32 _cf_pulse_count_total{};

        void _checkCFSignal();
        void _checkCF1Signal();
        void _calculateDefaultMultipliers();

};

#endif
