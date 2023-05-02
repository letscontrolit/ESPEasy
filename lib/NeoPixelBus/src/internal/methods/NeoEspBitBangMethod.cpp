/*-------------------------------------------------------------------------
NeoPixel library helper functions for Esp8266 and Esp32

Written by Michael C. Miller.

I invest time and resources providing this open source code,
please support me by dontating (see https://github.com/Makuna/NeoPixelBus)

-------------------------------------------------------------------------
This file is part of the Makuna/NeoPixelBus library.

NeoPixelBus is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

NeoPixelBus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with NeoPixel.  If not, see
<http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------*/

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#include <Arduino.h>

static inline uint32_t getCycleCount(void)
{
    uint32_t ccount;

#if defined(CONFIG_IDF_TARGET_ESP32C3)
    __asm__ __volatile__("csrr %0,0x7e2":"=r" (ccount));
    //ccount = esp_cpu_get_ccount();
#else
    __asm__ __volatile__("rsr %0,ccount":"=a" (ccount));
#endif
    return ccount;
}

void IRAM_ATTR neoEspBitBangWriteSpacingPixels(const uint8_t* pixels,
    const uint8_t* end,
    uint8_t pin,
    uint32_t t0h,
    uint32_t t1h,
    uint32_t period,
    size_t sizePixel,
    uint32_t tSpacing,
    bool invert)
{
    uint32_t setValue = _BV(pin);
    uint32_t clearValue = _BV(pin);
    uint8_t mask = 0x80;
    uint8_t subpix = *pixels++;
    uint8_t element = 0;
    uint32_t cyclesStart = 0; // trigger emediately
    uint32_t cyclesNext = 0;

#if defined(ARDUINO_ARCH_ESP32)
#if defined(CONFIG_IDF_TARGET_ESP32C3)
    volatile uint32_t* setRegister = &GPIO.out_w1ts.val;
    volatile uint32_t* clearRegister = &GPIO.out_w1tc.val;
    setValue = _BV(pin); 
    clearValue = _BV(pin); 
#else
    volatile uint32_t* setRegister = &GPIO.out_w1ts;
    volatile uint32_t* clearRegister = &GPIO.out_w1tc;
#endif // defined(CONFIG_IDF_TARGET_ESP32C3)
#else
    uint32_t setRegister = PERIPHS_GPIO_BASEADDR + GPIO_OUT_W1TS_ADDRESS;
    uint32_t clearRegister = PERIPHS_GPIO_BASEADDR + GPIO_OUT_W1TC_ADDRESS;
    if (pin == 16)
    {
        setRegister = RTC_GPIO_OUT;
        clearRegister = RTC_GPIO_OUT;
        // reading AND writing RTC_GPIO_OUT is too slow inside the loop so
        // we only do writing in the loop
        clearValue = (READ_PERI_REG(RTC_GPIO_OUT) & (uint32)0xfffffffe);
        setValue = clearValue | 1;
    }
#endif // defined(ARDUINO_ARCH_ESP32)

    if (invert)
    {
        std::swap(setRegister, clearRegister);
        std::swap(setValue, clearValue);
    }

    for (;;)
    {
        // do the checks here while we are waiting on time to pass
        uint32_t cyclesBit = t0h;
        if (subpix & mask)
        {
            cyclesBit = t1h;
        }

        // after we have done as much work as needed for this next bit
        // now wait for the HIGH
        while (((cyclesStart = getCycleCount()) - cyclesNext) < period);

        // set pin state
#if defined(ARDUINO_ARCH_ESP32)
        *setRegister = setValue;
#else
        WRITE_PERI_REG(setRegister, setValue);
#endif

        // wait for the LOW
        while ((getCycleCount() - cyclesStart) < cyclesBit);

        // reset pin start
#if defined(ARDUINO_ARCH_ESP32)
        *clearRegister = clearValue;
#else
        WRITE_PERI_REG(clearRegister, clearValue);
#endif

        cyclesNext = cyclesStart;

        // next bit
        mask >>= 1;
        if (mask == 0)
        {
            // no more bits to send in this byte
            // check for another byte
            if (pixels >= end)
            {
                // no more bytes to send so stop
                break;
            }
            // reset mask to first bit and get the next byte
            mask = 0x80;
            subpix = *pixels++;

            // if pixel spacing is needed
            if (tSpacing)
            {
                element++;
                if (element == sizePixel)
                {
                    element = 0;

                    // wait for pixel spacing
                    while ((getCycleCount() - cyclesNext) < tSpacing);
                }
            }
        }
    }
}


#endif //  defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
