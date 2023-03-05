#include "../PluginStructs/P042_data_struct.h"

#ifdef USES_P042

P042_data_struct::P042_data_struct() {
  //
}

P042_data_struct::~P042_data_struct() {
  if (Candle_pixels != nullptr) {
    SetPixelsBlack(); // Turn off
    Candle_pixels->show();
    delete Candle_pixels;
    Candle_pixels = nullptr;
  }
}

bool P042_data_struct::plugin_init(struct EventStruct *event) {
  if (P042_CONFIG_PIXELCOUNT == 0) { P042_CONFIG_PIXELCOUNT = P042_NUM_PIXEL; }
  Candle_red    = P042_CONFIG_RED;
  Candle_green  = P042_CONFIG_GREEN;
  Candle_blue   = P042_CONFIG_BLUE;
  Candle_bright = P042_CONFIG_BRIGHTNESS;
  Candle_pxlcnt = P042_CONFIG_PIXELCOUNT;
  Candle_pxlcnt = P042_CONFIG_PIXELCOUNT;
  segment       = Candle_pxlcnt / 4;

  if ((Candle_red == 0) && (Candle_green == 0) && (Candle_blue == 0)) {
    Candle_bright = P042_BRIGHT_START;
  }
  Candle_type  = static_cast<P042_SimType>(P042_CONFIG_CANDLETYPE);
  Candle_color = static_cast<P042_ColorType>(P042_CONFIG_COLORTYPE);

  if (!Candle_pixels || (GPIO_Set == false)) {
    GPIO_Set = validGpio(CONFIG_PIN1);

    if (GPIO_Set) {
      if (Candle_pixels) {
        delete Candle_pixels;
      }
      Candle_pixels = new (std::nothrow) Adafruit_NeoPixel(P042_CONFIG_PIXELCOUNT, CONFIG_PIN1, NEO_GRB + NEO_KHZ800);

      if (Candle_pixels != nullptr) {
        SetPixelsBlack();
        Candle_pixels->setBrightness(Candle_bright);
        Candle_pixels->begin();
      }

          # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLogMove(LOG_LEVEL_DEBUG, concat(F("CAND : Init WS2812 Pin : "), static_cast<int>(CONFIG_PIN1)));
      }
          # endif // ifndef BUILD_NO_DEBUG
    }
  }

  return Candle_pixels != nullptr;
}

bool P042_data_struct::plugin_read(struct EventStruct *event) {
  UserVar[event->BaseVarIndex]     = Candle_red << 16 | Candle_green << 8 | Candle_blue;
  UserVar[event->BaseVarIndex + 1] = Candle_bright;
  UserVar[event->BaseVarIndex + 2] = static_cast<float>(Candle_type);

  return true;
}

bool P042_data_struct::plugin_once_a_second(struct EventStruct *event) {
  Candle_pixels->setBrightness(Candle_bright);
  Candle_pixels->show(); // This sends the updated pixel color to the hardware.
  return true;
}

bool P042_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  switch (Candle_type)
  {
    case P042_SimType::TypeOff: // "Update" for OFF
    {
      SetPixelsBlack();
      break;
    }

    case P042_SimType::TypeSimpleCandle: // Update for LIGHT
    {
      type_Static_Light();
      break;
    }

    case P042_SimType::TypeAdvancedCandle: // Random Updates for Simple Candle, Advanced Candle, Fire Simulation
    case P042_SimType::TypeStaticLight:
    {
      if (timeOutReached(Candle_Update)) {
        if (Candle_type == P042_SimType::TypeAdvancedCandle) {
          type_Simple_Candle();
        }

        if (Candle_type == P042_SimType::TypeStaticLight) {
          type_Advanced_Candle();
        }
        Candle_Update = millis() + random(25, 150);
      }
      break;
    }

    case P042_SimType::TypePolice: // Update for Police
    {
      if (timeOutReached(Candle_Update)) {
        type_Police();
        Candle_Update = millis() + 150;
      }
      break;
    }

    case P042_SimType::TypeBlink: // Update for Blink
    {
      if (timeOutReached(Candle_Update)) {
        type_BlinkStrobe();
        Candle_Update = millis() + 100;
      }
      break;
    }

    case P042_SimType::TypeStrobe: // Update for Strobe
    {
      type_BlinkStrobe();
      break;
    }
    case P042_SimType::TypeColorFader: // Update for ColorFader
    {
      if (timeOutReached(Candle_Update)) {
        type_ColorFader();
        Candle_Update = millis() + 2000;
      }
      break;
    }
  }

  Candle_pixels->show();

  return true;
}

bool P042_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  bool   success = false;
  char   cmdSep  = ','; // Try comma first
  String cmd     = parseString(string, 1, cmdSep);

  if (!equals(cmd, F("candle"))) {
    cmdSep = ':'; // Fallback to colon
    cmd    = parseString(string, 1, cmdSep);
  }

  // Test
  // MQTT   : mosquitto_pub -d -t sensors/espeasy/ESP_Candle/cmd  -m "CANDLE_OFF"
  // HTTP   : http://192.168.30.183/tools?cmd=CANDLE%3A5%3AFF0000%3A200
  //          http://192.168.30.183/tools?cmd=CANDLE:4:FF0000:200
  // SERIAL : CANDLE:4:FF0000:200<CR><LF>

  // Commands
  // CANDLE:<FlameType>:<Color>:<Brightness>
  //    <FlameType>  : 1 Static Light, 2 Simple Candle, 3 Advanced Candle, 4 Police, 5 Blink, 6 Strobe, 7 Color Fader
  //    <Color>      : n.def.  Use the default color
  //                   RRGGBB  Use color in RRGGBB style (red, green blue) as HEX
  //    <Brightness> : 0-255
  // Samples:   CANDLE:2::100           Simple Candle with Default color and Brigthness at 100
  //            CANDLE:5:FF0000:200     Blink with RED Color and Brigthness at 200
  //            CANDLE:0::              Candle OFF
  //            CANDLE:1::255           Candle ON - White and full brigthness
  // 2023-01-21: command can also be in lowercase, and separator can either be comma or colon, but all have to be the same

  if (equals(cmd, F("candle"))) {
    const String val_Type   = parseString(string, 2, cmdSep);
    const String val_Color  = parseString(string, 3, cmdSep);
    const String val_Bright = parseString(string, 4, cmdSep);

    if (!val_Type.isEmpty()) {
      if ((val_Type.toInt() > -1) && (val_Type.toInt() < P042_FLAME_OPTIONS)) {
        P042_CONFIG_CANDLETYPE = val_Type.toInt(); // Type
        Candle_type            = static_cast<P042_SimType>(P042_CONFIG_CANDLETYPE);
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLogMove(LOG_LEVEL_DEBUG, concat(F("CAND : CMD - Type : "), val_Type));
        }
        # endif // ifndef BUILD_NO_DEBUG
      }
    }

    if (!val_Bright.isEmpty()) {
      if ((val_Bright.toInt() > -1) && (val_Bright.toInt() < 256)) {
        P042_CONFIG_BRIGHTNESS = val_Bright.toInt(); // Brightness
        Candle_bright          = P042_CONFIG_BRIGHTNESS;
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLogMove(LOG_LEVEL_DEBUG, concat(F("CAND : CMD - Bright : "), val_Bright));
        }
        # endif // ifndef BUILD_NO_DEBUG
      }
    }

    uint32_t number = 0;

    if (!val_Color.isEmpty() && validUIntFromString(concat(F("0x"), val_Color), number)) {
      // Split RGB to r, g, b values
      P042_CONFIG_RED       = (number >> 16) & 0xFF;
      P042_CONFIG_GREEN     = (number >> 8) & 0xFF;
      P042_CONFIG_BLUE      = number & 0xFF;
      Candle_red            = P042_CONFIG_RED;
      Candle_green          = P042_CONFIG_GREEN;
      Candle_blue           = P042_CONFIG_BLUE;
      P042_CONFIG_COLORTYPE = 1;
      Candle_color          = static_cast<P042_ColorType>(P042_CONFIG_COLORTYPE);

      # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("CAND : CMD - R ");
        log += Candle_red;
        log += F(" G ");
        log += Candle_green;
        log += F(" B ");
        log += Candle_blue;
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
      # endif // ifndef BUILD_NO_DEBUG
    } else {
      P042_CONFIG_COLORTYPE = 0;
      Candle_color          = static_cast<P042_ColorType>(P042_CONFIG_COLORTYPE);
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("CAND : CMD - Color : DEFAULT"));
      # endif // ifndef BUILD_NO_DEBUG
    }

    success = true;
  }

  return success;
}

void P042_data_struct::SetPixelsBlack() {
  for (int i = 0; i < Candle_pxlcnt; i++) {
    Candle_pixels->setPixelColor(i, Candle_pixels->Color(0, 0, 0));
  }
}

void P042_data_struct::SetPixelToColor(int PixelIdx) {
  Candle_pixels->setPixelColor(PixelIdx, Candle_pixels->Color(Candle_red, Candle_green, Candle_blue));
}

void P042_data_struct::type_Static_Light() {
  for (int i = 0; i < Candle_pxlcnt; i++) {
    if (Candle_color == P042_ColorType::ColorDefault) {
      Candle_pixels->setPixelColor(i, 255, 255, 255); // Default is white
    } else {
      Candle_pixels->setPixelColor(i, Candle_red, Candle_green, Candle_blue);
    }
  }
}

void P042_data_struct::type_Simple_Candle() {
  int r, g, b;

  if (Candle_color == P042_ColorType::ColorDefault) {
    r = 226, g = 42, b =  35; // Regular (orange) flame
    // r = 158, g =   8, b = 148;   // Purple flame
    // r =  74, g = 150, b =  12;   // Green flame
  } else {
    r = Candle_red, g = Candle_green, b = Candle_blue;
  }

  //  Flicker, based on our initial RGB values
  for (int i = 0; i < Candle_pxlcnt; i++) {
    int flicker = random(0, P042_RANDOM_PIXEL);
    int r1      = r - flicker;
    int g1      = g - flicker;
    int b1      = b - flicker;

    if (g1 < 0) { g1 = 0; }

    if (r1 < 0) { r1 = 0; }

    if (b1 < 0) { b1 = 0; }
    Candle_pixels->setPixelColor(i, r1, g1, b1);
  }
}

void P042_data_struct::type_Advanced_Candle() {
  Candle_Temp[0] = random(1, 4);                  // 1..4  LEDs in RED
  Candle_Temp[1] = random(1, 4) + Candle_Temp[0]; // 1..3  LEDs in Yellow / Orange
  Candle_Temp[2] = random(0, 2);                  // 0..1  Choose Yellow = 0 / Orange = 1

  int colorbase[3];
  int color1[3];
  int color2[3];
  int color3[3];

  if (Candle_color == P042_ColorType::ColorDefault) {
    colorbase[0] = 255; colorbase[1] = 120; colorbase[2] = 0; // Light Orange #FF7800
    color1[0]    = 115; color1[1] = 50; color1[2] = 0;        // Brown      #733200
    color2[0]    = 180; color2[1] = 80; color2[2] = 0;        // Orange     #B45000
    color3[0]    =  70; color3[1] = 30; color3[2] = 0;        // Dark brown #4A2000
  } else {
    colorbase[0] = Candle_red; colorbase[1] = Candle_green; colorbase[2] = Candle_blue;
    float hsv[3];

    // Calc HSV
    RGB2HSV(Candle_red, Candle_green, Candle_blue, hsv);
    float newH = hsv[0] - 5.0f;

    if (newH < 0) { newH += 359.0f; }
    float newV  = hsv[2] / 2.0f;
    float newV2 = hsv[2] / 4.0f;

    // Calc new RGBs
    HSV2RGB(newH,   hsv[1], hsv[2], color1);
    HSV2RGB(hsv[0], hsv[1], newV,   color2);
    HSV2RGB(newH,   hsv[1], newV2,  color3);
  }

  for (int j = 0; j < 4; j++) {
    for (int i = 1; i <= segment; i++) {
      if (i <= Candle_Temp[0]) {
        Candle_pixels->setPixelColor(j * segment + i - 1, colorbase[0], colorbase[1], colorbase[2]);
      }

      if ((i > Candle_Temp[0]) && (i <= Candle_Temp[1])) {
        if (Candle_Temp[2] == 0) {
          Candle_pixels->setPixelColor(j * segment + i - 1, color1[0], color1[1], color1[2]);
        } else {
          Candle_pixels->setPixelColor(j * segment + i - 1, color2[0], color2[1], color2[2]);
        }
      }

      if (i > Candle_Temp[1]) {
        Candle_pixels->setPixelColor(j * segment + i - 1, color3[0], color3[1], color3[2]);
      }
    }
  }
}

void P042_data_struct::type_Police() {
  Candle_Temp[0]++;

  if (Candle_Temp[0] > 3) {
    Candle_Temp[0] = 0;
  }

  for (unsigned int i = 0; i < 4; i++) {
    if (i == Candle_Temp[0])
    {
      for (int j = 0; j < segment; j++) {
        if (Candle_color == P042_ColorType::ColorDefault) {
          Candle_pixels->setPixelColor(i * segment + j, 0, 0, 255);
        } else {
          Candle_pixels->setPixelColor(i * segment + j, Candle_red, Candle_green, Candle_blue);
        }
      }
    } else {
      for (int j = 0; j < segment; j++) {
        Candle_pixels->setPixelColor(i * segment + j, 0, 0, 0);
      }
    }
  }
}

void P042_data_struct::type_BlinkStrobe() {
  Candle_Temp[0]++;

  if (Candle_Temp[0] > 1) {
    Candle_Temp[0] = 0;
  }

  for (int i = 0; i < Candle_pxlcnt; i++) {
    if (Candle_Temp[0] == 0) {
      Candle_pixels->setPixelColor(i, 0, 0, 0);
    } else {
      if (Candle_color == P042_ColorType::ColorDefault) {
        Candle_pixels->setPixelColor(i, 255, 255, 255); // Default is white
      } else {
        Candle_pixels->setPixelColor(i, Candle_red, Candle_green, Candle_blue);
      }
    }
  }
}

void P042_data_struct::type_ColorFader() {
  int   colors[3];
  float hsv[3];

  if (Candle_color != P042_ColorType::ColorDefault) {
    if ((Candle_Temp[0] > 254) && (Candle_Temp[1] == 1)) {
      Candle_Temp[1] = 0;
    }

    if ((Candle_Temp[0] < 55) && (Candle_Temp[1] == 0)) {
      Candle_Temp[1] = 1;
    }

    if (Candle_Temp[1] > 0) {
      Candle_Temp[0]++;
    } else {
      Candle_Temp[0]--;
    }

    // Calc HSV
    // void RGB2HSV(uint8_t r, uint8_t g, uint8_t b, float hsv[3])
    RGB2HSV(Candle_red, Candle_green, Candle_blue, hsv);

    // Calc RGB with new V
    // HSV2RGB(float hue, float sat, float val, int colors[3])
    // hue: 0-359, sat: 0-255, val (lightness): 0-255
    HSV2RGB(hsv[0], hsv[1], Candle_Temp[0], colors);

    for (int i = 0; i < Candle_pxlcnt; i++) {
      Candle_pixels->setPixelColor(i, colors[0], colors[1], colors[2]);
    }
  } else {
    Candle_Temp[0]++;

    if (Candle_Temp[0] > 359) {
      Candle_Temp[0] = 0;
    }

    // hue: 0-359, sat: 0-255, val (lightness): 0-255
    HSV2RGB(Candle_Temp[0], 255, 255, colors);

    for (int i = 0; i < Candle_pxlcnt; i++) {
      Candle_pixels->setPixelColor(i, colors[0], colors[1], colors[2]);
    }
  }
}

#endif // ifdef USES_P042
