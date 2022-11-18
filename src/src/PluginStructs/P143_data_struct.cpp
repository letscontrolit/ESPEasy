#include "../PluginStructs/P143_data_struct.h"

#ifdef USES_P143

/**************************************************************************
 * toString for P143_DeviceType_e
 *************************************************************************/
const __FlashStringHelper* toString(P143_DeviceType_e device) {
  switch (device) {
    case P143_DeviceType_e::AdafruitEncoder: return F("Adafruit");
    # if P143_FEATURE_INCLUDE_M5STACK
    case P143_DeviceType_e::M5StackEncoder: return F("M5Stack");
    # endif // if P143_FEATURE_INCLUDE_M5STACK
    # if P143_FEATURE_INCLUDE_DFROBOT
    case P143_DeviceType_e::DFRobotEncoder: return F("DFRobot");
    # endif // if P143_FEATURE_INCLUDE_DFROBOT
  }
  return F("");
}

# if P143_FEATURE_COUNTER_COLORMAPPING

/**************************************************************************
 * toString for P143_CounterMapping_e
 *************************************************************************/
const __FlashStringHelper* toString(P143_CounterMapping_e counter) {
  switch (counter) {
    case P143_CounterMapping_e::None: return F("None");
    case P143_CounterMapping_e::ColorMapping: return F("Color mapping");
    case P143_CounterMapping_e::ColorGradient: return F("Color gradient");
  }
  return F("");
}

# endif // if P143_FEATURE_COUNTER_COLORMAPPING

/**************************************************************************
 * toString for P143_ButtonAction_e
 *************************************************************************/
const __FlashStringHelper* toString(P143_ButtonAction_e action) {
  switch (action) {
    case P143_ButtonAction_e::PushButton: return F("Pushbutton");
    case P143_ButtonAction_e::PushButtonInverted: return F("Pushbutton (inverted)");
    case P143_ButtonAction_e::ToggleSwitch: return F("Toggle switch");
  }
  return F("");
}

/*******************************************************************
 * P143_CheckEncoderDefaultSettings: Helper to set config defaults after changing the Encoder type
 ******************************************************************/
void P143_CheckEncoderDefaultSettings(struct EventStruct *event) {
  if (P143_ENCODER_TYPE != P143_PREVIOUS_TYPE) {
    switch (static_cast<P143_DeviceType_e>(P143_ENCODER_TYPE)) {
      case P143_DeviceType_e::AdafruitEncoder:
        P143_ADAFRUIT_COLOR_AND_BRIGHTNESS = 0x0000001E; // Black, with 30 (0x1E) brightness (1..255)
        P143_OFFSET_POSITION               = 0;
        break;
      # if P143_FEATURE_INCLUDE_M5STACK
      case P143_DeviceType_e::M5StackEncoder:
        P143_ADAFRUIT_COLOR_AND_BRIGHTNESS = 0x0000001E; // Black, with 30 (0x1E) brightness (1..255)
        P143_M5STACK_COLOR_AND_SELECTION   = 0x00000000; // Black, with both Leds using Color mapping
        P143_OFFSET_POSITION               = 0;
        break;
      # endif // if P143_FEATURE_INCLUDE_M5STACK
      # if P143_FEATURE_INCLUDE_DFROBOT
      case P143_DeviceType_e::DFRobotEncoder:
        P143_DFROBOT_LED_GAIN = P143_DFROBOT_MAX_GAIN;
        P143_OFFSET_POSITION  = 0;
        break;
      # endif // if P143_FEATURE_INCLUDE_DFROBOT
    }
    P143_PREVIOUS_TYPE = P143_ENCODER_TYPE; // It's now up to date
  }
}

/**************************************************************************
 * Constructor
 *************************************************************************/
P143_data_struct::P143_data_struct(struct EventStruct *event) {
  _device          = static_cast<P143_DeviceType_e>(P143_ENCODER_TYPE);
  _i2cAddress      = P143_I2C_ADDR;
  _encoderPosition = P143_INITIAL_POSITION;
  _encoderMin      = P143_MINIMAL_POSITION;
  _encoderMax      = P143_MAXIMAL_POSITION;
  _brightness      = P143_NEOPIXEL_BRIGHTNESS;
  # if P143_FEATURE_INCLUDE_DFROBOT
  _initialOffset = P143_OFFSET_POSITION;
  # endif // if P143_FEATURE_INCLUDE_DFROBOT
}

/*****************************************************
 * Destructor
 ****************************************************/
P143_data_struct::~P143_data_struct() {
  switch (_device)
  {
    case P143_DeviceType_e::AdafruitEncoder:

      delete Adafruit_Seesaw;
      delete Adafruit_Spixel;
      break;
    # if P143_FEATURE_INCLUDE_M5STACK
    case P143_DeviceType_e::M5StackEncoder:
      break;
    # endif // if P143_FEATURE_INCLUDE_M5STACK
    # if P143_FEATURE_INCLUDE_DFROBOT
    case P143_DeviceType_e::DFRobotEncoder:
      break;
    # endif // if P143_FEATURE_INCLUDE_DFROBOT
  }
}

/**************************************************************************
 * plugin_init Initialize sensor and prepare for reading
 *************************************************************************/
bool P143_data_struct::plugin_init(struct EventStruct *event) {
  if (!_initialized) {
    switch (_device) {
      case P143_DeviceType_e::AdafruitEncoder:
      {
        Adafruit_Seesaw = new (std::nothrow) Adafruit_seesaw();
        Adafruit_Spixel = new (std::nothrow) seesaw_NeoPixel(1, P143_SEESAW_NEOPIX, NEO_GRB + NEO_KHZ800);

        if ((nullptr != Adafruit_Seesaw) && (nullptr != Adafruit_Spixel)) {
          _initialized = Adafruit_Seesaw->begin(_i2cAddress) && Adafruit_Spixel->begin(_i2cAddress);
          uint32_t version = ((Adafruit_Seesaw->getVersion() >> 16) & 0xFFFF);

          if (_initialized && (version != P143_ADAFRUIT_ENCODER_PRODUCTID)) { // Check Adafruit product ID
            _initialized = false;
          }

          if (_initialized) {
            // use a pin for the built in encoder switch
            Adafruit_Seesaw->pinMode(P143_SEESAW_SWITCH, INPUT_PULLUP);

            // set starting position
            Adafruit_Seesaw->setEncoderPosition(_encoderPosition);

            // Enable interrupts on Switch pin
            Adafruit_Seesaw->setGPIOInterrupts((uint32_t)1 << P143_SEESAW_SWITCH, 1);
            Adafruit_Seesaw->enableEncoderInterrupt();

            // We only have 1 pixel available...
            Adafruit_Spixel->setBrightness(_brightness); // Set brightness before color!
            _red   = P143_ADAFRUIT_COLOR_RED;
            _green = P143_ADAFRUIT_COLOR_GREEN;
            _blue  = P143_ADAFRUIT_COLOR_BLUE;
            Adafruit_Spixel->setPixelColor(0, _red, _green, _blue);
            Adafruit_Spixel->show();
          }
        }
        break;
      }
      # if P143_FEATURE_INCLUDE_M5STACK
      case P143_DeviceType_e::M5StackEncoder:
      {
        // Reset, only actually supported with upgraded firmware
        I2C_write8_reg(_i2cAddress, P143_M5STACK_REG_MODE, 0x00);

        // Check if we need to use the offset method
        // - Read current counter
        // - Write incremented value, only works with upgraded firmware
        // - re-read and if not changed we use an offset to handle passing the set limits
        int16_t encoderCount = I2C_readS16_LE_reg(_i2cAddress, P143_M5STACK_REG_ENCODER);
        encoderCount++;
        I2C_write16_reg(_i2cAddress, P143_M5STACK_REG_ENCODER, encoderCount << 8 || encoderCount >> 8);

        if (encoderCount != I2C_readS16_LE_reg(_i2cAddress, P143_M5STACK_REG_ENCODER)) {
          _useOffset       = true;
          _previousEncoder = encoderCount - 1;
          _offsetEncoder   = _previousEncoder - _encoderPosition;
        } else {
          // Don't need to use offset, set configured initial value
          encoderCount = _encoderPosition;
          I2C_write16_reg(_i2cAddress, P143_M5STACK_REG_ENCODER, encoderCount << 8 || encoderCount >> 8);
        }

        _red   = P143_ADAFRUIT_COLOR_RED; // Also used for M5Stack Led 1
        _green = P143_ADAFRUIT_COLOR_GREEN;
        _blue  = P143_ADAFRUIT_COLOR_BLUE;

        // Set LED initial state
        uint8_t data[4];
        data[0] = 1; // Led 1
        data[1] = applyBrightness(_red);
        data[2] = applyBrightness(_green);
        data[3] = applyBrightness(_blue);
        I2C_writeBytes_reg(_i2cAddress, P143_M5STACK_REG_LED, data, 4);
        data[0] = 2; // Led 2
        data[1] = applyBrightness(P143_M5STACK2_COLOR_RED);
        data[2] = applyBrightness(P143_M5STACK2_COLOR_GREEN);
        data[3] = applyBrightness(P143_M5STACK2_COLOR_BLUE);
        I2C_writeBytes_reg(_i2cAddress, P143_M5STACK_REG_LED, data, 4);
        _initialized = true;
        break;
      }
      # endif // if P143_FEATURE_INCLUDE_M5STACK
      # if P143_FEATURE_INCLUDE_DFROBOT
      case P143_DeviceType_e::DFRobotEncoder:
      {
        _initialized =  P143_DFROBOT_ENCODER_PID == I2C_read16_reg(_i2cAddress, P143_DFROBOT_ENCODER_PID_MSB_REG);

        if (_initialized) {
          // Set encoder position
          I2C_write16_reg(_i2cAddress, P143_DFROBOT_ENCODER_COUNT_MSB_REG, _initialOffset + _encoderPosition);

          // Set led gain
          I2C_write8_reg(_i2cAddress, P143_DFROBOT_ENCODER_GAIN_REG, P143_DFROBOT_LED_GAIN & 0xFF);
        }
        break;
      }
      # endif // if P143_FEATURE_INCLUDE_DFROBOT
    }

    // Set initial button state
    _buttonState = (P143_ButtonAction_e::PushButtonInverted == static_cast<P143_ButtonAction_e>(P143_PLUGIN_BUTTON_ACTION)) ? 0 : 1;

    UserVar[event->BaseVarIndex + 1] = _buttonState;

    # if P143_FEATURE_COUNTER_COLORMAPPING

    _mapping = static_cast<P143_CounterMapping_e>(P143_PLUGIN_COUNTER_MAPPING);

    // Load color mapping data
    LoadCustomTaskSettings(event->TaskIndex, _colorMapping, P143_STRINGS, 0);

    for (int i = P143_STRINGS - 1; i >= 0; i--) {
      _colorMapping[i].trim();

      if ((_colorMaps == -1) && !_colorMapping[i].isEmpty()) {
        _colorMaps = i;
      }
    }

    counterToColorMapping(event); // Update color
    # endif // if P143_FEATURE_COUNTER_COLORMAPPING

    if (loglevelActiveFor(_initialized ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR)) {
      String log = F("I2CEncoders: INIT ");
      log += toString(_device);
      log += F(", ");

      # if P143_FEATURE_INCLUDE_M5STACK

      if (_useOffset) {
        log += F("using Offset method, ");
      }
      # endif // if P143_FEATURE_INCLUDE_M5STACK

      if (!_initialized) {
        log += F("FAILED.");
      } else {
        log += F("Success.");
      }
      addLogMove(_initialized ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR, log);
    }
  }

  return _initialized;
}

/**************************************************************************
 * plugin_exit De-initialize and prepare for destructor
 *************************************************************************/
bool P143_data_struct::plugin_exit(struct EventStruct *event) {
  if (_initialized) {
    switch (_device) {
      case P143_DeviceType_e::AdafruitEncoder:
      {
        // Stop interrupthandler
        Adafruit_Seesaw->disableEncoderInterrupt();

        // Turn off Neopixel 0 by setting the R/G/B color to black
        Adafruit_Spixel->setPixelColor(0, 0, 0, 0);
        Adafruit_Spixel->show();
        break;
      }
      # if P143_FEATURE_INCLUDE_M5STACK
      case P143_DeviceType_e::M5StackEncoder:
      {
        // Turn off both LEDs
        uint8_t data[4] = { 0 };

        I2C_writeBytes_reg(_i2cAddress, P143_M5STACK_REG_LED, data, 4);
        break;
      }
      # endif // if P143_FEATURE_INCLUDE_M5STACK
      # if P143_FEATURE_INCLUDE_DFROBOT
      case P143_DeviceType_e::DFRobotEncoder:
        break;
      # endif // if P143_FEATURE_INCLUDE_DFROBOT
    }
  }
  return true;
}

/*****************************************************
 * plugin_read
 ****************************************************/
bool P143_data_struct::plugin_read(struct EventStruct *event)           {
  if (_initialized) {
    // Last obtained values
    UserVar[event->BaseVarIndex]     = _encoderPosition;
    UserVar[event->BaseVarIndex + 1] = _buttonState;
    return true;
  }
  return false;
}

/*****************************************************
 * plugin_ten_per_second
 ****************************************************/
bool P143_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  bool result = false;

  if (_initialized) {
    int32_t current = _encoderPosition;
    # if PLUGIN_143_DEBUG
    _oldPosition = _encoderPosition;
    # endif // if PLUGIN_143_DEBUG

    // Read encoder
    switch (_device) {
      case P143_DeviceType_e::AdafruitEncoder:
        current = Adafruit_Seesaw->getEncoderPosition();
        break;
      # if P143_FEATURE_INCLUDE_M5STACK
      case P143_DeviceType_e::M5StackEncoder:
        current = I2C_readS16_LE_reg(_i2cAddress, P143_M5STACK_REG_ENCODER);
        break;
      # endif // if P143_FEATURE_INCLUDE_M5STACK
      # if P143_FEATURE_INCLUDE_DFROBOT
      case P143_DeviceType_e::DFRobotEncoder:
        current = static_cast<int16_t>(I2C_read16_reg(_i2cAddress, P143_DFROBOT_ENCODER_COUNT_MSB_REG)) - _initialOffset;
        break;
      # endif // if P143_FEATURE_INCLUDE_DFROBOT
    }
    # if P143_FEATURE_INCLUDE_M5STACK
    const int32_t rawCurrent = current;

    if (_useOffset) {
      current -= _offsetEncoder;
    }
    # endif // if P143_FEATURE_INCLUDE_M5STACK
    const int32_t orgCurrent = current;

    // Check limits
    if (_encoderMin != _encoderMax) {
      if ((current < _encoderMin) || (current > _encoderMax)) {
        // Have to check separately, as it's possible to move multiple steps within 1/10th second
        if (current < _encoderMin) {
          # if P143_FEATURE_INCLUDE_M5STACK

          if (_useOffset) {
            _offsetEncoder = rawCurrent - _encoderMin;
          }
          # endif // if P143_FEATURE_INCLUDE_M5STACK
          current = _encoderMin; // keep minimal value
        }
        else if (current > _encoderMax) {
          # if P143_FEATURE_INCLUDE_M5STACK

          if (_useOffset) {
            _offsetEncoder = rawCurrent - _encoderMax;
          }
          # endif // if P143_FEATURE_INCLUDE_M5STACK
          current = _encoderMax; // keep maximal value
        }
        _previousEncoder = current;

        // (Re)Set Encoder to current position if outside the set boundaries
        if (current != orgCurrent) {
          switch (_device) {
            case P143_DeviceType_e::AdafruitEncoder:
              Adafruit_Seesaw->setEncoderPosition(current);
              break;
            # if P143_FEATURE_INCLUDE_M5STACK
            case P143_DeviceType_e::M5StackEncoder:

              // Set encoder position. NB: will only work if the encoder firmware is updated to v1.1
              if (!_useOffset) {
                int16_t down = current;
                I2C_write16_reg(_i2cAddress, P143_M5STACK_REG_ENCODER, down << 8 || down >> 8);
              }
              break;
            # endif // if P143_FEATURE_INCLUDE_M5STACK
            # if P143_FEATURE_INCLUDE_DFROBOT
            case P143_DeviceType_e::DFRobotEncoder:
              I2C_write16_reg(_i2cAddress, P143_DFROBOT_ENCODER_COUNT_MSB_REG, _initialOffset + _encoderPosition);
              break;
            # endif // if P143_FEATURE_INCLUDE_DFROBOT
          }
        }
      }
    }

    if (current != _encoderPosition) {
      // Generate event
      if (Settings.UseRules) {
        String taskEvent;
        taskEvent.reserve(64);
        taskEvent += getTaskDeviceName(event->TaskIndex);
        taskEvent += '#';
        taskEvent += getTaskValueName(event->TaskIndex, 0);
        taskEvent += '=';
        taskEvent += current;                    // Position
        taskEvent += ',';
        taskEvent += current - _encoderPosition; // Delta, positive = clock-wise
        eventQueue.addMove(std::move(taskEvent));
      }

      // Set task value
      _encoderPosition = current;

      UserVar[event->BaseVarIndex] = _encoderPosition;

      result = true;

      // Calculate colormapping
      # if P143_FEATURE_COUNTER_COLORMAPPING
      counterToColorMapping(event);
      # endif // if P143_FEATURE_COUNTER_COLORMAPPING

      # if PLUGIN_143_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("I2CEncoder : ");
        log += toString(_device);
        log += F(", Changed: ");
        log += _oldPosition;
        log += F(" to: ");
        log += _encoderPosition;
        log += F(", delta: ");
        log += _encoderPosition - _oldPosition;
        addLogMove(LOG_LEVEL_INFO, log);
      }
      # endif // if PLUGIN_143_DEBUG
    }
  }

  return result;
}

# if P143_FEATURE_COUNTER_COLORMAPPING

/*****************************************************
 * counterToColorMapping
 ****************************************************/
void P143_data_struct::counterToColorMapping(struct EventStruct *event) {
  int16_t iRed   = -1;
  int16_t iGreen = -1;
  int16_t iBlue  = -1;
  int16_t pRed   = -1;
  int16_t pGreen = -1;
  int16_t pBlue  = -1;
  int32_t iCount = INT32_MIN;
  int32_t pCount = INT32_MIN;

  switch (_mapping) {
    case P143_CounterMapping_e::ColorMapping:
    {
      for (int i = 0; i <= _colorMaps; i++) {
        if (!_colorMapping[i].isEmpty()) {
          if (iCount == INT32_MIN) {
            if (parseColorMapLine(_colorMapping[i], iCount, iRed, iGreen, iBlue)) {
              if (iCount < _encoderPosition) { // Reset, out of range
                iRed   = -1;
                iGreen = -1;
                iBlue  = -1;
                iCount = INT32_MIN;
              }
            }
          }
        }
      }
      break;
    }

    case P143_CounterMapping_e::ColorGradient:
    {
      for (int i = 0; i <= _colorMaps; i++) {
        if (!_colorMapping[i].isEmpty()) {
          if (iCount == INT32_MIN) {
            if (parseColorMapLine(_colorMapping[i], iCount, iRed, iGreen, iBlue)) {
              if ((iCount > _encoderPosition) || !rangeCheck(iCount, _encoderMin, _encoderMax)) {
                iRed   = -1;
                iGreen = -1;
                iBlue  = -1;
                iCount = INT32_MIN;
              }
            }
          }

          if (pCount == INT32_MIN) {
            if (parseColorMapLine(_colorMapping[i], pCount, pRed, pGreen, pBlue)) {
              if ((pCount < _encoderPosition) || !rangeCheck(pCount, _encoderMin, _encoderMax)) {
                pRed   = -1;
                pGreen = -1;
                pBlue  = -1;
                pCount = INT32_MIN;
              }
            }
          }
        }
      }

      // Calculate R/G/B gradient-values for current Counter within upper and lower range
      if ((pCount != iCount) && (iRed > -1) && (iGreen > -1) && (iBlue > -1) && (pRed > -1) && (pGreen > -1) && (pBlue > -1)) {
        iRed   = map(_encoderPosition, iCount, pCount, iRed, pRed);
        iGreen = map(_encoderPosition, iCount, pCount, iGreen, pGreen);
        iBlue  = map(_encoderPosition, iCount, pCount, iBlue, pBlue);
      }

      break;
    }
    case P143_CounterMapping_e::None:
      // Do nothing
      break;
  }

  // Updated Led color?
  if ((iRed > -1) && (iGreen > -1) && (iBlue > -1)) {
    #  if !defined(BUILD_NO_DEBUG) && PLUGIN_143_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("P143: Change color R:");
      log += iRed;
      log += F(", G:");
      log += iGreen;
      log += F(", B:");
      log += iBlue;
      #   ifdef LOG_LEVEL_DEBUG_DEV
      log += F(", pR:"); // DEV-only from here
      log += pRed;
      log += F(", pG:");
      log += pGreen;
      log += F(", pB:");
      log += pBlue;
      log += F(", iC:");
      log += iCount;
      log += F(", pC:");
      log += pCount;
      #   endif // ifdef LOG_LEVEL_DEBUG_DEV
      addLog(LOG_LEVEL_DEBUG, log);
    }
    #  endif // if !defined(BUILD_NO_DEBUG) && PLUGIN_143_DEBUG

    switch (_device) {
      case P143_DeviceType_e::AdafruitEncoder:
      {
        _red   = iRed;
        _green = iGreen;
        _blue  = iBlue;
        Adafruit_Spixel->setPixelColor(0, _red, _green, _blue);
        Adafruit_Spixel->show();
        break;
      }
      #  if P143_FEATURE_INCLUDE_M5STACK
      case P143_DeviceType_e::M5StackEncoder:
      {
        _red   = iRed;
        _green = iGreen;
        _blue  = iBlue;
        uint8_t data[4];
        data[0] = static_cast<uint8_t>(P143_M5STACK_SELECTION);
        data[1] = applyBrightness(_red);
        data[2] = applyBrightness(_green);
        data[3] = applyBrightness(_blue);
        I2C_writeBytes_reg(_i2cAddress, P143_M5STACK_REG_LED, data, 4);
        break;
      }
      #  endif // if P143_FEATURE_INCLUDE_M5STACK
      #  if P143_FEATURE_INCLUDE_DFROBOT
      case P143_DeviceType_e::DFRobotEncoder:
        break; // N/A
      #  endif // if P143_FEATURE_INCLUDE_DFROBOT
    }
  }
}

/*****************************************************
 * applyBrightness, calculate new color based on brightness, based on NeoPixel library setBrightness()
 ****************************************************/
uint8_t P143_data_struct::applyBrightness(uint8_t color) {
  if (_brightness) {
    color = (color * _brightness) >> 8;
  }

  return color;
}

/*****************************************************
 * parseColorMapLine, line prefixed with # is comment/disabled
 ****************************************************/
bool P143_data_struct::parseColorMapLine(const String& line,
                                         int32_t     & count,
                                         int16_t     & red,
                                         int16_t     & green,
                                         int16_t     & blue) {
  bool   result = false;
  String tmp    = parseString(line, 1);

  if (!tmp.isEmpty() && !tmp.startsWith(F("#"))) {
    count = tmp.toInt();

    tmp = parseString(line, 2);

    if (!tmp.isEmpty()) {
      red = tmp.toInt();

      tmp = parseString(line, 3);

      if (!tmp.isEmpty()) {
        green = tmp.toInt();

        tmp = parseString(line, 4);

        if (!tmp.isEmpty()) {
          blue   = tmp.toInt();
          result = true;
        }
      }
    }
  }

  return result;
}

/*****************************************************
 * rangeCheck
 ****************************************************/
bool P143_data_struct::rangeCheck(int32_t count,
                                  int32_t min,
                                  int32_t max) {
  bool result = true;

  if (min != max) {
    if ((count < min) || (count > max)) {
      result = false;
    }
  }
  return result;
}

# endif // if P143_FEATURE_COUNTER_COLORMAPPING

/*****************************************************
 * plugin_fifty_per_second, handle button actions
 ****************************************************/
bool P143_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  if (_initialized) {
    const uint8_t pressedState = 0; // button has this value if pressed
    uint8_t button             = _buttonLast;
    uint8_t state              = _buttonState;

    if (!_buttonIgnore) {
      // Read button
      switch (_device) {
        case P143_DeviceType_e::AdafruitEncoder:
          button = Adafruit_Seesaw->digitalRead(P143_SEESAW_SWITCH);
          break;
        # if P143_FEATURE_INCLUDE_M5STACK
        case P143_DeviceType_e::M5StackEncoder:
          button = I2C_read8_reg(_i2cAddress, P143_M5STACK_REG_BUTTON);
          break;
        # endif // if P143_FEATURE_INCLUDE_M5STACK
        # if P143_FEATURE_INCLUDE_DFROBOT
        case P143_DeviceType_e::DFRobotEncoder:
          uint8_t press = I2C_read8_reg(_i2cAddress, P143_DFROBOT_ENCODER_KEY_STATUS_REG);

          if ((press & 0x01) != 0) {
            I2C_write8_reg(_i2cAddress, P143_DFROBOT_ENCODER_KEY_STATUS_REG, 0x00); // Reset
            button = button ? 0 : 1;
          }

          break;
        # endif // if P143_FEATURE_INCLUDE_DFROBOT
      }
    } else {
      _buttonIgnore--; // Count down the ignore time, 20 msec/step
    }

    if (button != _buttonLast) {
      _buttonLast = button;

      switch (static_cast<P143_ButtonAction_e>(P143_PLUGIN_BUTTON_ACTION)) {
        case P143_ButtonAction_e::PushButton:
          state = button;
          break;
        case P143_ButtonAction_e::PushButtonInverted:
          state        = button == 0 ? 1 : 0;
          _buttonState = state == 0 ? 1 : 0; // Changed
          break;
        case P143_ButtonAction_e::ToggleSwitch:

          if (button == pressedState) {
            state = _buttonState == 0 ? 1 : 0; // Toggle
          }
          break;
      }
      _buttonIgnore = 5; // * 20 milliseconds (50/sec)
    }

    if (state != _buttonState) {
      // Generate event
      if (Settings.UseRules) {
        String taskEvent;
        taskEvent.reserve(64);
        taskEvent += getTaskDeviceName(event->TaskIndex);
        taskEvent += '#';
        taskEvent += getTaskValueName(event->TaskIndex, 1);
        taskEvent += '=';
        taskEvent += state; // New state
        eventQueue.addMove(std::move(taskEvent));
      }

      // Set task value
      _buttonState = state;

      UserVar[event->BaseVarIndex + 1] = _buttonState;

      return true;
    }
  }
  return false;
}

#endif // ifdef USES_P143
