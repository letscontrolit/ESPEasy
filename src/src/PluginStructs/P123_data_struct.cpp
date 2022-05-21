#include "../PluginStructs/P123_data_struct.h"

#ifdef USES_P123

# include "../ESPEasyCore/ESPEasyNetwork.h"

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Scheduler.h"
# include "../Helpers/StringConverter.h"
# include "../Helpers/SystemVariables.h"

# include "../Commands/InternalCommands.h"

/****************************************************************************
 * toString: Display-value for the touch action
 ***************************************************************************/
# ifdef P123_USE_EXTENDED_TOUCH
const __FlashStringHelper* toString(P123_touch_action_e action) {
  switch (action) {
    case P123_touch_action_e::Default: return F("Default");
    case P123_touch_action_e::ActivateGroup: return F("Activate Group");
    case P123_touch_action_e::IncrementGroup: return F("Next Group");
    case P123_touch_action_e::DecrementGroup: return F("Previous Group");
    case P123_touch_action_e::TouchAction_MAX: break;
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
      if (_maxButtonGroup > 0) {                     // Multiple groups?
        displayButtonGroup(event, _buttonGroup, -3); // Clear all groups
      }
      _buttonGroup = get8BitFromUL(P123_Settings.flags, P123_FLAGS_INITIAL_GROUP);
      # ifdef PLUGIN_123_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("P123 DEBUG group: ");
        log += _buttonGroup;
        log += F(", max group: ");
        log += _maxButtonGroup;
        addLogMove(LOG_LEVEL_INFO, log);
      }
      # endif // ifdef PLUGIN_123_DEBUG

      displayButtonGroup(event, _buttonGroup); // Initialize selected group and group 0

      # ifdef PLUGIN_123_DEBUG
      addLogMove(LOG_LEVEL_INFO, F("P123 DEBUG group done."));
      # endif // ifdef PLUGIN_123_DEBUG
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
 * mode: -2 = clear buttons in group, -3 = clear all buttongroups, -1 = draw buttons in group, 0 = initialize buttons
 */
void P123_data_struct::displayButtonGroup(const EventStruct *event,
                                          int8_t             buttonGroup,
                                          int8_t             mode) {
  for (int objectNr = 0; objectNr < static_cast<int>(TouchObjects.size()); objectNr++) {
    int8_t state = 99;
    int8_t group = get8BitFromUL(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_GROUP);

    if (!TouchObjects[objectNr].objectName.isEmpty() &&
        ((bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ENABLED) && (group == 0)) || (group > 0)) &&
        bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_BUTTON) &&
        ((group == buttonGroup) ||
         ((mode != -2) && (group == 0)) ||
         (mode == -3))) {
      if (bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ENABLED)) {
        if (mode == 0) {
          state = -1;
        } else {
          if (bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_INVERTED)) {
            state = TouchObjects[objectNr].TouchStates ? 0 : 1; // Act like an inverted button, 0 = On, 1 = Off
          } else {
            state = TouchObjects[objectNr].TouchStates ? 1 : 0; // Act like a button, 1 = On, 0 = Off
          }
        }
      } else {
        state = -2;
      }
      generateObjectEvent(event, objectNr, state, mode < 0, mode <= -2 ? -1 : 1);
    }
    # ifdef XX_PLUGIN_123_DEBUG // TODO Temporarily disabled

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("P123: button init, state: ");
      log += state;
      log += F(", group: ");
      log += buttonGroup;
      log += F(", mode: ");
      log += mode;
      log += F(", group: ");
      log += get8BitFromUL(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_GROUP);
      log += F(", en: ");
      log += bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_BUTTON);
      log += F(", object: ");
      log += objectNr;
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef PLUGIN_123_DEBUG

    delay(0);
  }

  // Send an event <taskname>#Group,<group>,<mode> with the selected group and the mode (-3..0)
  String eventCommand;
  eventCommand.reserve(24);
  eventCommand += getTaskDeviceName(event->TaskIndex);
  eventCommand += '#';
  eventCommand += F("Group");
  eventCommand += '='; // Add arguments
  eventCommand += buttonGroup;
  eventCommand += ',';
  eventCommand += mode;
  eventQueue.addMove(std::move(eventCommand));

  delay(0);
}

/**
 * Properly initialized? then true
 */
bool P123_data_struct::isInitialized() const {
  return touchscreen != nullptr;
}

int P123_data_struct::parseStringToInt(const String& string, uint8_t indexFind, char separator, int defaultValue) {
  String parsed = parseStringKeepCase(string, indexFind, separator);

  int result = defaultValue;

  validIntFromString(parsed, result);

  return result;
}

/**
 * Load the settings onto the webpage
 */
bool P123_data_struct::plugin_webform_load(struct EventStruct *event) {
  addFormSubHeader(F("Touch configuration"));

  addFormCheckBox(F("Flip rotation 180&deg;"), F("tch_rotation_flipped"), bitRead(P123_Settings.flags, P123_FLAGS_ROTATION_FLIPPED));
  # ifndef LIMIT_BUILD_SIZE
  addFormNote(F("Some touchscreens are mounted 180&deg; rotated on the display."));
  # endif // ifndef LIMIT_BUILD_SIZE

  addFormNumericBox(F("Touch minimum pressure"), F("tch_treshold"),
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
    addFormSelector(F("Events"), F("tch_events"), P123_EVENTS_OPTIONS, options3, optionValues3, choice3);

    addFormCheckBox(F("Initial Objectnames events"), F("tch_init_objectevent"), bitRead(P123_Settings.flags, P123_FLAGS_INIT_OBJECTEVENT));
    addFormNote(F("Will send state -1 but only for enabled On/Off button objects."));
  }

  addFormCheckBox(F("Prevent duplicate events"), F("tch_deduplicate"), bitRead(P123_Settings.flags, P123_FLAGS_DEDUPLICATE));

  # ifndef LIMIT_BUILD_SIZE

  if (!Settings.UseRules) {
    addFormNote(F("Tools / Advanced / Rules must be enabled for events to be fired."));
  }
  # endif // ifndef LIMIT_BUILD_SIZE

  addFormSubHeader(F("Calibration"));

  {
    const __FlashStringHelper *noYesOptions[2] = { F("No"), F("Yes") };
    int noYesOptionValues[2]                   = { 0, 1 };
    addFormSelector(F("Calibrate to screen resolution"),
                    F("tch_use_calibration"),
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
    addNumericBox(F("tch_cal_tl_x"),
                  P123_Settings.top_left.x,
                  0,
                  65535);
    html_TD();
    addNumericBox(F("tch_cal_tl_y"),
                  P123_Settings.top_left.y,
                  0,
                  65535);
    html_TD();
    addHtml(F("Bottom-right"));
    html_TD();
    addNumericBox(F("tch_cal_br_x"),
                  P123_Settings.bottom_right.x,
                  0,
                  65535);
    html_TD();
    addNumericBox(F("tch_cal_br_y"),
                  P123_Settings.bottom_right.y,
                  0,
                  65535);

    html_end_table();
  }

  addFormCheckBox(F("Enable logging for calibration"), F("tch_log_calibration"),
                  P123_Settings.logEnabled);

  addFormSubHeader(F("Touch objects"));

  # ifdef P123_USE_EXTENDED_TOUCH

  AdaGFXHtmlColorDepthDataList(F("adagfx65kcolors"), static_cast<AdaGFXColorDepth>(P123_COLOR_DEPTH));

  {
    String parsed;
    addRowLabel(F("Default On/Off button colors"));
    html_table(EMPTY_STRING, false); // Sub-table
    html_table_header(F("ON color"));
    html_table_header(F("OFF color"));
    html_table_header(F("Border color"));
    html_table_header(F("Caption color"));
    html_table_header(F("Disabled color"));
    html_table_header(F("Disabled caption color"));

    html_TR_TD(); // ON color
    parsed = AdaGFXcolorToString(P123_Settings.colorOn, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3000), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  ifdef P123_USE_TOOLTIPS
               , F("ON color")
               #  endif // ifdef P123_USE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // OFF color
    parsed = AdaGFXcolorToString(P123_Settings.colorOff, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3001), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  ifdef P123_USE_TOOLTIPS
               , F("OFF color")
               #  endif // ifdef P123_USE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // Border color
    parsed = AdaGFXcolorToString(P123_Settings.colorBorder, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3002), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  ifdef P123_USE_TOOLTIPS
               , F("Border color")
               #  endif // ifdef P123_USE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // Caption color
    parsed = AdaGFXcolorToString(P123_Settings.colorCaption, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3003), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  ifdef P123_USE_TOOLTIPS
               , F("Caption color")
               #  endif // ifdef P123_USE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // Disabled color
    parsed = AdaGFXcolorToString(P123_Settings.colorDisabled, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3004), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  ifdef P123_USE_TOOLTIPS
               , F("Disabled color")
               #  endif // ifdef P123_USE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // Disabled caption color
    parsed = AdaGFXcolorToString(P123_Settings.colorDisabledCaption, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3005), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  ifdef P123_USE_TOOLTIPS
               , F("Disabled caption color")
               #  endif // ifdef P123_USE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_end_table();
  }
  {
    addFormNumericBox(F("Initial button group"), F("tch_initial_group"),
                      get8BitFromUL(P123_Settings.flags, P123_FLAGS_INITIAL_GROUP), 0, P123_MAX_BUTTON_GROUPS
                      #  ifdef P123_USE_TOOLTIPS
                      , F("Initial group")
                      #  endif // ifdef P123_USE_TOOLTIPS
                      );
  }
  # endif // ifdef P123_USE_EXTENDED_TOUCH
  {
    addRowLabel(F("Object"));

    {
      html_table(EMPTY_STRING, false); // Sub-table
      html_table_header(F("&nbsp;#&nbsp;"));
      html_table_header(F("On"));
      html_table_header(F("Objectname"));
      html_table_header(F("Top-left x"));
      html_table_header(F("Top-left y"));
      # ifdef P123_USE_EXTENDED_TOUCH
      html_table_header(F("Button"));
      html_table_header(F("Layout"));
      html_table_header(F("ON color"));
      html_table_header(F("ON caption"));
      html_table_header(F("Border color"));
      html_table_header(F("Disab. cap. clr"));
      html_table_header(F("Touch action"));
      # else // ifdef P123_USE_EXTENDED_TOUCH
      html_table_header(F("On/Off button"));
      # endif // ifdef P123_USE_EXTENDED_TOUCH
      html_TR(); // New row
      html_table_header(EMPTY_STRING);
      html_table_header(EMPTY_STRING);
      # ifdef P123_USE_EXTENDED_TOUCH
      html_table_header(F("Button-group"));
      # else // ifdef P123_USE_EXTENDED_TOUCH
      html_table_header(EMPTY_STRING);
      # endif // ifdef P123_USE_EXTENDED_TOUCH
      html_table_header(F("Width"));
      html_table_header(F("Height"));
      html_table_header(F("Inverted"));
      # ifdef P123_USE_EXTENDED_TOUCH
      html_table_header(F("Font scale"));
      html_table_header(F("OFF color"));
      html_table_header(F("OFF caption"));
      html_table_header(F("Caption color"));
      html_table_header(F("Disabled clr"));
      html_table_header(F("Action group"));
      # endif // ifdef P123_USE_EXTENDED_TOUCH
    }
    # ifdef P123_USE_EXTENDED_TOUCH
    const __FlashStringHelper *buttonTypeOptions[] = {
      toString(Button_type_e::None),
      toString(Button_type_e::Square),
      toString(Button_type_e::Rounded),
      toString(Button_type_e::Circle),
      toString(Button_type_e::ArrowLeft),
      toString(Button_type_e::ArrowUp),
      toString(Button_type_e::ArrowRight),
      toString(Button_type_e::ArrowDown),
    };

    const int buttonTypeValues[] = {
      static_cast<int>(Button_type_e::None),
      static_cast<int>(Button_type_e::Square),
      static_cast<int>(Button_type_e::Rounded),
      static_cast<int>(Button_type_e::Circle),
      static_cast<int>(Button_type_e::ArrowLeft),
      static_cast<int>(Button_type_e::ArrowUp),
      static_cast<int>(Button_type_e::ArrowRight),
      static_cast<int>(Button_type_e::ArrowDown),
    };

    const __FlashStringHelper *buttonLayoutOptions[] = {
      toString(Button_layout_e::CenterAligned),
      toString(Button_layout_e::LeftAligned),
      toString(Button_layout_e::TopAligned),
      toString(Button_layout_e::RightAligned),
      toString(Button_layout_e::BottomAligned),
      toString(Button_layout_e::LeftTopAligned),
      toString(Button_layout_e::RightTopAligned),
      toString(Button_layout_e::LeftBottomAligned),
      toString(Button_layout_e::RightBottomAligned),
      toString(Button_layout_e::NoCaption),
      toString(Button_layout_e::Bitmap),
    };

    const int buttonLayoutValues[] = {
      static_cast<int>(Button_layout_e::CenterAligned),
      static_cast<int>(Button_layout_e::LeftAligned),
      static_cast<int>(Button_layout_e::TopAligned),
      static_cast<int>(Button_layout_e::RightAligned),
      static_cast<int>(Button_layout_e::BottomAligned),
      static_cast<int>(Button_layout_e::LeftTopAligned),
      static_cast<int>(Button_layout_e::RightTopAligned),
      static_cast<int>(Button_layout_e::LeftBottomAligned),
      static_cast<int>(Button_layout_e::RightBottomAligned),
      static_cast<int>(Button_layout_e::NoCaption),
      static_cast<int>(Button_layout_e::Bitmap),
    };

    const __FlashStringHelper *touchActionOptions[] = {
      toString(P123_touch_action_e::Default),
      toString(P123_touch_action_e::ActivateGroup),
      toString(P123_touch_action_e::IncrementGroup),
      toString(P123_touch_action_e::DecrementGroup),
    };

    const int touchActionValues[] = {
      static_cast<int>(P123_touch_action_e::Default),
      static_cast<int>(P123_touch_action_e::ActivateGroup),
      static_cast<int>(P123_touch_action_e::IncrementGroup),
      static_cast<int>(P123_touch_action_e::DecrementGroup),
    };

    # endif // ifdef P123_USE_EXTENDED_TOUCH

    uint8_t maxIdx = std::min(static_cast<int>(TouchObjects.size() + P123_EXTRA_OBJECT_COUNT), P123_MAX_OBJECT_COUNT);
    String  parsed;
    TouchObjects.resize(maxIdx, tP123_TouchObjects());

    for (int objectNr = 0; objectNr < maxIdx; objectNr++) {
      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtmlInt(objectNr + 1); // Arrayindex to objectindex

      html_TD();

      // Enable new entries
      bool enabled = bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ENABLED) || TouchObjects[objectNr].objectName.isEmpty();
      addCheckBox(getPluginCustomArgName(objectNr + 0),
                  enabled, false
                  # ifdef P123_USE_TOOLTIPS
                  , F("Enabled")
                  # endif // ifdef P123_USE_TOOLTIPS
                  );
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
      html_TD(); // (on/off) button (type)
      # ifdef P123_USE_EXTENDED_TOUCH
      addSelector(getPluginCustomArgName(objectNr + 800),
                  static_cast<int>(Button_type_e::Button_MAX),
                  buttonTypeOptions,
                  buttonTypeValues,
                  nullptr,
                  get8BitFromUL(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_BUTTONTYPE) & 0x0F, false, true, F("widenumber")
                  #  ifdef P123_USE_TOOLTIPS
                  , F("Buttontype")
                  #  endif // ifdef P123_USE_TOOLTIPS
                  );
      html_TD(); // button alignment
      addSelector(getPluginCustomArgName(objectNr + 900),
                  static_cast<int>(Button_layout_e::Alignment_MAX),
                  buttonLayoutOptions,
                  buttonLayoutValues,
                  nullptr,
                  get8BitFromUL(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_BUTTONTYPE) & 0xF0, false, true, F("widenumber")
                  #  ifdef P123_USE_TOOLTIPS
                  , F("Button alignment")
                  #  endif // ifdef P123_USE_TOOLTIPS
                  );
      # else // ifdef P123_USE_EXTENDED_TOUCH
      addCheckBox(getPluginCustomArgName(objectNr + 600),
                  bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_BUTTON), false
                  #  ifdef P123_USE_TOOLTIPS
                  , F("On/Off button")
                  #  endif // ifdef P123_USE_TOOLTIPS
                  );
      # endif // ifdef P123_USE_EXTENDED_TOUCH
      # ifdef P123_USE_EXTENDED_TOUCH
      html_TD(); // ON color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorOn, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1000), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
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
      html_TD(); // Border color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorBorder, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1700), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  ifdef P123_USE_TOOLTIPS
                 , F("Border color")
                 #  endif // ifdef P123_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // Disabled caption color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorDisabledCaption, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1900), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  ifdef P123_USE_TOOLTIPS
                 , F("Disabled caption color")
                 #  endif // ifdef P123_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // button action
      addSelector(getPluginCustomArgName(objectNr + 2000),
                  static_cast<int>(P123_touch_action_e::TouchAction_MAX),
                  touchActionOptions,
                  touchActionValues,
                  nullptr,
                  get8BitFromUL(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ACTIONGROUP) & 0xC0, false, true, F("widenumber")
                  #  ifdef P123_USE_TOOLTIPS
                  , F("Touch action")
                  #  endif // ifdef P123_USE_TOOLTIPS
                  );
      # endif // ifdef P123_USE_EXTENDED_TOUCH

      html_TR_TD(); // Start new row

      html_TD(2);   // Start with some blank columns
      # ifdef P123_USE_EXTENDED_TOUCH
      {
        #  ifdef P123_USE_TOOLTIPS
        String buttonGroupToolTip = F("Button-group [0..");
        buttonGroupToolTip += P123_MAX_BUTTON_GROUPS;
        buttonGroupToolTip += ']';
        #  endif // ifdef P123_USE_TOOLTIPS
        addNumericBox(getPluginCustomArgName(objectNr + 1600),
                      get8BitFromUL(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_GROUP), 0, P123_MAX_BUTTON_GROUPS
                      #  ifdef P123_USE_TOOLTIPS
                      , F("widenumber"), buttonGroupToolTip
                      #  endif // ifdef P123_USE_TOOLTIPS
                      );
      }
      # endif // ifdef P123_USE_EXTENDED_TOUCH
      html_TD(); // Width
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
      html_TD(); // inverted
      addCheckBox(getPluginCustomArgName(objectNr + 700),
                  bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_INVERTED), false
                  # ifdef P123_USE_TOOLTIPS
                  , F("Inverted")
                  # endif // ifdef P123_USE_TOOLTIPS
                  );
      # ifdef P123_USE_EXTENDED_TOUCH
      html_TD(); // font scale
      addNumericBox(getPluginCustomArgName(objectNr + 1200),
                    get4BitFromUL(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_FONTSCALE), 1, 10
                    #  ifdef P123_USE_TOOLTIPS
                    , F("widenumber"), F("Font scaling [1x..10x]")
                    #  endif // ifdef P123_USE_TOOLTIPS
                    );
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
      html_TD(); // Disabled color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorDisabled, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1800), parsed, P123_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  ifdef P123_USE_TOOLTIPS
                 , F("Disabled color")
                 #  endif // ifdef P123_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // Action Group
      addNumericBox(getPluginCustomArgName(objectNr + 2100),
                    get8BitFromUL(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ACTIONGROUP) & 0x3F, 0, P123_MAX_BUTTON_GROUPS
                    #  ifdef P123_USE_TOOLTIPS
                    , F("widenumber"), F("Action group")
                    #  endif // ifdef P123_USE_TOOLTIPS
                    );
      # endif // ifdef P123_USE_EXTENDED_TOUCH
    }
    html_end_table();

    addFormNumericBox(F("Debounce delay for On/Off buttons"), F("tch_debounce"),
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

  # ifdef P123_USE_EXTENDED_TOUCH
  String colorInput;
  # endif // ifdef P123_USE_EXTENDED_TOUCH
  config.reserve(80);

  uint32_t lSettings = 0u;

  bitWrite(lSettings, P123_FLAGS_SEND_XY,          bitRead(getFormItemInt(F("tch_events")), P123_FLAGS_SEND_XY));
  bitWrite(lSettings, P123_FLAGS_SEND_Z,           bitRead(getFormItemInt(F("tch_events")), P123_FLAGS_SEND_Z));
  bitWrite(lSettings, P123_FLAGS_SEND_OBJECTNAME,  bitRead(getFormItemInt(F("tch_events")), P123_FLAGS_SEND_OBJECTNAME));
  bitWrite(lSettings, P123_FLAGS_ROTATION_FLIPPED, isFormItemChecked(F("tch_rotation_flipped")));
  bitWrite(lSettings, P123_FLAGS_DEDUPLICATE,      isFormItemChecked(F("tch_deduplicate")));
  bitWrite(lSettings, P123_FLAGS_INIT_OBJECTEVENT, isFormItemChecked(F("tch_init_objectevent")));
  # ifdef P123_USE_EXTENDED_TOUCH
  set8BitToUL(lSettings, P123_FLAGS_INITIAL_GROUP, getFormItemInt(F("tch_initial_group"))); // Button group
  # endif // ifdef P123_USE_EXTENDED_TOUCH

  config += getFormItemInt(F("tch_use_calibration"));
  config += P123_SETTINGS_SEPARATOR;
  config += isFormItemChecked(F("tch_log_calibration")) ? 1 : 0;
  config += P123_SETTINGS_SEPARATOR;
  config += getFormItemInt(F("tch_cal_tl_x"));
  config += P123_SETTINGS_SEPARATOR;
  config += getFormItemInt(F("tch_cal_tl_y"));
  config += P123_SETTINGS_SEPARATOR;
  config += getFormItemInt(F("tch_cal_br_x"));
  config += P123_SETTINGS_SEPARATOR;
  config += getFormItemInt(F("tch_cal_br_y"));
  config += P123_SETTINGS_SEPARATOR;
  config += getFormItemInt(F("tch_debounce"));
  config += P123_SETTINGS_SEPARATOR;
  config += getFormItemInt(F("tch_treshold"));
  config += P123_SETTINGS_SEPARATOR;
  config += ull2String(lSettings);
  config += P123_SETTINGS_SEPARATOR;
  # ifdef P123_USE_EXTENDED_TOUCH
  colorInput = webArg(getPluginCustomArgName(3000)); // Default Color ON
  config    += AdaGFXparseColor(colorInput, _colorDepth);
  config    += P123_SETTINGS_SEPARATOR;
  colorInput = webArg(getPluginCustomArgName(3001)); // Default Color OFF
  config    += AdaGFXparseColor(colorInput, _colorDepth, false);
  config    += P123_SETTINGS_SEPARATOR;
  colorInput = webArg(getPluginCustomArgName(3002)); // Default Color Border
  config    += AdaGFXparseColor(colorInput, _colorDepth, false);
  config    += P123_SETTINGS_SEPARATOR;
  colorInput = webArg(getPluginCustomArgName(3003)); // Default Color caption
  config    += AdaGFXparseColor(colorInput, _colorDepth, false);
  config    += P123_SETTINGS_SEPARATOR;
  colorInput = webArg(getPluginCustomArgName(3004)); // Default Disabled Color
  config    += AdaGFXparseColor(colorInput, _colorDepth);
  config    += P123_SETTINGS_SEPARATOR;
  colorInput = webArg(getPluginCustomArgName(3005)); // Default Disabled Caption Color
  config    += AdaGFXparseColor(colorInput, _colorDepth, false);
  config    += P123_SETTINGS_SEPARATOR;
  # endif // ifdef P123_USE_EXTENDED_TOUCH

  settingsArray[P123_CALIBRATION_START] = config;

  # ifdef PLUGIN_123_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("Save settings: ");
    config.replace(P123_SETTINGS_SEPARATOR, ',');
    log += config;
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // ifdef PLUGIN_123_DEBUG

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
      bitWrite(flags, P123_OBJECT_FLAG_INVERTED, isFormItemChecked(getPluginCustomArgName(objectNr + 700))); // Inverted
      # ifdef P123_USE_EXTENDED_TOUCH
      uint8_t buttonType   = getFormItemInt(getPluginCustomArgName(objectNr + 800));
      uint8_t buttonLayout = getFormItemInt(getPluginCustomArgName(objectNr + 900));
      set8BitToUL(flags, P123_OBJECT_FLAG_BUTTONTYPE, buttonType | buttonLayout);                                       // Buttontype
      bitWrite(flags, P123_OBJECT_FLAG_BUTTON, (static_cast<Button_type_e>(buttonType & 0x07) != Button_type_e::None)); // On/Off button
      uint8_t buttonAction      = getFormItemInt(getPluginCustomArgName(objectNr + 2000));
      uint8_t buttonSelectGroup = getFormItemInt(getPluginCustomArgName(objectNr + 2100));
      set8BitToUL(flags, P123_OBJECT_FLAG_ACTIONGROUP, buttonAction | buttonSelectGroup);                               // ButtonAction
      uint8_t fontScale = getFormItemInt(getPluginCustomArgName(objectNr + 1200));
      set4BitToUL(flags, P123_OBJECT_FLAG_FONTSCALE, fontScale);                                                        // Font scaling
      uint8_t buttonGroup = getFormItemInt(getPluginCustomArgName(objectNr + 1600));
      set8BitToUL(flags, P123_OBJECT_FLAG_GROUP, buttonGroup);                                                          // Button group
      # else // ifdef P123_USE_EXTENDED_TOUCH
      bitWrite(flags, P123_OBJECT_FLAG_BUTTON, isFormItemChecked(getPluginCustomArgName(objectNr + 600)));              // On/Off button
      # endif // ifdef P123_USE_EXTENDED_TOUCH

      config += ull2String(flags);                                                                                      // Flags
      config += P123_SETTINGS_SEPARATOR;
      config += getFormItemInt(getPluginCustomArgName(objectNr + 200));                                                 // Top x
      config += P123_SETTINGS_SEPARATOR;
      config += getFormItemInt(getPluginCustomArgName(objectNr + 300));                                                 // Top y
      config += P123_SETTINGS_SEPARATOR;
      config += getFormItemInt(getPluginCustomArgName(objectNr + 400));                                                 // Bottom x
      config += P123_SETTINGS_SEPARATOR;
      config += getFormItemInt(getPluginCustomArgName(objectNr + 500));                                                 // Bottom y
      config += P123_SETTINGS_SEPARATOR;

      # ifdef P123_USE_EXTENDED_TOUCH
      colorInput = webArg(getPluginCustomArgName(objectNr + 1000)); // Color ON
      config    += AdaGFXparseColor(colorInput, _colorDepth, true);
      config    += P123_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1100)); // Color OFF
      config    += AdaGFXparseColor(colorInput, _colorDepth, true);
      config    += P123_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1500)); // Color caption
      config    += AdaGFXparseColor(colorInput, _colorDepth, true);
      config    += P123_SETTINGS_SEPARATOR;                         // Caption ON
      config    += wrapWithQuotesIfContainsParameterSeparatorChar(webArg(getPluginCustomArgName(objectNr + 1300)));
      config    += P123_SETTINGS_SEPARATOR;                         // Caption OFF
      config    += wrapWithQuotesIfContainsParameterSeparatorChar(webArg(getPluginCustomArgName(objectNr + 1400)));
      config    += P123_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1700)); // Color Border
      config    += AdaGFXparseColor(colorInput, _colorDepth, true);
      config    += P123_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1800)); // Disabled Color
      config    += AdaGFXparseColor(colorInput, _colorDepth, true);
      config    += P123_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1900)); // Disabled Caption Color
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

    if (loglevelActiveFor(LOG_LEVEL_INFO) &&
        !config.isEmpty()) {
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
bool P123_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
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
        log  = F("Touch calibration rx= ");    // Space before the logged values added for readability
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

      // No events to handle if rules not enabled
      if (Settings.UseRules) {
        if (success && bitRead(P123_Settings.flags, P123_FLAGS_SEND_XY)) { // Send events for each touch
          const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

          // Do NOT send a Z event for each touch?
          if (!bitRead(P123_Settings.flags, P123_FLAGS_SEND_Z) && validDeviceIndex(DeviceIndex)) {
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
              // Not touched yet or too long ago
              if ((TouchObjects[selectedObjectIndex].TouchTimers == 0) ||
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
                                           const int8_t       onOffState,
                                           const bool         groupSwitch,
                                           const int8_t       factor) {
  if ((objectIndex < 0) || // Range check
      (objectIndex >= static_cast<int8_t>(TouchObjects.size()))) {
    return;
  }
  String eventCommand;

  eventCommand.reserve(48);
  eventCommand  = getTaskDeviceName(event->TaskIndex);
  eventCommand += '#';
  eventCommand += TouchObjects[objectIndex].objectName;
  eventCommand += '=';                             // Add arguments

  if (onOffState < 0) {                            // Negative value: pass on unaltered (1)
    eventCommand += onOffState;                    // (%eventvalue#%)
  } else {                                         // Check for inverted output (1)
    if (bitRead(TouchObjects[objectIndex].flags, P123_OBJECT_FLAG_INVERTED)) {
      eventCommand += onOffState == 1 ? '0' : '1'; // Act like an inverted button, 0 = On, 1 = Off
    } else {
      eventCommand += onOffState == 1 ? '1' : '0'; // Act like a button, 1 = On, 0 = Off
    }
  }

  if (P123_CONFIG_DISPLAY_TASK != event->TaskIndex) {         // Add arguments for display
    eventCommand += ',';
    eventCommand += TouchObjects[objectIndex].top_left.x;     // (2)
    eventCommand += ',';
    eventCommand += TouchObjects[objectIndex].top_left.y;     // (3)
    eventCommand += ',';
    eventCommand += TouchObjects[objectIndex].width_height.x; // (4)
    eventCommand += ',';
    eventCommand += TouchObjects[objectIndex].width_height.y; // (5)
    eventCommand += ',';
    eventCommand += objectIndex + 1;                          // Adjust to displayed index (6)
    eventCommand += ',';                                      // (7)
    eventCommand += get8BitFromUL(TouchObjects[objectIndex].flags, P123_OBJECT_FLAG_BUTTONTYPE) * factor;
    # ifdef P123_USE_EXTENDED_TOUCH
    eventCommand += ',';                                      // (8)
    eventCommand += AdaGFXcolorToString(TouchObjects[objectIndex].colorOn == 0
                                        ? P123_Settings.colorOn
                                        : TouchObjects[objectIndex].colorOn,
                                        _colorDepth);
    eventCommand += ','; // (9)
    eventCommand += AdaGFXcolorToString(TouchObjects[objectIndex].colorOff == 0
                                        ? P123_Settings.colorOff
                                        : TouchObjects[objectIndex].colorOff,
                                        _colorDepth);
    eventCommand += ','; // (10)
    eventCommand += AdaGFXcolorToString(TouchObjects[objectIndex].colorCaption == 0
                                        ? P123_Settings.colorCaption
                                        : TouchObjects[objectIndex].colorCaption,
                                        _colorDepth);
    eventCommand += ','; // (11)
    eventCommand += get4BitFromUL(TouchObjects[objectIndex].flags, P123_OBJECT_FLAG_FONTSCALE);
    eventCommand += ','; // (12)
    eventCommand += wrapWithQuotesIfContainsParameterSeparatorChar(TouchObjects[objectIndex].captionOn.isEmpty() ?
                                                                   TouchObjects[objectIndex].objectName :
                                                                   TouchObjects[objectIndex].captionOn);
    eventCommand += ','; // (13)
    eventCommand += wrapWithQuotesIfContainsParameterSeparatorChar(TouchObjects[objectIndex].captionOff.isEmpty() ?
                                                                   TouchObjects[objectIndex].objectName :
                                                                   TouchObjects[objectIndex].captionOff);
    eventCommand += ','; // (14)
    eventCommand += AdaGFXcolorToString(TouchObjects[objectIndex].colorBorder == 0
                                        ? P123_Settings.colorBorder
                                        : TouchObjects[objectIndex].colorBorder,
                                        _colorDepth);
    eventCommand += ','; // (15)
    eventCommand += AdaGFXcolorToString(TouchObjects[objectIndex].colorDisabled == 0
                                        ? P123_Settings.colorDisabled
                                        : TouchObjects[objectIndex].colorDisabled,
                                        _colorDepth);
    eventCommand += ','; // (16)
    eventCommand += AdaGFXcolorToString(TouchObjects[objectIndex].colorDisabledCaption == 0
                                        ? P123_Settings.colorDisabledCaption
                                        : TouchObjects[objectIndex].colorDisabledCaption,
                                        _colorDepth);
    # endif // ifdef P123_USE_EXTENDED_TOUCH
    eventCommand += ',';
    eventCommand += P123_CONFIG_DISPLAY_TASK + 1; // What TaskIndex? (17) or (8)
    eventCommand += ',';                          // Group (18) or (9)
    eventCommand += get8BitFromUL(TouchObjects[objectIndex].flags, P123_OBJECT_FLAG_GROUP);
    eventCommand += ',';                          // Select Group (19) or (10)
    uint8_t action = get8BitFromUL(TouchObjects[objectIndex].flags, P123_OBJECT_FLAG_ACTIONGROUP);

    if (!groupSwitch && (static_cast<P123_touch_action_e>(action & 0xC0) != P123_touch_action_e::Default)) {
      switch (static_cast<P123_touch_action_e>(action & 0xC0)) {
        case P123_touch_action_e::ActivateGroup:
          eventCommand += action & 0x3F;
          break;
        case P123_touch_action_e::IncrementGroup:
          eventCommand += -2;
          break;
        case P123_touch_action_e::DecrementGroup:
          eventCommand += -3;
          break;
        case P123_touch_action_e::Default:
        case P123_touch_action_e::TouchAction_MAX:
          eventCommand += -1; // Ignore
          break;
      }
    } else {
      eventCommand += -1; // No group to activate
    }
  }

  eventQueue.addMove(std::move(eventCommand));

  delay(0);
}

/**
 * Load the touch objects from the settings, and initialize then properly where needed.
 */
void P123_data_struct::loadTouchObjects(const EventStruct *event) {
  # ifdef PLUGIN_123_DEBUG
  addLogMove(LOG_LEVEL_INFO, F("P123 DEBUG loadTouchObjects"));
  # endif // PLUGIN_123_DEBUG
  LoadCustomTaskSettings(event->TaskIndex, settingsArray, P123_ARRAY_SIZE, 0);

  lastObjectIndex = P123_OBJECT_INDEX_START - 1; // START must be > 0!!!

  objectCount     = 0;
  _minButtonGroup = 0;
  _maxButtonGroup = 0;

  for (uint8_t i = P123_OBJECT_INDEX_END; i >= P123_OBJECT_INDEX_START; i--) {
    if (!settingsArray[i].isEmpty() && (lastObjectIndex < P123_OBJECT_INDEX_START)) {
      lastObjectIndex = i;
      objectCount++; // Count actual number of objects
    }
  }

  // Get calibration and common settings
  P123_Settings.calibrationEnabled = parseStringToInt(settingsArray[P123_CALIBRATION_START],
                                                      P123_CALIBRATION_ENABLED, P123_SETTINGS_SEPARATOR) == 1;
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
  # ifdef P123_USE_EXTENDED_TOUCH
  P123_Settings.colorOn = parseStringToInt(settingsArray[P123_CALIBRATION_START],
                                           P123_COMMON_DEF_COLOR_ON, P123_SETTINGS_SEPARATOR);
  P123_Settings.colorOff = parseStringToInt(settingsArray[P123_CALIBRATION_START],
                                            P123_COMMON_DEF_COLOR_OFF, P123_SETTINGS_SEPARATOR);
  P123_Settings.colorBorder = parseStringToInt(settingsArray[P123_CALIBRATION_START],
                                               P123_COMMON_DEF_COLOR_BORDER, P123_SETTINGS_SEPARATOR);
  P123_Settings.colorCaption = parseStringToInt(settingsArray[P123_CALIBRATION_START],
                                                P123_COMMON_DEF_COLOR_CAPTION, P123_SETTINGS_SEPARATOR);
  P123_Settings.colorDisabled = parseStringToInt(settingsArray[P123_CALIBRATION_START],
                                                 P123_COMMON_DEF_COLOR_DISABLED, P123_SETTINGS_SEPARATOR);
  P123_Settings.colorDisabledCaption = parseStringToInt(settingsArray[P123_CALIBRATION_START],
                                                        P123_COMMON_DEF_COLOR_DISABCAPT, P123_SETTINGS_SEPARATOR);

  if ((P123_Settings.colorOn              == 0u) &&
      (P123_Settings.colorOff             == 0u) &&
      (P123_Settings.colorCaption         == 0u) &&
      (P123_Settings.colorBorder          == 0u) &&
      (P123_Settings.colorDisabled        == 0u) &&
      (P123_Settings.colorDisabledCaption == 0u)) {
    P123_Settings.colorOn              = ADAGFX_GREEN;
    P123_Settings.colorOff             = ADAGFX_RED;
    P123_Settings.colorCaption         = ADAGFX_WHITE;
    P123_Settings.colorBorder          = ADAGFX_WHITE;
    P123_Settings.colorDisabled        = P123_DEFAULT_COLOR_DISABLED;
    P123_Settings.colorDisabledCaption = P123_DEFAULT_COLOR_DISABLED_CAPTION;
  }
  # endif // ifdef P123_USE_EXTENDED_TOUCH

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
        TouchObjects[t].colorOn              = parseStringToInt(settingsArray[i], P123_OBJECT_COLOR_ON, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].colorOff             = parseStringToInt(settingsArray[i], P123_OBJECT_COLOR_OFF, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].colorCaption         = parseStringToInt(settingsArray[i], P123_OBJECT_COLOR_CAPTION, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].captionOn            = parseStringKeepCase(settingsArray[i], P123_OBJECT_CAPTION_ON, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].captionOff           = parseStringKeepCase(settingsArray[i], P123_OBJECT_CAPTION_OFF, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].colorBorder          = parseStringToInt(settingsArray[i], P123_OBJECT_COLOR_BORDER, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].colorDisabled        = parseStringToInt(settingsArray[i], P123_OBJECT_COLOR_DISABLED, P123_SETTINGS_SEPARATOR);
        TouchObjects[t].colorDisabledCaption = parseStringToInt(settingsArray[i], P123_OBJECT_COLOR_DISABCAPT, P123_SETTINGS_SEPARATOR);

        if (get8BitFromUL(TouchObjects[t].flags, P123_OBJECT_FLAG_GROUP) > _maxButtonGroup) {
          _maxButtonGroup = get8BitFromUL(TouchObjects[t].flags, P123_OBJECT_FLAG_GROUP);
        }
        # endif // ifdef P123_USE_EXTENDED_TOUCH

        TouchObjects[t].SurfaceAreas = 0; // Reset runtime stuff
        TouchObjects[t].TouchTimers  = 0;
        TouchObjects[t].TouchStates  = false;

        t++;

        settingsArray[i].clear(); // Free a little memory
      }
    }
  }

  if (_maxButtonGroup > 0) {
    _minButtonGroup = 1;
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

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("P123 DEBUG Rotation set: ");
    log += n;
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // PLUGIN_123_DEBUG
}

/**
 * Set rotationFlipped
 */
void P123_data_struct::setRotationFlipped(bool flipped) {
  _flipped = flipped;
  # ifdef PLUGIN_123_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("P123 DEBUG RotationFlipped set: ");
    log += flipped;
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // PLUGIN_123_DEBUG
}

/**
 * Determine if calibration is enabled and usable.
 */
bool P123_data_struct::isCalibrationActive() {
  return _useCalibration
         && (P123_Settings.top_left.x != 0 ||
             P123_Settings.top_left.y != 0 ||
             P123_Settings.bottom_right.x != 0 ||
             P123_Settings.bottom_right.y != 0); // Enabled and any value != 0 => Active
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
    uint8_t group = get8BitFromUL(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_GROUP);

    if (!TouchObjects[objectNr].objectName.isEmpty()
        && bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ENABLED)
        && (TouchObjects[objectNr].width_height.x != 0)
        && (TouchObjects[objectNr].width_height.y != 0) // Not initial could be valid
        && ((group == 0) || (group == _buttonGroup))) { // Group 0 is always active
      if (TouchObjects[objectNr].SurfaceAreas == 0) {   // Need to calculate the surface area
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
 * Set the enabled/disabled state of an object.
 */
bool P123_data_struct::setTouchObjectState(struct EventStruct *event, const String& touchObject, bool state) {
  if (touchObject.isEmpty()) { return false; }
  bool success = false;

  for (size_t objectNr = 0; objectNr < TouchObjects.size(); objectNr++) {
    if (!TouchObjects[objectNr].objectName.isEmpty()
        && touchObject.equalsIgnoreCase(TouchObjects[objectNr].objectName)) {
      bool currentState = bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ENABLED);

      if (state != currentState) {
        bitWrite(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ENABLED, state);
        success = true;

        if (bitRead(P123_Settings.flags, P123_FLAGS_SEND_OBJECTNAME) &&
            bitRead(P123_Settings.flags, P123_FLAGS_INIT_OBJECTEVENT)) {
          generateObjectEvent(event, objectNr, state ? (TouchObjects[objectNr].TouchStates ? 1 : -1) : -2);
        }
      }
      # ifdef PLUGIN_123_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("P123 setTouchObjectState: obj: ");
        log += touchObject;

        if (success) {
          log += F(", new state: ");
          log += (state ? F("en") : F("dis"));
          log += F("abled.");
        } else {
          log += F("failed!");
        }
        addLogMove(LOG_LEVEL_INFO, log);
      }
      # endif // PLUGIN_123_DEBUG
    }
  }
  return success;
}

/**
 * Set the on/off state of a touch-button object.
 */
bool P123_data_struct::setTouchButtonOnOff(struct EventStruct *event, const String& touchObject, bool state) {
  if (touchObject.isEmpty()) { return false; }
  bool success = false;

  for (size_t objectNr = 0; objectNr < TouchObjects.size(); objectNr++) {
    if (!TouchObjects[objectNr].objectName.isEmpty()
        && touchObject.equalsIgnoreCase(TouchObjects[objectNr].objectName)
        && bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_ENABLED)
        && bitRead(TouchObjects[objectNr].flags, P123_OBJECT_FLAG_BUTTON)) {
      bool currentState = TouchObjects[objectNr].TouchStates;

      success = true; // Always success if matched button

      if (state != currentState) {
        TouchObjects[objectNr].TouchStates = state;

        if (bitRead(P123_Settings.flags, P123_FLAGS_SEND_OBJECTNAME) &&
            bitRead(P123_Settings.flags, P123_FLAGS_INIT_OBJECTEVENT)) {
          generateObjectEvent(event, objectNr, state ? 1 : 0);
        }
      }
      # ifdef PLUGIN_123_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("P123 setTouchButtonOnOff: obj: ");
        log += touchObject;
        log += F(", (new) state: ");
        log += (state ? F("on") : F("off"));
        addLogMove(LOG_LEVEL_INFO, log);
      }
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

/**
 * Set the desired button group, must be between the minimum and maximum found values
 */
bool P123_data_struct::setButtonGroup(const EventStruct *event,
                                      int8_t             buttonGroup) {
  if ((buttonGroup >= 0) && (buttonGroup <= _maxButtonGroup)) {
    if (buttonGroup != _buttonGroup) {
      displayButtonGroup(event, _buttonGroup, -2);
      _buttonGroup = buttonGroup;
      displayButtonGroup(event, _buttonGroup, -1);
    }
    return true;
  }
  return false;
}

/**
 * Increment button group, if max. group > 0 then min. group = 1
 */
bool P123_data_struct::incrementButtonGroup(const EventStruct *event) {
  if (_buttonGroup < _maxButtonGroup) {
    displayButtonGroup(event, _buttonGroup, -2);
    _buttonGroup++;
    displayButtonGroup(event, _buttonGroup, -1);
    return true;
  }
  return false;
}

/**
 * Decrement button group, if max. group > 0 then min. group = 1
 */
bool P123_data_struct::decrementButtonGroup(const EventStruct *event) {
  if (_buttonGroup > _minButtonGroup) {
    displayButtonGroup(event, _buttonGroup, -2);
    _buttonGroup--;
    displayButtonGroup(event, _buttonGroup, -1);
    return true;
  }
  return false;
}

#endif // ifdef USES_P123
