//  This sketch is based on the SDK example here:
//  https://github.com/raspberrypi/pico-examples/tree/master/pio/ws2812

/**
   Copyright (c) 2020 Raspberry Pi (Trading) Ltd.

   SPDX-License-Identifier: BSD-3-Clause
*/

#if defined(ARDUINO_ARCH_RP2040)

#include <stdlib.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "rp2040_pio.h"

void rp2040Init(uint8_t pin, bool is800KHz)
{
    // todo get free sm
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    if (is800KHz)
    {
        // 800kHz, 8 bit transfers
        ws2812_program_init(pio, sm, offset, pin, 800000, 8);
    }
    else
    {
        // 400kHz, 8 bit transfers
        ws2812_program_init(pio, sm, offset, pin, 400000, 8);
    }
}
 
void  rp2040Show(uint8_t pin, uint8_t *pixels, uint32_t numBytes, bool is800KHz)
{
    static bool init = true;
    
    if (init)
    {
        // On first pass through initialise the PIO
        rp2040Init(pin, is800KHz);
        init = false;
    }

    while(numBytes--)
        // Bits for transmission must be shifted to top 8 bits
        pio_sm_put_blocking(pio0, 0, ((uint32_t)*pixels++)<< 24);
}

#endif // KENDRYTE_K210
