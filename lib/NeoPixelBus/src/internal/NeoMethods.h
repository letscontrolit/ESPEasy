/*-------------------------------------------------------------------------
NeoMethods includes all the classes that describe pulse/data sending methods using
bitbang, SPI, or other platform specific hardware peripherl support.  

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
#pragma once

// Generic Two Wire (clk and data) methods
//
#include "methods/DotStarGenericMethod.h"
#include "methods/Lpd8806GenericMethod.h"
#include "methods/Lpd6803GenericMethod.h"
#include "methods/Ws2801GenericMethod.h"
#include "methods/P9813GenericMethod.h"
#include "methods/Tlc5947GenericMethod.h"
#include "methods/Sm16716GenericMethod.h"
#include "methods/Mbi6033GenericMethod.h"

// Platform specific and One Wire (data) methods
//
#if defined(ARDUINO_ARCH_ESP8266)

#include "methods/NeoEsp8266DmaMethod.h"
#include "methods/NeoEsp8266I2sDmx512Method.h"
#include "methods/NeoEsp8266UartMethod.h"
#include "methods/NeoEspBitBangMethod.h"

#elif defined(ARDUINO_ARCH_ESP32)

#include "methods/NeoEsp32I2sMethod.h"
#include "methods/NeoEsp32RmtMethod.h"
#include "methods/NeoEspBitBangMethod.h"
#include "methods/DotStarEsp32DmaSpiMethod.h"
#include "methods/NeoEsp32I2sXMethod.h"

#elif defined(ARDUINO_ARCH_NRF52840) // must be before __arm__

#include "methods/NeoNrf52xMethod.h"

#elif defined(__arm__) // must be before ARDUINO_ARCH_AVR due to Teensy incorrectly having it set

#include "methods/NeoArmMethod.h"

#elif defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_MEGAAVR)

#include "methods/NeoAvrMethod.h"

#else
#error "Platform Currently Not Supported, please add an Issue at Github/Makuna/NeoPixelBus"
#endif
