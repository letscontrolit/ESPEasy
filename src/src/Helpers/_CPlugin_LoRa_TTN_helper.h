#ifndef HELPERS__CPLUGIN_LORA_TTN_HELPER_H
#define HELPERS__CPLUGIN_LORA_TTN_HELPER_H

#include "../../ESPEasy_common.h"


// #######################################################################################################
// #  Helper functions to encode data for use on LoRa/TTN network.
// #######################################################################################################

#if defined(USES_PACKED_RAW_DATA)

#include "../DataStructs/ESPEasy_packed_raw_data.h"


String getPackedFromPlugin(struct EventStruct *event, uint8_t sampleSetCount);

// Compute the air time for a packet in msec.
// Formula used from https://www.loratools.nl/#/airtime
// @param pl   Payload length in bytes
// @param sf   Spreading factor 7 - 12
// @param bw   Bandwidth 125 kHz default for LoRaWAN. 250 kHz also supported.
// @param cr   Code Rate 4 / (CR + 4) = 4/5.  4/5 default for LoRaWAN
// @param n_preamble Preamble length Default for frame = 8, beacon = 10
// @param header    Explicit header Default on for LoRaWAN
// @param crc       CRC Default on for LoRaWAN
float getLoRaAirTime(uint8_t  pl,
                     uint8_t  sf,
                     uint16_t bw         = 125,
                     uint8_t  cr         = 1,
                     uint8_t  n_preamble = 8,
                     bool     header     = true,
                     bool     crc        = true);

#endif // USES_PACKED_RAW_DATA




#endif