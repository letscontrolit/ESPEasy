#pragma once

#if FEATURE_TASKVALUE_UNIT_OF_MEASURE

# include "../DataStructs/ESPEasy_EventStruct.h"
# include "../Helpers/StringConverter.h"

const uint64_t UOM_GROUP_APPARENT_POWER     = 0x0000000000000001; // 1024
const uint64_t UOM_GROUP_AIR_QUALITY        = 0x0000000000000002;
const uint64_t UOM_GROUP_AREA               = 0x0000000000000004;
const uint64_t UOM_GROUP_PRESSURE           = 0x0000000000000008;
const uint64_t UOM_GROUP_BLOOD_GLUCOSE      = 0x0000000000000010;
const uint64_t UOM_GROUP_DATA_RATE          = 0x0000000000000020;
const uint64_t UOM_GROUP_DATA_SIZE          = 0x0000000000000040; // 1030
const uint64_t UOM_GROUP_DISTANCE           = 0x0000000000000080;
const uint64_t UOM_GROUP_DURATION           = 0x0000000000000100;
const uint64_t UOM_GROUP_ENERGY_DISTANCE    = 0x0000000000000200;
const uint64_t UOM_GROUP_ENERGY_STORAGE     = 0x0000000000000400;
const uint64_t UOM_GROUP_FREQUENCY          = 0x0000000000000800; // 1035
const uint64_t UOM_GROUP_GAS                = 0x0000000000001000;
const uint64_t UOM_GROUP_PERC_HUM_BAT_MOIST = 0x0000000000002000;
const uint64_t UOM_GROUP_ILLUMINANCE        = 0x0000000000004000;
const uint64_t UOM_GROUP_IRRADIANCE         = 0x0000000000008000;
const uint64_t UOM_GROUP_MONETARY           = 0x0000000000010000; // 1040
const uint64_t UOM_GROUP_NITROGEN_OXIDE     = 0x0000000000020000;
const uint64_t UOM_GROUP_VOC_OZONE          = 0x0000000000040000;
const uint64_t UOM_GROUP_PH                 = 0x0000000000080000;
const uint64_t UOM_GROUP_PM_CO_CO2_NOX      = 0x0000000000100000;
const uint64_t UOM_GROUP_POWER              = 0x0000000000200000; // 1045
const uint64_t UOM_GROUP_RADIATION          = 0x0000000000400000;
const uint64_t UOM_GROUP_REACTIVE_POWER     = 0x0000000000800000;
const uint64_t UOM_GROUP_SIGNAL_STRENGTH    = 0x0000000001000000;
const uint64_t UOM_GROUP_SOUND_PRESSURE     = 0x0000000002000000;
const uint64_t UOM_GROUP_SPEED              = 0x0000000004000000; // 1050
const uint64_t UOM_GROUP_TEMPERATURE        = 0x0000000008000000;
const uint64_t UOM_GROUP_VOLTAGE_CURRENT    = 0x0000000010000000;
const uint64_t UOM_GROUP_VOLUME_WATER_CONS  = 0x0000000020000000;
const uint64_t UOM_GROUP_VOLUME_FLOW_RATE   = 0x0000000040000000;
const uint64_t UOM_GROUP_WEIGHT             = 0x0000000080000000; // 1055
const uint64_t UOM_GROUP_WIND_DIRECTION     = 0x0000000100000000;
const uint64_t UOM_GROUP_VARIOUS_UNITS      = 0x0000000200000000;
const uint64_t UOM_GROUP_ALL                = std::numeric_limits<uint64_t>::max();
const uint64_t UOM_GROUP_NONE               = 0x0;

# define UOM_dB          72
# define UOM_dBm         73
# define UOM_milliVolt   119
# define UOM_Mbps        97 // Mbit/s
# define UOM_usec        39
# define UOM_GHz         53
# define UOM_MHz         132
# define UOM_ppm         33
# define UOM_degC        1
# define UOM_kB          79
# define UOM_Byte        78
# define UOM_percent     4

extern const uint16_t unit_of_measure_map[] PROGMEM;
extern const uint16_t unit_of_measure_map_size;

String   toUnitOfMeasureName(const uint32_t unitOfMeasureIndex,
                             const String & defUoM = EMPTY_STRING);
int      getUnitOfMeasureIndex(const String& uomName);

bool     getDefaultUoMforSensorVType(EventStruct *event);
uint16_t getUoMGroupForUoM(const String& uom);
uint16_t getUoMGroupForUoM(const int findUom);
#endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE
