#ifndef ESPEASY_PACKED_RAW_DATA_H
#define ESPEASY_PACKED_RAW_DATA_H

#include "../../ESPEasy_common.h"

// Data types used in packed encoder.
// p_uint16_1e2 means it is a 16 bit unsigned int, but multiplied by 100 first.
// This allows to store 2 decimals of a floating point value in 8 bits, ranging from 0.00 ... 2.55
// For example p_int24_1e6 is a 24-bit signed value, ideal to store a GPS coordinate
// with 6 decimals using only 3 bytes instead of 4 a normal float would use.
//
// PackedData_uintX_1eY = 0x11XY  (X= #bytes, Y=exponent)
// PackedData_intX_1eY  = 0x12XY  (X= #bytes, Y=exponent)
typedef uint32_t PackedData_enum;
#define PackedData_uint8        0x1110
#define PackedData_uint16       0x1120
#define PackedData_uint24       0x1130
#define PackedData_uint32       0x1140
#define PackedData_uint8_1e3    0x1113
#define PackedData_uint8_1e2    0x1112
#define PackedData_uint8_1e1    0x1111
#define PackedData_uint16_1e5   0x1125
#define PackedData_uint16_1e4   0x1124
#define PackedData_uint16_1e3   0x1123
#define PackedData_uint16_1e2   0x1122
#define PackedData_uint16_1e1   0x1121
#define PackedData_uint24_1e6   0x1136
#define PackedData_uint24_1e5   0x1135
#define PackedData_uint24_1e4   0x1134
#define PackedData_uint24_1e3   0x1133
#define PackedData_uint24_1e2   0x1132
#define PackedData_uint24_1e1   0x1131
#define PackedData_uint32_1e6   0x1146
#define PackedData_uint32_1e5   0x1145
#define PackedData_uint32_1e4   0x1144
#define PackedData_uint32_1e3   0x1143
#define PackedData_uint32_1e2   0x1142
#define PackedData_uint32_1e1   0x1141
#define PackedData_int8         0x1210
#define PackedData_int16        0x1220
#define PackedData_int24        0x1230
#define PackedData_int32        0x1240
#define PackedData_int8_1e3     0x1213
#define PackedData_int8_1e2     0x1212
#define PackedData_int8_1e1     0x1211
#define PackedData_int16_1e5    0x1225
#define PackedData_int16_1e4    0x1224
#define PackedData_int16_1e3    0x1223
#define PackedData_int16_1e2    0x1222
#define PackedData_int16_1e1    0x1221
#define PackedData_int24_1e6    0x1236
#define PackedData_int24_1e5    0x1235
#define PackedData_int24_1e4    0x1234
#define PackedData_int24_1e3    0x1233
#define PackedData_int24_1e2    0x1232
#define PackedData_int24_1e1    0x1231
#define PackedData_int32_1e6    0x1246
#define PackedData_int32_1e5    0x1245
#define PackedData_int32_1e4    0x1244
#define PackedData_int32_1e3    0x1243
#define PackedData_int32_1e2    0x1242
#define PackedData_int32_1e1    0x1241
#define PackedData_pluginid     1
#define PackedData_latLng       2
#define PackedData_hdop         3
#define PackedData_altitude     4
#define PackedData_vcc          5
#define PackedData_pct_8        6

uint8_t getPackedDataTypeSize(PackedData_enum dtype, float& factor, float& offset);

void LoRa_uintToBytes(uint64_t value, uint8_t byteSize, byte *data, uint8_t& cursor);

void LoRa_intToBytes(int64_t value, uint8_t byteSize, byte *data, uint8_t& cursor);

String LoRa_base16Encode(byte *data, size_t size);

String LoRa_addInt(uint64_t value, PackedData_enum datatype);

String LoRa_addFloat(float value, PackedData_enum datatype);


#endif // ESPEASY_PACKED_RAW_DATA_H
