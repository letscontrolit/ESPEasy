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

#include <Arduino.h>
#include "HLW8012.h"

#include <GPIO_Direct_Access.h>



void HLW8012::begin(
    unsigned char cf_pin,
    unsigned char cf1_pin,
    unsigned char sel_pin,
    unsigned char currentWhen,
    unsigned long pulse_timeout
    ) {

    _cf_pin = cf_pin;
    _cf1_pin = cf1_pin;
    _sel_pin = sel_pin;
    _current_mode = currentWhen;
    _pulse_timeout = pulse_timeout;

    pinMode(_cf_pin, INPUT_PULLUP);
    pinMode(_cf1_pin, INPUT_PULLUP);
    pinMode(_sel_pin, OUTPUT);

    _calculateDefaultMultipliers();

    _mode = _current_mode;
    _current_sample.reset();
    digitalWrite(_sel_pin, _mode);
}

void HLW8012::setMode(hlw8012_mode_t mode) {
    const unsigned char newMode = (mode == MODE_CURRENT) ? _current_mode : 1 - _current_mode;
    if (_mode == newMode) {
      return;
    }
    if (newMode == _current_mode) {
        _current_sample.reset();
    } else {
        _voltage_sample.reset();
    }
    _mode = newMode;
    DIRECT_pinWrite(_sel_pin, _mode);
}

hlw8012_mode_t HLW8012::getMode() {
    return (_mode == _current_mode) ? MODE_CURRENT : MODE_VOLTAGE;
}

hlw8012_mode_t HLW8012::toggleMode() {
    hlw8012_mode_t new_mode = getMode() == MODE_CURRENT ? MODE_VOLTAGE : MODE_CURRENT;
    setMode(new_mode);
    return new_mode;
}

float HLW8012::getCurrent(bool &valid) {

    // Power measurements are more sensitive to switch offs,
    // so we first check if power is 0 to set _current to 0 too
    getActivePower(valid);
    if (_power == 0) {
        _current = 0;
        return _current;
    }
    if (valid) {
        const float voltage = getVoltage(valid);
        if (valid && voltage > 1) {
            _current = _power / voltage;
            return _current;
        }
    } 
    return _current;
}

float HLW8012::getCF1Current(bool &valid) {
    _checkCF1Signal();

    if (!_current_sample.updated(valid)) {
        return _cf1_current;
    }

    float pulsefreq{};
    if (_current_sample.getPulseFreq(pulsefreq)) {
      _cf1_current = pulsefreq * _current_multiplier / 2.0f;

      // Add limit for CF1 current 
      // as it is highly unlikely any cos-phi will ever be worse than 0.25 
      bool tmpvalid{};
      const float current = getCurrent(tmpvalid);
      const float upperLimit = 4.0f * current;
      if (upperLimit < _cf1_current) {
          _cf1_current = upperLimit;
      } else if (current > _cf1_current) {
        // Cos-phi will never be > 1.0, so set it to the same 
        // as current computed from the CF pin and voltage
        _cf1_current = current;
      }
    } else {
      _cf1_current = 0.0f;
      // Consider always valid, as the current will be 0 
      // when we received no pulse for a while
      _current_sample.setValid(true);
    }
    
    return _cf1_current;
}


float HLW8012::getVoltage(bool &valid) {
    _checkCF1Signal();
    if (!_voltage_sample.updated(valid)) {
        return _voltage;
    }
    float pulsefreq{};
    // Do not automatically claim voltage is valid as it is highly unlikely the voltage will ever be 0.
    // The device will then not be powered.
    valid = _voltage_sample.getPulseFreq(pulsefreq);
    _voltage = valid
       ? pulsefreq * _voltage_multiplier / 2.0f
       : 0.0;
    return _voltage;
}

float HLW8012::getActivePower(bool &valid) {
    _checkCFSignal();
    if (!_power_sample.updated(valid)) {
        return _power;
    }
    float pulsefreq{};
    valid = _power_sample.getPulseFreq(pulsefreq);
    _power = valid 
      ? pulsefreq * _power_multiplier / 2.0f
      : 0.0f;
    return _power;
}

float HLW8012::getApparentPower(bool &valid) {
    bool valid_cur, valid_volt = false;
    float current = getCF1Current(valid_cur);
    if (!valid_cur) {
        current = getCurrent(valid_cur);
    }
    const float voltage = getVoltage(valid_volt);
    valid = valid_cur && valid_volt;
    return voltage * current;
}

float HLW8012::getReactivePower(bool &valid) {
    bool valid_active, valid_apparent = false;
    const float active = getActivePower(valid_active);
    const float apparent = getApparentPower(valid_apparent);
    valid = valid_active && valid_apparent;
    if (valid && (apparent > active)) {
        return sqrtf((apparent * apparent) - (active * active));
    } else {
        return 0.0f;
    }
}

float HLW8012::getPowerFactor(bool &valid) {
    bool valid_active, valid_apparent = false;
    const float active = getActivePower(valid_active);
    const float apparent = getApparentPower(valid_apparent);
    valid = valid_active && valid_apparent;
    if (!valid) return 0.0f;
    if (active > apparent) return 1.0f;
    if (static_cast<int>(apparent) == 0) return 0.0f;
    return active / apparent;
}

float HLW8012::getEnergy() {
    /*
    Pulse count is directly proportional to energy:
    P = m*f (m=power multiplier, f = Frequency)
    f = N/t (N=pulse count, t = time)
    E = P*t = m*N  (E=energy)
    */
    const float pulse_count = _cf_pulse_count_total;
    return pulse_count * _power_multiplier / 1000000.0f / 2.0f;
}

void HLW8012::resetEnergy() {
    _cf_pulse_count_total = 0;
}

void HLW8012::expectedCurrent(float value) {
    bool valid = false;
    if (static_cast<int>(_current) == 0) getCurrent(valid);
    const float current = _current; // Copy volatile
    if (valid && static_cast<int>(current) > 0) _current_multiplier *= (value / current);
}

void HLW8012::expectedVoltage(float value) {
    bool valid = false;
    if (static_cast<int>(_voltage) == 0) getVoltage(valid);
    const float voltage = _voltage;
    if (valid && static_cast<int>(voltage) > 0) _voltage_multiplier *= (value / voltage);
}

void HLW8012::expectedActivePower(float value) {
    bool valid = false;
    if (static_cast<int>(_power) == 0) getActivePower(valid);
    const float power = _power;
    if (valid && static_cast<int>(power) > 0) _power_multiplier *= (value / power);
}

void HLW8012::resetMultipliers() {
    _calculateDefaultMultipliers();
}

void HLW8012::setResistors(float current, float voltage_upstream, float voltage_downstream) {
    if (voltage_downstream > 0) {
        if (current > 0.0f) {
          _current_resistor = current;
        }
        _voltage_resistor = (voltage_upstream + voltage_downstream) / voltage_downstream;
        _calculateDefaultMultipliers();
    }
}

unsigned long IRAM_ATTR HLW8012::filter(unsigned long oldvalue, unsigned long newvalue) {
    if (oldvalue == 0) {
        return newvalue;
    }

    oldvalue += 3 * newvalue;
    oldvalue >>= 2;
    return oldvalue;
}


void IRAM_ATTR HLW8012::cf_interrupt() {
  	++_cf_pulse_count_total;
    
    const auto res = _power_sample.add();
    if (res == HLW8012_sample::result_e::Enough ||
        res == HLW8012_sample::result_e::Expired) {
        _power_sample.reset();
    }
}

void IRAM_ATTR HLW8012::cf1_interrupt() {
    const unsigned char mode = _mode;

    const auto res = (mode == _current_mode) 
        ? _current_sample.add()
        : _voltage_sample.add();
    
    if (res == HLW8012_sample::result_e::NotEnough) {
        return;
    }

    // switch to other mode.
    const unsigned char newMode = 1 - mode;
    if (newMode == _current_mode) {
        _current_sample.reset();
    } else {
        _voltage_sample.reset();
    }
    DIRECT_pinWrite_ISR(_sel_pin, newMode);
    _mode = newMode;
}

void HLW8012::_checkCFSignal() {
    const auto res = _power_sample.getState();
    if (res == HLW8012_sample::result_e::Expired) {
        // Mark as invalid
        _power_sample.reset();
    }
}

void HLW8012::_checkCF1Signal() {
    const auto res = (_mode == _current_mode) 
        ? _current_sample.getState()
        : _voltage_sample.getState();
    if (res == HLW8012_sample::result_e::Expired) {
        // Mark as invalid
        // Switch to other
        toggleMode();
    }
}

// These are the multipliers for current, voltage and power as per datasheet
// These values divided by output period (in useconds) give the actual value
// For power a frequency of 1Hz means around 12W
// For current a frequency of 1Hz means around 15mA
// For voltage a frequency of 1Hz means around 0.5V
void HLW8012::_calculateDefaultMultipliers() {
    constexpr float current_factor = 1000000.0 * 512 * V_REF / 24.0 / F_OSC;
    _current_multiplier = current_factor / _current_resistor;

    constexpr float voltage_factor = 1000000.0 * 512 * V_REF / 2.0 / F_OSC;
    _voltage_multiplier = voltage_factor * _voltage_resistor;

    constexpr float power_factor = 1000000.0 * 128 * V_REF * V_REF / 48.0 / F_OSC;
    _power_multiplier = power_factor * _voltage_resistor / _current_resistor;
}
