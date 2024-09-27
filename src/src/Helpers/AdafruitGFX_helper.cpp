#include "../Helpers/AdafruitGFX_helper.h"
#include "../../_Plugin_Helper.h"

#ifdef PLUGIN_USES_ADAFRUITGFX

# include "../Helpers/StringConverter.h"
# include "../Helpers/StringGenerator_Web.h"
# include "../WebServer/Markup_Forms.h"

# if ADAGFX_FONTS_INCLUDED
#  include "../Static/Fonts/Seven_Segment24pt7b.h"
#  include "../Static/Fonts/Seven_Segment18pt7b.h"
#  include "../Static/Fonts/FreeSans9pt7b.h"
#  ifdef ADAGFX_FONTS_EXTRA_5PT_INCLUDED
#   ifdef ADAGFX_FONTS_EXTRA_5PT_TOMTHUMB
#    include <Fonts/TomThumb.h> // Available in Adafruit_GFX library
#   endif // ifdef ADAGFX_FONTS_EXTRA_5PT_TOMTHUMB
#  endif // ifdef ADAGFX_FONTS_EXTRA_5PT_INCLUDED
#  ifdef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
#   ifdef ADAGFX_FONTS_EXTRA_8PT_ANGELINA
#    include "../Static/Fonts/angelina8pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ANGELINA
#   ifdef ADAGFX_FONTS_EXTRA_8PT_NOVAMONO
#    include "../Static/Fonts/NovaMono8pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_NOVAMONO
#   ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACE
#    include "../Static/Fonts/unispace8pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACE
#   ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACEITALIC
#    include "../Static/Fonts/unispace_italic8pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACEITALIC
#   ifdef ADAGFX_FONTS_EXTRA_8PT_WHITERABBiT
#    include "../Static/Fonts/whitrabt8pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_WHITERABBiT
#   ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTO
#    include "../Static/Fonts/Roboto_Regular8pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTO
#   ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTOCONDENSED
#    include "../Static/Fonts/RobotoCondensed_Regular8pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTOCONDENSED
#   ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTOMONO
#    include "../Static/Fonts/RobotoMono_Regular8pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTOMONO
#  endif // ifdef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
#  ifdef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
#   ifdef ADAGFX_FONTS_EXTRA_12PT_ANGELINA
#    include "../Static/Fonts/angelina12pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ANGELINA
#   ifdef ADAGFX_FONTS_EXTRA_12PT_NOVAMONO
#    include "../Static/Fonts/NovaMono12pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_NOVAMONO
#   ifdef ADAGFX_FONTS_EXTRA_12PT_REPETITIONSCROLLiNG
#    include "../Static/Fonts/RepetitionScrolling12pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_REPETITIONSCROLLiNG
#   ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACE
#    include "../Static/Fonts/unispace12pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACE
#   ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACEITALIC
#    include "../Static/Fonts/unispace_italic12pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACEITALIC
#   ifdef ADAGFX_FONTS_EXTRA_12PT_WHITERABBiT
#    include "../Static/Fonts/whitrabt12pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_WHITERABBiT
#   ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTO
#    include "../Static/Fonts/Roboto_Regular12pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTO
#   ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTOCONDENSED
#    include "../Static/Fonts/RobotoCondensed_Regular12pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTOCONDENSED
#   ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTOMONO
#    include "../Static/Fonts/RobotoMono_Regular12pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTOMONO
#  endif // ifdef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
#  ifdef ADAGFX_FONTS_EXTRA_16PT_INCLUDED
#   ifdef ADAGFX_FONTS_EXTRA_16PT_AMERIKASANS
#    include "../Static/Fonts/AmerikaSans16pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_16PT_AMERIKASANS
#   ifdef ADAGFX_FONTS_EXTRA_16PT_WHITERABBiT
#    include "../Static/Fonts/whitrabt16pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_16PT_WHITERABBiT
#   ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTO
#    include "../Static/Fonts/Roboto_Regular16pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTO
#   ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTOCONDENSED
#    include "../Static/Fonts/RobotoCondensed_Regular16pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTOCONDENSED
#   ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTOMONO
#    include "../Static/Fonts/RobotoMono_Regular16pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTOMONO
#  endif // ifdef ADAGFX_FONTS_EXTRA_16PT_INCLUDED
#  ifdef ADAGFX_FONTS_EXTRA_18PT_INCLUDED
#   ifdef ADAGFX_FONTS_EXTRA_18PT_WHITERABBiT
#    include "../Static/Fonts/whitrabt18pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_18PT_WHITERABBiT
#   ifdef ADAGFX_FONTS_EXTRA_18PT_SEVENSEG_B
#    include "../Static/Fonts/7segment18pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_18PT_SEVENSEG_B
#   ifdef ADAGFX_FONTS_EXTRA_18PT_LCD14COND
#    include "../Static/Fonts/LCD14cond18pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_18PT_LCD14COND
#  endif // ifdef ADAGFX_FONTS_EXTRA_18PT_INCLUDED
#  ifdef ADAGFX_FONTS_EXTRA_20PT_INCLUDED
#   ifdef ADAGFX_FONTS_EXTRA_20PT_WHITERABBiT
#    include "../Static/Fonts/whitrabt20pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_20PT_WHITERABBiT
#  endif // ifdef ADAGFX_FONTS_EXTRA_20PT_INCLUDED
#  ifdef ADAGFX_FONTS_EXTRA_24PT_INCLUDED
#   ifdef ADAGFX_FONTS_EXTRA_24PT_SEVENSEG_B
#    include "../Static/Fonts/7segment24pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_24PT_SEVENSEG_B
#   ifdef ADAGFX_FONTS_EXTRA_24PT_LCD14COND
#    include "../Static/Fonts/LCD14cond24pt7b.h"
#   endif // ifdef ADAGFX_FONTS_EXTRA_24PT_LCD14COND
#  endif // ifdef ADAGFX_FONTS_EXTRA_24PT_INCLUDED
# endif  // if ADAGFX_FONTS_INCLUDED

# if FEATURE_SD && defined(ADAGFX_ENABLE_BMP_DISPLAY)
#  include <SD.h>
# endif // if FEATURE_SD && defined(ADAGFX_ENABLE_BMP_DISPLAY)


/******************************************************************************************
 * get the display text for a 'text print mode' enum value
 *****************************************************************************************/
const __FlashStringHelper* toString(const AdaGFXTextPrintMode& mode) {
  switch (mode) {
    case AdaGFXTextPrintMode::ContinueToNextLine: return F("Continue to next line");
    case AdaGFXTextPrintMode::TruncateExceedingMessage: return F("Truncate exceeding message");
    case AdaGFXTextPrintMode::ClearThenTruncate: return F("Clear then truncate exceeding message");
    case AdaGFXTextPrintMode::TruncateExceedingCentered: return F("Truncate, centered if maxWidth set");
    case AdaGFXTextPrintMode::MAX: break;
  }
  return F("None");
}

/******************************************************************************************
 * get the display text for a color depth enum value
 *****************************************************************************************/
const __FlashStringHelper* toString(const AdaGFXColorDepth& colorDepth) {
  switch (colorDepth) {
    case AdaGFXColorDepth::Monochrome: return F("Monochrome");
    case AdaGFXColorDepth::BlackWhiteRed: return F("Monochrome + 1 color");
    case AdaGFXColorDepth::BlackWhite2Greyscales: return F("Monochrome + 2 grey levels");
    # if ADAGFX_SUPPORT_7COLOR
    case AdaGFXColorDepth::SevenColor: return F("eInk - 7 colors");
    # endif // if ADAGFX_SUPPORT_7COLOR
    # if ADAGFX_SUPPORT_8and16COLOR
    case AdaGFXColorDepth::EightColor: return F("TFT - 8 colors");
    case AdaGFXColorDepth::SixteenColor: return F("TFT - 16 colors");
    # endif // if ADAGFX_SUPPORT_8and16COLOR
    case AdaGFXColorDepth::FullColor: return F("Full color - 65535 colors");
  }
  return F("None");
}

# if ADAGFX_ENABLE_BUTTON_DRAW

/******************************************************************************************
 * get the display text for a button type enum value
 *****************************************************************************************/
const __FlashStringHelper* toString(const Button_type_e button) {
  switch (button) {
    case Button_type_e::None: return F("None");
    case Button_type_e::Square: return F("Square");
    case Button_type_e::Rounded: return F("Rounded");
    case Button_type_e::Circle: return F("Circle");
    case Button_type_e::ArrowLeft: return F("Arrow, left");
    case Button_type_e::ArrowUp: return F("Arrow, up");
    case Button_type_e::ArrowRight: return F("Arrow, right");
    case Button_type_e::ArrowDown: return F("Arrow, down");
  }
  return F("Unsupported!");
}

/******************************************************************************************
 * get the display text for a button layout enum value
 *****************************************************************************************/
const __FlashStringHelper* toString(const Button_layout_e layout) {
  switch (layout) {
    case Button_layout_e::CenterAligned: return F("Centered");
    case Button_layout_e::LeftAligned: return F("Left-aligned");
    case Button_layout_e::TopAligned: return F("Top-aligned");
    case Button_layout_e::RightAligned: return F("Right-aligned");
    case Button_layout_e::BottomAligned: return F("Bottom-aligned");
    case Button_layout_e::LeftTopAligned: return F("Left-Top-aligned");
    case Button_layout_e::RightTopAligned: return F("Right-Top-aligned");
    case Button_layout_e::RightBottomAligned: return F("Right-Bottom-aligned");
    case Button_layout_e::LeftBottomAligned: return F("Left-Bottom-aligned");
    case Button_layout_e::NoCaption: return F("No Caption");
    #  if ADAGFX_ENABLE_BMP_DISPLAY
    case Button_layout_e::Bitmap: return F("Bitmap image");
    #  endif // if ADAGFX_ENABLE_BMP_DISPLAY
    #  if ADAGFX_ENABLE_BUTTON_SLIDER
    case Button_layout_e::Slider: return F("Slide control");
    #  endif // if ADAGFX_ENABLE_BUTTON_SLIDER
  }
  return F("Unsupported!");
}

# endif // if ADAGFX_ENABLE_BUTTON_DRAW

/*****************************************************************************************
 * Show a selector for all available 'Text print mode' options, for use in PLUGIN_WEBFORM_LOAD
 ****************************************************************************************/
void AdaGFXFormTextPrintMode(const __FlashStringHelper *id,
                             uint8_t                    selectedIndex) {
  const __FlashStringHelper *textModes[] = { // Be sure to use all available modes from enum!
    toString(AdaGFXTextPrintMode::ContinueToNextLine),
    toString(AdaGFXTextPrintMode::TruncateExceedingMessage),
    toString(AdaGFXTextPrintMode::ClearThenTruncate),
    toString(AdaGFXTextPrintMode::TruncateExceedingCentered),
  };
  const int textModeOptions[] = {
    static_cast<int>(AdaGFXTextPrintMode::ContinueToNextLine),
    static_cast<int>(AdaGFXTextPrintMode::TruncateExceedingMessage),
    static_cast<int>(AdaGFXTextPrintMode::ClearThenTruncate),
    static_cast<int>(AdaGFXTextPrintMode::TruncateExceedingCentered),
  };

  addFormSelector(F("Text print Mode"), id, sizeof(textModeOptions) / sizeof(int), textModes, textModeOptions, selectedIndex);
}

void AdaGFXFormColorDepth(const __FlashStringHelper *id,
                          uint16_t                   selectedIndex,
                          bool                       enabled) {
  # if ADAGFX_SUPPORT_7COLOR
  #  if ADAGFX_SUPPORT_8and16COLOR
  const int colorDepthCount = 7 + 1;
  #  else // if ADAGFX_SUPPORT_8and16COLOR
  const int colorDepthCount = 5 + 1;
  #  endif // if ADAGFX_SUPPORT_8and16COLOR
  # else // if ADAGFX_SUPPORT_7COLOR
  #  if ADAGFX_SUPPORT_8and16COLOR
  const int colorDepthCount = 6 + 1;
  #  else // if ADAGFX_SUPPORT_8and16COLOR
  const int colorDepthCount = 4 + 1;
  #  endif // if ADAGFX_SUPPORT_8and16COLOR
  # endif // if ADAGFX_SUPPORT_7COLOR
  const __FlashStringHelper *colorDepths[colorDepthCount] = { // Be sure to use all available modes from enum!
    toString(static_cast<AdaGFXColorDepth>(0)),               // include None
    toString(AdaGFXColorDepth::Monochrome),
    toString(AdaGFXColorDepth::BlackWhiteRed),
    toString(AdaGFXColorDepth::BlackWhite2Greyscales),
    # if ADAGFX_SUPPORT_7COLOR
    toString(AdaGFXColorDepth::SevenColor),
    # endif // if ADAGFX_SUPPORT_7COLOR
    # if ADAGFX_SUPPORT_8and16COLOR
    toString(AdaGFXColorDepth::EightColor),
    toString(AdaGFXColorDepth::SixteenColor),
    # endif // if ADAGFX_SUPPORT_8and16COLOR
    toString(AdaGFXColorDepth::FullColor)
  };
  const int colorDepthOptions[colorDepthCount] = {
    0,
    static_cast<int>(AdaGFXColorDepth::Monochrome),
    static_cast<int>(AdaGFXColorDepth::BlackWhiteRed),
    static_cast<int>(AdaGFXColorDepth::BlackWhite2Greyscales),
    # if ADAGFX_SUPPORT_7COLOR
    static_cast<int>(AdaGFXColorDepth::SevenColor),
    # endif // if ADAGFX_SUPPORT_7COLOR
    # if ADAGFX_SUPPORT_8and16COLOR
    static_cast<int>(AdaGFXColorDepth::EightColor),
    static_cast<int>(AdaGFXColorDepth::SixteenColor),
    # endif // if ADAGFX_SUPPORT_8and16COLOR
    static_cast<int>(AdaGFXColorDepth::FullColor)
  };

  addRowLabel_tr_id(F("Display Color-depth"), id);
  addSelector(id, colorDepthCount, colorDepths, colorDepthOptions, NULL, selectedIndex, false, enabled);
}

/*****************************************************************************************
 * Show a selector for Rotation options, supported by Adafruit_GFX
 ****************************************************************************************/
void AdaGFXFormRotation(const __FlashStringHelper *id,
                        uint8_t                    selectedIndex) {
  const __FlashStringHelper *rotationOptions[] = { F("Normal"), F("+90&deg;"), F("+180&deg;"), F("+270&deg;") };
  const int rotationOptionValues[]             = { 0, 1, 2, 3 };

  addFormSelector(F("Rotation"), id, 4, rotationOptions, rotationOptionValues, selectedIndex);
}

/*****************************************************************************************
 * Show a checkbox & note to disable background-fill for text
 ****************************************************************************************/
void AdaGFXFormTextBackgroundFill(const __FlashStringHelper *id,
                                  uint8_t                    selectedIndex) {
  addFormCheckBox(F("Background-fill for text"), id, selectedIndex);
  # ifndef LIMIT_BUILD_SIZE
  addFormNote(F("Fill entire line-height with background color."));
  # endif // ifndef LIMIT_BUILD_SIZE
}

/*****************************************************************************************
 * Show a checkbox & note to enable col/row mode for txp, txz and txtfull subcommands
 ****************************************************************************************/
void AdaGFXFormTextColRowMode(const __FlashStringHelper *id,
                              bool                       selectedState) {
  addFormCheckBox(F("Text Coordinates in col/row"), id, selectedState);
  # ifndef LIMIT_BUILD_SIZE
  addFormNote(F("Unchecked: Coordinates in pixels. Applies only to 'txp', 'txz' and 'txtfull' subcommands."));
  # endif // ifndef LIMIT_BUILD_SIZE
}

/*****************************************************************************************
 * Show a checkbox & note to enable -1 px compatibility mode for txp and txtfull subcommands
 ****************************************************************************************/
void AdaGFXFormOnePixelCompatibilityOption(const __FlashStringHelper *id,
                                           uint8_t                    selectedIndex) {
  addFormCheckBox(F("Use -1px offset for txp &amp; txtfull"), id, selectedIndex);
  # ifndef LIMIT_BUILD_SIZE
  addFormNote(F("This is for compatibility with the original plugin implementation."));
  # endif // ifndef LIMIT_BUILD_SIZE
}

/*****************************************************************************************
 * Show 2 input fields for Foreground and Background color, translated to known color names or hex with # prefix
 ****************************************************************************************/
void AdaGFXFormForeAndBackColors(const __FlashStringHelper *foregroundId,
                                 uint16_t                   foregroundColor,
                                 const __FlashStringHelper *backgroundId,
                                 uint16_t                   backgroundColor,
                                 AdaGFXColorDepth           colorDepth) {
  String color = AdaGFXcolorToString(foregroundColor, colorDepth);

  AdaGFXHtmlColorDepthDataList(F("adagfxFGBGcolors"), colorDepth);
  addRowLabel(F("Foreground color"));
  addTextBox(foregroundId, color, 11, false, false,
             EMPTY_STRING, F("")
             # if FEATURE_TOOLTIPS
             , F("Foreground color")
             # endif // if FEATURE_TOOLTIPS
             , F("adagfxFGBGcolors")
             );
  color = AdaGFXcolorToString(backgroundColor, colorDepth);
  addRowLabel(F("Background color"));
  addTextBox(backgroundId, color, 11, false, false,
             EMPTY_STRING, F("")
             # if FEATURE_TOOLTIPS
             , F("Background color")
             # endif // if FEATURE_TOOLTIPS
             , F("adagfxFGBGcolors")
             );
  # ifndef LIMIT_BUILD_SIZE
  addFormNote(F("Use Color name, '#RGB565' (# + 1..4 hex nibbles) or '#RRGGBB' (# + 6 hex nibbles RGB color)."));
  addFormNote(F("NB: Colors stored as RGB565 value!"));
  # else // ifndef LIMIT_BUILD_SIZE
  addFormNote(F("Use Color name, # + 1..4 hex RGB565 or # + 6 hex nibbles RGB color."));
  # endif // ifndef LIMIT_BUILD_SIZE
}

/*****************************************************************************************
 * Show a pin selector and percentage 1..100 for Backlight settings
 ****************************************************************************************/
void AdaGFXFormBacklight(const __FlashStringHelper *backlightPinId,
                         int8_t                     backlightPin,
                         const __FlashStringHelper *backlightPercentageId,
                         uint16_t                   backlightPercentage) {
  addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output_optional(F("Backlight ")), backlightPinId, backlightPin);

  addFormNumericBox(F("Backlight percentage"), backlightPercentageId, backlightPercentage, 0, 100);
  addUnit(F("0-100%"));
}

/*****************************************************************************************
 * Show pin selector, inverse option and timeout inputs for Displaybutton settings
 ****************************************************************************************/
void AdaGFXFormDisplayButton(const __FlashStringHelper *buttonPinId,
                             int8_t                     buttonPin,
                             const __FlashStringHelper *buttonInverseId,
                             bool                       buttonInverse,
                             const __FlashStringHelper *displayTimeoutId,
                             int                        displayTimeout) {
  addFormPinSelect(PinSelectPurpose::Generic_input, F("Display button"), buttonPinId, buttonPin);

  addFormCheckBox(F("Inversed Logic"), buttonInverseId, buttonInverse);

  addFormNumericBox(F("Display Timeout"), displayTimeoutId, displayTimeout, 0);
  addUnit(F("0 = off"));
}

/*****************************************************************************************
 * Show a numeric input 1..10 for Font scaling setting
 ****************************************************************************************/
void AdaGFXFormFontScaling(const __FlashStringHelper *fontScalingId,
                           uint8_t                    fontScaling,
                           uint8_t                    maxScale) {
  addFormNumericBox(F("Font scaling"), fontScalingId, fontScaling, 1, maxScale);
  String unit = F("1x..");

  unit += maxScale;
  unit += 'x';
  addUnit(unit);
}

/*****************************************************************************************
 * Show a selector for line-spacing setting, supported by Adafruit_GFX
 ****************************************************************************************/
void AdaGFXFormLineSpacing(const __FlashStringHelper *id,
                           uint8_t                    selectedIndex) {
  String lineSpacings[16];
  int    lineSpacingOptions[16];

  for (uint8_t i = 0; i < 16; ++i) {
    if (15 == i) {
      # ifndef LIMIT_BUILD_SIZE
      lineSpacings[i] = F("Auto, using font height * scaling");
      # else // ifndef LIMIT_BUILD_SIZE
      lineSpacings[i] = F("Auto");
      # endif // ifndef LIMIT_BUILD_SIZE
    } else {
      lineSpacings[i] = i;
    }
    lineSpacingOptions[i] = i;
  }
  addFormSelector(F("Linespacing"), id, 16, lineSpacings, lineSpacingOptions, selectedIndex);
  addUnit(F("px"));
}

/****************************************************************************
 * AdaGFXparseTemplate: Replace variables and adjust unicode special characters to Adafruit font
 ***************************************************************************/
String AdaGFXparseTemplate(const String      & tmpString,
                           const uint8_t       lineSize,
                           AdafruitGFX_helper *gfxHelper) {
  # if ADAGFX_PARSE_SUBCOMMAND

  String result = tmpString;

  if (nullptr != gfxHelper) {
    String trigger = gfxHelper->getTrigger();

    if (!trigger.isEmpty()) {
      int16_t prefixTrigger  = result.indexOf(ADAGFX_PARSE_PREFIX);
      int16_t postfixTrigger = result.indexOf(ADAGFX_PARSE_POSTFIX, prefixTrigger + 1);

      while ((prefixTrigger > -1) && (postfixTrigger > -1) && (postfixTrigger > prefixTrigger)) { // Might be valid
        String subcommand = result.substring(prefixTrigger + ADAGFX_PARSE_POSTFIX_LEN, postfixTrigger);

        if (!subcommand.isEmpty()) {
          const String command = strformat(F("%s,%s"), trigger.c_str(), subcommand.c_str());

          #  ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
            addLogMove(ADAGFX_LOG_LEVEL, concat(F("AdaGFX: inline cmd: "), command));
          }
          #  endif // ifndef BUILD_NO_DEBUG

          if (gfxHelper->processCommand(command)) { // Execute command and remove from result incl. pre/postfix
            result.remove(prefixTrigger, (postfixTrigger - prefixTrigger) + ADAGFX_PARSE_POSTFIX_LEN);
            prefixTrigger  = result.indexOf(ADAGFX_PARSE_PREFIX);
            postfixTrigger = result.indexOf(ADAGFX_PARSE_POSTFIX, prefixTrigger + 1);
          } else { // If the command fails, exit further processing
            prefixTrigger  = -1;
            postfixTrigger = -1;
            #  ifndef BUILD_NO_DEBUG
            addLog(ADAGFX_LOG_LEVEL, F("AdaGFX: inline cmd: unknown"));
            #  endif // ifndef BUILD_NO_DEBUG
          }
        } else {
          prefixTrigger  = -1;
          postfixTrigger = -1;
        }
      }
    }
  }
  result = parseTemplate_padded(result, lineSize);
  # else // if ADAGFX_PARSE_SUBCOMMAND
  String result = parseTemplate_padded(tmpString, lineSize);
  # endif // if ADAGFX_PARSE_SUBCOMMAND

  const char euro[4]       = { 0xe2, 0x82, 0xac, 0 }; // Unicode euro symbol
  const char euro_ascii[2] = { 0xED, 0 };             // Euro symbol
  result.replace(euro, euro_ascii);

  char unicodePrefix = 0xc2;

  if (result.indexOf(unicodePrefix) != -1) {
    const char degree[3]       = { 0xc2, 0xb0, 0 };  // Unicode degree symbol
    const char degree_ascii[2] = { 0xf7, 0 };        // degree symbol
    result.replace(degree, degree_ascii);

    const char pound[3]       = { 0xc2, 0xa3, 0 };   // Unicode pound symbol
    const char pound_ascii[2] = { 0x9C, 0 };         // pound symbol
    result.replace(pound, pound_ascii);

    const char yen[3]       = { 0xc2, 0xa5, 0 };     // Unicode yen symbol
    const char yen_ascii[2] = { 0x9D, 0 };           // yen symbol
    result.replace(yen, yen_ascii);

    const char cent[3]       = { 0xc2, 0xa2, 0 };    // Unicode cent symbol
    const char cent_ascii[2] = { 0x9B, 0 };          // cent symbol
    result.replace(cent, cent_ascii);

    const char mu[3]       = { 0xc2, 0xb5, 0 };      // Unicode mu/micro (µ) symbol
    const char mu_ascii[2] = { 0xe5, 0 };            // mu/micro symbol
    result.replace(mu, mu_ascii);

    const char plusmin[3]       = { 0xc2, 0xb1, 0 }; // Unicode plusminus symbol
    const char plusmin_ascii[2] = { 0xf0, 0 };       // plusminus symbol
    result.replace(plusmin, plusmin_ascii);

    const char laquo[3]       = { 0xc2, 0xab, 0 };   // Unicode left aquo symbol
    const char laquo_ascii[2] = { 0xae, 0 };         // left aquo symbol
    result.replace(laquo, laquo_ascii);

    const char raquo[3]       = { 0xc2, 0xbb, 0 };   // Unicode right aquote symbol
    const char raquo_ascii[2] = { 0xaf, 0 };         // right aquote symbol
    result.replace(raquo, raquo_ascii);

    const char half[3]       = { 0xc2, 0xbd, 0 };    // Unicode half 1/2 symbol
    const char half_ascii[2] = { 0xab, 0 };          // half 1/2 symbol
    result.replace(half, half_ascii);

    const char quart[3]       = { 0xc2, 0xbc, 0 };   // Unicode quart 1/4 symbol
    const char quart_ascii[2] = { 0xac, 0 };         // quart 1/4 symbol
    result.replace(quart, quart_ascii);

    const char sup2[3]       = { 0xc2, 0xb2, 0 };    // Unicode superscript 2 symbol
    const char sup2_ascii[2] = { 0xfc, 0 };          // superscript 2 symbol
    result.replace(sup2, sup2_ascii);

    // Unsupported characters, replace by something useful
    const char sup1[3]       = { 0xc2, 0xb9, 0 };   // Unicode superscript 1 symbol
    const char sup1_ascii[2] = { 0x31, 0 };         // regular 1 (missing from font)
    result.replace(sup1, sup1_ascii);

    const char sup3[3]       = { 0xc2, 0xb3, 0 };   // Unicode superscript 3 symbol
    const char sup3_ascii[2] = { 0x33, 0 };         // regular 3 (missing from font)
    result.replace(sup3, sup3_ascii);

    const char frac34[3]       = { 0xc2, 0xbe, 0 }; // Unicode fraction 3/4 symbol
    const char frac34_ascii[2] = { 0x5c, 0 };       // regular \ (missing from font)
    result.replace(frac34, frac34_ascii);
    delay(0);
  }

  unicodePrefix = 0xc3;

  if (result.indexOf(unicodePrefix) != -1) {
    // See: https://github.com/letscontrolit/ESPEasy/issues/2081

    const char umlautAE_uni[3]   = { 0xc3, 0x84, 0 };  // Unicode Umlaute AE
    const char umlautAE_ascii[2] = { 0x8e, 0 };        // Umlaute A
    result.replace(umlautAE_uni, umlautAE_ascii);

    const char umlaut_ae_uni[3]  = { 0xc3, 0xa4, 0 };  // Unicode Umlaute ae
    const char umlautae_ascii[2] = { 0x84, 0 };        // Umlaute a
    result.replace(umlaut_ae_uni, umlautae_ascii);

    const char umlautOE_uni[3]   = { 0xc3, 0x96, 0 };  // Unicode Umlaute OE
    const char umlautOE_ascii[2] = { 0x99, 0 };        // Umlaute O
    result.replace(umlautOE_uni, umlautOE_ascii);

    const char umlaut_oe_uni[3]  = { 0xc3, 0xb6, 0 };  // Unicode Umlaute oe
    const char umlautoe_ascii[2] = { 0x94, 0 };        // Umlaute o
    result.replace(umlaut_oe_uni, umlautoe_ascii);

    const char umlautUE_uni[3]   = { 0xc3, 0x9c, 0 };  // Unicode Umlaute UE
    const char umlautUE_ascii[2] = { 0x9a, 0 };        // Umlaute U
    result.replace(umlautUE_uni, umlautUE_ascii);

    const char umlaut_ue_uni[3]  = { 0xc3, 0xbc, 0 };  // Unicode Umlaute ue
    const char umlautue_ascii[2] = { 0x81, 0 };        // Umlaute u
    result.replace(umlaut_ue_uni, umlautue_ascii);

    const char divide_uni[3]   = { 0xc3, 0xb7, 0 };    // Unicode divide symbol
    const char divide_ascii[2] = { 0xf5, 0 };          // Divide symbol
    result.replace(divide_uni, divide_ascii);

    const char umlaut_sz_uni[3]   = { 0xc3, 0x9f, 0 }; // Unicode Umlaute sz
    const char umlaut_sz_ascii[2] = { 0xe0, 0 };       // Umlaute B
    result.replace(umlaut_sz_uni, umlaut_sz_ascii);

    // Unsupported characters, replace by something useful
    const char times[3]       = { 0xc3, 0x97, 0 }; // Unicode multiplication symbol
    const char times_ascii[2] = { 0x78, 0 };       // regular x (missing from font)
    result.replace(times, times_ascii);
    delay(0);
  }

  // Handle '{0xNN...}' hex values in template, where NN can be any hex value from 01..FF (practically 20..FF).
  int16_t hexPrefix = 0;
  int16_t hexPostfix;
  const String hexSeparators = F(" ,.:;-");

  while (((hexPrefix = result.indexOf(F("{0x"), hexPrefix)) > -1) &&
         ((hexPostfix = result.indexOf('}', hexPrefix)) > -1)) {
    String replace;

    for (int16_t ci = hexPrefix + 3; ci < hexPostfix - 1; ci += 2) { // Multiple of 2 only
      uint32_t hexValue = hexToUL(result.substring(ci, ci + 2));

      if (hexValue > 0) {
        replace += static_cast<char>(hexValue);
      }

      while (hexSeparators.indexOf(result.substring(ci + 2, ci + 3)) > -1 && ci < hexPostfix) {
        ci++;
      }
    }

    if (!replace.isEmpty()) {
      result.replace(result.substring(hexPrefix, hexPostfix + 1), replace);
    }
  }

  for (uint16_t l = result.length(); l > 0 && isSpace(result[l - 1]); --l) { // Right-trim
    result.remove(l - 1);
  }

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
    addLogMove(ADAGFX_LOG_LEVEL, strformat(F("AdaGFX: parse result: '%s'"), result.c_str()));
  }
  # endif // ifndef BUILD_NO_DEBUG
  return result;
}

// AdafruitGFX_helper class methods

/****************************************************************************
 * parameterized constructors
 ***************************************************************************/
AdafruitGFX_helper::AdafruitGFX_helper(Adafruit_GFX              *display,
                                       const String             & trigger,
                                       const uint16_t             res_x,
                                       const uint16_t             res_y,
                                       const AdaGFXColorDepth   & colorDepth,
                                       const AdaGFXTextPrintMode& textPrintMode,
                                       const uint8_t              fontscaling,
                                       const uint16_t             fgcolor,
                                       const uint16_t             bgcolor,
                                       const bool                 useValidation,
                                       const bool                 textBackFill,
                                       const uint8_t              defaultFontId)
  : _display(display), _trigger(trigger), _res_x(res_x), _res_y(res_y), _colorDepth(colorDepth),
  _textPrintMode(textPrintMode), _fontscaling(fontscaling), _fgcolor(fgcolor), _bgcolor(bgcolor),
  _useValidation(useValidation), _textBackFill(textBackFill), _defaultFontId(defaultFontId)
{
  addLog(LOG_LEVEL_INFO, F("AdaGFX_helper: GFX Init."));
}

# if ADAGFX_ENABLE_BMP_DISPLAY
AdafruitGFX_helper::AdafruitGFX_helper(Adafruit_SPITFT           *display,
                                       const String             & trigger,
                                       const uint16_t             res_x,
                                       const uint16_t             res_y,
                                       const AdaGFXColorDepth   & colorDepth,
                                       const AdaGFXTextPrintMode& textPrintMode,
                                       const uint8_t              fontscaling,
                                       const uint16_t             fgcolor,
                                       const uint16_t             bgcolor,
                                       const bool                 useValidation,
                                       const bool                 textBackFill,
                                       const uint8_t              defaultFontId)
  : _tft(display), _trigger(trigger), _res_x(res_x), _res_y(res_y), _colorDepth(colorDepth),
  _textPrintMode(textPrintMode), _fontscaling(fontscaling), _fgcolor(fgcolor), _bgcolor(bgcolor),
  _useValidation(useValidation), _textBackFill(textBackFill), _defaultFontId(defaultFontId)
{
  _display = _tft;
  addLog(LOG_LEVEL_INFO, F("AdaGFX_helper: TFT Init."));
}

# endif // if ADAGFX_ENABLE_BMP_DISPLAY

/****************************************************************************
 * common initialization, called from constructors
 ***************************************************************************/
void AdafruitGFX_helper::initialize() {
  _trigger.toLowerCase(); // store trigger in lowercase
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
    addLogMove(ADAGFX_LOG_LEVEL, strformat(F("AdaGFX: Init, x: %d, y: %d, colors: %d, trigger: %s, %s"),
                                           _res_x, _res_y, static_cast<uint16_t>(_colorDepth),
                                           _trigger.c_str(), getFeatures().c_str()));
  }
  # endif // ifndef BUILD_NO_DEBUG

  _display_x = _res_x; // Store initial resolution
  _display_y = _res_y;

  # if ADAGFX_ENABLE_FRAMED_WINDOW
  defineWindow(0, 0, _res_x, _res_y, 0, 0); // Add window 0 at rotation 0
  # endif // if ADAGFX_ENABLE_FRAMED_WINDOW

  if (_fontscaling < 1) { _fontscaling = 1; }

  if (nullptr != _display) {
    # if ADAGFX_FONTS_INCLUDED
    setFontById(_defaultFontId);
    # endif // if ADAGFX_FONTS_INCLUDED
    _display->setTextSize(_fontscaling);
    _display->setTextColor(_fgcolor, _bgcolor); // initialize text colors
    _display->setTextWrap(_textPrintMode == AdaGFXTextPrintMode::ContinueToNextLine);
  }
}

/****************************************************************************
 * Show enabled features of the helper
 ***************************************************************************/
String AdafruitGFX_helper::getFeatures() {
  String log = F("Features:"
                 # if (defined(ADAGFX_USE_ASCIITABLE) && ADAGFX_USE_ASCIITABLE)
                 " asciitable,"
                 # endif // if (defined(ADAGFX_USE_ASCIITABLE) && ADAGFX_USE_ASCIITABLE)
                 # if (defined(ADAGFX_ENABLE_EXTRA_CMDS) && ADAGFX_ENABLE_EXTRA_CMDS)
                 " lm/lmr,"
                 # endif // if (defined(ADAGFX_ENABLE_EXTRA_CMDS) && ADAGFX_ENABLE_EXTRA_CMDS)
                 # if (defined(ADAGFX_ENABLE_BMP_DISPLAY) && ADAGFX_ENABLE_BMP_DISPLAY)
                 " bmp,"
                 # endif // if (defined(ADAGFX_ENABLE_BMP_DISPLAY) && ADAGFX_ENABLE_BMP_DISPLAY)
                 # if (defined(ADAGFX_ENABLE_BUTTON_DRAW) && ADAGFX_ENABLE_BUTTON_DRAW)
                 " btn,"
                 # endif // if (defined(ADAGFX_ENABLE_BUTTON_DRAW) && ADAGFX_ENABLE_BUTTON_DRAW)`
                 # if (defined(ADAGFX_ENABLE_FRAMED_WINDOW) && ADAGFX_ENABLE_FRAMED_WINDOW)
                 " win,"
                 # endif // if (defined(ADAGFX_ENABLE_FRAMED_WINDOW) && ADAGFX_ENABLE_FRAMED_WINDOW)
                 # if (defined(ADAGFX_ENABLE_GET_CONFIG_VALUE) && ADAGFX_ENABLE_GET_CONFIG_VALUE)
                 " getconf,"
                 # endif // if (defined(ADAGFX_ENABLE_GET_CONFIG_VALUE) && ADAGFX_ENABLE_GET_CONFIG_VALUE)
                 );

  if (log.endsWith(F(","))) {
    log.remove(log.length() - 1);
  }
  return log;
}

/****************************************************************************
 * getCursorXY: get the current (text) cursor coordinates, either in pixels or cols/rows, depending on related setting
 ***************************************************************************/
void AdafruitGFX_helper::getCursorXY(int16_t& currentX,
                                     int16_t& currentY) {
  _lastX = _display->getCursorX();
  _lastY = _display->getCursorY();

  if (_columnRowMode && (_lastX != 0)) { _lastX /= _fontwidth; }

  if (_columnRowMode && (_lastY != 0)) { _lastY /= _fontheight; }
  currentX = _lastX;
  currentY = _lastY;
}

/****************************************************************************
 * setTxtfullCompensation: x and/or y values defined here are subtracted from x and y position
 * - set to 1 for x-1/y-1 pixel for P095 and P096
 * - set to 2 for y+1 pixel
 * - set to 3 for x+1 pixel
 ***************************************************************************/
void AdafruitGFX_helper::setTxtfullCompensation(uint8_t compensation) {
  switch (compensation) {
    case 1: // P095
    {
      _x_compensation = 1;
      _y_compensation = 1;
      break;
    }
    case 2:
    {
      _x_compensation = 0;
      _y_compensation = -1;
      break;
    }
    case 3:
    {
      _x_compensation = -1;
      _y_compensation = 0;
      break;
    }
    default:
    {
      _x_compensation = 0;
      _y_compensation = 0;
      break;
    }
  }
}

/****************************************************************************
 * invertDisplay(): Store display-inverted state and proxy to _display
 ***************************************************************************/
void AdafruitGFX_helper::invertDisplay(bool i) {
  _displayInverted = i;
  _display->invertDisplay(_displayInverted);
}

/****************************************************************************
 * processCommand: Parse string to <command>,<subcommand>[,<arguments>...] and execute that command
 ***************************************************************************/
const char adagfx_commands[] PROGMEM =
  "txt|txp|txz|txl|txc|txs|txtfull|clear|rot|tpm|" // 0..9
  # if ADAGFX_USE_ASCIITABLE
  "asciitable|"
  # endif // if ADAGFX_USE_ASCIITABLE
  "font|l|lh|lv|"
  # if ADAGFX_ENABLE_EXTRA_CMDS
  "lm|lmr|"
  # endif // if ADAGFX_ENABLE_EXTRA_CMDS
  "r|rf|c|" // 10..19
  "cf|t|tf|rr|rrf|px|pxh|pxv|"
  # if ADAGFX_ENABLE_BMP_DISPLAY
  "bmp|"
  # endif // if ADAGFX_ENABLE_BMP_DISPLAY
  # if ADAGFX_ENABLE_BUTTON_DRAW
  "btn|"              // 20..29
  # endif // if ADAGFX_ENABLE_BUTTON_DRAW
  # if ADAGFX_ENABLE_FRAMED_WINDOW
  "win|defwin|delwin" // 30..
  # endif // if ADAGFX_ENABLE_FRAMED_WINDOW
;
enum class adagfx_commands_e : int8_t {
  invalid = -1,
  txt     = 0, // 0
  txp,
  txz,
  txl,
  txc,
  txs,
  txtfull,
  clear,
  rot,
  tpm,        // 9
  # if ADAGFX_USE_ASCIITABLE
  asciitable, // 10
  # endif // if ADAGFX_USE_ASCIITABLE
  font,
  l,
  lh,
  lv,
  # if ADAGFX_ENABLE_EXTRA_CMDS
  lm,
  lmr,
  # endif // if ADAGFX_ENABLE_EXTRA_CMDS
  r,
  rf,
  c,  // 19
  cf, // 20
  t,
  tf,
  rr,
  rrf,
  px,
  pxh,
  pxv,
  # if ADAGFX_ENABLE_BMP_DISPLAY
  bmp,
  # endif // if ADAGFX_ENABLE_BMP_DISPLAY
  # if ADAGFX_ENABLE_BUTTON_DRAW
  btn, // 29
  # endif // if ADAGFX_ENABLE_BUTTON_DRAW
  # if ADAGFX_ENABLE_FRAMED_WINDOW
  win, // 30
  defwin,
  delwin,
  # endif // if ADAGFX_ENABLE_FRAMED_WINDOW
};
# if ADAGFX_FONTS_INCLUDED

// *** Don't forget to add the | separator at the end of a (new) font-name! (except for the last one in the list)
const char adagfx_fonts[] PROGMEM =
  "default|sevenseg24|sevenseg18|freesans|"
  #  ifdef ADAGFX_FONTS_EXTRA_5PT_INCLUDED
  #   ifdef ADAGFX_FONTS_EXTRA_5PT_TOMTHUMB
  "tomthumb|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_5PT_TOMTHUMB
  #  endif // ifdef ADAGFX_FONTS_EXTRA_5PT_INCLUDED
  #  ifdef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
  #   ifdef ADAGFX_FONTS_EXTRA_8PT_ANGELINA
  "angelina8prop|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ANGELINA
  #   ifdef ADAGFX_FONTS_EXTRA_8PT_NOVAMONO
  "novamono8pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_NOVAMONO
  #   ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACE
  "unispace8pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACE
  #   ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACEITALIC
  "unispaceitalic8pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACEITALIC
  #   ifdef ADAGFX_FONTS_EXTRA_8PT_WHITERABBiT
  "whiterabbit8pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_WHITERABBiT
  #   ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTO
  "roboto8pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTO
  #   ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTOCONDENSED
  "robotocond8pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTOCONDENSED
  #   ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTOMONO
  "robotomono8pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTOMONO
  #  endif // ifdef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
  #  ifdef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
  #   ifdef ADAGFX_FONTS_EXTRA_12PT_ANGELINA
  "angelina12prop|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ANGELINA
  #   ifdef ADAGFX_FONTS_EXTRA_12PT_NOVAMONO
  "novamono12pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_NOVAMONO
  #   ifdef ADAGFX_FONTS_EXTRA_12PT_REPETITIONSCROLLiNG
  "repetitionscrolling12pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_REPETITIONSCROLLiNG
  #   ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACE
  "unispace12pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACE
  #   ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACEITALIC
  "unispaceitalic12pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACEITALIC
  #   ifdef ADAGFX_FONTS_EXTRA_12PT_WHITERABBiT
  "whiterabbit12pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_WHITERABBiT
  #   ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTO
  "roboto12pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTO
  #   ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTOCONDENSED
  "robotocond12pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTOCONDENSED
  #   ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTOMONO
  "robotomono12pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTOMONO
  #  endif // ifdef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
  #  ifdef ADAGFX_FONTS_EXTRA_16PT_INCLUDED
  #   ifdef ADAGFX_FONTS_EXTRA_16PT_AMERIKASANS
  "amerikasans16pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_16PT_AMERIKASANS
  #   ifdef ADAGFX_FONTS_EXTRA_16PT_WHITERABBiT
  "whiterabbit16pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_16PT_WHITERABBiT
  #   ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTO
  "roboto16pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTO
  #   ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTOCONDENSED
  "robotocond16pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTOCONDENSED
  #   ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTOMONO
  "robotomono16pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTOMONO
  #  endif // ifdef ADAGFX_FONTS_EXTRA_16PT_INCLUDED
  #  ifdef ADAGFX_FONTS_EXTRA_18PT_INCLUDED
  #   ifdef ADAGFX_FONTS_EXTRA_18PT_WHITERABBiT
  "whiterabbit18pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_18PT_WHITERABBiT
  #   ifdef ADAGFX_FONTS_EXTRA_18PT_SEVENSEG_B
  "sevenseg18b|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_18PT_SEVENSEG_B
  #   ifdef ADAGFX_FONTS_EXTRA_18PT_LCD14COND
  "lcd14cond18pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_18PT_LCD14COND
  #  endif // ifdef ADAGFX_FONTS_EXTRA_18PT_INCLUDED
  #  ifdef ADAGFX_FONTS_EXTRA_20PT_INCLUDED
  #   ifdef ADAGFX_FONTS_EXTRA_20PT_WHITERABBiT
  "whiterabbit20pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_20PT_WHITERABBiT
  #  endif // ifdef ADAGFX_FONTS_EXTRA_20PT_INCLUDED
  #  ifdef ADAGFX_FONTS_EXTRA_24PT_INCLUDED
  #   ifdef ADAGFX_FONTS_EXTRA_24PT_SEVENSEG_B
  "sevenseg24b|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_24PT_SEVENSEG_B
  #   ifdef ADAGFX_FONTS_EXTRA_24PT_LCD14COND
  "lcd14cond24pt|"
  #   endif // ifdef ADAGFX_FONTS_EXTRA_24PT_LCD14COND
  #  endif // ifdef ADAGFX_FONTS_EXTRA_24PT_INCLUDED
  "";

struct tFontArgs {
  constexpr tFontArgs(const GFXfont *f,
                      uint8_t        width,
                      uint8_t        height,
                      int8_t         offset,
                      bool           proportional,
                      uint8_t        fontId)
    : _f(f), _width(width), _height(height), _offset(offset),
    _proportional(proportional), _fontId(fontId) {}

  const GFXfont *_f;
  uint8_t        _width;
  uint8_t        _height;
  int8_t         _offset;
  bool           _proportional;
  uint8_t        _fontId;
};

/* *INDENT-OFF* */
constexpr tFontArgs fontargs[] =
{
  { nullptr,                        6,                9,   0,   false,  0u },
  { &Seven_Segment24pt7b,           21,               42,  35,  true,   1u },
  { &Seven_Segment18pt7b,           16,               33,  26,  true,   2u },
  { &FreeSans9pt7b,                 10,               16,  12,  false,  3u },
  # ifdef ADAGFX_FONTS_EXTRA_5PT_INCLUDED
  #  ifdef ADAGFX_FONTS_EXTRA_5PT_TOMTHUMB
  { &TomThumb,                      5,                 6,   5,  false,  4u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_5PT_TOMTHUMB
  # endif // ifdef ADAGFX_FONTS_EXTRA_5PT_INCLUDED
  # ifdef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
  #  ifdef ADAGFX_FONTS_EXTRA_8PT_ANGELINA
  { &angelina8pt7b,                 6,                16,  12,  true,   5u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ANGELINA
  #  ifdef ADAGFX_FONTS_EXTRA_8PT_NOVAMONO
  { &NovaMono8pt7b,                 9,                16,  12,  false,  6u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_8PT_NOVAMONO
  #  ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACE
  { &unispace8pt7b,                 13,               24,  20,  false,  7u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACE
  #  ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACEITALIC
  { &unispace_italic8pt7b,          13,               24,  20,  false,  8u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACEITALIC
  #  ifdef ADAGFX_FONTS_EXTRA_8PT_WHITERABBiT
  { &whitrabt8pt7b,                 10,               16,  12,  false,  9u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_8PT_WHITERABBiT
  #  ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTO
  { &Roboto_Regular8pt7b,           10,               16,  12,  true,  10u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTO
  #  ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTOCONDENSED
  { &RobotoCondensed_Regular8pt7b,  9,                16,  12,  true,  11u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTOCONDENSED
  #  ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTOMONO
  { &RobotoMono_Regular8pt7b,       10,               16,  12,  false, 12u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ROBOTOMONO
  # endif  // ifdef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
  # ifdef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
  #  ifdef ADAGFX_FONTS_EXTRA_12PT_ANGELINA
  { &angelina12pt7b,                8,                22,  18,  true,  13u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ANGELINA
  #  ifdef ADAGFX_FONTS_EXTRA_12PT_NOVAMONO
  { &NovaMono12pt7b,                13,               26,  22,  false, 14u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_12PT_NOVAMONO
  #  ifdef ADAGFX_FONTS_EXTRA_12PT_REPETITIONSCROLLiNG
  { &RepetitionScrolling12pt7b,     13,               22,  18,  false, 15u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_12PT_REPETITIONSCROLLiNG
  #  ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACE
  { &unispace12pt7b,                18,               30,  26,  false, 16u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACE
  #  ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACEITALIC
  { &unispace_italic12pt7b,         18,               30,  26,  false, 17u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACEITALIC
  #  ifdef ADAGFX_FONTS_EXTRA_12PT_WHITERABBiT
  { &whitrabt12pt7b,                13,               20,  16,  false, 18u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_12PT_WHITERABBiT
  #  ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTO
  { &Roboto_Regular12pt7b,          13,               20,  20,  true,  19u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTO
  #  ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTOCONDENSED
  { &RobotoCondensed_Regular12pt7b, 13,               20,  20,  true,  20u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTOCONDENSED
  #  ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTOMONO
  { &RobotoMono_Regular12pt7b,      13,               20,  20,  false, 21u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ROBOTOMONO
  # endif  // ifdef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
  # ifdef ADAGFX_FONTS_EXTRA_16PT_INCLUDED
  #  ifdef ADAGFX_FONTS_EXTRA_16PT_AMERIKASANS
  { &AmerikaSans16pt7b,             17,               30,  26,  true,  22u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_16PT_AMERIKASANS
  #  ifdef ADAGFX_FONTS_EXTRA_16PT_WHITERABBiT
  { &whitrabt16pt7b,                18,               26,  22,  false, 23u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_16PT_WHITERABBiT
  #  ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTO
  { &Roboto_Regular16pt7b,          18,               27,  23,  true,  24u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTO
  #  ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTOCONDENSED
  { &RobotoCondensed_Regular16pt7b, 18,               27,  23,  true,  25u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTOCONDENSED
  #  ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTOMONO
  { &RobotoMono_Regular16pt7b,      18,               27,  23,  false, 26u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_16PT_ROBOTOMONO
  # endif  // ifdef ADAGFX_FONTS_EXTRA_16PT_INCLUDED
  # ifdef ADAGFX_FONTS_EXTRA_18PT_INCLUDED
  #  ifdef ADAGFX_FONTS_EXTRA_18PT_WHITERABBiT
  { &whitrabt18pt7b,                21,               30,  26,  false, 27u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_18PT_WHITERABBiT
  #  ifdef ADAGFX_FONTS_EXTRA_18PT_SEVENSEG_B
  { &_7segment18pt7b,               21,               30,  30,  false, 28u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_18PT_SEVENSEG_B
  #  ifdef ADAGFX_FONTS_EXTRA_18PT_LCD14COND
  { &LCD14cond18pt7b,               24,               30,  30,  false, 29u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_18PT_LCD14COND
  # endif // ifdef ADAGFX_FONTS_EXTRA_18PT_INCLUDED
  # ifdef ADAGFX_FONTS_EXTRA_20PT_INCLUDED
  #  ifdef ADAGFX_FONTS_EXTRA_20PT_WHITERABBiT
  { &whitrabt20pt7b,                24,               32,  28,  false, 30u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_20PT_WHITERABBiT
  # endif  // ifdef ADAGFX_FONTS_EXTRA_20PT_INCLUDED
  # ifdef ADAGFX_FONTS_EXTRA_24PT_INCLUDED
  #  ifdef ADAGFX_FONTS_EXTRA_24PT_SEVENSEG_B
  { &_7segment24pt7b,               26,               34,  38,  false, 31u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_24PT_SEVENSEG_B
  #  ifdef ADAGFX_FONTS_EXTRA_24PT_LCD14COND
  { &LCD14cond24pt7b,               26,               34,  38,  false, 32u },
  #  endif // ifdef ADAGFX_FONTS_EXTRA_24PT_LCD14COND
  # endif  // ifdef ADAGFX_FONTS_EXTRA_24PT_INCLUDED
};
/* *INDENT-ON* */
# endif // if ADAGFX_FONTS_INCLUDED

String AdaGFXgetFontName(uint8_t fontId, bool includeFontId) {
  # if ADAGFX_FONTS_INCLUDED
  const uint32_t idx = AdaGFXgetFontIndexForFontId(fontId);
  char   tmp[30]{}; // Longest name so far is 23 + \0
  String fontName(GetTextIndexed(tmp, sizeof(tmp), idx, adagfx_fonts));

  if (includeFontId) {
    fontName = strformat(F("%s (%d)"), tmp, fontargs[idx]._fontId);
  }
  return fontName;
  # else // if ADAGFX_FONTS_INCLUDED
  return EMPTY_STRING;
  # endif // if ADAGFX_FONTS_INCLUDED
}

uint32_t AdaGFXgetFontIndexForFontId(uint8_t fontId) {
  # if ADAGFX_FONTS_INCLUDED
  constexpr uint32_t font_max = NR_ELEMENTS(fontargs);

  for (uint32_t idx = 0; idx < font_max; ++idx) {
    if (fontargs[idx]._fontId == fontId) {
      return idx;
    }
  }
  # endif // if ADAGFX_FONTS_INCLUDED
  return 0;
}

void AdaGFXFormDefaultFont(const __FlashStringHelper *id,
                           uint8_t                    selectedIndex) {
  # if ADAGFX_FONTS_INCLUDED
  constexpr uint32_t font_max = NR_ELEMENTS(fontargs);

  addRowLabel_tr_id(F("Default font"), id);
  addSelector_Head(id);

  char tmp[30]{}; // Longest name so far is 23 + \0

  for (uint32_t idx = 0; idx < font_max; ++idx) {
    const bool selected = (fontargs[idx]._fontId == selectedIndex);
    GetTextIndexed(tmp, sizeof(tmp), idx, adagfx_fonts);
    addSelector_Item(strformat(F("%s (%d)"), tmp, fontargs[idx]._fontId),
                     fontargs[idx]._fontId,
                     selected);
  }
  addSelector_Foot();
  # endif // if ADAGFX_FONTS_INCLUDED
}

# if ADAGFX_FONTS_INCLUDED
void AdafruitGFX_helper::setFontById(uint8_t fontId) {
  constexpr int font_max = NR_ELEMENTS(fontargs);
  const int     font_i   = AdaGFXgetFontIndexForFontId(fontId);

  if ((font_i >= 0) && (font_i < font_max)) {
    _fontId = fontargs[font_i]._fontId;
    _display->setFont(fontargs[font_i]._f);
    calculateTextMetrics(fontargs[font_i]._width,
                         fontargs[font_i]._height,
                         fontargs[font_i]._offset,
                         fontargs[font_i]._proportional);
  }
}

# endif // if ADAGFX_FONTS_INCLUDED

bool AdafruitGFX_helper::processCommand(const String& string) {
  bool success = false;

  if ((nullptr == _display) || _trigger.isEmpty()) { return success; }

  const String cmd        = parseString(string, 1); // lower case
  const String subcommand = parseString(string, 2);

  # if ADAGFX_ENABLE_FRAMED_WINDOW || ADAGFX_ARGUMENT_VALIDATION
  uint16_t res_x = _res_x;
  uint16_t res_y = _res_y;
  # endif // if ADAGFX_ENABLE_FRAMED_WINDOW || ADAGFX_ARGUMENT_VALIDATION
  uint16_t _xo = 0;
  uint16_t _yo = 0;

  # if ADAGFX_ENABLE_FRAMED_WINDOW
  getWindowLimits(res_x, res_y);
  getWindowOffsets(_xo, _yo);
  # endif // if ADAGFX_ENABLE_FRAMED_WINDOW

  if (!(cmd.equals(_trigger) ||
        isAdaGFXTrigger(cmd)) ||
      subcommand.isEmpty()) { return success; } // Only support own trigger, and at least a non=empty subcommand

  std::vector<String>  sParams;
  std::vector<int32_t> nParams;
  uint8_t emptyCount = 0;
  int     argCount   = 0;
  bool    loop       = true;

  while (loop) { // Process all provided arguments
    // 0-offset + 1st and 2nd argument used by trigger/subcommand, don't trim off spaces
    sParams.push_back(parseStringKeepCaseNoTrim(string, argCount + 3));
    nParams.push_back(0);
    validIntFromString(sParams[argCount], nParams[argCount]);

    if (sParams[argCount].isEmpty()) {
      emptyCount++;
    } else {
      emptyCount = 0;                                           // Reset empty counter
    }
    loop = emptyCount < 3 || argCount <= ADAGFX_PARSE_MAX_ARGS; // Keep picking up arguments until we have the last 3 empty

    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
      addLog(LOG_LEVEL_DEBUG_DEV, strformat(F(":%d %s"), argCount, sParams[argCount].c_str()));
    }
    # endif // ifndef BUILD_NO_DEBUG

    argCount++;
  }
  argCount -= emptyCount; // Not counting the empty arguments

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
    addLog(ADAGFX_LOG_LEVEL, strformat(F("AdaGFX: command: %s argCount: %d:%s"), _trigger.c_str(), argCount, string.c_str()));
  }
  # endif // ifndef BUILD_NO_DEBUG

  const int subcommand_i = GetCommandCode(subcommand.c_str(), adagfx_commands);

  if (subcommand_i < 0) { return false; } // Fail fast

  const adagfx_commands_e subcmd = static_cast<adagfx_commands_e>(subcommand_i);
  const bool currentColRowState  = _columnRowMode;

  # if ADAGFX_ARGUMENT_VALIDATION

  // Optimize some coordinate checks, with less than 3 occurrences there is no gain
  const bool invCoord_0_1     = argCount >= 2 && invalidCoordinates(nParams[0], nParams[1]);
  const bool invCoord_2_3     = argCount >= 4 && invalidCoordinates(nParams[2], nParams[3]);
  const bool invCoord_0_2_1_3 = argCount >= 4 && invalidCoordinates(nParams[0] + nParams[2],
                                                                    nParams[1] + nParams[3]);
  # endif // if ADAGFX_ARGUMENT_VALIDATION

  switch (subcmd)
  {
    case adagfx_commands_e::invalid:
      break;
    case adagfx_commands_e::txt:                                    // txt: Print text at last cursor position, ends at next line!
      _display->println(parseStringToEndKeepCaseNoTrim(string, 3)); // Print entire rest of provided line
      success = true;
      break;
    case adagfx_commands_e::txp:                                    // txp: Text position

      if (argCount == 2) {
        # if ADAGFX_ARGUMENT_VALIDATION

        if (!invalidCoordinates(nParams[0], nParams[1], _columnRowMode))
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          if (_columnRowMode) {
            _display->setCursor(nParams[0] * _fontwidth + _xo, nParams[1] * _fontheight + _yo);
          } else {
            _display->setCursor(nParams[0] + _xo - _x_compensation, nParams[1] + _yo - _y_compensation);
          }
        }
        success = true;
      }
      break;
    case adagfx_commands_e::txz: // txz: Text at position

      if (argCount >= 3) {
        # if ADAGFX_ARGUMENT_VALIDATION

        if (!invalidCoordinates(nParams[0], nParams[1], _columnRowMode))
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          printText(parseStringToEndKeepCaseNoTrim(string, 5).c_str(),
                    nParams[0] + _xo - _x_compensation,
                    nParams[1] + _yo - _y_compensation,
                    _fontscaling,
                    _fgcolor,
                    _bgcolor);
        }
        success = true;
      }
      break;
    case adagfx_commands_e::txl: // txl: Text at line(s)

      if (argCount >= 2) {
        uint8_t _line   = 0;
        uint8_t _column = 0;
        uint8_t idx     = 0;
        setColumnRowMode(true); // this command is by default set to Column/Row mode

        while (idx < argCount && !sParams[idx + 1].isEmpty()) {
          if (nParams[idx] > 0) {
            _line = nParams[idx];
          } else {
            _line++;
          }
          printText(sParams[idx + 1].c_str(), _column, _line - 1, _fontscaling, _fgcolor, _bgcolor);
          idx += 2;
        }
        setColumnRowMode(currentColRowState);
        success = true;
      }
      break;
    case adagfx_commands_e::txc: // txc: Textcolor, fg and opt. bg colors

      if ((argCount == 1) || (argCount == 2)) {
        _fgcolor = AdaGFXparseColor(sParams[0], _colorDepth);

        if (argCount == 1) {
          _bgcolor = _fgcolor; // Transparent background
          _display->setTextColor(_fgcolor);
        } else {               // argCount=2
          _bgcolor = AdaGFXparseColor(sParams[1], _colorDepth);
          _display->setTextColor(_fgcolor, _bgcolor);
        }
        success = true;
      }
      break;
    case adagfx_commands_e::txs: // txs: Text size = font scaling, 1..10

      if ((argCount == 1) && (nParams[0] >= 0) && (nParams[0] <= 10)) {
        _fontscaling = nParams[0];
        _display->setTextSize(_fontscaling);
        calculateTextMetrics(_fontwidth, _fontheight, _heightOffset, _isProportional);
        success = true;
      }
      break;
    case adagfx_commands_e::txtfull: // txtfull: Text at position, with size and color

      if ((argCount >= 3) && (argCount <= 8)) {
        uint16_t par3color = argCount < 5 || sParams[3].isEmpty() ? _fgcolor : AdaGFXparseColor(sParams[3], _colorDepth);
        uint16_t par4color = argCount < 6 || sParams[4].isEmpty() ? _bgcolor : AdaGFXparseColor(sParams[4], _colorDepth);

        # if ADAGFX_ARGUMENT_VALIDATION

        if (!invalidCoordinates(nParams[0] - _x_compensation,
                                nParams[1] - _y_compensation,
                                _columnRowMode))
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          success = true;

          switch (argCount) {
            case 3: // single text

              printText(sParams[2].c_str(),
                        nParams[0] - _x_compensation,
                        nParams[1] - _y_compensation,
                        _fontscaling,
                        _fgcolor,
                        _bgcolor);
              break;
            case 4: // text + size

              printText(sParams[3].c_str(),
                        nParams[0] - _x_compensation,
                        nParams[1] - _y_compensation,
                        nParams[2],
                        _fgcolor,
                        _bgcolor);
              break;
            case 5: // text + size + color

              printText(sParams[4].c_str(),
                        nParams[0] - _x_compensation,
                        nParams[1] - _y_compensation,
                        nParams[2],
                        par3color,
                        par3color); // transparent bg
              break;
            case 6:                 // text + size + color + bkcolor

              printText(sParams[5].c_str(),
                        nParams[0] - _x_compensation,
                        nParams[1] - _y_compensation,
                        nParams[2],
                        par3color,
                        par4color);
              break;
            case 7: // 7: text + size + color + bkcolor + printmode
            case 8: // as 7 but: + maxwidth

            {
              AdaGFXTextPrintMode tmpPrintMode = _textPrintMode;

              if ((nParams[5] >= 0) && (nParams[5] < static_cast<int>(AdaGFXTextPrintMode::MAX))) {
                _textPrintMode = static_cast<AdaGFXTextPrintMode>(nParams[5]);
                _display->setTextWrap(_textPrintMode == AdaGFXTextPrintMode::ContinueToNextLine);
              }
              printText(sParams[argCount - 1].c_str(),
                        nParams[0] - _x_compensation,
                        nParams[1] - _y_compensation,
                        nParams[2],
                        par3color,
                        par4color,
                        argCount == 8 ? nParams[argCount - 2] : 0);

              if (_textPrintMode != tmpPrintMode) {
                _textPrintMode = tmpPrintMode;
                _display->setTextWrap(_textPrintMode == AdaGFXTextPrintMode::ContinueToNextLine);
              }
              break;
            }
            default:
              success = false;
              break;
          }
        }
      }
      break;
    case adagfx_commands_e::clear: // clear: Clear display
      # if ADAGFX_ENABLE_FRAMED_WINDOW

      if (_window == 0)
      # endif // if ADAGFX_ENABLE_FRAMED_WINDOW
      {
        _display->fillScreen(argCount == 0 ? _bgcolor : AdaGFXparseColor(sParams[0], _colorDepth));
      }
      # if ADAGFX_ENABLE_FRAMED_WINDOW
      else {
        // logWindows(F("clear ")); // Use for debugging only
        uint16_t _w = 0, _h = 0;
        getWindowLimits(_w, _h);
        _display->fillRect(_xo, _yo, _w, _h,
                           argCount == 0 ? _bgcolor : AdaGFXparseColor(sParams[0], _colorDepth));
      }
      # endif // if ADAGFX_ENABLE_FRAMED_WINDOW
      success = true;
      break;
    case adagfx_commands_e::rot: // rot: Rotation

      if ((argCount == 1) && (nParams[0] >= 0) && (nParams[0] <= 3)) {
        setRotation(nParams[0]);
        success = true;
      }
      break;
    case adagfx_commands_e::tpm: // tpm: Text Print Mode

      if ((argCount == 1) && ((nParams[0] < 0) || (nParams[0] >= static_cast<int>(AdaGFXTextPrintMode::MAX)))) {
        _textPrintMode = static_cast<AdaGFXTextPrintMode>(nParams[0]);
        _display->setTextWrap(_textPrintMode == AdaGFXTextPrintMode::ContinueToNextLine);
        success = true;
      }
      break;
    # if ADAGFX_USE_ASCIITABLE
    case adagfx_commands_e::asciitable: // Show ASCII table
    {
      String line;
      const int16_t start        = 0x80 + (argCount >= 1 && nParams[0] >= -4 && nParams[0] < 4 ? nParams[0] * 0x20 : 0);
      const uint8_t scale        = (argCount == 2 && nParams[1] > 0 && nParams[1] <= 10 ? nParams[1] : 2);
      const uint8_t currentScale = _fontscaling;

      if (_fontscaling != scale) { // Set fontscaling
        _fontscaling = scale;
        _display->setTextSize(_fontscaling);
        calculateTextMetrics(_fontwidth, _fontheight, _heightOffset, _isProportional);
      }
      line.reserve(_textcols);
      _display->setCursor(0, 0);
      int16_t row = 0;
      setColumnRowMode(true);

      for (int16_t i = start; i <= 0xFF && row < _textrows; ++i) {
        if ((i % 4 == 0) && (line.length() > (_textcols - 8u))) { // 8 = 4x space + char
          printText(line.c_str(), 0, row, _fontscaling, _fgcolor, _bgcolor);
          line.clear();
          row++;
        }

        if (line.isEmpty()) {
          line += formatToHex(i, 2);
        }
        line += ' ';
        line += static_cast<char>(((i == 0x0A) || (i == 0x0D) ? 0x20 : i)); // Show a space instead of CR/LF
      }

      if (row < _textrows) {
        printText(line.c_str(), 0, row, _fontscaling, _fgcolor, _bgcolor);
      }

      setColumnRowMode(currentColRowState); // Restore

      if (_fontscaling != currentScale) {   // Restore if needed
        _fontscaling = currentScale;
        _display->setTextSize(_fontscaling);
        calculateTextMetrics(_fontwidth, _fontheight, _heightOffset, _isProportional);
      }
      success = true;
      break;
    }
    # endif // if ADAGFX_USE_ASCIITABLE
    case adagfx_commands_e::font: // font: Change font

      # if ADAGFX_FONTS_INCLUDED

      if (argCount == 1) {
        int font_i = 0;
        sParams[0].toLowerCase();

        constexpr int font_max = NR_ELEMENTS(fontargs);

        if ((nParams[0] > 0) || equals(sParams[0], F("0"))) {
          font_i = AdaGFXgetFontIndexForFontId(nParams[0]); // Set font by fontId
        } else {
          font_i = GetCommandCode(sParams[0].c_str(), adagfx_fonts);
        }

        if ((font_i >= 0) && (font_i < font_max)) {
          _fontId = fontargs[font_i]._fontId;
          _display->setFont(fontargs[font_i]._f);
          calculateTextMetrics(fontargs[font_i]._width,
                               fontargs[font_i]._height,
                               fontargs[font_i]._offset,
                               fontargs[font_i]._proportional);
          success = true;
        }
      }
      # endif  // if ADAGFX_FONTS_INCLUDED
      break;
    case adagfx_commands_e::l: // l: Line

      if (argCount == 5) {
        # if ADAGFX_ARGUMENT_VALIDATION

        if (!(invCoord_0_1 ||
              invCoord_2_3))
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          _display->drawLine(nParams[0] + _xo, nParams[1] + _yo, nParams[2] + _xo, nParams[3] + _yo,
                             AdaGFXparseColor(sParams[4], _colorDepth));
          success = true;
        }
      }
      break;
    case adagfx_commands_e::lh: // lh: Horizontal line

      if (argCount == 3) {
        # if ADAGFX_ARGUMENT_VALIDATION

        if (!((nParams[0] < 0) || (nParams[0] > res_x)))
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          _display->drawFastHLine(_xo, nParams[0] + _yo, nParams[1], AdaGFXparseColor(sParams[2], _colorDepth));
          success = true;
        }
      }
      break;
    case adagfx_commands_e::lv: // lv: Vertical line

      if (argCount == 3)
      {
        # if ADAGFX_ARGUMENT_VALIDATION

        if (!((nParams[0] < 0) || (nParams[0] > res_y)))
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          _display->drawFastVLine(nParams[0] + _xo, _yo, nParams[1], AdaGFXparseColor(sParams[2], _colorDepth));
          success = true;
        }
      }
      break;
    # if ADAGFX_ENABLE_EXTRA_CMDS
    case adagfx_commands_e::lm:
    case adagfx_commands_e::lmr: // lm/lmr: Multi-line, multiple coordinates

      if (argCount >= 5) {
        uint16_t mcolor         = AdaGFXparseColor(sParams[0], _colorDepth);
        bool     mloop          = true;
        uint8_t  parCount       = 0;
        uint8_t  optCount       = 0;
        int  cx                 = -1;
        int  cy                 = -1;
        bool closeLine          = false;
        const bool relativeMode = (adagfx_commands_e::lmr == subcmd); // Use Relative mode
        success = true;

        while (mloop) {
          sParams[optCount] = parseString(string, parCount + 4);       // 0-offset + 1st and 2nd cmd-argument and 1 for color argument

          if (!validIntFromString(sParams[optCount], nParams[optCount]) && !sParams[optCount].isEmpty()) {
            mcolor = AdaGFXparseColor(sParams[optCount], _colorDepth); // Interpret as a color

            if (optCount > 0) { optCount--; }
          }
          mloop     = !sParams[optCount].isEmpty();
          closeLine = equals(sParams[optCount], 'c');

          if (mloop) { parCount++; optCount++; } // Next argument

          if ((optCount == 4) || closeLine) { // 0..3 = 4th argument or close the line
            if (relativeMode) {
              nParams[2] += nParams[0];
              nParams[3] += nParams[1];
            }
            #  if ADAGFX_ARGUMENT_VALIDATION

            if (invalidCoordinates(nParams[0], nParams[1]) ||
                invalidCoordinates(nParams[2], nParams[3])) {
              success = false;
              mloop   = false; // break out
            } else
            #  endif // if ADAGFX_ARGUMENT_VALIDATION
            {
              if (closeLine) {
                nParams[2] = cx;
                nParams[3] = cy;
                mloop      = false; // Exit after closing the line
              }
              #  ifndef BUILD_NO_DEBUG

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                addLog(LOG_LEVEL_INFO, strformat(F("AdaGFX: cmd: lm x/y/x1/y1:%d/%d/%d/%d loop:%c color:%s"),
                                                 nParams[0], nParams[1], nParams[2], nParams[3],
                                                 mloop ? 'T' : 'f', AdaGFXcolorToString(mcolor, _colorDepth).c_str()));
              }
              #  endif // ifndef BUILD_NO_DEBUG
              _display->drawLine(nParams[0] + _xo, nParams[1] + _yo, nParams[2] + _xo, nParams[3] + _yo, mcolor);

              if ((cx == -1) && (cy == -1)) {
                cx = nParams[0];
                cy = nParams[1];
              }
              nParams[0] = nParams[2]; // Move second set to first set
              nParams[1] = nParams[3];
              optCount   = 2;          // Get second set of arguments only
            }
          }
        }
      }
      break;
    # endif // if ADAGFX_ENABLE_EXTRA_CMDS
    case adagfx_commands_e::r:  // r: Rectangle
    case adagfx_commands_e::rf: // rf: Rectangled, filled

      if ((argCount == 5) ||
          (argCount == 6)) {
        # if ADAGFX_ARGUMENT_VALIDATION

        if (!(invCoord_0_1 ||
              invCoord_0_2_1_3))
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          if ((adagfx_commands_e::rf == subcmd) && (argCount == 6)) {
            _display->fillRect(nParams[0] + _xo, nParams[1] + _yo, nParams[2], nParams[3], AdaGFXparseColor(sParams[5], _colorDepth));
          }
          _display->drawRect(nParams[0] + _xo, nParams[1] + _yo, nParams[2], nParams[3], AdaGFXparseColor(sParams[4], _colorDepth));
          success = true;
        }
      }
      break;
    case adagfx_commands_e::c:  // c: Circle
    case adagfx_commands_e::cf: // cf: Circle, filled

      if ((argCount == 4) ||
          (argCount == 5)) {
        # if ADAGFX_ARGUMENT_VALIDATION

        if (!(invCoord_0_1 ||
              invalidCoordinates(nParams[2], 0))) // Also check radius
        # endif  // if ADAGFX_ARGUMENT_VALIDATION
        {
          if ((adagfx_commands_e::cf == subcmd) && (argCount == 5)) {
            _display->fillCircle(nParams[0] + _xo, nParams[1] + _yo, nParams[2], AdaGFXparseColor(sParams[4], _colorDepth));
          }
          _display->drawCircle(nParams[0] + _xo, nParams[1] + _yo, nParams[2], AdaGFXparseColor(sParams[3], _colorDepth));
          success = true;
        }
      }
      break;
    case adagfx_commands_e::t:  // t: Triangle
    case adagfx_commands_e::tf: // tf: Triangle, filled

      if ((argCount == 7) ||
          (argCount == 8)) {
        # if ADAGFX_ARGUMENT_VALIDATION

        if (!(invCoord_0_1 ||
              invCoord_2_3 ||
              invalidCoordinates(nParams[4], nParams[5])))
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          if ((adagfx_commands_e::tf == subcmd) && (argCount == 8)) {
            _display->fillTriangle(nParams[0] + _xo,
                                   nParams[1] + _yo,
                                   nParams[2] + _xo,
                                   nParams[3] + _yo,
                                   nParams[4] + _xo,
                                   nParams[5] + _yo,
                                   AdaGFXparseColor(sParams[7], _colorDepth));
          }
          _display->drawTriangle(nParams[0] + _xo,
                                 nParams[1] + _yo,
                                 nParams[2] + _xo,
                                 nParams[3] + _yo,
                                 nParams[4] + _xo,
                                 nParams[5] + _yo,
                                 AdaGFXparseColor(sParams[6], _colorDepth));
          success = true;
        }
      }
      break;
    case adagfx_commands_e::rr:  // rr: Rounded rectangle
    case adagfx_commands_e::rrf: // rrf: Rounded rectangle, filled

      if ((argCount == 6) ||
          (argCount == 7)) {
        # if ADAGFX_ARGUMENT_VALIDATION

        if (!(invCoord_0_1 ||
              invCoord_0_2_1_3 ||
              invalidCoordinates(nParams[4], 0))) // Also check radius
        # endif  // if ADAGFX_ARGUMENT_VALIDATION
        {
          if ((adagfx_commands_e::rrf == subcmd) && (argCount == 7)) {
            _display->fillRoundRect(nParams[0] + _xo,
                                    nParams[1] + _yo,
                                    nParams[2],
                                    nParams[3],
                                    nParams[4],
                                    AdaGFXparseColor(sParams[6], _colorDepth));
          }
          _display->drawRoundRect(nParams[0] + _xo,
                                  nParams[1] + _yo,
                                  nParams[2],
                                  nParams[3],
                                  nParams[4],
                                  AdaGFXparseColor(sParams[5], _colorDepth));
          success = true;
        }
      }
      break;
    case adagfx_commands_e::px: // px: Pixel

      if (argCount == 3) {
        # if ADAGFX_ARGUMENT_VALIDATION

        if (!invCoord_0_1)
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          _display->drawPixel(nParams[0] + _xo, nParams[1] + _yo, AdaGFXparseColor(sParams[2], _colorDepth));
          success = true;
        }
      }
      break;
    case adagfx_commands_e::pxh:
    case adagfx_commands_e::pxv: // pxh/pxv: Pixels, hor./vert.

      if (argCount > 2) {
        // incremented merged loop is smaller than 2 separate loops
        # if ADAGFX_ARGUMENT_VALIDATION

        if (!invCoord_0_1)
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          _display->startWrite();
          _display->writePixel(nParams[0] + _xo, nParams[1] + _yo, AdaGFXparseColor(sParams[2], _colorDepth));
          loop = true;
          uint8_t h     = 0;
          uint8_t v     = 0;
          bool    isPxh = (adagfx_commands_e::pxh == subcmd);

          if (isPxh) {
            h++;
          } else {
            v++;
          }

          while (loop) {
            String color = parseString(string, h + v + 5); // 5 = 2 + 3 already parsed merged loop is smaller than 2 separate loops

            if (color.isEmpty()
            # if ADAGFX_ARGUMENT_VALIDATION
                || invalidCoordinates(nParams[0] + h + _xo, nParams[1] + v + _yo)
            # endif // if ADAGFX_ARGUMENT_VALIDATION
                ) {
              loop = false;
            } else {
              _display->writePixel(nParams[0] + h + _xo, nParams[1] + v + _yo, AdaGFXparseColor(color, _colorDepth));

              if (isPxh) {
                h++;
              } else {
                v++;
              }
            }
            delay(0);
          }
          _display->endWrite();
          success = true;
        }
      }
      break;
    # if ADAGFX_ENABLE_BMP_DISPLAY
    case adagfx_commands_e::bmp: // bmp,x,y,filename.bmp : show bmp from file

      if ((argCount == 3) && !sParams[2].isEmpty()) {
        success = showBmp(sParams[2], nParams[0] + _xo, nParams[1] + _yo);
      }
      break;
    # endif // if ADAGFX_ENABLE_BMP_DISPLAY
    # if ADAGFX_ENABLE_BUTTON_DRAW
    case adagfx_commands_e::btn:

      if ((argCount >= 8) && (nParams[7] != 0)) {
        // btn,state,m,x,y,w,h,id,type[,ONclr,OFFclr,Captionclr,fontscale,ONcaption,OFFcapt,Borderclr,DisabClr,DisabCaptclr],TaskIndex,Group,SelGrp,objectname
        // ev: 1     2 3 4 5 6 7  8     9     10     11         12        13        14      15        16       17,18,19,20,21
        // nP: 0     1 2 3 4 5 6  7     8     9      10         11        12        13      14        15       16,17,18,19,20
        // : Draw a button
        // m=mode: -2 = disabled, -1 = initial, 0 = default
        // state:  0 = off, 1 = on, -2 = off + disabled, -1 = on + disabled
        // id: < 0 = clear area
        // type & 0x0F: 0 = none, 1 = rectangle, 2 = rounded rect., 3 = circle,
        // type & 0xF0 = CenterAligned, LeftAligned, TopAligned, RightAligned, BottomAligned, LeftTopAligned, RightTopAligned,
        //               RightBottomAligned, LeftBottomAligned, NoCaption, Bitmap, Slider (not a button)
        // (*clr = color, TaskIndex, Group and SelGrp are ignored)
        #  if ADAGFX_ARGUMENT_VALIDATION

        if (!(invCoord_2_3 ||
              invalidCoordinates(nParams[2] + nParams[4], nParams[3] + nParams[5])))
        #  endif  // if ADAGFX_ARGUMENT_VALIDATION
        {
          // All checked out OK
          // Default values
          uint16_t onColor              = ADAGFX_BLUE;
          uint16_t offColor             = ADAGFX_RED;
          uint16_t captionColor         = ADAGFX_WHITE;
          uint8_t  fontScale            = 0;
          uint16_t borderColor          = ADAGFX_WHITE;
          uint16_t disabledColor        = 0x9410; // Medium grey
          uint16_t disabledCaptionColor = 0x5A69; // Dark grey
          success = true;

          if (!sParams[8].isEmpty()) { onColor = AdaGFXparseColor(sParams[8], _colorDepth); }

          if (!sParams[9].isEmpty()) { offColor = AdaGFXparseColor(sParams[9], _colorDepth); }

          if (!sParams[10].isEmpty()) { captionColor = AdaGFXparseColor(sParams[10], _colorDepth); }

          if ((nParams[11] > 0) && (nParams[11] <= 10)) { fontScale = nParams[11]; }

          if (!sParams[14].isEmpty()) { borderColor = AdaGFXparseColor(sParams[14], _colorDepth); }

          if (!sParams[15].isEmpty()) { disabledColor = AdaGFXparseColor(sParams[15], _colorDepth); }

          if (!sParams[16].isEmpty()) { disabledCaptionColor = AdaGFXparseColor(sParams[16], _colorDepth); }

          uint16_t   fillColor = onColor;
          uint16_t   textColor = captionColor;
          const bool clearArea = nParams[7] < 0;
          nParams[7] = std::abs(nParams[7]);

          const Button_type_e   buttonType   = static_cast<Button_type_e>(nParams[7] & 0x0F);
          const Button_layout_e buttonLayout = static_cast<Button_layout_e>(nParams[7] & 0xF0);

          // Check mode & state: -2, -1, 0, 1 to select used colors
          #  if ADAGFX_ENABLE_BUTTON_SLIDER

          if (buttonLayout == Button_layout_e::Slider) {
            if (nParams[1] == -2) {
              fillColor = disabledColor;
              textColor = disabledCaptionColor;
            } else if (clearArea) {
              fillColor   = _bgcolor; //
              borderColor = _bgcolor;
            }
          } else
          #  endif // if ADAGFX_ENABLE_BUTTON_SLIDER
          {
            if (nParams[0] == 0) {
              fillColor = offColor;
            }

            if ((nParams[1] == -2) || (nParams[0] < 0)) {
              fillColor = disabledColor;
              textColor = disabledCaptionColor;
            } else if (clearArea) {
              fillColor   = _bgcolor; //
              borderColor = _bgcolor;
            }
          }

          // Clear the area?
          if ((buttonType != Button_type_e::None) ||
              clearArea) {
            drawButtonShape(
              #  if ADAGFX_ENABLE_BUTTON_SLIDER
              buttonLayout == Button_layout_e::Slider ? Button_type_e::Square :
              #  endif // if ADAGFX_ENABLE_BUTTON_SLIDER
              buttonType, // Clear full square for slider
              nParams[2] + _xo, nParams[3] + _yo, nParams[4], nParams[5],
              _bgcolor, _bgcolor);
          }

          // Check button-type bits (mask: 0x0F) to draw correct shape
          if (!clearArea) {
            drawButtonShape(buttonType,
                            nParams[2] + _xo, nParams[3] + _yo, nParams[4], nParams[5],
                            fillColor, borderColor);
          }

          // Display caption? (or bitmap)
          if (!clearArea &&
              (buttonLayout != Button_layout_e::NoCaption)) {
            int16_t  x1, y1;
            uint16_t w1, h1, w2, h2;
            String   newString;

            // Determine alignment parameters
            if ((nParams[0] == 1) || (nParams[0] == -1) // 1 = on+enabled, -1 = on+disabled
                #  if ADAGFX_ENABLE_BUTTON_SLIDER
                || (buttonLayout == Button_layout_e::Slider)
                #  endif // if ADAGFX_ENABLE_BUTTON_SLIDER
                ) {
              newString = sParams[12].isEmpty() ? sParams[6] : sParams[12];
            } else {
              newString = sParams[13].isEmpty() ? sParams[6] : sParams[13];
            }
            newString = AdaGFXparseTemplate(newString, 20);

            _display->setTextSize(fontScale);                             // set scaling
            _display->getTextBounds(newString, 0, 0, &x1, &y1, &w1, &h1); // get caption length and height in pixels
            _display->getTextBounds(F(" "),    0, 0, &x1, &y1, &w2, &h2); // measure space width for little margins

            // Check button-alignment bits (mask 0xF0) for caption placement, modifies the x/y arguments passed!
            // Little margin is: from left/right: half of the width of a space, from top/bottom: half of height of the font used

            switch (buttonLayout) {
              case Button_layout_e::CenterAligned:
                nParams[2] += (nParams[4] / 2 - w1 / 2);  // center horizontically
                nParams[3] += (nParams[5] / 2 - h1 / 2);  // center vertically
                break;
              case Button_layout_e::LeftAligned:
                nParams[2] += w2 / 2;                     // A little margin from left
                nParams[3] += (nParams[5] / 2 - h1 / 2);  // center vertically
                break;
              case Button_layout_e::TopAligned:
                nParams[2] += (nParams[4] / 2 - w1 / 2);  // center horizontically
                nParams[3] += h1 / 2;                     // A little margin from top
                break;
              case Button_layout_e::RightAligned:
                nParams[2] += (nParams[4] - w1) - w2 / 2; // right-align + a little margin
                nParams[3] += (nParams[5] / 2 - h1 / 2);  // center vertically
                break;
              case Button_layout_e::BottomAligned:
                nParams[2] += (nParams[4] / 2 - w1 / 2);  // center horizontically
                nParams[3] += (nParams[5] - h1 * 1.5);    // bottom align + a little margin
                break;
              case Button_layout_e::LeftTopAligned:
                nParams[2] += w2 / 2;                     // A little margin from left
                nParams[3] += h1 / 2;                     // A little margin from top
                break;
              case Button_layout_e::RightTopAligned:
                nParams[2] += (nParams[4] - w1) - w2 / 2; // right-align + a little margin
                nParams[3] += h1 / 2;                     // A little margin from top
                break;
              case Button_layout_e::RightBottomAligned:
                nParams[2] += (nParams[4] - w1) - w2 / 2; // right-align + a little margin
                nParams[3] += (nParams[5] - h1 * 1.5);    // bottom align + a little margin
                break;
              case Button_layout_e::LeftBottomAligned:
                nParams[2] += w2 / 2;                     // A little margin from left
                nParams[3] += (nParams[5] - h1 * 1.5);    // bottom align + a little margin
                break;
              #  if ADAGFX_ENABLE_BMP_DISPLAY
              case Button_layout_e::Bitmap:
              {                     // Use ON/OFF caption to specify (full) bitmap filename
                if (!newString.isEmpty()) {
                  int32_t offX = 0; // Allow optional arguments for x and y offset values, usage:
                  int32_t offY = 0; // [x,[y,]]filename.bmp

                  if (newString.indexOf(',') > -1) {
                    String tmp = parseString(newString, 1);
                    validIntFromString(tmp, offX);
                    newString = parseStringToEndKeepCase(newString, 2);

                    if (newString.indexOf(',') > -1) {
                      tmp = parseString(newString, 1);
                      validIntFromString(tmp, offY);
                      newString = parseStringToEndKeepCase(newString, 2);
                    }
                  }
                  success = showBmp(newString, nParams[2] + _xo + offX, nParams[3] + _yo + offY);
                } else {
                  success = false;
                }
                break;
              }
              #  endif // if ADAGFX_ENABLE_BMP_DISPLAY
              case Button_layout_e::NoCaption:
              #  if ADAGFX_ENABLE_BUTTON_SLIDER
              case Button_layout_e::Slider: // Nothing to do here (yet)
              #  endif // if ADAGFX_ENABLE_BUTTON_SLIDER
                break;
            }

            if ((buttonLayout != Button_layout_e::NoCaption)
                #  if ADAGFX_ENABLE_BUTTON_SLIDER
                && (buttonLayout != Button_layout_e::Slider)
                #  endif // if ADAGFX_ENABLE_BUTTON_SLIDER
                #  if ADAGFX_ENABLE_BMP_DISPLAY
                && (buttonLayout != Button_layout_e::Bitmap)
                #  endif // if ADAGFX_ENABLE_BMP_DISPLAY
                ) {
              // Set position and colors, then print
              _display->setCursor(nParams[2] + _xo, nParams[3] + _yo);
              _display->setTextColor(textColor, textColor); // transparent bg results in button color
              _display->print(newString);

              // restore colors
              _display->setTextColor(_fgcolor, _bgcolor);
            }
            #  if ADAGFX_ENABLE_BUTTON_SLIDER

            if (buttonLayout == Button_layout_e::Slider) {
              // 1) Determine direction from w/h
              const bool isVertical   = nParams[4] < nParams[5]; // width < height
              const bool showAsCircle = borderColor == fillColor;

              // determine value and range
              int16_t offI2            = 5;  // half of indicator width
              int16_t offG2            = 3;  // half of Gauge width
              int16_t offP             = 0;  // Offset for indicator
              int16_t zeroLine         = -1; // Draw a range zero-line at this offset? only when >= 0
              int     percentage       = 0;
              float   gaugeValue       = 0.0f;
              int16_t lowRange         = 0;
              int16_t highRange        = 100;
              float   rangeFrom        = 0.0f;
              float   rangeTo          = 0.0f;
              float   range            = 100.0f; // For percentage the range is 100
              bool    useRange         = false;
              bool    hasRangeReversed = false;  // Range low value left or top, high value right or bottom

              if (!validFloatFromString(newString, gaugeValue)) {
                percentage = nParams[0];         // Value as provided
              }

              // Have a range?
              if (!sParams[13].isEmpty()) { // Off caption can hold range: <from>,<to>
                String tmp           = parseString(sParams[13], 1);
                const bool validFrom = validFloatFromString(tmp, rangeFrom);
                tmp = parseString(sParams[13], 2);

                if (validFrom && validFloatFromString(tmp, rangeTo) &&
                    !essentiallyEqual(rangeFrom, 0.0f) && !essentiallyEqual(rangeTo, 0.0f)) {
                  useRange         = true;
                  lowRange         = static_cast<uint16_t>(rangeFrom);
                  highRange        = static_cast<uint16_t>(rangeTo);
                  hasRangeReversed = lowRange > highRange; // Range high value left or top, low value right or bottom?
                }
              }

              // 2) Draw center-line from 0 to 100%
              // 3) Draw gauge for used/filled part
              // 4) Draw indicator at correct percentage index
              if (showAsCircle) { // Circle indicator or full width bar indicator
                offI2 = (isVertical ? nParams[4] : nParams[5]) / 4;
              }

              if (useRange) { // Calculate range-boundaries
                range = abs(max(rangeTo, rangeFrom) - min(rangeFrom, rangeTo));

                if (gaugeValue > max(rangeTo, rangeFrom)) {
                  gaugeValue = max(rangeTo, rangeFrom);
                } else if (gaugeValue < min(rangeFrom, rangeTo)) {
                  gaugeValue = min(rangeFrom, rangeTo);
                } else {
                  gaugeValue -= min(rangeFrom, rangeTo); // Give it the correct Offset
                }

                if (((lowRange < 0) && (highRange > 0)) ||
                    ((lowRange > 0) && (highRange < 0))) {
                  zeroLine = map(0, lowRange, highRange, 0, isVertical ? nParams[5] : nParams[4]);
                }
              }
              percentage = static_cast<int>(gaugeValue);

              offP = ((((isVertical ? nParams[5] : nParams[4]) - (2 * offI2)) / range) * percentage) - 1; // keep within button borders

              if (hasRangeReversed) {
                offP = (isVertical ? nParams[5] : nParams[4]) - (2 * offI2) - offP - 1;                   // flip
              }

              if (isVertical) {
                // centerline
                _display->drawLine(nParams[2] + _xo + nParams[4] / 2, nParams[3] + _yo + (nParams[5] - offI2 - 1),
                                   nParams[2] + _xo + nParams[4] / 2, nParams[3] + _yo + offI2, textColor);

                if (zeroLine > -1) {
                  _display->drawLine(nParams[2] + _xo, nParams[3] + _yo + (nParams[5] - zeroLine - 1),
                                     nParams[2] + _xo + nParams[4], nParams[3] + _yo + (nParams[5] - zeroLine - 1), textColor);
                }

                // Gauge
                if (hasRangeReversed) {
                  int16_t bar = nParams[5] - (2 * offI2);
                  _display->fillRoundRect(nParams[2] + _xo + (nParams[4] / 2) - offG2, nParams[3] + _yo + offI2 + 0,
                                          2 * offG2, bar - offP, offG2, textColor);
                } else {
                  _display->fillRoundRect(nParams[2] + _xo + (nParams[4] / 2) - offG2, nParams[3] + _yo + (nParams[5] - offI2 - offP - 1),
                                          2 * offG2, offP, offG2, textColor);
                }

                // Indicator/drag-handle
                if (showAsCircle) { // Circle indicator
                  _display->fillCircle(nParams[2] + _xo + nParams[4] / 2,
                                       nParams[3] + _yo + (nParams[5] - offI2 - offP - 2) + (percentage == 100 ? 1 : 0),
                                       trunc(nParams[4] / 4), textColor);
                } else {
                  _display->fillRoundRect(nParams[2] + _xo + 1,
                                          nParams[3] + _yo + (nParams[5] - (2 * offI2) - offP - (hasRangeReversed ? 0 : 1)),
                                          nParams[4] - 2, 2 * offI2, offI2, textColor);
                }
              } else { // : if !isVertical -> isHorizontal
                // centerline
                _display->drawLine(nParams[2] + _xo + offI2 + 1, nParams[3] + _yo + nParams[5] / 2,
                                   nParams[2] + _xo + nParams[4] - offI2, nParams[3] + _yo + nParams[5] / 2, textColor);

                if (zeroLine > -1) {
                  _display->drawLine(nParams[2] + _xo + zeroLine + 1, nParams[3] + _yo,
                                     nParams[2] + _xo + zeroLine + 1, nParams[3] + _yo + nParams[5], textColor);
                }

                // Gauge
                if (hasRangeReversed) {
                  int16_t bar = nParams[4] - (2 * offI2);
                  _display->fillRoundRect(nParams[2] + _xo + offI2 + offP + 2, nParams[3] + _yo + (nParams[5] / 2) - offG2,
                                          bar - offP, offG2 * 2, offG2, textColor);
                } else {
                  _display->fillRoundRect(nParams[2] + _xo + offI2 + 1, nParams[3] + _yo + (nParams[5] / 2) - offG2,
                                          offP, offG2 * 2, offG2, textColor);
                }

                // Indicator/drag-handle
                if (showAsCircle) { // Circle indicator
                  _display->fillCircle(nParams[2] + _xo + offP + offI2 + 1 - (percentage == 100 ? 1 : 0),
                                       nParams[3] + _yo + (nParams[5] / 2),
                                       trunc(nParams[5] / 4), textColor);
                } else {
                  _display->fillRoundRect(nParams[2] + _xo + offP + (hasRangeReversed ? 0 : 1), nParams[3] + _yo + 1,
                                          2 * offI2, nParams[5] - 2, offI2, textColor);
                }
              }

              // 5) Draw percentage in center if Fontsize > 0
              if (fontScale > 0) {
                nParams[2] += (nParams[4] / 2 - w1 / 2);      // center horizontically
                nParams[3] += (nParams[5] / 2 - h1 / 2);      // center vertically
                _display->setCursor(nParams[2] + _xo, nParams[3] + _yo);
                _display->setTextColor(textColor, fillColor); // regular bg color for readability
                _display->print(newString);
              }
            }
            #  endif // if ADAGFX_ENABLE_BUTTON_SLIDER

            // restore font scaling
            _display->setTextSize(_fontscaling);
          }
        }
      }
      break;
    # endif // if ADAGFX_ENABLE_BUTTON_DRAW
    # if ADAGFX_ENABLE_FRAMED_WINDOW
    case adagfx_commands_e::win: // win: select window by id

      if ((argCount >= 1) && (argCount <= 2)) {
        success = selectWindow(nParams[0], nParams[1]);
      }
      break;
    case adagfx_commands_e::defwin: // defwin: define window

      if ((argCount >= 5) && (argCount <= 6)) {
        const int8_t rot = _rotation;
        #  if ADAGFX_ARGUMENT_VALIDATION
        const int16_t curWin = getWindow();

        if (curWin != 0) { selectWindow(0); }           // Validate against raw window coordinates

        if (argCount == 6) { setRotation(nParams[5]); } // Use requested rotation

        if (invCoord_0_1 ||
            invCoord_0_2_1_3) {
          if (curWin != 0) { selectWindow(curWin); }  // restore current window

          if (rot != _rotation) { setRotation(rot); } // Restore rotation
        } else
        #  endif  // if ADAGFX_ARGUMENT_VALIDATION
        {
          #  if ADAGFX_ARGUMENT_VALIDATION

          if (curWin != 0) { selectWindow(curWin); } // restore current window
          #  endif // if ADAGFX_ARGUMENT_VALIDATION

          if (nParams[4] > 0) {                  // Window 0 is the raw window, having the full size, created at initialization of this
            success = true;

            // helper instance
            #  ifndef BUILD_NO_DEBUG
            int16_t win = // avoid compiler warning
            #  endif // ifndef BUILD_NO_DEBUG
            defineWindow(nParams[0],
                         nParams[1],
                         nParams[2],
                         nParams[3],
                         nParams[4],
                         argCount == 6 ? nParams[5] : _rotation);
            #  ifndef BUILD_NO_DEBUG

            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              addLogMove(LOG_LEVEL_INFO, strformat(F("AdaGFX defined window id: %d"), win));
            }
            #  endif // ifndef BUILD_NO_DEBUG

            if (rot != _rotation) { setRotation(rot); } // Restore rotation, also update new window
          }
        }
      }
      break;
    case adagfx_commands_e::delwin: // delwin: delete window, don't delete window 0

      if ((argCount == 1) && (nParams[0] > 0)) {
        // logWindows(F(" deLwin ")); // use for debugging only

        success = deleteWindow(nParams[0]);
      }
      break;
      # endif // if ADAGFX_ENABLE_FRAMED_WINDOW
  }
  return success;
}

/****************************************************************************
 * Get a config value from the plugin
 ***************************************************************************/
# if ADAGFX_ENABLE_GET_CONFIG_VALUE
const char adagfx_getcommands[] PROGMEM = "win|iswin|width|height|length|textheight|rot|txs|tpm"
                                          #  if ADAGFX_FONTS_INCLUDED
                                          "|font"
                                          #  endif // if ADAGFX_FONTS_INCLUDED
;
enum class adagfx_getcommands_e : int8_t {
  invalid = -1,
  win     = 0,
  iswin,
  width,
  height,
  length,
  textheight,
  rot,
  txs,
  tpm,
  #  if ADAGFX_FONTS_INCLUDED
  font,
  #  endif // if ADAGFX_FONTS_INCLUDED
};

bool AdafruitGFX_helper::pluginGetConfigValue(String& string) {
  bool success = false;
  char sep     = '.';

  if ((-1 == string.indexOf(sep)) && (string.indexOf(',') >= 0)) {
    sep = ',';
  }
  String command = parseString(string, 1, sep);

  const int command_i            = GetCommandCode(command.c_str(), adagfx_getcommands);
  const adagfx_getcommands_e cmd = static_cast<adagfx_getcommands_e>(command_i);

  switch (cmd) {
    case adagfx_getcommands_e::win:
    {                                   // win: get current window id
      #  if ADAGFX_ENABLE_FRAMED_WINDOW // if feature enabled
      string  = getWindow();
      success = true;
      #  endif // if ADAGFX_ENABLE_FRAMED_WINDOW
      break;
    }
    case adagfx_getcommands_e::iswin:
    {                                   // iswin: check if windows exists
      #  if ADAGFX_ENABLE_FRAMED_WINDOW // if feature enabled
      command = parseString(string, 2, sep);
      int32_t win = 0;

      if (validIntFromString(command, win)) {
        string = validWindow(static_cast<uint8_t>(win));
      } else {
        string = '0';
      }
      success = true; // Always correct, just return 'false' if wrong
      #  endif // if ADAGFX_ENABLE_FRAMED_WINDOW
      break;
    }
    case adagfx_getcommands_e::width:
    case adagfx_getcommands_e::height:
      // width/height: get window width or height
    {
      #  if ADAGFX_ENABLE_FRAMED_WINDOW // if feature enabled
      uint16_t w = 0, h = 0;
      getWindowLimits(w, h);

      if (adagfx_getcommands_e::width == cmd) {
        string = w;
      } else {
        string = h;
      }
      success = true;
      #  endif // if ADAGFX_ENABLE_FRAMED_WINDOW
      break;
    }
    case adagfx_getcommands_e::length:
    case adagfx_getcommands_e::textheight:
      // length/textheight: get text length or height
    {
      int16_t  x1, y1;
      uint16_t w1, h1;
      String   newString = AdaGFXparseTemplate(parseStringToEndKeepCaseNoTrim(string, 2), 0);
      _display->getTextBounds(newString, 0, 0, &x1, &y1, &w1, &h1); // Count length and height

      if (adagfx_getcommands_e::length == cmd) {
        string = w1;
      } else {
        string = h1;
      }
      success = true;
      break;
    }
    case adagfx_getcommands_e::rot:
    { // rot: get current rotation setting
      string  = _rotation;
      success = true;
      break;
    }
    case adagfx_getcommands_e::txs:
    { // txs: get current text scaling setting
      string  = _fontscaling;
      success = true;
      break;
    }
    case adagfx_getcommands_e::tpm:
    { // tpm: get current text print mode setting
      string  = static_cast<int>(_textPrintMode);
      success = true;
      break;
    }
    #  if ADAGFX_FONTS_INCLUDED
    case adagfx_getcommands_e::font:
      string  = AdaGFXgetFontName(_fontId);
      success = true;
      break;
    #  endif // if ADAGFX_FONTS_INCLUDED
    case adagfx_getcommands_e::invalid:
      break;
  }

  return success;
}

# endif // if ADAGFX_ENABLE_GET_CONFIG_VALUE

/****************************************************************************
 * draw a button shape with provided color, can also clear a previously drawn button
 ***************************************************************************/
# if ADAGFX_ENABLE_BUTTON_DRAW
void AdafruitGFX_helper::drawButtonShape(const Button_type_e& buttonType,
                                         const int          & x,
                                         const int          & y,
                                         const int          & w,
                                         const int          & h,
                                         const uint16_t     & fillColor,
                                         const uint16_t     & borderColor) {
  switch (buttonType) {
    case Button_type_e::Square: // Rectangle
    {
      _display->fillRect(x, y, w, h, fillColor);
      _display->drawRect(x, y, w, h, borderColor);
      break;
    }
    case Button_type_e::Rounded:     // Rounded Rectangle
    {
      int16_t radius = (w + h) / 20; // average 10 % corner radius w/h
      _display->fillRoundRect(x, y, w, h, radius, fillColor);
      _display->drawRoundRect(x, y, w, h, radius, borderColor);
      break;
    }
    case Button_type_e::Circle:     // Circle
    {
      int16_t radius = (w + h) / 4; // average radius
      _display->fillCircle(x + (w / 2), y + (h / 2), radius, fillColor);
      _display->drawCircle(x + (w / 2), y + (h / 2), radius, borderColor);
      break;
    }
    case Button_type_e::ArrowLeft:
    { // draw: left-center, right-top, right-bottom
      _display->fillTriangle(x, y + h / 2, x + w, y,
                             x + w, y + h, fillColor);
      _display->drawTriangle(x, y + h / 2, x + w, y,
                             x + w, y + h, borderColor);
      break;
    }
    case Button_type_e::ArrowUp:
    { // draw: top-center, right-bottom, left-bottom
      _display->fillTriangle(x + w / 2, y, x + w, y + h,
                             x, y + h, fillColor);
      _display->drawTriangle(x + w / 2, y, x + w, y + h,
                             x, y + h, borderColor);
      break;
    }
    case Button_type_e::ArrowRight:
    { // draw: left-top, right-center, left-bottom
      _display->fillTriangle(x, y, x + w, y + h / 2,
                             x, y + h, fillColor);
      _display->drawTriangle(x, y, x + w, y + h / 2,
                             x, y + h, borderColor);
      break;
    }
    case Button_type_e::ArrowDown:
    { // draw: left-top, right-top, bottom-center
      _display->fillTriangle(x, y, x + w, y,
                             x + w / 2, y + h, fillColor);
      _display->drawTriangle(x, y, x + w, y,
                             x + w / 2, y + h, borderColor);
      break;
    }
    case Button_type_e::None:
      break;
  }
}

# endif // if ADAGFX_ENABLE_BUTTON_DRAW

/****************************************************************************
 * printText: Print text on display at a specific pixel or column/row location
 ***************************************************************************/
void AdafruitGFX_helper::printText(const char     *string,
                                   const int16_t & X,
                                   const int16_t & Y,
                                   const uint8_t & textSize,
                                   const uint16_t& color,
                                   uint16_t        bkcolor,
                                   const uint16_t& maxWidth) {
  int16_t  _x        = X;
  int16_t  _y        = Y + (_heightOffset * textSize);
  uint16_t _w        = 0;
  int16_t  xText     = 0;
  int16_t  yText     = 0;
  uint16_t wText     = 0;
  uint16_t wChar     = 0;
  uint16_t hText     = 0;
  int16_t  oTop      = 0;
  int16_t  oBottom   = 0;
  int16_t  oLeft     = 0;
  uint16_t xOffset   = 0;
  uint16_t hChar1    = 0;
  uint16_t wChar1    = 0;
  String   newString = string;
  uint16_t res_x     = _res_x;

  # if ADAGFX_ENABLE_FRAMED_WINDOW
  uint16_t res_y   = _res_y;
  uint16_t yOffset = 0;
  # endif // if ADAGFX_ENABLE_FRAMED_WINDOW

  # if ADAGFX_ENABLE_FRAMED_WINDOW
  getWindowLimits(res_x, res_y);
  getWindowOffsets(xOffset, yOffset);
  _x += xOffset;
  _y += yOffset;
  # endif // if ADAGFX_ENABLE_FRAMED_WINDOW

  _display->setTextSize(textSize);
  _display->getTextBounds(String('A'), 0, 0, &xText, &yText, &wChar1, &hChar1); // Calculate ~1 char height

  if (_columnRowMode) {
    _x = X * (_fontwidth * textSize);                                           // We need this multiple times

    if (15 == _lineSpacing) {
      _y = (Y * (_fontheight * textSize)) + (_heightOffset * textSize);
    } else {
      _y = (Y * (hChar1 + _lineSpacing)) + _heightOffset; // Apply explicit line spacing
    }
  }

  _display->setCursor(_x, _y);
  _display->setTextColor(color, bkcolor);

  if (_textPrintMode != AdaGFXTextPrintMode::ContinueToNextLine) {
    # if ADAGFX_ENABLE_FRAMED_WINDOW

    if (0 == getWindow()) // Only on Window 0
    # endif // if ADAGFX_ENABLE_FRAMED_WINDOW
    {
      wChar = wChar1;
    }
    _display->getTextBounds(newString, _x, _y, &xText, &yText, &wText, &hText);   // Calculate length

    while ((newString.length() > 0) && (((_x - xOffset) + wText) > res_x + wChar)) {
      newString.remove(newString.length() - 1);                                   // Cut last character off
      _display->getTextBounds(newString, _x, _y, &xText, &yText, &wText, &hText); // Re-calculate length
    }
  }

  _display->getTextBounds(newString, _x, _y, &xText, &yText, &wText, &hText); // Calculate length

  if ((maxWidth > 0) && ((_x - xOffset) + maxWidth <= res_x)) {
    res_x = (_x - xOffset) + maxWidth;
    _w    = maxWidth;

    if ((_textPrintMode == AdaGFXTextPrintMode::TruncateExceedingCentered) &&
        (maxWidth > wText)) {
      oLeft = (_w - (wText + 2 * (xText - _x))) / 2;
    }
  } else {
    _w = wText + 2 * (xText - _x);
  }

  if (_textBackFill && (color != bkcolor)) { // Fill extra space above and below text
    oTop    -= textSize;
    oBottom += textSize;
    _y      += textSize;
  }

  if ((_textPrintMode == AdaGFXTextPrintMode::ClearThenTruncate) ||
      (color != bkcolor)) { // Clear before print
    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLogMove(LOG_LEVEL_DEBUG, strformat(F("printText: clear: _x:%d, oTop:%d, _y:%d, xTx:%d, yTx:%d, wTx:%d,"
                                              " hTx:%d, oBot:%d, _res_x/max:%d/%d, str:%s"),
                                            _x, oTop, _y, xText, yText, wText,
                                            hText, oBottom, _res_x, res_x, newString.c_str()));
    }
    # endif // ifndef BUILD_NO_DEBUG

    if (bkcolor == color) { bkcolor = _bgcolor; } // To get at least the text readable

    if (_textPrintMode == AdaGFXTextPrintMode::ClearThenTruncate) { // oTop is negative so subtract to add...
      _display->fillRect(_x + oTop, yText, res_x - (_x - xOffset), hText + oBottom - oTop, bkcolor); // Clear text area to right edge of
                                                                                                     // screen
    } else {
      _display->fillRect(_x + oTop, yText, _w, hText + oBottom - oTop, bkcolor); // Clear text area
    }

    delay(0);
  }

  _display->setCursor(_x + oLeft, _y); // add left offset to center, _y may be updated
  _display->print(newString);
}

/****************************************************************************
 * getTextSize length and height in pixels
 ***************************************************************************/
uint16_t AdafruitGFX_helper::getTextSize(const String& text,
                                         uint16_t    & h) {
  int16_t  x;
  int16_t  y;
  uint16_t w;

  _display->getTextBounds(text.c_str(), 0, 0, &x, &y, &w, &h); // Count length and height in pixels
  return w;
}

/****************************************************************************
 * color565: convert r, g, b colors to rgb565 (by bit-shifting)
 ***************************************************************************/
uint16_t color565(const uint8_t& red,
                  const uint8_t& green,
                  const uint8_t& blue) {
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}

/****************************************************************************
 * AdaGFXparseColor: translate color name, rgb565 hex #rGgb or rgb hex #RRGGBB to an RGB565 value,
 * also applies color reduction to mono(2), duo(3), quadro(4), septo(7), octo(8), quinto(16)-chrome colors
 ***************************************************************************/

// Parse color string to RGB565 color
// param [in] s : The color string (white, red, ...)
// Param [in] colorDepth: The requiresed color depth, default: FullColor
// param [in] defaultWhite: Return White color if empty, default: true
// return : color (default ADAGFX_WHITE)
const char adagfx_colornames[] PROGMEM = "black|white|inverse|red|yellow|dark|light|green|blue|orange|navy|darkcyan|"
                                         "darkgreen|maroon|purple|olive|lightgrey|darkgrey|cyan|magenta|greenyellow|pink";
enum class adagfx_colornames_e : int8_t {
  invalid = -1,
  black   = 0,
  white,
  inverse,
  red,
  yellow,
  dark,
  light,
  green,
  blue,
  orange,
  navy,
  darkcyan,
  darkgreen,
  maroon,
  purple,
  olive,
  lightgrey,
  darkgrey,
  cyan,
  magenta,
  greenyellow,
  pink,
};

uint16_t AdaGFXparseColor(String                & s,
                          const AdaGFXColorDepth& colorDepth,
                          const bool              emptyIsBlack) {
  s.toLowerCase();
  int32_t   result  = -1; // No result yet
  const int color_i = GetCommandCode(s.c_str(), adagfx_colornames);

  const adagfx_colornames_e color = static_cast<adagfx_colornames_e>(color_i);

  if ((colorDepth == AdaGFXColorDepth::Monochrome) ||
      (colorDepth == AdaGFXColorDepth::BlackWhiteRed) ||
      (colorDepth == AdaGFXColorDepth::BlackWhite2Greyscales)) { // Only a limited set of colors is supported
    switch (color) {
      case adagfx_colornames_e::black: return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_BLACK);
      case adagfx_colornames_e::inverse: return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_INVERSE);
      case adagfx_colornames_e::yellow: // Synonym for red
      case adagfx_colornames_e::red: return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_RED);
      case adagfx_colornames_e::dark: return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_DARK);
      case adagfx_colornames_e::light: return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_LIGHT);

      // case adagfx_colornames_e::white: return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_WHITE);
      // If we get this far, return the default
      default:
        return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_WHITE);
    }
  # if ADAGFX_SUPPORT_7COLOR
  } else if (colorDepth == AdaGFXColorDepth::SevenColor) {
    switch (color) {
      case adagfx_colornames_e::black: result  = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLACK); break;
      case adagfx_colornames_e::white: result  = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_WHITE); break;
      case adagfx_colornames_e::green: result  = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_GREEN); break;
      case adagfx_colornames_e::blue: result   = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLUE); break;
      case adagfx_colornames_e::red: result    = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_RED); break;
      case adagfx_colornames_e::yellow: result = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_YELLOW); break;
      case adagfx_colornames_e::orange: result = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_ORANGE); break;
      default:
        break;
    }
  # endif // if ADAGFX_SUPPORT_7COLOR
  } else { // Some predefined colors
    switch (color) {
      case adagfx_colornames_e::black: result       = ADAGFX_BLACK; break;
      case adagfx_colornames_e::navy: result        = ADAGFX_NAVY; break;
      case adagfx_colornames_e::darkgreen: result   = ADAGFX_DARKGREEN; break;
      case adagfx_colornames_e::darkcyan: result    = ADAGFX_DARKCYAN; break;
      case adagfx_colornames_e::maroon: result      = ADAGFX_MAROON; break;
      case adagfx_colornames_e::purple: result      = ADAGFX_PURPLE; break;
      case adagfx_colornames_e::olive: result       = ADAGFX_OLIVE; break;
      case adagfx_colornames_e::lightgrey: result   = ADAGFX_LIGHTGREY; break;
      case adagfx_colornames_e::darkgrey: result    = ADAGFX_DARKGREY; break;
      case adagfx_colornames_e::blue: result        = ADAGFX_BLUE; break;
      case adagfx_colornames_e::green: result       = ADAGFX_GREEN; break;
      case adagfx_colornames_e::cyan: result        = ADAGFX_CYAN; break;
      case adagfx_colornames_e::red: result         = ADAGFX_RED; break;
      case adagfx_colornames_e::magenta: result     = ADAGFX_MAGENTA; break;
      case adagfx_colornames_e::yellow: result      = ADAGFX_YELLOW; break;
      case adagfx_colornames_e::white: result       = ADAGFX_WHITE; break;
      case adagfx_colornames_e::orange: result      = ADAGFX_ORANGE; break;
      case adagfx_colornames_e::greenyellow: result = ADAGFX_GREENYELLOW; break;
      case adagfx_colornames_e::pink: result        = ADAGFX_PINK; break;
      default:
        break;
    }
  }

  // Parse default hex #rgb565 (hex) string (1-4 hex nibbles accepted!)
  if ((result == -1) && (s.length() >= 2) && (s.length() <= 5) && (s[0] == '#')) {
    result = hexToUL(&s[1]);
  }

  // Parse default hex #RRGGBB string (must be 6 hex nibbles!)
  if ((result == -1) && (s.length() == 7) && (s[0] == '#')) {
    // convrt to long value in base16, then split up into r, g, b values
    const uint32_t number = hexToUL(&s[1]);

    // uint32_t r = number >> 16 & 0xFF;
    // uint32_t g = number >> 8 & 0xFF;
    // uint32_t b = number & 0xFF;
    // convert to color565 (as used by adafruit lib)
    result = color565(number >> 16 & 0xFF, number >> 8 & 0xFF, number & 0xFF);
  }

  if ((result == -1) || (result == ADAGFX_WHITE)) { // Default & don't convert white
    # if ADAGFX_SUPPORT_8and16COLOR

    if (
      #  if ADAGFX_SUPPORT_7COLOR
      (colorDepth >= AdaGFXColorDepth::SevenColor) &&
      #  endif // if ADAGFX_SUPPORT_7COLOR
      (colorDepth <= AdaGFXColorDepth::SixteenColor)) {
      result = static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_BLACK); // Monochrome fallback, compatible 7-color
    } else
    # endif // if ADAGFX_SUPPORT_8and16COLOR
    {
      if (emptyIsBlack) {
        result = ADAGFX_BLACK;
      } else {
        result = ADAGFX_WHITE; // Color fallback value
      }
    }
  } else {
    // Reduce colors?
    switch (colorDepth) {
      case AdaGFXColorDepth::Monochrome:
      case AdaGFXColorDepth::BlackWhiteRed:
      case AdaGFXColorDepth::BlackWhite2Greyscales:
        // Unsupported at this point, but compiler needs the cases because of the enum class
        break;
      # if ADAGFX_SUPPORT_7COLOR
      case AdaGFXColorDepth::SevenColor:
        result = AdaGFXrgb565ToColor7(result); // Convert
        break;
      # endif  // if ADAGFX_SUPPORT_7COLOR
      # if ADAGFX_SUPPORT_8and16COLOR
      case AdaGFXColorDepth::EightColor:
        result = color565((result >> 11 & 0x1F) / 4, (result >> 5 & 0x3F) / 4, (result & 0x1F) / 4); // reduce colors factor 4
        break;
      case AdaGFXColorDepth::SixteenColor:
        result = color565((result >> 11 & 0x1F) / 2, (result >> 5 & 0x3F) / 2, (result & 0x1F) / 2); // reduce colors factor 2
        break;
      # endif // if ADAGFX_SUPPORT_8and16COLOR
      case AdaGFXColorDepth::FullColor:
        // No color reduction
        break;
    }
  }
  return static_cast<uint16_t>(result);
}

const __FlashStringHelper* AdaGFXcolorToString_internal(const uint16_t        & color,
                                                        const AdaGFXColorDepth& colorDepth,
                                                        bool                    blackIsEmpty);

// Add a single optionvalue of a color to a datalist (internal/private)
void AdaGFXaddHtmlDataListColorOptionValue(uint16_t         color,
                                           AdaGFXColorDepth colorDepth) {
  const __FlashStringHelper *clr = AdaGFXcolorToString_internal(color, colorDepth, false);

  if (!equals(clr, '*')) {
    datalistAddValue(clr);
  }
}

/*****************************************************************************************
 * Generate a html 'datalist' of the colors available for selected colorDepth, with id provided
 ****************************************************************************************/
void AdaGFXHtmlColorDepthDataList(const __FlashStringHelper *id,
                                  const AdaGFXColorDepth   & colorDepth) {
  datalistStart(id);

  switch (colorDepth) {
    case AdaGFXColorDepth::BlackWhiteRed:
    case AdaGFXColorDepth::BlackWhite2Greyscales:
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_RED),   colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_DARK),  colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_LIGHT), colorDepth);

    // Fall through
    case AdaGFXColorDepth::Monochrome:
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_BLACK),   colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_WHITE),   colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_INVERSE), colorDepth);
      break;
    # if ADAGFX_SUPPORT_7COLOR
    case AdaGFXColorDepth::SevenColor:
    {
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLACK),  colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_WHITE),  colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_GREEN),  colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLUE),   colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_RED),    colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_YELLOW), colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_ORANGE), colorDepth);
      break;
    }
    # endif // if ADAGFX_SUPPORT_7COLOR
    # if ADAGFX_SUPPORT_8and16COLOR
    case AdaGFXColorDepth::EightColor: // TODO: Sort out the actual 8/16 color options
    case AdaGFXColorDepth::SixteenColor:
    # endif // if ADAGFX_SUPPORT_8and16COLOR
    case AdaGFXColorDepth::FullColor:
    {
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_BLACK,       colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_NAVY,        colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_DARKGREEN,   colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_DARKCYAN,    colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_MAROON,      colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_PURPLE,      colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_OLIVE,       colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_LIGHTGREY,   colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_DARKGREY,    colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_BLUE,        colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_GREEN,       colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_CYAN,        colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_RED,         colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_MAGENTA,     colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_YELLOW,      colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_WHITE,       colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_ORANGE,      colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_GREENYELLOW, colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_PINK,        colorDepth);
      break;
    }
  }
  datalistFinish();
}

/*****************************************************************************************
 * Convert an RGB565 color (number) to it's name or the #rgb565 hex string, based on depth
 ****************************************************************************************/
String AdaGFXcolorToString(const uint16_t        & color,
                           const AdaGFXColorDepth& colorDepth,
                           bool                    blackIsEmpty) {
  String result = AdaGFXcolorToString_internal(color, colorDepth, blackIsEmpty);

  if (equals(result, '*')) {
    result  = '#';
    result += String(color, HEX);
    result.toUpperCase();
  }
  return result;
}

const __FlashStringHelper* AdaGFXcolorToString_internal(const uint16_t        & color,
                                                        const AdaGFXColorDepth& colorDepth,
                                                        bool                    blackIsEmpty) {
  switch (colorDepth) {
    case AdaGFXColorDepth::Monochrome:
    case AdaGFXColorDepth::BlackWhiteRed:
    case AdaGFXColorDepth::BlackWhite2Greyscales:
    {
      switch (color) {
        case static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_BLACK): return blackIsEmpty ? F("") : F("black");
        case ADAGFX_WHITE: // Fall through
        case static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_WHITE): return F("white");
        case static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_INVERSE): return F("inverse");
        case static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_RED): return F("red");
        case static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_DARK): return F("dark");
        case static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_LIGHT): return F("light");
        default:
          break;
      }
      break;
    }
    # if ADAGFX_SUPPORT_7COLOR
    case AdaGFXColorDepth::SevenColor:
    {
      switch (color) {
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLACK): return blackIsEmpty ? F("") : F("black");
        case ADAGFX_WHITE: // Fall through
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_WHITE): return F("white");
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_GREEN): return F("green");
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLUE): return F("blue");
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_RED): return F("red");
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_YELLOW): return F("yellow");
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_ORANGE): return F("orange");
        default:
          break;
      }
      break;
    }
    # endif // if ADAGFX_SUPPORT_7COLOR
    # if ADAGFX_SUPPORT_8and16COLOR
    case AdaGFXColorDepth::EightColor:
    case AdaGFXColorDepth::SixteenColor:
    # endif // if ADAGFX_SUPPORT_8and16COLOR
    case AdaGFXColorDepth::FullColor:
    {
      switch (color) {
        case ADAGFX_BLACK:  return blackIsEmpty ? F("") : F("black");
        case ADAGFX_NAVY: return F("navy");
        case ADAGFX_DARKGREEN: return F("darkgreen");
        case ADAGFX_DARKCYAN: return F("darkcyan");
        case ADAGFX_MAROON: return F("maroon");
        case ADAGFX_PURPLE: return F("purple");
        case ADAGFX_OLIVE: return F("olive");
        case ADAGFX_LIGHTGREY: return F("lightgrey");
        case ADAGFX_DARKGREY: return F("darkgrey");
        case ADAGFX_BLUE: return F("blue");
        case ADAGFX_GREEN: return F("green");
        case ADAGFX_CYAN: return F("cyan");
        case ADAGFX_RED: return F("red");
        case ADAGFX_MAGENTA: return F("magenta");
        case ADAGFX_YELLOW: return F("yellow");
        case ADAGFX_WHITE: return F("white");
        case ADAGFX_ORANGE: return F("orange");
        case ADAGFX_GREENYELLOW: return F("greenyellow");
        case ADAGFX_PINK: return F("pink");
        default:
          break;
      }
      break;
    }
  }
  return F("*");
}

# if ADAGFX_SUPPORT_7COLOR

/****************************************************************************
 * AdaGFXrgb565ToColor7: Convert a rgb565 color to the 7 colors supported by 7-color eInk displays
 * Borrowed from https://github.com/ZinggJM/GxEPD2 color7() routine
 ***************************************************************************/
uint16_t AdaGFXrgb565ToColor7(const uint16_t& color) {
  const uint16_t red   = (color & 0xF800);
  const uint16_t green = (color & 0x07E0) << 5;
  const uint16_t blue  = (color & 0x001F) << 11;
  uint16_t cv7         = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_WHITE); // Default = white

  if ((red < 0x8000) && (green < 0x8000) && (blue < 0x8000)) {
    cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLACK);                // black
  }
  else if ((red >= 0x8000) && (green >= 0x8000) && (blue >= 0x8000)) {
    cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_WHITE);                // white
  }
  else if ((red >= 0x8000) && (blue >= 0x8000)) {
    if (red > blue) {
      cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_RED);
    } else {
      cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLUE); // red, blue
    }
  }
  else if ((green >= 0x8000) && (blue >= 0x8000)) {
    if (green > blue) {
      cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_GREEN);
    } else {
      cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLUE); // green, blue
    }
  }
  else if ((red >= 0x8000) && (green >= 0x8000)) {
    static const uint16_t y2o_lim = ((ADAGFX_YELLOW - ADAGFX_ORANGE) / 2 + (ADAGFX_ORANGE & 0x07E0)) << 5;

    if (green > y2o_lim) {
      cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_YELLOW);
    } else {
      cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_ORANGE); // yellow, orange
    }
  }
  else if (red >= 0x8000) {
    cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_RED);   // red
  }
  else if (green >= 0x8000) {
    cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_GREEN); // green
  }
  else {
    cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLUE);  // blue
  }
  return cv7;
}

# endif // if ADAGFX_SUPPORT_7COLOR

/****************************************************************************
 * getTextMetrics: Returns the metrics related to current font
 ***************************************************************************/
void AdafruitGFX_helper::getTextMetrics(uint16_t& textcols,
                                        uint16_t& textrows,
                                        uint8_t & fontwidth,
                                        uint8_t & fontheight,
                                        uint8_t & fontscaling,
                                        uint8_t & heightOffset,
                                        uint16_t& xpix,
                                        uint16_t& ypix) {
  textcols     = _textcols;
  textrows     = _textrows;
  fontwidth    = _fontwidth;
  fontheight   = _fontheight;
  fontscaling  = _fontscaling;
  heightOffset = _heightOffset;
  # if ADAGFX_ENABLE_FRAMED_WINDOW
  getWindowLimits(xpix, ypix);
  # else // if ADAGFX_ENABLE_FRAMED_WINDOW
  xpix = _res_x;
  ypix = _res_y;
  # endif // if ADAGFX_ENABLE_FRAMED_WINDOW
}

/****************************************************************************
 * getColors: Returns the current text colors
 ***************************************************************************/
void AdafruitGFX_helper::getColors(uint16_t& fgcolor,
                                   uint16_t& bgcolor) {
  fgcolor = _fgcolor;
  bgcolor = _bgcolor;
}

/****************************************************************************
 * calculateTextMetrics: Recalculate the text mertics based on supplied font parameters
 ***************************************************************************/
void AdafruitGFX_helper::calculateTextMetrics(const uint8_t fontwidth,
                                              const uint8_t fontheight,
                                              const int8_t  heightOffset,
                                              const bool    isProportional) {
  uint16_t res_x = _res_x;
  uint16_t res_y = _res_y;

  # if ADAGFX_ENABLE_FRAMED_WINDOW
  getWindowLimits(res_x, res_y);
  # endif // if ADAGFX_ENABLE_FRAMED_WINDOW

  _fontwidth      = fontwidth;
  _fontheight     = fontheight;
  _heightOffset   = heightOffset;
  _isProportional = isProportional;
  _textcols       = res_x / (_fontwidth * _fontscaling);
  _textrows       = res_y / ((_fontheight + _heightOffset) * _fontscaling);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
    addLogMove(ADAGFX_LOG_LEVEL, strformat(F("AdaGFX: tr: %s x: %d, y: %d, text columns: %d rows: %d"),
                                           _trigger.c_str(), res_x, res_y, _textcols, _textrows));
  }
  # endif // ifndef BUILD_NO_DEBUG
}

# if ADAGFX_ARGUMENT_VALIDATION

/****************************************************************************
 * invalidCoordinates: Check if X/Y coordinates stay within the limits of the display,
 * default pixel-mode, colRowMode true = character mode.
 * If Y == 0 then X is allowed the max. value of the display size.
 * *** Returns TRUE when invalid !! ***
 ***************************************************************************/
bool AdafruitGFX_helper::invalidCoordinates(const int  X,
                                            const int  Y,
                                            const bool colRowMode) {
  uint16_t res_x = _res_x;
  uint16_t res_y = _res_y;

  #  if ADAGFX_ENABLE_FRAMED_WINDOW
  getWindowLimits(res_x, res_y);
  #  endif // if ADAGFX_ENABLE_FRAMED_WINDOW

  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
    addLogMove(ADAGFX_LOG_LEVEL, strformat(F("invalidCoordinates: X:%d/%d Y:%d/%d"),
                                           X, colRowMode ? _textcols : res_x, Y, colRowMode ? _textrows : res_y));
  }
  #  endif // ifndef BUILD_NO_DEBUG

  if (!_useValidation) { return false; }

  if (colRowMode) {
    return !((X >= 0) && (X <= _textcols) &&
             (Y >= 0) && (Y <= _textrows));
  } else {
    if (Y == 0) { // Y == 0: Accept largest x/y size value for x
      return !((X >= 0) && (X <= std::max(res_x, res_y)));
    } else {
      return !((X >= 0) && (X <= res_x) &&
               (Y >= 0) && (Y <= res_y));
    }
  }
}

# endif // if ADAGFX_ARGUMENT_VALIDATION

void AdafruitGFX_helper::setValidation(const bool& state) {
  _useValidation = state;
}

/****************************************************************************
 * rotate the display (and all windows)
 ***************************************************************************/
void AdafruitGFX_helper::setRotation(uint8_t m) {
  const uint8_t rotation = m & 3;

  _display->setRotation(m); // Set rotation 0/1/2/3
  _rotation = rotation;

  switch (rotation) {
    case 0:
    case 2:
      _res_x = _display_x;
      _res_y = _display_y;
      break;
    case 1:
    case 3:
      _res_x = _display_y;
      _res_y = _display_x;
      break;
  }
  # if ADAGFX_ENABLE_FRAMED_WINDOW

  for (uint8_t i = 0; i < _windows.size(); ++i) {                // Swap x/y for all matching windows
    switch (rotation) {
      case 0:                                                    // 0 degrees
        _windows[i].top_left.x     = _windows[i].org_top_left.x; // All original
        _windows[i].top_left.y     = _windows[i].org_top_left.y;
        _windows[i].width_height.x = _windows[i].org_width_height.x;
        _windows[i].width_height.y = _windows[i].org_width_height.y;
        break;
      case 1:                                                        // +90 degrees
        _windows[i].top_left.x     = _windows[i].org_top_left.y;
        _windows[i].top_left.y     = _display_x - (_windows[i].org_top_left.x + _windows[i].org_width_height.x);
        _windows[i].width_height.x = _windows[i].org_width_height.y; // swapped width/height
        _windows[i].width_height.y = _windows[i].org_width_height.x;
        break;
      case 2:                                                        // +180 degrees
        _windows[i].top_left.x     = _display_x - (_windows[i].org_top_left.x + _windows[i].org_width_height.x);
        _windows[i].top_left.y     = _display_y - (_windows[i].org_top_left.y + _windows[i].org_width_height.y);
        _windows[i].width_height.x = _windows[i].org_width_height.x;
        _windows[i].width_height.y = _windows[i].org_width_height.y;
        break;
      case 3:                                                        // +270 degrees
        _windows[i].top_left.x     = _display_y - (_windows[i].org_top_left.y + _windows[i].org_width_height.y);
        _windows[i].top_left.y     = _windows[i].org_top_left.x;
        _windows[i].width_height.x = _windows[i].org_width_height.y; // swapped width/height
        _windows[i].width_height.y = _windows[i].org_width_height.x;
        break;
    }
    _windows[i].rotation = rotation;
  }

  // logWindows(F("rot ")); // For debugging only
  # endif // if ADAGFX_ENABLE_FRAMED_WINDOW
  calculateTextMetrics(_fontwidth, _fontheight, _heightOffset, _isProportional);
}

# if ADAGFX_ENABLE_BMP_DISPLAY

/****************************************************************************
 * CPA (Copy/paste/adapt) from Adafruit_ImageReader::coreBMP()
 * Changes:
 * - No 'load to memory' feature
 * - No special handling of SD Filesystem/FAT, but File only
 * - Adds support for non-SPI displays (like NeoPixel Matrix, and possibly I2C displays, once supported)
 ***************************************************************************/
bool AdafruitGFX_helper::showBmp(const String& filename,
                                 int16_t       x,
                                 int16_t       y) {
  uint32_t offset;               // Start of image data in file
  uint32_t headerSize;           // Indicates BMP version
  uint32_t compression = 0;      // BMP compression mode
  uint32_t colors      = 0;      // Number of colors in palette
  uint32_t rowSize;              // >bmpWidth if scanline padding
  uint8_t  sdbuf[3 * BUFPIXELS]; // BMP read buf (R+G+B/pixel)

  uint32_t destidx = 0;
  uint32_t bmpPos  = 0;          // Next pixel position in file
  int bmpWidth;                  // BMP width & height in pixels
  int bmpHeight;
  int loadWidth;
  int loadHeight;                // Region being loaded (clipped)
  int loadX;
  int loadY;                     // "
  int row;                       // Current pixel pos.
  int col;
  uint16_t *quantized = NULL;    // 16-bit 5/6/5 color palette
  uint16_t  tftbuf[BUFPIXELS];
  uint16_t *dest = tftbuf;       // TFT working buffer, or NULL if to canvas
  int16_t   drow = 0;
  int16_t   dcol = 0;
  uint8_t   planes;              // BMP planes
  uint8_t   depth;               // BMP bit depth
  uint8_t   r;                   // Current pixel colors
  uint8_t   g;
  uint8_t   b;
  uint8_t   bitIn = 0;           // Bit number for 1-bit data in

  #  if ((3 * BUFPIXELS) <= 255)
  uint8_t srcidx = sizeof sdbuf; // Current position in sdbuf
  #  else // if ((3 * BUFPIXELS) <= 255)
  uint16_t srcidx = sizeof sdbuf;
  #  endif // if ((3 * BUFPIXELS) <= 255)
  bool flip     = true;  // BMP is stored bottom-to-top
  bool transact = true;  // Enable transaction support to work proper with SD czrd, when enabled
  bool status   = false; // IMAGE_SUCCESS on valid file

  bool canTransact = (nullptr != _tft);

  // If BMP is being drawn off the right or bottom edge of the screen,
  // nothing to do here. NOT an error, just a trivial clip operation.
  if (_tft && ((x >= _tft->width()) || (y >= _tft->height()))) {
    addLog(LOG_LEVEL_INFO, F("showBmp: coordinates off display"));
    return false;
  }

  // Open requested file on storage
  // Search flash file system first, then SD if present
  file = tryOpenFile(filename, "r");

  if (!file) {
    addLog(LOG_LEVEL_ERROR, F("showBmp: file not found"));
    return false;
  }

  // Parse BMP header. 0x4D42 (ASCII 'BM') is the Windows BMP signature.
  // There are other values possible in a .BMP file but these are super
  // esoteric (e.g. OS/2 struct bitmap array) and NOT supported here!
  if (readLE16() == 0x4D42) { // BMP signature
    (void)readLE32();         // Read & ignore file size
    (void)readLE32();         // Read & ignore creator bytes
    offset = readLE32();      // Start of image data
    // Read DIB header
    headerSize = readLE32();
    bmpWidth   = readLE32();
    bmpHeight  = readLE32();

    // If bmpHeight is negative, image is in top-down order.
    // This is not canon but has been observed in the wild.
    if (bmpHeight < 0) {
      bmpHeight = -bmpHeight;
      flip      = false;
    }
    planes = readLE16();
    depth  = readLE16(); // Bits per pixel

    // Compression mode is present in later BMP versions (default = none)
    if (headerSize > 12) {
      compression = readLE32();
      (void)readLE32();    // Raw bitmap data size; ignore
      (void)readLE32();    // Horizontal resolution, ignore
      (void)readLE32();    // Vertical resolution, ignore
      colors = readLE32(); // Number of colors in palette, or 0 for 2^depth
      (void)readLE32();    // Number of colors used (ignore)
      // File position should now be at start of palette (if present)
    }

    if (!colors) {
      colors = 1 << depth;
    }
    #  ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(F("showBmp: bitmap w:%d, h:%d, dpt:%d, colors:%d, cmp:%d, pl:%d, x:%d, y:%d"),
                                       bmpWidth, bmpHeight, depth, colors, compression, planes, x, y));
    }
    #  endif // ifndef BUILD_NO_DEBUG

    loadWidth  = bmpWidth;
    loadHeight = bmpHeight;
    loadX      = 0;
    loadY      = 0;

    if (_display) {
      // Crop area to be loaded (if destination is TFT)
      if (x < 0) {
        loadX      = -x;
        loadWidth += x;
        x          = 0;
      }

      if (y < 0) {
        loadY       = -y;
        loadHeight += y;
        y           = 0;
      }

      if ((x + loadWidth) > _display->width()) {
        loadWidth = _display->width() - x;
      }

      if ((y + loadHeight) > _display->height()) {
        loadHeight = _display->height() - y;
      }
    }
    #  ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, strformat(F("showBmp: x:%d, y:%d, dw:%d, dh:%d"),
                                           x, y, _display->width(), _display->height()));
    }
    #  endif // ifndef BUILD_NO_DEBUG

    if ((planes == 1) && (compression == 0)) { // Only uncompressed is handled
      // BMP rows are padded (if needed) to 4-byte boundary
      rowSize = ((depth * bmpWidth + 31) / 32) * 4;

      if ((depth == 24) || (depth == 1)) { // BGR or 1-bit bitmap format
        // if (dest) {                     // Supported format, alloc OK, etc.
        status = true;

        if ((loadWidth > 0) && (loadHeight > 0)) { // Clip top/left
          _display->startWrite();                  // Start SPI (regardless of transact)

          if (canTransact) {
            _tft->setAddrWindow(x, y, loadWidth, loadHeight);
          }

          if ((depth >= 16) ||
              (quantized = (uint16_t *)malloc(colors * sizeof(uint16_t)))) {
            if (depth < 16) {
              // Load and quantize color table
              for (uint16_t c = 0; c < colors; ++c) {
                b = file.read();
                g = file.read();
                r = file.read();
                (void)file.read(); // Ignore 4th byte
                quantized[c] =     // -V522
                               ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
              }
            }

            for (row = 0; row < loadHeight; ++row) { // For each scanline...
              delay(0);                              // Keep ESP8266 happy

              // Seek to start of scan line.  It might seem labor-intensive
              // to be doing this on every line, but this method covers a
              // lot of gritty details like cropping, flip and scanline
              // padding. Also, the seek only takes place if the file
              // position actually needs to change (avoids a lot of cluster
              // math in SD library).
              if (flip) { // Bitmap is stored bottom-to-top order (normal BMP)
                bmpPos = offset + (bmpHeight - 1 - (row + loadY)) * rowSize;
              } else {    // Bitmap is stored top-to-bottom
                bmpPos = offset + (row + loadY) * rowSize;
              }

              if (depth == 24) {
                bmpPos += loadX * 3;
              } else {
                bmpPos += loadX / 8;
                bitIn   = 7 - (loadX & 7);
              }

              constexpr size_t errorcode = (size_t)-1;
              size_t pos = file.position();
              if (pos == errorcode) {
                pos = 0;
              }

              if (pos != bmpPos) {        // Need seek?
                if (transact && canTransact) {
                  _tft->dmaWait();
                  _tft->endWrite();                   // End TFT SPI transaction
                }
                file.seek(bmpPos);                    // Seek = SD transaction
                srcidx = sizeof sdbuf;                // Force buffer reload
              }

              for (col = 0; col < loadWidth; ++col) { // For each pixel...
                if (srcidx >= sizeof sdbuf) {         // Time to load more?
                  if (transact && canTransact) {
                    _tft->dmaWait();
                    _tft->endWrite();                 // End TFT SPI transact
                  }
                  file.read(sdbuf, sizeof sdbuf);     // Load from SD

                  if (transact && canTransact) {
                    _display->startWrite();           // Start TFT SPI transact
                  }

                  if (destidx) {                      // If buffered TFT data
                    // Non-blocking writes (DMA) have been temporarily
                    // disabled until this can be rewritten with two
                    // alternating 'dest' buffers (else the nonblocking
                    // data out is overwritten in the dest[] write below).
                    // tft->writePixels(dest, destidx, false); // Write it
                    delay(0);

                    if (canTransact) {
                      _tft->writePixels(dest, destidx, true); // Write it
                    } else {
                      // loop over buffer

                      for (uint16_t p = 0; p < destidx; ++p) {
                        _display->drawPixel(x + p, y + drow, dest[p]);
                      }
                    }

                    if (col % 33 == 0) { delay(0); }
                    destidx = 0; // and reset dest index
                  }

                  srcidx = 0;    // Reset bmp buf index
                }

                if (depth == 24) {
                  // Convert each pixel from BMP to 565 format, save in dest
                  b               = sdbuf[srcidx++];
                  g               = sdbuf[srcidx++]; // -V557
                  r               = sdbuf[srcidx++]; // -V557
                  dest[destidx++] =
                    ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                } else {
                  // Extract 1-bit color index
                  uint8_t n = (sdbuf[srcidx] >> bitIn) & 1;

                  if (!bitIn) {
                    srcidx++;
                    bitIn = 7;
                  } else {
                    bitIn--;
                  }

                  // Look up in palette, store in tft dest buf
                  dest[destidx++] = quantized[n];
                }
                dcol++;
              }                                           // end pixel loop

              if (_tft) {                                 // Drawing to TFT?
                delay(0);

                if (destidx) {                            // Any remainders?
                  // See notes above re: DMA
                  _tft->writePixels(dest, destidx, true); // Write it
                  destidx = 0;                            // and reset dest index
                }
                _tft->dmaWait();
                _tft->endWrite();                         // update display
              } else {
                // loop over buffer
                if (destidx) {
                  for (uint16_t p = 0; p < destidx; ++p) {
                    _display->drawPixel(x + p, y + drow, dest[p]);

                    if (p % 100 == 0) { delay(0); }
                  }
                  destidx = 0; // and reset dest index
                }
              }

              drow++;
              dcol = 0;
            }                  // end scanline loop

            if (quantized) {
              free(quantized); // Palette no longer needed
            }
            delay(0);
          }                    // end depth>24 or quantized malloc OK
        }                      // end top/left clip
        // }                         // end malloc check
      }                        // end depth check
    } // end planes/compression check

    if (status) {
    #  ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, F("showBmp: Done."));
    #  endif // ifndef BUILD_NO_DEBUG
    } else {
      addLog(LOG_LEVEL_ERROR, F("showBmp: Only uncompressed and 24 or 1 bit color-depth supported."));
    }
  } else { // end signature
    addLog(LOG_LEVEL_ERROR, F("showBmp: File signature error."));
  }

  file.close();
  return status; // -V680

  // }
}

/*!
    @brief   Reads a little-endian 16-bit unsigned value from currently-
             open File, converting if necessary to the microcontroller's
             native endianism. (BMP files use little-endian values.)
    @return  Unsigned 16-bit value, native endianism.
 */
uint16_t AdafruitGFX_helper::readLE16(void) {
  // Big-endian or unknown. Byte-by-byte read will perform reversal if needed.
  return file.read() | ((uint16_t)file.read() << 8);
}

/*!
    @brief   Reads a little-endian 32-bit unsigned value from currently-
             open File, converting if necessary to the microcontroller's
             native endianism. (BMP files use little-endian values.)
    @return  Unsigned 32-bit value, native endianism.
 */
uint32_t AdafruitGFX_helper::readLE32(void) {
  // Big-endian or unknown. Byte-by-byte read will perform reversal if needed.
  return file.read() | ((uint32_t)file.read() << 8) |
         ((uint32_t)file.read() << 16) | ((uint32_t)file.read() << 24);
}

# endif // if ADAGFX_ENABLE_BMP_DISPLAY

# if ADAGFX_ENABLE_FRAMED_WINDOW

/****************************************************************************
 * Check if the requested id is a valid window id
 ***************************************************************************/
bool AdafruitGFX_helper::validWindow(const uint8_t& windowId) {
  return getWindowIndex(windowId) != -1;
}

/****************************************************************************
 * Select this window id as the default
 ***************************************************************************/
bool AdafruitGFX_helper::selectWindow(const uint8_t& windowId,
                                      const int8_t & rotation) {
  const int16_t result = getWindowIndex(windowId);

  if (result != -1) {
    _windowIndex = result;
    _window      = windowId;
  }
  return result != -1;
}

/****************************************************************************
 * Return the index of the windowId in _windows, -1 if not found
 ***************************************************************************/
int16_t AdafruitGFX_helper::getWindowIndex(const int16_t& windowId) {
  size_t result = 0;

  for (auto win = _windows.begin(); win != _windows.end(); win++, ++result) {
    if ((*win).id == windowId) {
      break;
    }
  }
  return result == _windows.size() ? -1 : result;
}

/****************************************************************************
 * Get the offset for the currently active window
 ***************************************************************************/
void AdafruitGFX_helper::getWindowOffsets(uint16_t& xOffset,
                                          uint16_t& yOffset) {
  xOffset = _windows[_windowIndex].top_left.x;
  yOffset = _windows[_windowIndex].top_left.y;
}

/****************************************************************************
 * Get the limits for the currently active window
 ***************************************************************************/
void AdafruitGFX_helper::getWindowLimits(uint16_t& xLimit,
                                         uint16_t& yLimit) {
  xLimit = _windows[_windowIndex].width_height.x;
  yLimit = _windows[_windowIndex].width_height.y;
}

/****************************************************************************
 * Define a window and return the ID
 ***************************************************************************/
uint8_t AdafruitGFX_helper::defineWindow(const int16_t& x,
                                         const int16_t& y,
                                         const int16_t& w,
                                         const int16_t& h,
                                         int16_t        windowId,
                                         const int8_t & rotation) {
  int16_t result = getWindowIndex(windowId);

  if (result < 0) {
    result = static_cast<int16_t>(_windows.size()); // previous size
    _windows.push_back(tWindowObject());            // add new

    if (windowId < 0) {
      windowId = 0;

      for (auto it = _windows.begin(); it != _windows.end(); it++) {
        if ((*it).id == windowId) { windowId++; } // Generate a new window id
      }
    }
    _windows[result].id = windowId;
  }
  _windows[result].top_left.x     = x;
  _windows[result].top_left.y     = y;
  _windows[result].width_height.x = w;
  _windows[result].width_height.y = h;

  if (rotation >= 0) {
    _windows[result].rotation = rotation & 3;
  } else {
    _windows[result].rotation = _rotation;
  }

  // Adjust original coordinate/sizes based on rotation
  switch (_windows[result].rotation) {
    case 0:                                    // 0 degrees
      _windows[result].org_top_left.x     = x; // All original
      _windows[result].org_top_left.y     = y;
      _windows[result].org_width_height.x = w;
      _windows[result].org_width_height.y = h;
      break;
    case 1:                                                       // +90 degrees
      _windows[result].org_top_left.x     = _display_x - (y + h); // swapped x/y
      _windows[result].org_top_left.y     = x;
      _windows[result].org_width_height.x = h;                    // swapped width/height
      _windows[result].org_width_height.y = w;
      break;
    case 2:                                                       // +180 degrees
      _windows[result].org_top_left.x     = _display_x - (x + w);
      _windows[result].org_top_left.y     = _display_y - (y + h);
      _windows[result].org_width_height.x = w;                    // unchanged
      _windows[result].org_width_height.y = h;
      break;
    case 3:                                                       // +270 degrees
      _windows[result].org_top_left.x     = y;
      _windows[result].org_top_left.y     = _display_x - (x + w);
      _windows[result].org_width_height.x = h;                    // swapped width/height
      _windows[result].org_width_height.y = w;
      break;
  }

  return _windows[result].id;
}

/****************************************************************************
 * Remove a window definition
 ***************************************************************************/
bool AdafruitGFX_helper::deleteWindow(const uint8_t& windowId) {
  const int16_t result = getWindowIndex(windowId);

  if (result > -1) {
    _windows.erase(_windows.begin() + result);
    return true;
  }
  return false;
}

/****************************************************************************
 * log all current known window definitions
 ***************************************************************************/
void AdafruitGFX_helper::logWindows(const String& prefix) {
  #  ifndef BUILD_NO_DEBUG

  for (auto it = _windows.begin(); it != _windows.end(); it++) {
    addLogMove(LOG_LEVEL_INFO, strformat(F("AdaGFX window %s: %d, x:%d, y:%d, w:%d, h:%d"
                                           ", rot%d, current: %d, org x:%d, y:%d, w:%d, h:%d"),
                                         prefix.c_str(), (*it).id, (*it).top_left.x, (*it).top_left.y,
                                         (*it).width_height.x, (*it).width_height.y,
                                         (*it).rotation, getWindow(), (*it).org_top_left.x, (*it).org_top_left.y,
                                         (*it).org_width_height.x, (*it).org_width_height.y));
  }
  #  endif // ifndef BUILD_NO_DEBUG
}

# endif // if ADAGFX_ENABLE_FRAMED_WINDOW

#endif // ifdef PLUGIN_USES_ADAFRUITGFX
