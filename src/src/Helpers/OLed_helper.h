#ifndef HELPERS_OLED_HELPER_H
#define HELPERS_OLED_HELPER_H
#include "../../_Plugin_Helper.h"
#include "SSD1306.h"
#include "SH1106Wire.h"

/**
 * Helper with OLed related functions
 *
 * Changelog:
 * 2022-10-09 tonhuisman: Deduplicate code by moving the OLed I2C Address check to OLed_helper
 * 2022-06-20 tonhuisman: Add optional values list to OLedFormController selector
 * 2022-06-18 tonhuisman: Created helper with FormSelectors for Controller, Rotation, Contrast and Sizes
 *                        and generic function OLedSetContrast
 */

#define OLED_CONTRAST_OFF  0x01
#define OLED_CONTRAST_LOW  0x40
#define OLED_CONTRAST_MED  0xCF
#define OLED_CONTRAST_HIGH 0xFF

void OLedFormController(const __FlashStringHelper *id,
                        const int                 *values,
                        uint8_t                    selectedIndex);
void OLedFormRotation(const __FlashStringHelper *id,
                      uint8_t                    selectedIndex);
void OLedFormContrast(const __FlashStringHelper *id,
                      uint8_t                    selectedIndex);
void OLedFormSizes(const __FlashStringHelper *id,
                   const int                 *values,
                   uint8_t                    selectedIndex,
                   bool                       reloadOnChange = false);
void OLedSetContrast(OLEDDisplay   *_display,
                     const uint8_t& OLED_contrast);
bool OLedI2CAddressCheck(uint8_t                    function,
                         int                        checkI2cAddress,
                         const __FlashStringHelper *id,
                         int8_t                     deviceAddress);

#endif // ifndef HELPERS_OLED_HELPER_H
