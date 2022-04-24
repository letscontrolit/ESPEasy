#include "../PluginStructs/P123_data_struct.h"

#ifdef USES_P123

# include "../ESPEasyCore/ESPEasyNetwork.h"

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Scheduler.h"
# include "../Helpers/StringConverter.h"
# include "../Helpers/SystemVariables.h"

# include "../Commands/InternalCommands.h"

/****************************************************************************
 * toString: Display-value for the button selected
 ***************************************************************************/
# ifdef P123_USE_EXTENDED_TOUCH
const __FlashStringHelper* toString(Button_type_e button) {
  switch (button) {
    case Button_type_e::None: return F("None");
    case Button_type_e::Square: return F("Square");
    case Button_type_e::Rounded: return F("Rounded");
    case Button_type_e::Circle: return F("Circle");
    case Button_type_e::Button_MAX: break;
  }
  return F("Unsupported!");
}

# endif // ifdef P123_USE_EXTENDED_TOUCH

/**
 * Constructor
 */
P123_data_struct::P123_data_struct() : touchscreen(nullptr) {}

/**
 * Destructor
 */
P123_data_struct::~P123_data_struct() {
  reset();
}

/**
 * Proper reset and cleanup.
 */
void P123_data_struct::reset() {
  # ifdef PLUGIN_123_DEBUG
  addLogMove(LOG_LEVEL_INFO, F("P123 DEBUG Touchscreen reset."));
  # endif // PLUGIN_123_DEBUG

  if (isInitialized()) {
    delete touchscreen;
    touchscreen = nullptr;
  }
}

/**
 * Initialize data and set up the touchscreen.
 */
bool P123_data_struct::init(const EventStruct *event,
                            uint8_t            rotation,
                            uint16_t           ts_x_res,
                            uint16_t           ts_y_res,
                            uint16_t           displayTask,
                            AdaGFXColorDepth   colorDepth) {
  _rotation    = rotation;
  _ts_x_res    = ts_x_res;
  _ts_y_res    = ts_y_res;
  _displayTask = displayTask;
  _colorDepth  = colorDepth;

  reset();

  touchscreen = new (std::nothrow) Adafruit_FT6206();

  if (isInitialized()) {
    loadTouchObjects(event);

    touchscreen->begin(P123_Settings.treshold);

    if (bitRead(P123_Settings.flags, P123_FLAGS_SEND_OBJECTNAME) &&
        bitRead(P123_Settings.flags, P123_FLAGS_INIT_OBJECTEVENT)) {
      for (int objectNr = 0; objectNr < static_cast<int>(TouchObjects.size()); objectNr++) {
        if (!TouchObjects[objectNr].objectName.isEmpty()
            && bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ENABLED)
            && bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_BUTTON)) {
          generateObjectEvent(event, objectNr, -1);
        }
      }
    }

    # ifdef PLUGIN_123_DEBUG
    addLogMove(LOG_LEVEL_INFO, F("P123 DEBUG Plugin & touchscreen initialized."));
  } else {
    addLogMove(LOG_LEVEL_INFO, F("P123 DEBUG Touchscreen initialisation FAILED."));
    # endif // PLUGIN_123_DEBUG
  }
  return isInitialized();
}

/**
 * Properly initialized? then true
 */
bool P123_data_struct::isInitialized() const {
  return touchscreen != nullptr;
}

int P123_data_struct::parseStringToInt(const String& string, uint8_t indexFind, char separator, int defaultValue) {
  String parsed = parseStringKeepCase(string, indexFind, separator);

  // if (parsed.isEmpty()) {
  //   return defaultValue;
  // }
  int result = defaultValue;

  validIntFromString(parsed, result);

  return result;
}

/**
 * Load the settings onto the webpage
 */
bool P123_data_struct::plugin_webform_load(struct EventStruct *event) {
  addFormSubHeader(F("Touch configuration"));

  addFormCheckBox(F("Flip rotation 180&deg;"), F("p123_rotation_flipped"), bitRead(P123_Settings.flags, P123_FLAGS_ROTATION_FLIPPED));
  addFormNote(F("Some touchscreens are mounted 180&deg; rotated on the display."));

  addFormNumericBox(F("Touch minimum pressure"), F("p123_treshold"),
                    P123_Settings.treshold, 0, 255);

  uint8_t choice3 = 0u;

  bitWrite(choice3, P123_FLAGS_SEND_XY,         bitRead(P123_Settings.flags, P123_FLAGS_SEND_XY));
  bitWrite(choice3, P123_FLAGS_SEND_Z,          bitRead(P123_Settings.flags, P123_FLAGS_SEND_Z));
  bitWrite(choice3, P123_FLAGS_SEND_OBJECTNAME, bitRead(P123_Settings.flags, P123_FLAGS_SEND_OBJECTNAME));
  {
    # define P123_EVENTS_OPTIONS 6
    const __FlashStringHelper *options3[P123_EVENTS_OPTIONS] =
    { F("None"),
      F("X and Y"),
      F("X, Y and Z"),
      F("Objectnames only"),
      F("Objectnames, X and Y"),
      F("Objectnames, X, Y and Z")
    };
    int optionValues3[P123_EVENTS_OPTIONS] = { 0, 1, 3, 4, 5, 7 }; // Already used as a bitmap!
    addFormSelector(F("Events"), F("p123_events"), P123_EVENTS_OPTIONS, options3, optionValues3, choice3);
    addFormCheckBox(F("Initial Objectnames events"), F("p123_init_objectevent"), bitRead(P123_Settings.flags, P123_FLAGS_INIT_OBJECTEVENT));
    addFormNote(F("Will send state -1 but only for enabled On/Off button objects."));
  }

  addFormCheckBox(F("Prevent duplicate events"), F("p123_deduplicate"), bitRead(P123_Settings.flags, P123_FLAGS_DEDUPLICATE));

  if (!Settings.UseRules) {
    addFormNote(F("Tools / Advanced / Rules must be enabled for events to be fired."));
  }

  addFormSubHeader(F("Calibration"));

  {
    const __FlashStringHelper *noYesOptions[2] = { F("No"), F("Yes") };
    int noYesOptionValues[2]                   = { 0, 1 };
    addFormSelector(F("Calibrate to screen resolution"),
                    F("p123_use_calibration"),
                    2,
                    noYesOptions,
                    noYesOptionValues,
                    P123_Settings.calibrationEnabled ? 1 : 0,
                    true);
  }

  if (P123_Settings.calibrationEnabled) {
    addRowLabel(F("Calibration"));
    html_table(EMPTY_STRING, false); // Sub-table
    html_table_header(F(""));
    html_table_header(F("x"));
    html_table_header(F("y"));
    html_table_header(F(""));
    html_table_header(F("x"));
    html_table_header(F("y"));

    html_TR_TD();
    addHtml(F("Top-left"));
    html_TD();
    addNumericBox(F("p123_cal_tl_x"),
                  P123_Settings.top_left.x,
                  0,
                  65535);
    html_TD();
    addNumericBox(F("p123_cal_tl_y"),
                  P123_Settings.top_left.y,
                  0,
                  65535);
    html_TD();
    addHtml(F("Bottom-right"));
    html_TD();
    addNumericBox(F("p123_cal_br_x"),
                  P123_Settings.bottom_right.x,
                  0,
                  65535);
    html_TD();
    addNumericBox(F("p123_cal_br_y"),
                  P123_Settings.bottom_right.y,
                  0,
                  65535);

    html_end_table();

    // addFormNote(F("At least 1 x/y value must be <> 0 to enable calibration."));
  }

  addFormCheckBox(F("Enable logging for calibration"), F("p123_log_calibration"),
                  P123_Settings.logEnabled);

  addFormSubHeader(F("Touch objects"));

  {
    addRowLabel(F("Object"));
    # ifdef P123_USE_EXTENDED_TOUCH
    html_table(F("multirow"), false); // Sub-table
    # else // ifdef P123_USE_EXTENDED_TOUCH
    html_table(EMPTY_STRING,  false); // Sub-table
    # endif // ifdef P123_USE_EXTENDED_TOUCH
    html_table_header(F("&nbsp;#&nbsp;"));
    html_table_header(F("On"));
    html_table_header(F("Objectname"));
    html_table_header(F("Top-left x"));
    html_table_header(F("Top-left y"));
    html_table_header(F("On/Off button"));
    # ifdef P123_USE_EXTENDED_TOUCH
    html_table_header(F("ON color"));
    html_table_header(F("ON caption"));
    html_table_header(F("Buttontype"));
    html_table_header(F("Background"));
    # endif // ifdef P123_USE_EXTENDED_TOUCH
    html_TR(); // New row
    html_table_header(EMPTY_STRING);
    html_table_header(EMPTY_STRING);
    html_table_header(EMPTY_STRING);
    html_table_header(F("Width"));
    html_table_header(F("Height"));
    html_table_header(F("Inverted"));
    # ifdef P123_USE_EXTENDED_TOUCH
    html_table_header(F("OFF color"));
    html_table_header(F("OFF caption"));
    html_table_header(F("Caption color"));
    html_table_header(F("Highlight color"));
    # endif // ifdef P123_USE_EXTENDED_TOUCH

    # ifdef P123_USE_EXTENDED_TOUCH
    const __FlashStringHelper *buttonTypeOptions[] = {
      toString(Button_type_e::None),
      toString(Button_type_e::Square),
      toString(Button_type_e::Rounded),
      toString(Button_type_e::Circle)
    };
    const int buttonTypeValues[] = {
      static_cast<int>(Button_type_e::None),
      static_cast<int>(Button_type_e::Square),
      static_cast<int>(Button_type_e::Rounded),
      static_cast<int>(Button_type_e::Circle)
    };
    # endif // ifdef P123_USE_EXTENDED_TOUCH

    uint8_t maxIdx = std::min(static_cast<int>(TouchObjects.size() + P123_EXTRA_OBJECT_COUNT), P123_MAX_OBJECT_COUNT);
    String  parsed;
    TouchObjects.resize(maxIdx, tP123_TouchObjects());

    # ifdef P123_USE_EXTENDED_TOUCH
    AdaGFXHtmlColorDepthDataList(F("adagfx65kcolors"), static_cast<AdaGFXColorDepth>(P123_COLOR_DEPTH));
    # endif // ifdef P123_USE_EXTENDED_TOUCH

    for (int objectNr = 0; objectNr < maxIdx; objectNr++) {
      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtmlInt(objectNr + 1); // Arrayindex to objectindex

      html_TD();

      // Enable new entries
      bool enabled = bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ENABLED) || TouchObjects[objectNr].objectName.isEmpty();
      addCheckBox(getPluginCustomArgName(objectNr + 0),
                  enabled, false);
      html_TD(); // Name
      addTextBox(getPluginCustomArgName(objectNr + 100),
                 TouchObjects[objectNr].objectName,
                 P123_MaxObjectNameLength - 1,
                 false, false, EMPTY_STRING, EMPTY_STRING);
      html_TD(); // top-x
      addNumericBox(getPluginCustomArgName(objectNr + 200),
                    TouchObjects[objectNr].top_left.x, 0, 65535
                    # ifdef P123_USE_TOOLTIPS
                    , F("widenumber"), F("Top-left x")
                    # endif // ifdef P123_USE_TOOLTIPS
                    );
      html_TD(); // top-y
      addNumericBox(getPluginCustomArgName(objectNr + 300),
                    TouchObjects[objectNr].top_left.y, 0, 65535
                    # ifdef P123_USE_TOOLTIPS
                    , F("widenumber"), F("Top-left y")
                    # endif // ifdef P123_USE_TOOLTIPS
                    );
      html_TD(); // on/off button
      addCheckBox(getPluginCustomArgName(objectNr + 600),
                  bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_BUTTON), false
                  # ifdef P123_USE_TOOLTIPS
                  , F("On/Off button")
                  # endif // ifdef P123_USE_TOOLTIPS
                  );
      # ifdef P123_USE_EXTENDED_TOUCH
      html_TD();                               // ON color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorOn, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1000), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber") // |list=\"adagfx65kcolors\"
                 #  ifdef P123_USE_TOOLTIPS
                 , F("ON color")
                 #  endif // ifdef P123_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // ON Caption
      addTextBox(getPluginCustomArgName(objectNr + 1300),
                 TouchObjects[objectNr].captionOn,
                 P123_MaxObjectNameLength - 1,
                 false,
                 false,
                 EMPTY_STRING,
                 F("wide")
                  #  ifdef P123_USE_TOOLTIPS
                 , F("ON caption")
                  #  endif // ifdef P123_USE_TOOLTIPS
                 );
      html_TD(); // button-type
      addSelector(getPluginCustomArgName(objectNr + 800),
                  static_cast<int>(Button_type_e::Button_MAX),
                  buttonTypeOptions,
                  buttonTypeValues,
                  nullptr,
                  get8BitFromUL(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_BUTTONTYPE), false, true, F("widenumber")
                  #  ifdef P123_USE_TOOLTIPS
                  , F("Buttontype")
                  #  endif // ifdef P123_USE_TOOLTIPS
                  );
      html_TD(); // Background color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorBackground, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1700), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  ifdef P123_USE_TOOLTIPS
                 , F("Background color")
                 #  endif // ifdef P123_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      # endif // ifdef P123_USE_EXTENDED_TOUCH

      html_TR_TD(); // Start new row

      html_TD(3);   // Start with some blank columns
      // html_TD();
      // Width
      addNumericBox(getPluginCustomArgName(objectNr + 400),
                    TouchObjects[objectNr].width_height.x, 0, 65535
                    # ifdef P123_USE_TOOLTIPS
                    , F("widenumber"), F("Width")
                    # endif // ifdef P123_USE_TOOLTIPS
                    );
      html_TD(); // Height
      addNumericBox(getPluginCustomArgName(objectNr + 500),
                    TouchObjects[objectNr].width_height.y, 0, 65535
                    # ifdef P123_USE_TOOLTIPS
                    , F("widenumber"), F("Height")
                    # endif // ifdef P123_USE_TOOLTIPS
                    );

      # ifdef P123_USE_EXTENDED_TOUCH

      // Colored
      // addCheckBox(getPluginCustomArgName(objectNr + 900),
      //             bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_COLORED), false
      //             # ifdef P123_USE_TOOLTIPS
      //             , F("Colored")
      //             # endif // ifdef P123_USE_TOOLTIPS
      //             );
      // html_TD(); // Use caption
      // addCheckBox(getPluginCustomArgName(objectNr + 1200),
      //             bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_CAPTION), false
      //             # ifdef P123_USE_TOOLTIPS
      //             , F("Use Caption")
      //             # endif // ifdef P123_USE_TOOLTIPS
      //             );
      # endif // ifdef P123_USE_EXTENDED_TOUCH
      html_TD(); // inverted
      addCheckBox(getPluginCustomArgName(objectNr + 700),
                  bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_INVERTED), false
                  # ifdef P123_USE_TOOLTIPS
                  , F("Inverted")
                  # endif // ifdef P123_USE_TOOLTIPS
                  );
      # ifdef P123_USE_EXTENDED_TOUCH
      html_TD(); // OFF color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorOff, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1100), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  ifdef P123_USE_TOOLTIPS
                 , F("OFF color")
                 #  endif // ifdef P123_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // OFF Caption
      addTextBox(getPluginCustomArgName(objectNr + 1400),
                 TouchObjects[objectNr].captionOff,
                 P123_MaxObjectNameLength - 1,
                 false,
                 false,
                 EMPTY_STRING,
                 F("wide")
                  #  ifdef P123_USE_TOOLTIPS
                 , F("OFF caption")
                  #  endif // ifdef P123_USE_TOOLTIPS
                 );
      html_TD(); // Caption color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorCaption, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1500), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  ifdef P123_USE_TOOLTIPS
                 , F("Caption color")
                 #  endif // ifdef P123_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // Highlight color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorHighlight, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1600), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  ifdef P123_USE_TOOLTIPS
                 , F("Highlight color")
                 #  endif // ifdef P123_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      # endif // ifdef P123_USE_EXTENDED_TOUCH
    }
    html_end_table();

    addFormNumericBox(F("Debounce delay for On/Off buttons"), F("p123_debounce"),
                      P123_Settings.debounceMs, 0, 255);
    addUnit(F("0-255 msec."));
  }
  return false;
}

/**
 * Save the settings from the web page to flash
 */
bool P123_data_struct::plugin_webform_save(struct EventStruct *event) {
  String config;

  config.reserve(80);

  uint32_t lSettings = 0u;

  bitWrite(lSettings, P123_FLAGS_SEND_XY,          bitRead(getFormItemInt(F("p123_events")), P123_FLAGS_SEND_XY));
  bitWrite(lSettings, P123_FLAGS_SEND_Z,           bitRead(getFormItemInt(F("p123_events")), P123_FLAGS_SEND_Z));
  bitWrite(lSettings, P123_FLAGS_SEND_OBJECTNAME,  bitRead(getFormItemInt(F("p123_events")), P123_FLAGS_SEND_OBJECTNAME));
  bitWrite(lSettings, P123_FLAGS_ROTATION_FLIPPED, isFormItemChecked(F("p123_rotation_flipped")));
  bitWrite(lSettings, P123_FLAGS_DEDUPLICATE,      isFormItemChecked(F("p123_deduplicate")));
  bitWrite(lSettings, P123_FLAGS_INIT_OBJECTEVENT, isFormItemChecked(F("p123_init_objectevent")));

  config += getFormItemInt(F("p123_use_calibration"));
  config += P123_SETTINGS_SEPARATOR;
  config += isFormItemChecked(F("p123_log_calibration")) ? 1 : 0;
  config += P123_SETTINGS_SEPARATOR;
  config += getFormItemInt(F("p123_cal_tl_x"));
  config += P123_SETTINGS_SEPARATOR;
  config += getFormItemInt(F("p123_cal_tl_y"));
  config += P123_SETTINGS_SEPARATOR;
  config += getFormItemInt(F("p123_cal_br_x"));
  config += P123_SETTINGS_SEPARATOR;
  config += getFormItemInt(F("p123_cal_br_y"));
  config += P123_SETTINGS_SEPARATOR;
  config += getFormItemInt(F("p123_treshold"));
  config += P123_SETTINGS_SEPARATOR;
  config += getFormItemInt(F("p123_debounce"));
  config += P123_SETTINGS_SEPARATOR;
  config += lSettings;
  config += P123_SETTINGS_SEPARATOR;

  settingsArray[P123_CALIBRATION_START] = config;
  {
    String log = F("Save settings: ");
    config.replace(P123_SETTINGS_SEPARATOR, ',');
    log += config;
    addLogMove(LOG_LEVEL_INFO, log);
  }

  String error;

  for (int objectNr = 0; objectNr < P123_MAX_OBJECT_COUNT; objectNr++) {
    config.clear();
    config += webArg(getPluginCustomArgName(objectNr + 100));           // Name

    if (!config.isEmpty()) {                                            // Empty name => skip entry
      if (!ExtraTaskSettings.checkInvalidCharInNames(config.c_str())) { // Check for invalid characters in objectname
        error += F("Invalid character in objectname #");
        error += objectNr;
        error += F(". Do not use ',-+/*=^%!#[]{}()' or space.\n");
      }
      config += P123_SETTINGS_SEPARATOR;
      uint32_t flags = 0u;
      bitWrite(flags, P123_OBJECT_FLAG_ENABLED,  isFormItemChecked(getPluginCustomArgName(objectNr + 0)));   // Enabled
      bitWrite(flags, P123_OBJECT_FLAG_BUTTON,   isFormItemChecked(getPluginCustomArgName(objectNr + 600))); // On/Off button
      bitWrite(flags, P123_OBJECT_FLAG_INVERTED, isFormItemChecked(getPluginCustomArgName(objectNr + 700))); // Inverted
      # ifdef P123_USE_EXTENDED_TOUCH

      // bitWrite(flags, P123_OBJECT_FLAG_COLORED,  isFormItemChecked(getPluginCustomArgName(objectNr + 900)));   // Colored
      // bitWrite(flags, P123_OBJECT_FLAG_CAPTION,  isFormItemChecked(getPluginCustomArgName(objectNr + 1200)));  // Use caption
      set8BitToUL(flags, P123_OBJECT_FLAG_BUTTONTYPE, getFormItemInt(getPluginCustomArgName(objectNr + 800))); // Buttontype
      # endif // ifdef P123_USE_EXTENDED_TOUCH
      config += flags;                                                                                         // Flags
      config += P123_SETTINGS_SEPARATOR;
      config += getFormItemInt(getPluginCustomArgName(objectNr + 200));                                        // Top x
      config += P123_SETTINGS_SEPARATOR;
      config += getFormItemInt(getPluginCustomArgName(objectNr + 300));                                        // Top y
      config += P123_SETTINGS_SEPARATOR;
      config += getFormItemInt(getPluginCustomArgName(objectNr + 400));                                        // Bottom x
      config += P123_SETTINGS_SEPARATOR;
      config += getFormItemInt(getPluginCustomArgName(objectNr + 500));                                        // Bottom y
      config += P123_SETTINGS_SEPARATOR;

      # ifdef P123_USE_EXTENDED_TOUCH
      String colorInput;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1000));                // Color ON
      config    += AdaGFXparseColor(colorInput, _colorDepth, true);
      config    += P123_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1100));                // Color OFF
      config    += AdaGFXparseColor(colorInput, _colorDepth, true);
      config    += P123_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1500));                // Color caption
      config    += AdaGFXparseColor(colorInput, _colorDepth, true);
      config    += P123_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1600));                // Color Highlight
      config    += AdaGFXparseColor(colorInput, _colorDepth, true);
      config    += P123_SETTINGS_SEPARATOR;
      config    += enquoteString(webArg(getPluginCustomArgName(objectNr + 1300))); // Caption ON
      config    += P123_SETTINGS_SEPARATOR;
      config    += enquoteString(webArg(getPluginCustomArgName(objectNr + 1400))); // Caption OFF
      config    += P123_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1700));                // Color Background
      config    += AdaGFXparseColor(colorInput, _colorDepth, true);
      config    += P123_SETTINGS_SEPARATOR;
      # endif // ifdef P123_USE_EXTENDED_TOUCH
    }
    config.trim();

    while (!config.isEmpty() && config[config.length() - 1] == P123_SETTINGS_SEPARATOR) {
      config.remove(config.length() - 1);
    }

    settingsArray[objectNr + P123_OBJECT_INDEX_START] = config;

    # ifdef PLUGIN_123_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Save object #");
      log += objectNr;
      log += F(" settings: ");
      config.replace(P123_SETTINGS_SEPARATOR, ',');
      log += config;
      addLogMove(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef PLUGIN_123_DEBUG
  }

  if (error.length() > 0) {
    addHtmlError(error);
  }

  error = SaveCustomTaskSettings(event->TaskIndex, settingsArray, P123_ARRAY_SIZE, 0);

  if (!error.isEmpty()) {
    addHtmlError(error);
    return false;
  }
  return true;
}

/**
 * Every 10th second we check if the screen is touched
 */
bool P123_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  bool success = false;

  if (isInitialized()) {
    if (touched()) {
      int16_t x = 0, y = 0, ox = 0, oy = 0, rx, ry;
      int16_t z = 0;
      readData(x, y, z, ox, oy);

      rx = x;
      ry = y;
      scaleRawToCalibrated(x, y); // Map to screen coordinates if so configured

      // Avoid event-storms by deduplicating coordinates
      if (!_deduplicate ||
          (_deduplicate && ((P123_VALUE_X != x) || (P123_VALUE_Y != y) || (P123_VALUE_Z != z)))) {
        success      = true;
        P123_VALUE_X = x;
        P123_VALUE_Y = y;
        P123_VALUE_Z = z;
      }

      if (success &&
          P123_Settings.logEnabled &&
          loglevelActiveFor(LOG_LEVEL_INFO)) { // REQUIRED for calibration and setting up objects, so do not make this optional!
        String log;
        log.reserve(72);
        log  = F("Touch calibration rx= ");    // Space before the logged values was added for readability
        log += rx;
        log += F(", ry= ");
        log += ry;
        log += F("; z= "); // Always log the z value even if not used.
        log += z;
        log += F(", x= ");
        log += x;
        log += F(", y= ");
        log += y;

        log += F("; ox= ");
        log += ox;
        log += F(", oy= ");
        log += oy;
        addLogMove(LOG_LEVEL_INFO, log);
      }

      if (Settings.UseRules) {                                                                     // No events to handle if rules not
                                                                                                   // enabled
        if (success && bitRead(P123_Settings.flags, P123_FLAGS_SEND_XY)) {                         // Send events for each touch
          const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

          if (!bitRead(P123_Settings.flags, P123_FLAGS_SEND_Z) && validDeviceIndex(DeviceIndex)) { // Do NOT send a Z event for each
            // touch?
            Device[DeviceIndex].VType      = Sensor_VType::SENSOR_TYPE_DUAL;
            Device[DeviceIndex].ValueCount = 2;
          }
          sendData(event);                                                                         // Send X/Y(/Z) event

          if (!bitRead(P123_Settings.flags, P123_FLAGS_SEND_Z) && validDeviceIndex(DeviceIndex)) { // Reset device configuration
            Device[DeviceIndex].VType      = Sensor_VType::SENSOR_TYPE_TRIPLE;
            Device[DeviceIndex].ValueCount = 3;
          }
        }

        if (bitRead(P123_Settings.flags, P123_FLAGS_SEND_OBJECTNAME)) { // Send events for objectname if within reach
          String selectedObjectName;
          int8_t selectedObjectIndex = -1;

          if (isValidAndTouchedTouchObject(x, y, selectedObjectName, selectedObjectIndex)) {
            if ((selectedObjectIndex > -1) && bitRead(TouchObjects[selectedObjectIndex].flags, P123_OBJECT_FLAG_BUTTON)) {
              if ((TouchObjects[selectedObjectIndex].TouchTimers == 0) ||

                  // Not touched yet or too long ago
                  (TouchObjects[selectedObjectIndex].TouchTimers < (millis() - (1.5 * P123_Settings.debounceMs)))) {
                // From now wait the debounce time
                TouchObjects[selectedObjectIndex].TouchTimers = millis() + P123_Settings.debounceMs;
              } else {
                // Debouncing time elapsed?
                if (TouchObjects[selectedObjectIndex].TouchTimers <= millis()) {
                  TouchObjects[selectedObjectIndex].TouchStates = !TouchObjects[selectedObjectIndex].TouchStates;
                  TouchObjects[selectedObjectIndex].TouchTimers = 0;
                  generateObjectEvent(event, selectedObjectIndex, TouchObjects[selectedObjectIndex].TouchStates ? 1 : 0);
                }
              }
            } else {
              // Matching object is found, send <TaskDeviceName>#<ObjectName> event with x, y and z as %eventvalue1/2/3%
              String eventCommand;
              eventCommand.reserve(48);
              eventCommand  = getTaskDeviceName(event->TaskIndex);
              eventCommand += '#';
              eventCommand += selectedObjectName;
              eventCommand += '='; // Add arguments
              eventCommand += x;
              eventCommand += ',';
              eventCommand += y;
              eventCommand += ',';
              eventCommand += z;
              eventQueue.addMove(std::move(eventCommand));
            }
          }
        }
      }
    }
  }
  return success;
}

/**
 * generate an event for a touch object
 * When a display is configured add x,y coordinate, width,height of the object, objectIndex, and TaskIndex of display
 **************************************************************************/
void P123_data_struct::generateObjectEvent(const EventStruct *event,
                                           const int8_t       objectIndex,
                                           const int8_t       onOffState) {
  String eventCommand;

  eventCommand.reserve(48);
  eventCommand  = getTaskDeviceName(event->TaskIndex);
  eventCommand += '#';
  eventCommand += TouchObjects[objectIndex].objectName;
  eventCommand += '=';                             // Add arguments

  if (onOffState < 0) {                            // Negative value: pass on unaltered
    eventCommand += onOffState;
  } else {                                         // Check for inverted output
    if (bitRead(TouchObjects[objectIndex].flags, P123_OBJECT_FLAG_INVERTED)) {
      eventCommand += onOffState == 1 ? '0' : '1'; // Act like an inverted button, 0 = On, 1 = Off
    } else {
      eventCommand += onOffState == 1 ? '1' : '0'; // Act like a button, 1 = On, 0 = Off
    }
  }

  if (P123_CONFIG_DISPLAY_TASK != event->TaskIndex) { // Add arguments for display
    eventCommand += ',';
    eventCommand += TouchObjects[objectIndex].top_left.x;
    eventCommand += ',';
    eventCommand += TouchObjects[objectIndex].top_left.y;
    eventCommand += ',';
    eventCommand += TouchObjects[objectIndex].width_height.x;
    eventCommand += ',';
    eventCommand += TouchObjects[objectIndex].width_height.y;
    eventCommand += ',';
    eventCommand += objectIndex + 1;              // Adjust to displayed index
    eventCommand += ',';
    eventCommand += P123_CONFIG_DISPLAY_TASK + 1; // What TaskIndex?
  }
  eventQueue.addMove(std::move(eventCommand));
}

/**
 * draw a button using the mode and state
 * //TODO: Complete implementation
 * will probably need:
 * - Access to the AdafruitGFX_Helper object
 * - Access to the Display object in the AdafruitGFX_Helper
 * - Access to the list of available fonts, to be able to change the font for button captions, and to center the caption on the button
 */
# ifdef P123_USE_EXTENDED_TOUCH
void P123_data_struct::drawButton(DrawButtonMode_e   buttonMode,
                                  int8_t             buttonIndex,
                                  const EventStruct *event) {
  if ((buttonIndex < 0) || (buttonIndex >= static_cast<int8_t>(TouchObjects.size()))) { return; } // Selfprotection

  if (!Settings.TaskDeviceEnabled[_displayTask]) { return; }                 // No active DisplayTask is no drawing buttons

  Button_type_e bType = static_cast<Button_type_e>(get8BitFromUL(TouchObjects[buttonIndex].flags, P123_OBJECT_FLAG_BUTTONTYPE));
  String cmdPrefix;
  String btnDrawShape;
  int8_t xa = 0, ya = 0, wa = 0, ha = 0;

  cmdPrefix.reserve(30);
  cmdPrefix  = getTaskDeviceName(_displayTask);
  cmdPrefix += '.'; // a period
  cmdPrefix += ADAGFX_UNIVERSAL_TRIGGER;
  cmdPrefix += ',';

  btnDrawShape.reserve(50);

  if (bType != Button_type_e::None) {
    switch (buttonMode) {
      case DrawButtonMode_e::Initialize:
        break;
      case DrawButtonMode_e::State:
      {
        xa = 1; ya = 1;
        wa = -2; ha = -2;
        break;
      }
      case DrawButtonMode_e::Highlight:
        break;
    }
  }

  switch (bType) {
    case Button_type_e::None:
      break;
    case Button_type_e::Square:
      btnDrawShape = F("r");
      break;
    case Button_type_e::Rounded:
      btnDrawShape = F("rr");
      break;
    case Button_type_e::Circle:
      btnDrawShape = F("c");
      break;
    case Button_type_e::Button_MAX:
      break;
  }

  if ((bType != Button_type_e::None) &&
      (buttonMode == DrawButtonMode_e::Initialize)) { btnDrawShape += 'f'; }

  switch (bType) {
    case Button_type_e::None:
      break;
    case Button_type_e::Square:
      btnDrawShape += ',';
      btnDrawShape += TouchObjects[buttonIndex].top_left.x + xa;
      btnDrawShape += ',';
      btnDrawShape += TouchObjects[buttonIndex].top_left.y + ya;
      btnDrawShape += ',';
      btnDrawShape += TouchObjects[buttonIndex].width_height.x + wa;
      btnDrawShape += ',';
      btnDrawShape += TouchObjects[buttonIndex].width_height.y + ha;
      btnDrawShape += ',';

      if (buttonMode == DrawButtonMode_e::Initialize) {
        btnDrawShape += AdaGFXcolorToString(TouchObjects[buttonIndex].colorCaption, _colorDepth);
      } else {
        btnDrawShape += AdaGFXcolorToString(TouchObjects[buttonIndex].colorBackground, _colorDepth);
      }

      if (buttonMode == DrawButtonMode_e::Initialize) {
        btnDrawShape += ',';
        btnDrawShape += AdaGFXcolorToString(TouchObjects[buttonIndex].colorBackground, _colorDepth);
      }
      break;
    case Button_type_e::Rounded:
      break;
    case Button_type_e::Circle:
      break;
    case Button_type_e::Button_MAX:
      break;
  }

  if (bType != Button_type_e::None) {
    String btnDrawCmd;
    btnDrawCmd.reserve(80);
    btnDrawCmd  = cmdPrefix;
    btnDrawCmd += btnDrawShape;
    ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_RULES, btnDrawCmd.c_str());
  }
}

# endif // ifdef P123_USE_EXTENDED_TOUCH

/**
 * Load the touch objects from the settings, and initialize then properly where needed.
 */
void P123_data_struct::loadTouchObjects(const EventStruct *event) {
  # ifdef PLUGIN_123_DEBUG
  addLogMove(LOG_LEVEL_INFO, F("P123 DEBUG loadTouchObjects"));
  # endif // PLUGIN_123_DEBUG
  LoadCustomTaskSettings(event->TaskIndex, settingsArray, P123_ARRAY_SIZE, 0);

  lastObjectIndex = P123_OBJECT_INDEX_START - 1; // START must be > 0!!!

  objectCount = 0;

  for (uint8_t i = P123_OBJECT_INDEX_END; i >= P123_OBJECT_INDEX_START; i--) {
    if (!settingsArray[i].isEmpty() && (lastObjectIndex < P123_OBJECT_INDEX_START)) {
      lastObjectIndex = i;
      objectCount++; // Count actual number of objects
    }
  }

  // Get calibration and common settings
  P123_Settings.calibrationEnabled = parseStringToInt(settingsArray[P123_CALIBRATION_START],
                                                      P123_CALIBRATION_ENABLED,     P123_SETTINGS_SEPARATOR) == 1;
  P123_Settings.logEnabled = parseStringToInt(settingsArray[P123_CALIBRATION_START],
                                              P123_CALIBRATION_LOG_ENABLED, P123_SETTINGS_SEPARATOR) == 1;
  int lSettings = 0;

  bitWrite(lSettings, P123_FLAGS_SEND_XY,         P123_TS_SEND_XY);
  bitWrite(lSettings, P123_FLAGS_SEND_Z,          P123_TS_SEND_Z);
  bitWrite(lSettings, P123_FLAGS_SEND_OBJECTNAME, P123_TS_SEND_OBJECTNAME);
  P123_Settings.flags = parseStringToInt(settingsArray[P123_CALIBRATION_START],
                                         P123_COMMON_FLAGS, P123_SETTINGS_SEPARATOR, lSettings);
  P123_Settings.top_left.x     = parseStringToInt(settingsArray[P123_CALIBRATION_START], P123_CALIBRATION_TOP_X, P123_SETTINGS_SEPARATOR);
  P123_Settings.top_left.y     = parseStringToInt(settingsArray[P123_CALIBRATION_START], P123_CALIBRATION_TOP_Y, P123_SETTINGS_SEPARATOR);
  P123_Settings.bottom_right.x = parseStringToInt(settingsArray[P123_CALIBRATION_START], P123_CALIBRATION_BOTTOM_X, P123_SETTINGS_SEPARATOR);
  P123_Settings.bottom_right.y = parseStringToInt(settingsArray[P123_CALIBRATION_START], P123_CALIBRATION_BOTTOM_Y, P123_SETTINGS_SEPARATOR);
  P123_Settings.debounceMs     = parseStringToInt(settingsArray[P123_CALIBRATION_START], P123_COMMON_DEBOUNCE_MS, P123_SETTINGS_SEPARATOR,
                                                  P123_DEBOUNCE_MILLIS);
  P123_Settings.treshold = parseStringToInt(settingsArray[P123_CALIBRATION_START], P123_COMMON_TOUCH_TRESHOLD, P123_SETTINGS_SEPARATOR,
                                            P123_TS_TRESHOLD);

  settingsArray[P123_CALIBRATION_START].clear(); // Free a little memory

  // Buffer some settings, mostly for readability, but also to be able to set from write command
  _flipped     = bitRead(P123_Settings.flags, P123_FLAGS_ROTATION_FLIPPED);
  _deduplicate = bitRead(P123_Settings.flags, P123_FLAGS_DEDUPLICATE);

  TouchObjects.clear();

  if (objectCount > 0) {
    TouchObjects.reserve(objectCount);
    uint8_t t = 0u;

    for (uint8_t i = P123_OBJECT_INDEX_START; i <= lastObjectIndex; i++) {
      if (!settingsArray[i].isEmpty()) {
        TouchObjects.push_back(tP123_TouchObjects());
        TouchObjects[t].flags          = parseStringToInt(settingsArray[i], P123_OBJECT_FLAGS, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].objectName     = parseStringKeepCase(settingsArray[i], P123_OBJECT_NAME, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].top_left.x     = parseStringToInt(settingsArray[i], P123_OBJECT_COORD_TOP_X, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].top_left.y     = parseStringToInt(settingsArray[i], P123_OBJECT_COORD_TOP_Y, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].width_height.x = parseStringToInt(settingsArray[i], P123_OBJECT_COORD_WIDTH, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].width_height.y = parseStringToInt(settingsArray[i], P123_OBJECT_COORD_HEIGHT, P123_SETTINGS_SEPARATOR);
        # ifdef P123_USE_EXTENDED_TOUCH
        TouchObjects[t].colorOn         = parseStringToInt(settingsArray[i], P123_OBJECT_COLOR_ON, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].colorOff        = parseStringToInt(settingsArray[i], P123_OBJECT_COLOR_OFF, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].colorCaption    = parseStringToInt(settingsArray[i], P123_OBJECT_COLOR_CAPTION, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].colorHighlight  = parseStringToInt(settingsArray[i], P123_OBJECT_COLOR_HIGHLIGHT, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].captionOn       = parseStringKeepCase(settingsArray[i], P123_OBJECT_CAPTION_ON, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].captionOff      = parseStringKeepCase(settingsArray[i], P123_OBJECT_CAPTION_OFF, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].colorBackground = parseStringToInt(settingsArray[i], P123_OBJECT_COLOR_BACKGROUND, P123_SETTINGS_SEPARATOR);
        # endif // ifdef P123_USE_EXTENDED_TOUCH

        TouchObjects[t].SurfaceAreas = 0; // Reset runtime stuff
        TouchObjects[t].TouchTimers  = 0;
        TouchObjects[t].TouchStates  = false;

        t++;

        settingsArray[i].clear(); // Free a little memory
      }
    }
  }
}

/**
 * Check if the screen is touched.
 */
bool P123_data_struct::touched() {
  if (isInitialized()) {
    return touchscreen->touched();
  }
  return false;
}

/**
 * Read the raw data if the touchscreen is initialized.
 */
void P123_data_struct::readData(int16_t& x, int16_t& y, int16_t& z, int16_t& ox, int16_t& oy) {
  if (isInitialized()) {
    FT_Point p = touchscreen->getPoint();

    int16_t _x = p.x;
    int16_t _y = p.y;

    // Rotate, as the driver doesn't provide that, use native touch-panel resolution
    switch (_rotation) {
      case TOUCHOBJECTS_HELPER_ROTATION_90:

        if (_flipped) {
          p.x = map(_y, 0, P123_TOUCH_Y_NATIVE, P123_TOUCH_Y_NATIVE, 0);
          p.y = _x;
        } else {
          p.x = _y;
          p.y = map(_x, 0, P123_TOUCH_X_NATIVE, P123_TOUCH_X_NATIVE, 0);
        }
        break;
      case TOUCHOBJECTS_HELPER_ROTATION_180:

        if (!_flipped) { // Change only when not flipped
          p.x = map(_x, 0, P123_TOUCH_X_NATIVE, P123_TOUCH_X_NATIVE, 0);
          p.y = map(_y, 0, P123_TOUCH_Y_NATIVE, P123_TOUCH_Y_NATIVE, 0);
        }
        break;
      case TOUCHOBJECTS_HELPER_ROTATION_270:

        if (_flipped) {
          p.x = _y;
          p.y = map(_x, 0, P123_TOUCH_X_NATIVE, P123_TOUCH_X_NATIVE, 0);
        } else {
          p.x = map(_y, 0, P123_TOUCH_Y_NATIVE, P123_TOUCH_Y_NATIVE, 0);
          p.y = _x;
        }
        break;
      default:

        if (_flipped) {
          p.x = map(p.x, 0, P123_TOUCH_X_NATIVE, P123_TOUCH_X_NATIVE, 0);
          p.y = map(p.y, 0, P123_TOUCH_Y_NATIVE, P123_TOUCH_Y_NATIVE, 0);
        }
        break;
    }

    x  = p.x;
    y  = p.y;
    z  = p.z;
    ox = _x;
    oy = _y;
  }
}

/**
 * Set rotation
 */
void P123_data_struct::setRotation(uint8_t n) {
  _rotation = n;
  # ifdef PLUGIN_123_DEBUG
  String log = F("P123 DEBUG Rotation set: ");
  log += n;
  addLogMove(LOG_LEVEL_INFO, log);
  # endif // PLUGIN_123_DEBUG
}

/**
 * Set rotationFlipped
 */
void P123_data_struct::setRotationFlipped(bool flipped) {
  _flipped = flipped;
  # ifdef PLUGIN_123_DEBUG
  String log = F("P123 DEBUG RotationFlipped set: ");
  log += flipped;
  addLogMove(LOG_LEVEL_INFO, log);
  # endif // PLUGIN_123_DEBUG
}

/**
 * Determine if calibration is enabled and usable.
 */
bool P123_data_struct::isCalibrationActive() {
  return _useCalibration
         && (P123_Settings.top_left.x != 0
             || P123_Settings.top_left.y != 0
             || P123_Settings.bottom_right.x != 0
             || P123_Settings.bottom_right.y != 0); // Enabled and any value != 0 => Active
}

/**
 * Check within the list of defined objects if we touched one of them.
 * The smallest matching surface is selected if multiple objects overlap.
 * Returns state, and sets selectedObjectName to the best matching object
 */
bool P123_data_struct::isValidAndTouchedTouchObject(int16_t x,
                                                    int16_t y,
                                                    String& selectedObjectName,
                                                    int8_t& selectedObjectIndex) {
  uint32_t lastObjectArea = 0u;
  bool     selected       = false;

  for (int objectNr = 0; objectNr < static_cast<int>(TouchObjects.size()); objectNr++) {
    if (!TouchObjects[objectNr].objectName.isEmpty()
        && bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ENABLED)
        && (TouchObjects[objectNr].width_height.x != 0)
        && (TouchObjects[objectNr].width_height.y != 0)) { // Not initial could be valid
      if (TouchObjects[objectNr].SurfaceAreas == 0) {      // Need to calculate the surface area
        TouchObjects[objectNr].SurfaceAreas = TouchObjects[objectNr].width_height.x * TouchObjects[objectNr].width_height.y;
      }

      if ((TouchObjects[objectNr].top_left.x <= x)
          && (TouchObjects[objectNr].top_left.y <= y)
          && ((TouchObjects[objectNr].width_height.x + TouchObjects[objectNr].top_left.x) >= x)
          && ((TouchObjects[objectNr].width_height.y + TouchObjects[objectNr].top_left.y) >= y)
          && ((lastObjectArea == 0)
              || (TouchObjects[objectNr].SurfaceAreas < lastObjectArea))) { // Select smallest area that fits the coordinates
        selectedObjectName  = TouchObjects[objectNr].objectName;
        selectedObjectIndex = objectNr;
        lastObjectArea      = TouchObjects[objectNr].SurfaceAreas;
        selected            = true;
      }
      # ifdef PLUGIN_123_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("P123 DEBUG Touched: obj: ");
        log += TouchObjects[objectNr].objectName;
        log += ',';
        log += TouchObjects[objectNr].top_left.x;
        log += ',';
        log += TouchObjects[objectNr].top_left.y;
        log += ',';
        log += TouchObjects[objectNr].width_height.x;
        log += ',';
        log += TouchObjects[objectNr].width_height.y;
        log += F(" surface:");
        log += TouchObjects[objectNr].SurfaceAreas;
        log += F(" x,y:");
        log += x;
        log += ',';
        log += y;
        log += F(" sel:");
        log += selectedObjectName;
        log += '/';
        log += selectedObjectIndex;
        log += '/';
        log += selected ? 'T' : 'f';
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
      # endif // PLUGIN_123_DEBUG
    }
  }
  return selected;
}

/**
 * Set the enabled/disabled state by inserting or deleting an underscore '_' as the first character of the object name.
 * Checks if the name doesn't exceed the max. length.
 */
bool P123_data_struct::setTouchObjectState(const String& touchObject, bool state) {
  if (touchObject.isEmpty()) { return false; }
  String findObject; // = (state ? F("_") : F("")); // When enabling, try to find a disabled object

  findObject += touchObject;
  String thisObject;
  bool   success = false;

  thisObject.reserve(P123_MaxObjectNameLength);

  for (int objectNr = 0; objectNr < static_cast<int>(TouchObjects.size()); objectNr++) {
    if ((!TouchObjects[objectNr].objectName.isEmpty())
        && findObject.equalsIgnoreCase(TouchObjects[objectNr].objectName)) {
      // uint32_t objectFlags = parseStringToInt(settingsArray[objectNr], P123_OBJECT_FLAGS, P123_SETTINGS_SEPARATOR);
      bool enabled = bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ENABLED);

      if (state != enabled) {
        bitWrite(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ENABLED, state);
        success = true;

        // String config;
        // config.reserve(settingsArray[objectNr].length() + 1);
        // config += objectFlags;
        // config += P123_SETTINGS_SEPARATOR;

        // // Rest of the string
        // config                 += parseStringToEndKeepCase(settingsArray[objectNr], P123_OBJECT_NAME, P123_SETTINGS_SEPARATOR);
        // settingsArray[objectNr] = config; // Store
      }
      # ifdef PLUGIN_123_DEBUG
      String log = F("P123 setTouchObjectState: obj: ");
      log += thisObject;

      if (success) {
        log += F(", new state: ");
        log += (state ? F("en") : F("dis"));
        log += F("abled.");
      } else {
        log += F("failed!");
      }
      addLogMove(LOG_LEVEL_INFO, log);
      # endif // PLUGIN_123_DEBUG
    }
  }
  return success;
}

/**
 * Scale the provided raw coordinates to screen-resolution coordinates if calibration is enabled/configured
 */
void P123_data_struct::scaleRawToCalibrated(int16_t& x, int16_t& y) {
  if (isCalibrationActive()) {
    int16_t lx = x - P123_Settings.top_left.x;

    if (lx <= 0) {
      x = 0;
    } else {
      if (lx > P123_Settings.bottom_right.x) {
        lx = P123_Settings.bottom_right.x;
      }
      float x_fact = static_cast<float>(P123_Settings.bottom_right.x - P123_Settings.top_left.x) /
                     static_cast<float>(_ts_x_res);
      x = static_cast<uint16_t>(round(lx / x_fact));
    }
    int16_t ly = y - P123_Settings.top_left.y;

    if (ly <= 0) {
      y = 0;
    } else {
      if (ly > P123_Settings.bottom_right.y) {
        ly = P123_Settings.bottom_right.y;
      }
      float y_fact = (P123_Settings.bottom_right.y - P123_Settings.top_left.y) / _ts_y_res;
      y = static_cast<uint16_t>(round(ly / y_fact));
    }
  }
}

/******************************************
 * enquoteString wrap in ", ' or ` unless all 3 quote types are used
 * TODO: Replace with wrapWithQuotes() once available
 *****************************************/
String P123_data_struct::enquoteString(const String& input) {
  char quoteChar = '"';

  if (input.indexOf(quoteChar) > -1) {
    quoteChar = '\'';

    if (input.indexOf(quoteChar) > -1) {
      quoteChar = '`';

      if (input.indexOf(quoteChar) > -1) {
        return input; // All types of supported quotes used, return original string
      }
    }
  }
  String result;

  result.reserve(input.length() + 2);
  result  = quoteChar;
  result += input;
  result += quoteChar;

  return result;
}

#endif // ifdef USES_P123
