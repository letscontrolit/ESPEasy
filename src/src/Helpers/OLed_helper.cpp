#include "../Helpers/OLed_helper.h"

/**************************************************************************
 * Select controller type, SSD1306 or SH1106
 *************************************************************************/
void OLedFormController(const __FlashStringHelper *id,
                        const int                 *values,
                        uint8_t                    selectedIndex) {
  const __FlashStringHelper *controllerOptions[2] = {
    F("SSD1306 (128x64 dot controller)"),
    F("SH1106 (132x64 dot controller)")
  };
  const int controllerValues[2] = {
    1,
    2 };

  addFormSelector(F("Controller"), id, 2, controllerOptions, values == nullptr ? controllerValues : values, selectedIndex);
}

/**************************************************************************
 * Select OLed rotation, normal or rotated (180 degrees)
 *************************************************************************/
void OLedFormRotation(const __FlashStringHelper *id,
                      uint8_t                    selectedIndex) {
  const __FlashStringHelper *rotationOptions[] = {
    F("Normal"),
    F("Rotated") };
  const int rotationValues[] = {
    1,
    2 };

  addFormSelector(F("Rotation"), id, 2, rotationOptions, rotationValues, selectedIndex);
}

/**************************************************************************
 * Select contrast setting, default = high
 *************************************************************************/
void OLedFormContrast(const __FlashStringHelper *id,
                      uint8_t                    selectedIndex) {
  const __FlashStringHelper *contrastOptions[3] = {
    F("Low"),
    F("Medium"),
    F("High") };
  const int contrastValues[3] = {
    OLED_CONTRAST_LOW,
    OLED_CONTRAST_MED,
    OLED_CONTRAST_HIGH };

  addFormSelector(F("Contrast"), id, 3, contrastOptions, contrastValues, selectedIndex == 0 ? OLED_CONTRAST_HIGH : selectedIndex);
}

/**************************************************************************
 * Select one of available sizes, values are different per plugin...
 *************************************************************************/
void OLedFormSizes(const __FlashStringHelper *id,
                   const int                 *values,
                   uint8_t                    selectedIndex,
                   bool                       reloadOnChange) {
  const __FlashStringHelper *options3[3] = {
    F("128x64"),
    F("128x32"),
    F("64x48") };

  addFormSelector(F("Display Size"), id, 3, options3, values, selectedIndex, reloadOnChange);
}

/**************************************************************************
 * Set the contrast of the provided OLed display, does null-check on display object
 *************************************************************************/
void OLedSetContrast(OLEDDisplay   *_display,
                     const uint8_t& OLED_contrast) {
  if (_display == nullptr) { // Sanity check
    return;
  }
  char contrast  = 100;
  char precharge = 241;
  char comdetect = 64;

  switch (OLED_contrast) {
    case OLED_CONTRAST_OFF:
      _display->displayOff();
      return; // Done
    case OLED_CONTRAST_LOW:
      contrast = 10; precharge = 5; comdetect = 0;
      break;
    case OLED_CONTRAST_MED:
      contrast = OLED_CONTRAST_MED; precharge = 0x1F;
      break;
    case OLED_CONTRAST_HIGH:
    default:
      contrast = OLED_CONTRAST_HIGH;
      break;
  }

  _display->displayOn();
  _display->setContrast(contrast, precharge, comdetect);
}

/**************************************************************************
 * Handle PLUGIN_WEBFORM_SHOW_I2C_PARAMS and PLUGIN_HAS_I2C_ADDRESS plugin events
 *************************************************************************/
bool OLedI2CAddressCheck(uint8_t                    function,
                         int                        checkI2cAddress,
                         const __FlashStringHelper *id,
                         int8_t                     deviceAddress) {
  bool success                     = false;
  const uint8_t i2cAddressValues[] = { 0x3c, 0x3d };

  if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
    addFormSelectorI2C(id, 2, i2cAddressValues, deviceAddress);
  } else {
    success = intArrayContains(2, i2cAddressValues, checkI2cAddress);
  }
  return success;
}
