#pragma once

#include "ESPEasy_common.h"

uint8_t getIndexFromBitmap(const uint16_t _bitmap[],
                           uint8_t        _index,
                           uint8_t        _max,
                           size_t       & _highest);

uint8_t getIdFromBitmap(const uint16_t _bitmap[],
                        uint8_t        _index,
                        uint8_t        _max,
                        uint8_t        _invalid = 255u);
