// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter

#include "../PluginStructs/P050_data_struct.h"


#ifdef USES_P050

# include "../Helpers/ESPEasy_Storage.h"

# include <Adafruit_TCS34725.h>

P050_data_struct::P050_data_struct(uint16_t integrationSetting, uint16_t gainSetting) {

  // Map integration time setting (uint16_t to enum)
  _integrationTime = static_cast<tcs34725IntegrationTime_t>(integrationSetting);

  // Map gain setting (uint16_t -> enum)
  _gain = static_cast<tcs34725Gain_t>(gainSetting);

  /* Initialise with specific int time and gain values */
  tcs = Adafruit_TCS34725(_integrationTime, _gain);

  resetCalibration();

  // String log = F("P050_data sizeof(CalibrationSettings): ");
  // log += sizeof(CalibrationSettings);
  // addLog(LOG_LEVEL_INFO, log);
}

/**
 * resetCalibration
 * Effectgively sets matrix[0][0], matrix[1][1] and matrix[2][2] to 1.0f, all other fields to 0.0f 
 */
void P050_data_struct::resetCalibration() {
  // Initialize calibrationn defaults
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      CalibrationSettings.matrix[i][j] = i == j ? 1.0f : 0.0f;
    }
  }
}

/**
 * applyCalibration : calibrate r/g/b inputs (uint16_t) to rc/gc/bc outputs (float, by reference)
 */
void P050_data_struct::applyCalibration(uint16_t r, uint16_t g, uint16_t b, float *rc, float *gc, float *bc) {
  *rc = CalibrationSettings.matrix[0][0] * (float)r + CalibrationSettings.matrix[0][1] * (float)g + CalibrationSettings.matrix[0][2] * (float)b;
  *gc = CalibrationSettings.matrix[1][0] * (float)r + CalibrationSettings.matrix[1][1] * (float)g + CalibrationSettings.matrix[2][2] * (float)b;
  *bc = CalibrationSettings.matrix[2][0] * (float)r + CalibrationSettings.matrix[2][1] * (float)g + CalibrationSettings.matrix[2][2] * (float)b;
}

bool P050_data_struct::loadSettings(taskIndex_t taskIndex) {
  LoadCustomTaskSettings(taskIndex, (uint8_t *)&(CalibrationSettings), sizeof(CalibrationSettings));
  return  true;
}

bool P050_data_struct::saveSettings(taskIndex_t taskIndex) {
  SaveCustomTaskSettings(taskIndex, (uint8_t *)&(CalibrationSettings), sizeof(CalibrationSettings));
  return true;
}

#endif // ifdef USES_P050
