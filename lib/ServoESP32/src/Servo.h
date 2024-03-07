/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2010, LeafLabs, LLC.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/

/**
 * Arduino srl - www.arduino.org
 * Base on lib for stm32f4 (d2a4a47):
 * https://github.com/arduino-libraries/Servo/blob/master/src/stm32f4/ServoTimers.h
 * 2017 Jul 5: Edited by Jaroslav Páral (jarekparal) - paral@robotikabrno.cz
 */

// clang-format off
#pragma once

#include "Arduino.h"

#if ESP_IDF_VERSION_MAJOR >= 5
#include <driver/ledc.h>
#endif

class ServoBase {
   protected:
    // The main purpose of ServoBase is to make sure that multiple instances of
    // ServoTemplate class with different types share channel_next_free.
    static int channel_next_free;
};

template <class T>
class ServoTemplate : public ServoBase {
// From esp32-hal-ledc.c
#ifdef SOC_LEDC_SUPPORT_HS_MODE
#define LEDC_CHANNELS (SOC_LEDC_CHANNEL_NUM << 1)
#else
#define LEDC_CHANNELS (SOC_LEDC_CHANNEL_NUM)
#endif
   public:
    /**
     * Default min/max pulse widths (in microseconds) and angles
     * (in degrees).  Values chosen for Arduino compatibility.
     * These values  are part of the public API; DO NOT CHANGE THEM.
     */
    static constexpr int DEFAULT_MIN_ANGLE = 0;
    static constexpr int DEFAULT_MAX_ANGLE = 180;

    static const int DEFAULT_MIN_PULSE_WIDTH_US = 544;   // the shortest pulse sent to a servo
    static const int DEFAULT_MAX_PULSE_WIDTH_US = 2400;  // the longest pulse sent to a servo

    static const int DEFAULT_FREQUENCY = 50;

#if ESP_IDF_VERSION_MAJOR >= 5
    static constexpr int TIMER_RESOLUTION = (16 < SOC_LEDC_TIMER_BIT_WIDTH) ? 16 : SOC_LEDC_TIMER_BIT_WIDTH;
#else
    static constexpr int TIMER_RESOLUTION = (16 < SOC_LEDC_TIMER_BIT_WIDE_NUM) ? 16 : SOC_LEDC_TIMER_BIT_WIDE_NUM; //   std::min(16, SOC_LEDC_TIMER_BIT_WIDE_NUM);
#endif
    static constexpr int PERIOD_TICKS = (1 << TIMER_RESOLUTION) - 1;

    static const int CHANNEL_NOT_ATTACHED = -1;

    // Pin number of unattached pins
    static const int PIN_NOT_ATTACHED = -1;

    /**
     * @brief Construct a new ServoTemplate instance.
     *
     * The new instance will not be attached to any pin.
     */
    ServoTemplate() { _resetFields(); }

    /**
     * @brief Destruct a ServoTemplate instance.
     *
     * Call _() and detach().
     */
    ~ServoTemplate() { detach(); }

    /**
     * @brief Associate this instance with a servomotor whose input is
     *        connected to pin.
     * @param pin Pin connected to the servo pulse width input. This
     *            pin must be capable of PWM output (all ESP32 pins).
     *
     * @param channel Channel which is set to ESP32 Arduino function ledcSetup().
     *                Channel must be number between 0 - 15.
     *                It is possible to use automatic channel setup with constant
     *                Servo::CHANNEL_NOT_ATTACHED.
     *
     * @param minAngle Target angle (in degrees or radians) associated with
     *                 minPulseWidthUs.  Defaults to DEFAULT_MIN_ANGLE = 0.
     *
     * @param maxAngle Target angle (in degrees or radians) associated with
     *                 maxPulseWidthUs.  Defaults to DEFAULT_MAX_ANGLE = 180.
     *
     * @param minPulseWidthUs Minimum pulse width to write to pin, in
     *                        microseconds.  This will be associated
     *                        with a minAngle angle.  Defaults to
     *                        DEFAULT_MIN_PULSE_WIDTH_US = 544.
     *
     * @param maxPulseWidthUs Maximum pulse width to write to pin, in
     *                        microseconds.  This will be associated
     *                        with a maxAngle angle. Defaults to
     *                        DEFAULT_MAX_PULSE_WIDTH_US = 2400.
     *
     * @param frequency Frequency in hz to send PWM at.
     *                  Defaults to DEFAULT_FREQUENCY.
     *
     * @sideeffect May set pinMode(pin, PWM).
     *
     * @return true if successful, false when pin doesn't support PWM.
     */
    bool attach(int pin, int channel = CHANNEL_NOT_ATTACHED, T minAngle = DEFAULT_MIN_ANGLE,
                T maxAngle = DEFAULT_MAX_ANGLE, int minPulseWidthUs = DEFAULT_MIN_PULSE_WIDTH_US,
                int maxPulseWidthUs = DEFAULT_MAX_PULSE_WIDTH_US, int frequency = DEFAULT_FREQUENCY) {
        int tempPeriodUs = std::round(1000000.0 / frequency);
        if (tempPeriodUs <= maxPulseWidthUs) {
            return false;
        }
#if ESP_IDF_VERSION_MAJOR < 5
        if (channel == CHANNEL_NOT_ATTACHED) {
            if (channel_next_free == LEDC_CHANNELS) {
                return false;
            }
            _channel = channel_next_free;
            channel_next_free++;
        } else {
            _channel = channel;
        }
#endif

        _pin = pin;
        _minAngle = minAngle;
        _maxAngle = maxAngle;
        _minPulseWidthUs = minPulseWidthUs;
        _maxPulseWidthUs = maxPulseWidthUs;
        _periodUs = tempPeriodUs;

#if ESP_IDF_VERSION_MAJOR < 5
        ledcSetup(_channel, frequency, TIMER_RESOLUTION);
        ledcAttachPin(_pin, _channel);
#else
        ledcDetach(_pin);  // See: https://github.com/espressif/arduino-esp32/issues/9212
        ledcAttach(_pin, frequency, TIMER_RESOLUTION);
#endif
        return true;
    }

    /**
     * @brief Stop driving the servo pulse train.
     *
     * If not currently attached to a motor, this function has no effect.
     *
     * @return true if this call did anything, false otherwise.
     */
    bool detach() {
        if (!this->attached()) {
            return false;
        }
#if ESP_IDF_VERSION_MAJOR < 5
        if (_channel == (channel_next_free - 1))
            channel_next_free--;

        ledcDetachPin(_pin);
#else
        ledcDetach(_pin);
#endif
        _pin = PIN_NOT_ATTACHED;
        return true;
    }

    /**
     * @brief Set the servomotor target angle.
     *
     * @param angle Target angle, in degrees or radians.  If the target
     *              angle is outside the range specified at attach() time, it
     *              will be clamped to lie in that range.
     *
     * @see ServoTemplate::attach()
     */
    void write(T angle) {
        angle = constrain(angle, _minAngle, _maxAngle);
        writeMicroseconds(_angleToUs(angle));
    }

    /**
     * @brief Set the pulse width, in microseconds.
     *
     * @param pulseWidthUs Pulse width to send to the servomotor, in
     *                     microseconds. If outside of the range
     *                     specified at attach() time, it is clamped to
     *                     lie in that range.
     *
     * @see ServoTemplate::attach()
     */
    void writeMicroseconds(int pulseWidthUs) {
        if (!attached()) {
            return;
        }
        pulseWidthUs = constrain(pulseWidthUs, _minPulseWidthUs, _maxPulseWidthUs);
        _pulseWidthTicks = _usToTicks(pulseWidthUs);
#if ESP_IDF_VERSION_MAJOR < 5
        ledcWrite(_channel, _pulseWidthTicks);
#else
        ledcWrite(_pin, _pulseWidthTicks);
#endif
    }

    /**
     * Get the servomotor's target angle, in degrees or radians.  This will
     * lie inside the range specified at attach() time.
     *
     * @see ServoTemplate::attach()
     */
    T read() const { return _usToAngle(readMicroseconds()); }

    /**
     * Get the current pulse width, in microseconds.  This will
     * lie within the range specified at attach() time.
     *
     * @see ServoTemplate::attach()
     */
    int readMicroseconds() const {
        if (!this->attached()) {
            return 0;
        }
#if ESP_IDF_VERSION_MAJOR < 5
        const int duty = ledcRead(_channel);
#else
        const int duty = ledcRead(_pin);
#endif
        return _ticksToUs(duty);
    }

    /**
     * @brief Check if this instance is attached to a servo.
     * @return true if this instance is attached to a servo, false otherwise.
     * @see ServoTemplate::attachedPin()
     */
    bool attached() const { return _pin != PIN_NOT_ATTACHED; }

    /**
     * @brief Get the pin this instance is attached to.
     * @return Pin number if currently attached to a pin, PIN_NOT_ATTACHED
     *         otherwise.
     * @see ServoTemplate::attach()
     */
    int attachedPin() const { return _pin; }

   private:
    void _resetFields(void) {
        _pin = PIN_NOT_ATTACHED;
        _pulseWidthTicks = 0;
#if ESP_IDF_VERSION_MAJOR < 5
        _channel = CHANNEL_NOT_ATTACHED;
#endif
        _minAngle = DEFAULT_MIN_ANGLE;
        _maxAngle = DEFAULT_MAX_ANGLE;
        _minPulseWidthUs = DEFAULT_MIN_PULSE_WIDTH_US;
        _maxPulseWidthUs = DEFAULT_MAX_PULSE_WIDTH_US;
        _periodUs = 1000000 / DEFAULT_FREQUENCY;
    }

    T mapTemplate(T x, T in_min, T in_max, T out_min, T out_max) const {
        // FIXME TD-er: Disable this check till we have C++17
        /*
        if constexpr (std::is_floating_point_v<T>) {
            return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        } else */
        {
            // Use normal map with integers, because extra care is needed.
            return map(x, in_min, in_max, out_min, out_max);
        }
    }

    int _usToTicks(int us) const { return std::round((PERIOD_TICKS * us) / _periodUs); }
    int _ticksToUs(int duty) const { return std::round((_periodUs * duty) / PERIOD_TICKS); }
    T _usToAngle(int us) const { return mapTemplate((T)us, (T)_minPulseWidthUs, (T)_maxPulseWidthUs, _minAngle, _maxAngle); }
    int _angleToUs(T angle) const {
        return (int)mapTemplate(angle, _minAngle, _maxAngle, _minPulseWidthUs, _maxPulseWidthUs);
    }

    int _pin;
    int _pulseWidthTicks;
#if ESP_IDF_VERSION_MAJOR < 5
    int _channel;
#endif
    int _minPulseWidthUs, _maxPulseWidthUs;
    T _minAngle, _maxAngle;
    int _periodUs;
};

// For backwards compatability, naming the int version simply "Servo" allow
// users to upgrade library without complications.
using Servo = ServoTemplate<int>;

// Use ServoFloat for float precision
using ServoFloat = ServoTemplate<float>;

// Use ServoDouble for double precision
using ServoDouble = ServoTemplate<double>;
