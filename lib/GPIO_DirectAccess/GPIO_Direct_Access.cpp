#include "GPIO_Direct_Access.h"

#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP8266)
  # ifndef CORE_POST_3_0_0
    #  define IRAM_ATTR ICACHE_RAM_ATTR
  # endif // ifndef CORE_POST_3_0_0
#endif // if defined(ARDUINO_ARCH_ESP8266)


#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

IO_REG_TYPE DIRECT_pinRead(IO_REG_TYPE pin)
{
  return DIRECT_READ(reg, PIN_TO_BITMASK(pin));
}

void DIRECT_pinWrite(IO_REG_TYPE pin, bool pinstate)
{
  if (pinstate) { DIRECT_WRITE_HIGH(reg, PIN_TO_BITMASK(pin)); }
  else { DIRECT_WRITE_LOW(reg, PIN_TO_BITMASK(pin)); }
}

void  DIRECT_PINMODE_OUTPUT(IO_REG_TYPE pin)
{
  DIRECT_MODE_OUTPUT(reg, PIN_TO_BITMASK(pin));
}

void  DIRECT_PINMODE_INPUT(IO_REG_TYPE pin)
{
  DIRECT_MODE_INPUT(reg, PIN_TO_BITMASK(pin));
}

IO_REG_TYPE IRAM_ATTR DIRECT_pinRead_ISR(IO_REG_TYPE pin)
{
  return DIRECT_READ(reg, PIN_TO_BITMASK(pin));
}

void IRAM_ATTR DIRECT_pinWrite_ISR(IO_REG_TYPE pin, bool pinstate)
{
  if (pinstate) { DIRECT_WRITE_HIGH(reg, PIN_TO_BITMASK(pin)); }
  else { DIRECT_WRITE_LOW(reg, PIN_TO_BITMASK(pin)); }
}

void  IRAM_ATTR DIRECT_PINMODE_OUTPUT_ISR(IO_REG_TYPE pin)
{
  DIRECT_MODE_OUTPUT(reg, PIN_TO_BITMASK(pin));
}

void  IRAM_ATTR DIRECT_PINMODE_INPUT_ISR(IO_REG_TYPE pin)
{
  DIRECT_MODE_INPUT(reg, PIN_TO_BITMASK(pin));
}

#endif // if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
