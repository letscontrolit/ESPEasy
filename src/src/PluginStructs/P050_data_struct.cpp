// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter

#include "../PluginStructs/P050_data_struct.h"


#ifdef USES_P050

# include <Adafruit_TCS34725.h>

P050_data_struct::P050_data_struct(uint16_t integrationSetting, uint16_t gainSetting) {
  // Map integration time setting (uint16_t to enum)
  _integrationTime = static_cast<tcs34725IntegrationTime_t>(integrationSetting);

  // Map gain setting (uint16_t -> enum)
  _gain = static_cast<tcs34725Gain_t>(gainSetting);

  /* Initialise with specific int time and gain values */
  tcs = Adafruit_TCS34725(_integrationTime, _gain);

  resetTransformation();

  // String log = F("P050_data sizeof(TransformationSettings): ");
  // log += sizeof(TransformationSettings);
  // addLog(LOG_LEVEL_INFO, log);
}

/**
 * resetTransformation
 * Effectgively sets matrix[0][0], matrix[1][1] and matrix[2][2] to 1.0f, all other fields to 0.0f
 */
void P050_data_struct::resetTransformation() {
  // Initialize Transformationn defaults
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      TransformationSettings.matrix[i][j] = i == j ? 1.0f : 0.0f;
    }
  }
}

/**
 * applyTransformation : calibrate r/g/b inputs (uint16_t) to rc/gc/bc outputs (float, by reference)
 */
void P050_data_struct::applyTransformation(uint16_t r, uint16_t g, uint16_t b, float *rc, float *gc, float *bc) {
  applyTransformation(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), rc, gc, bc);
}

/**
 * applyTransformation : calibrate normalized r/g/b inputs (float) to rc/gc/bc outputs (float, by reference)
 */
void P050_data_struct::applyTransformation(float nr, float ng, float nb, float *rc, float *gc, float *bc) {
  *rc = TransformationSettings.matrix[0][0] * nr +
        TransformationSettings.matrix[0][1] * ng +
        TransformationSettings.matrix[0][2] * nb;
  *gc = TransformationSettings.matrix[1][0] * nr +
        TransformationSettings.matrix[1][1] * ng +
        TransformationSettings.matrix[1][2] * nb;
  *bc = TransformationSettings.matrix[2][0] * nr +
        TransformationSettings.matrix[2][1] * ng +
        TransformationSettings.matrix[2][2] * nb;
}

bool P050_data_struct::loadSettings(taskIndex_t taskIndex) {
  LoadCustomTaskSettings(taskIndex, reinterpret_cast<uint8_t *>(&TransformationSettings), sizeof(TransformationSettings));
  return true;
}

bool P050_data_struct::saveSettings(taskIndex_t taskIndex) {
  SaveCustomTaskSettings(taskIndex, reinterpret_cast<const uint8_t *>(&TransformationSettings), sizeof(TransformationSettings));
  return true;
}

String P050_data_struct::generate_cal_id(int i, int j) {
  return strformat(F("cal_%c_%d"), static_cast<char>('a' + i), j);
}

#endif // ifdef USES_P050
