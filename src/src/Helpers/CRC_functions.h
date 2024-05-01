#ifndef HELPERS_CRC_FUNCTIONS_H
#define HELPERS_CRC_FUNCTIONS_H

#include "../../ESPEasy_common.h"

int           calc_CRC16(const String& text);

int IRAM_ATTR calc_CRC16(const char *ptr,
                         int         count);

uint32_t      calc_CRC32(const uint8_t *data,
                         size_t         length);

uint8_t       calc_CRC8(const uint8_t *data,
                        size_t         length);

bool          calc_CRC8(uint8_t MSB,
                        uint8_t LSB,
                        uint8_t CRC);


#endif // ifndef HELPERS_CRC_FUNCTIONS_H
