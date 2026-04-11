#include "../Helpers/ESPEasy_UnitOfMeasure.h"

#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
# include "../DataTypes/SensorVType.h"

// *** DO NOT CHANGE ORDER, SAVED IN TASK SETTINGS! ***
const char unit_of_measure_list[] PROGMEM =
  "|"                                                                             // 0 = Empty/none
  "°C|°F|K|"                                                                      // 1..3
  "%|"                                                                            // 4
  "Pa|hPa|bar|mbar|inHg|psi|"                                                     // 5..10
  "W|kW|"                                                                         // 11..12
  "V|"                                                                            // 13
  "Wh|kWh|"                                                                       // 14..15
  "A|VA|"                                                                         // 16..17
  "mm|cm|m|km|"                                                                   // 18..21
  "L|mL|m³|ft³|"                                                                  // 22..25
  "m³/h|ft³/h|"                                                                   // 26..27
  "lx|"                                                                           // 28
  "UV index|"                                                                     // 29
  "µg/m³|mg/m³|p/m³|ppm|ppb|"                                                     // 30..34
  "°|"                                                                            // 35
  "€|$|¢|"                                                                        // 36..38
  "μs|ms|s|min|h|d|w|m|y|"                                                        // 39..47
  "in|ft|yd|mi|"                                                                  // 48..51
  "Hz|GHz|"                                                                       // 52..53
  "gal|fl. oz|"                                                                   // 54..55
  "m²|"                                                                           // 56
  "g|kg|mg|µg|"                                                                   // 57..60
  "oz|lb|"                                                                        // 61..62
  "µS/cm|"                                                                        // 63
  "W/m²|"                                                                         // 64
  "mm/h|"                                                                         // 65
  "mm/s|in/s|m/s|in/h|km/h|mph|"                                                  // 66..71
  "dB|dBm|"                                                                       // 72..73
  "bit|kbit|Mbit|Gbit|B|kB|MB|GB|TB|PB|EB|ZB|YB|KiB|MiB|GiB|TiB|PiB|EiB|ZiB|YiB|" // 74..94
  "bit/s|kbit/s|Mbit/s|Gbit/s|B/s|kB/s|MB/s|GB/s|KiB/s|MiB/s|GiB/s|"              // 95..105
  "ft/s|kn|"                                                                      // 106..107
  "mW|MW|GW|TW|"                                                                  // 108..111
  "BTU/(h⋅ft²)|"                                                                  // 112
  "pH|"                                                                           // 113
  "cbar|mmHg|kPa|"                                                                // 114..116
  "mA|µA|mV|µV|kV|"                                                               // 117..121
  "cm²|km²|mm²|in²|ft²|yd²|mi²|ac|ha|"                                            // 122..130
  "kHz|MHz|"                                                                      // 131..132
  "mWh|MWh|GWh|TWh|cal|kcal|Mcal|Gcal|J|kJ|MJ|GJ|"                                // 133..144
  "var|kvar|varh|kvarh|"                                                          // 145..148
  "st|"                                                                           // 149
  "mg/dL|mmol/L|"                                                                 // 150..151
  "μSv|μSv/h|"                                                                    // 152..153
  "m³/s|ft³/min|L/h|L/min|L/s|gal/min|mL/s|"                                      // 154..160
  "g/m³|kWh/100km|Wh/km|mi/kWh|km/kWh|"                                           // 161..165
  "in/d|mm/d|"                                                                    // 166 .. 167
  "Ah|"                                                                           // 168
;                                                                                 // *** DO NOT CHANGE ORDER, SAVED IN TASK SETTINGS! ***


// Not stored, when UoM index >= 1024 it's a label-index with 1024 subtracted
const char unit_of_measure_labels[] PROGMEM =
  "Apparent power|Air quality/CO/CO2|Area|(Atmospheric) Pressure|" // A 1024..1027
  "Blood glucose concentr.|"                                       // B 1028
  "Data rate|Data size|Distance|Duration|"                         // D 1029..1032
  "Energy distance|Energy(-storage)|"                              // E 1033..1034
  "Frequency|"                                                     // F 1035
  "Gas|"                                                           // G 1036
  "Percent Hum./Batt./Moist.|"                                     // H 1037
  "Illuminance|Irradiance|"                                        // I 1038..1039
  "Monetary|"                                                      // M 1040
  "Nitrogen (di-/mon-)oxide|"                                      // N 1041
  "Voc/Ozone|"                                                     // O 1042
  "Ph|PM/CO/CO2/NO(x)/Voc/Ozone|Power|"                            // P 1043..1045
  "Radiation|Reactive energy/power|"                               // R 1046..1047
  "Signal strength|Sound pressure|Speed|"                          // S 1048..1050
  "Temperature|"                                                   // T 1051
  "Voltage/Current|Volume/Water cons.|Volume flow rate|"           // V 1052..1054
  "Weight|Wind direction/angle|"                                   // W 1055..1056
  "Various units|"                                                 // Additional 1057
;

const uint16_t unit_of_measure_map[] PROGMEM = {
  1051, 1,   2,    3,                                                                                                    // Temperature
  1037, 4,                                                                                                               // Percent Battery,
                                                                                                                         // Humidity,
                                                                                                                         // Moisture
  1027, 8,   6,    116,    7,      115,     10,     5,      114,    9,                                                   // (Atmospheric)
                                                                                                                         // Pressure
  1052, 13,  119,  120,    121,    16,      117,    118,                                                                 // Voltage/Current
  1045, 11,  12,   108,    109,    110,     111,                                                                         // Power
  1024, 17,                                                                                                              // Apparent power
  1047, 145, 146,  147,    148,    168,                                                                                  // Reactive
                                                                                                                         // power/energy
  1044, 30,  31,   32,     33,     34,                                                                                   // Particle matter
  1031, 18,  19,   20,     21,     48,      49,     50,     51,                                                          // Distance
  1055, 57,  58,   59,     60,     61,      62,     149,                                                                 // Weight
  1053, 22,  23,   24,     25,     54,      55,                                                                          // Volume/Water
  1054, 26,  27,   153,    154,    155,     156,    157,    158,    159,   160,                                          // Volume flow rate
  1032, 39,  40,   41,     42,     43,      44,     45,     46,     47,                                                  // Duration
  1034, 14,  15,   133,    134,    135,     136,    137,    138,    139,   140,   141, 142, 143, 144,                    // Energy(-storage)
  1033, 162, 163,  164,    165,                                                                                          // Energy distance
  1050, 66,  67,   68,     69,     70,      71,     65,     106,    107,                                                 // Speed
  1056, 35,                                                                                                              // (Wind)
                                                                                                                         // direction/angle
  1038, 28,                                                                                                              // Illuminance
  1039, 64,  112,                                                                                                        // Irradiance
  1046, 152, 153,                                                                                                        // Radiation
  1057, 29,  63,   161,    166,    167,                                                                                  // Various units
  1035, 52,  53,   131,    132,                                                                                          // Frequency
  1043, 113,                                                                                                             // Potential
                                                                                                                         // hydrogen
  1026, 56,  122,  123,    124,    125,     126,    127,    128,    129,   130,                                          // Area
  1029, 95,  96,   97,     98,     99,      100,    101,    102,    103,   104,   105,                                   // Data rate
  1030, 74,  75,   76,     77,     78,      79,     80,     81,     82,    83,    84,  85,  86,  87,88, 89, 90, 91, 92, 93, 94, // Data size
  1049, 72,  73,                                                                                                         // Sound pressure
  1028, 150, 151,                                                                                                        // Blood glucose
  1040, 36,  37,   38,                                                                                                   // Monetary
};
const uint16_t unit_of_measure_map_size = NR_ELEMENTS(unit_of_measure_map);

String toUnitOfMeasureName(const uint32_t unitOfMeasureIndex,
                           const String & defUoM) {
  char   tmp[26]{}; // "PM/CO/CO2/NO(x)/Voc/Ozone" + \0
  String result;

  if (unitOfMeasureIndex < 1024) {
    result = GetTextIndexed(tmp, sizeof(tmp), unitOfMeasureIndex, unit_of_measure_list);
  } else {
    result = GetTextIndexed(tmp, sizeof(tmp), unitOfMeasureIndex - 1024, unit_of_measure_labels);
  }

  return result.isEmpty() ? defUoM : result;
}

int getUnitOfMeasureIndex(const String& uomName) {
  return GetCommandCode(uomName.c_str(), unit_of_measure_list);
}

/**
 * getUoMGroupForUoM: Get the selector group for the requested Unit of Measurement name
 */
uint16_t getUoMGroupForUoM(const String& uom) {
  return getUoMGroupForUoM(getUnitOfMeasureIndex(uom));
}

/**
 * getUoMGroupForUoM: Get the selector group for the requested Unit of Measurement index
 */
uint16_t getUoMGroupForUoM(const int findUom) {
  if ((findUom < 0)) { return 0; }
  const uint16_t find = findUom;
  uint16_t result     = 0;
  uint16_t last       = 0;

  for (uint16_t idx = 0; idx < unit_of_measure_map_size; ++idx) {
    const uint16_t uomIdx = pgm_read_word_near(&unit_of_measure_map[idx]);

    if (uomIdx < 1024) {
      if (find == uomIdx) {
        result = last;
        break;
      }
    } else {
      last = uomIdx;
    }
  }
  return result;
}

/**
 * getDefaultUoMforSensorVType: get the default UoM parameters into Par64[] array for Sensor_VType::SENSOR_TYPE_TEMP_HUM,
 * Sensor_VType::SENSOR_TYPE_TEMP_BARO, Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO and Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO
 */
bool getDefaultUoMforSensorVType(EventStruct *event) {
  switch (event->sensorType) { // These are the only default UoM values to be handled
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM:
      event->Par64_1 = UOM_GROUP_TEMPERATURE;
      event->Par64_2 = UOM_GROUP_PERC_HUM_BAT_MOIST;
      return true;
    case Sensor_VType::SENSOR_TYPE_TEMP_BARO:
      event->Par64_1 = UOM_GROUP_TEMPERATURE;
      event->Par64_2 = UOM_GROUP_PRESSURE;
      return true;
    case Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO:
      event->Par64_1 = UOM_GROUP_TEMPERATURE;
      event->Par64_2 = UOM_GROUP_NONE;
      event->Par64_3 = UOM_GROUP_PRESSURE;
      return true;
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO:
      event->Par64_1 = UOM_GROUP_TEMPERATURE;
      event->Par64_2 = UOM_GROUP_PERC_HUM_BAT_MOIST;
      event->Par64_3 = UOM_GROUP_PRESSURE;
      return true;
    default:
      break;
  }
  return false;
}

#endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE
