#ifdef USES_P042
//#######################################################################################################
//######################################## Plugin 042: NeoPixel Candle ##################################
//#######################################################################################################

// PROJECT INFO
// Wifi Candle for ESPEasy by Dominik Schmidt (10.2016)


// INCLUDE jscolor (http://jscolor.com/)
//   * Download the lib from here: http://jscolor.com/release/latest.zip
//   * Extract jscolor.min.js
//   * Now open the Web UI of your ESPEasy with this URL:
//     http://<IP-ESPEasy>/upload
//   * Select Browse ... and choose the extracted jscolor.min.js File (ensure the ...min... version !!)
//   * Press Upload und you are done.

// Add the Adafruit Neopixel Library to your library path. You will find it here:
// https://github.com/adafruit/Adafruit_NeoPixel
// That´s all :-) Now ESPEasy has a new 25ms "Timer loop" and Neopixel Support.

// NOTES
// Please keep in mind that you can add tasks which produce a very large delay while reading the sensor.
// For example the DS18B20 is very slow in reading the values. This can slow down the simulation and you
// will notice that the candle did not run smooth. So keep an eye on your tasks and don't add to much other tasks.

// HARDWARE
// The Wifi Candle uses 20 WS2812 RGB pixels. They are all connected in one row.
// I build a wooden wick with 5 pixels on each side. (A picture is here : http://www.esp8266.nu/forum/viewtopic.php?f=2&t=2147)
// The pixels are connected to 5V and the data pin I use is GPIO13 (but you can choose another one).
// Please ensure that you use a strong power supply because the pixels consume a lot of power when they
// shine in white with high brightness!
// I also placed a 100µF capacitor at the end of the WS2812 chain on +5/GND just to ensure a good power stability.
// btw ... My Testboard was a NodeMCU V3.

// QUESTIONS
// Send me an email at dominik@logview.info
// or place a comment in the Forum:
// http://www.esp8266.nu/forum/viewtopic.php?f=2&t=2147

// Candle Infos
// http://www.howtodotip.com/how+to+do+arduino+candle++3
// https://codebender.cc/sketch:129316#Neopixel%20Candle.ino
// http://www.instructables.com/id/Garden-Arduino-Lights/?ALLSTEPS                    Garten Beleuchtung
// http://www.instructables.com/id/Arduino-Controlled-Electric-Candle/?ALLSTEPS       InstaMorph Kerze
// https://github.com/danesparza/Halloweenfire/blob/master/halloweenfire.ino          Halloweenfire

// RGB / HSV Converter
// https://github.com/ratkins/RGBConverter          Lib
// https://www.ruinelli.ch/rgb-to-hsv               Code
// http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both    Code Sammlung

#include <Adafruit_NeoPixel.h>

#define NUM_PIXEL       20         // Defines the amount of LED Pixel
#define NUM_PIXEL_ROW    5         // Defines the amount of LED Pixel per Row
#define RANDOM_PIXEL    70         // Defines the Flicker Level for Simple Candle
#define BRIGHT_START   128         // Defines the Start Brightness
#define BASE_TEMP       21         // Defines the Base Temp for TempRange Transformation

enum SimType {
  TypeOff,
  TypeSimpleCandle,
  TypeAdvancedCandle,
  TypeStaticLight,
  TypePolice,
  TypeBlink,
  TypeStrobe,
  TypeColorFader
};

enum ColorType {
  ColorDefault,
  ColorSelected
};

byte Candle_red = 0;
byte Candle_green = 0;
byte Candle_blue = 0;
byte Candle_bright = 128;
SimType Candle_type = TypeSimpleCandle;
ColorType Candle_color = ColorDefault;

// global variables
unsigned long Candle_Update = 0;
word Candle_Temp[4] = { 0, 0, 0 };     // Temp variables
int Candle_Temp4 = 0;
boolean GPIO_Set = false;

Adafruit_NeoPixel *Candle_pixels;

#define PLUGIN_042
#define PLUGIN_ID_042         42
#define PLUGIN_NAME_042       "Output - NeoPixel (Candle)"
#define PLUGIN_VALUENAME1_042 "Color"
#define PLUGIN_VALUENAME2_042 "Brightness"
#define PLUGIN_VALUENAME3_042 "Type"

boolean Plugin_042(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_042;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_042);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_042));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_042));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_042));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addHtml(F("<script src=\"jscolor.min.js\"></script>\n"));

        char tmpString[128];
        String options[8];
        // int optionValues[8];

        options[0] = F("Off");
        options[1] = F("Static Light");
        options[2] = F("Simple Candle");
        options[3] = F("Advanced Candle");
        options[4] = F("Police");
        options[5] = F("Blink");
        options[6] = F("Strobe");
        options[7] = F("Color Fader");

        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][4];
        if (choice > sizeof(options) - 1)
        {
          choice = 2;
        }

        // Candle Type Selection
        addFormSelector(F("Flame Type"), F("web_Candle_Type"), 8, options, NULL, choice);

        // Advanced Color options
        Candle_color = (ColorType)Settings.TaskDevicePluginConfig[event->TaskIndex][5];
        addHtml(F("<TR><TD>Color Handling:<TD>")); // checked
        addHtml(F("<input type='radio' id='web_Color_Default' name='web_Color_Type' value='0'"));
        if (Candle_color == ColorDefault) {
          addHtml(F(" checked>"));
        } else {
          addHtml(F(">"));
        }
        addHtml(F("<label for='web_Color_Default'> Use default color</label><br>"));
        addHtml(F("<input type='radio' id='web_Color_Selected' name='web_Color_Type' value='1'"));
        if (Candle_color == ColorSelected) {
          addHtml(F(" checked>"));
        } else {
          addHtml(F(">"));
        }
        addHtml(F("<label for='web_Color_Selected'> Use selected color</label><br>"));

        // Color Selection
        char hexvalue[7] = {0};
        sprintf(hexvalue, "%02X%02X%02X",     // Create Hex value for color
                Settings.TaskDevicePluginConfig[event->TaskIndex][0],
                Settings.TaskDevicePluginConfig[event->TaskIndex][1],
                Settings.TaskDevicePluginConfig[event->TaskIndex][2]);

        // http://jscolor.com/examples/
        addHtml(F("<TR><TD>Color:<TD><input class=\"jscolor {onFineChange:'update(this)'}\" value='"));
        addHtml(hexvalue);
        addHtml(F("'>"));
        addFormNumericBox(F("RGB Color"), F("web_RGB_Red"), Settings.TaskDevicePluginConfig[event->TaskIndex][0], 0, 255);
        addNumericBox(F("web_RGB_Green"), Settings.TaskDevicePluginConfig[event->TaskIndex][1], 0, 255);
        addNumericBox(F("web_RGB_Blue"), Settings.TaskDevicePluginConfig[event->TaskIndex][2], 0, 255);

        // Brightness Selection
        addHtml(F("<TR><TD>Brightness:<TD>min<input type='range' id='web_Bright_Slide' min='0' max='255' value='"));
        addHtml(String(Settings.TaskDevicePluginConfig[event->TaskIndex][3]));
        addHtml(F("'> max"));

        sprintf_P(tmpString, PSTR("<TR><TD>Brightness Value:<TD><input type='text' name='web_Bright_Text' id='web_Bright_Text' size='3' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][3]);
        addHtml(tmpString);

        // Some Javascript we need to update the items
        addHtml(F("<script script type='text/javascript'>"));
        addHtml(F("function update(picker) {"));
        addHtml(F("    document.getElementById('web_RGB_Red').value = Math.round(picker.rgb[0]);"));
        addHtml(F("    document.getElementById('web_RGB_Green').value = Math.round(picker.rgb[1]);"));
        addHtml(F("    document.getElementById('web_RGB_Blue').value = Math.round(picker.rgb[2]);"));
        addHtml(F("}"));
        addHtml(F("</script>"));

        addHtml(F("<script type='text/javascript'>window.addEventListener('load', function(){"));
        addHtml(F("var slider = document.getElementById('web_Bright_Slide');"));
        addHtml(F("slider.addEventListener('change', function(){"));
        addHtml(F("document.getElementById('web_Bright_Text').value = this.value;"));
        addHtml(F("});"));
        addHtml(F("});</script>"));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("web_RGB_Red"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("web_RGB_Green"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("web_RGB_Blue"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = getFormItemInt(F("web_Bright_Text"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = getFormItemInt(F("web_Candle_Type"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][5] = getFormItemInt(F("web_Color_Type"));

        Candle_red = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        Candle_green = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        Candle_blue = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        if (Candle_bright > 255) {
          Candle_bright = 255;
        }
        Candle_bright = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
        Candle_type = (SimType)Settings.TaskDevicePluginConfig[event->TaskIndex][4];
        Candle_color = (ColorType)Settings.TaskDevicePluginConfig[event->TaskIndex][5];

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Candle_red = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        Candle_green = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        Candle_blue = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        Candle_bright = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
        if (Candle_red == 0 && Candle_green == 0 && Candle_blue == 0) {
          Candle_bright = BRIGHT_START;
        }
        Candle_type = (SimType)Settings.TaskDevicePluginConfig[event->TaskIndex][4];
        Candle_color = (ColorType)Settings.TaskDevicePluginConfig[event->TaskIndex][5];

        if (!Candle_pixels || GPIO_Set == false)
        {
          GPIO_Set = Settings.TaskDevicePin1[event->TaskIndex] > -1;
          if (Candle_pixels) {
            delete Candle_pixels;
          }
          Candle_pixels = new Adafruit_NeoPixel(NUM_PIXEL, Settings.TaskDevicePin1[event->TaskIndex], NEO_GRB + NEO_KHZ800);
          SetPixelsBlack();
          Candle_pixels->setBrightness(Candle_bright);
          Candle_pixels->begin();
          String log = F("CAND : Init WS2812 Pin : ");
          log += Settings.TaskDevicePin1[event->TaskIndex];
          addLog(LOG_LEVEL_DEBUG, log);
        }

        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        Candle_pixels->setBrightness(Candle_bright);
        Candle_pixels->show(); // This sends the updated pixel color to the hardware.
        success = true;
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      {
        switch (Candle_type)
        {
          case 0:   // "Update" for OFF
            {
              type_Off();
              break;
            }

          case 1:   // Update for LIGHT
            {
              type_Static_Light();
              break;
            }

          case 2: // Random Updates for Simple Candle, Advanced Candle, Fire Simulation
          case 3:
            {
              if (timeOutReached(Candle_Update)) {
                if (Candle_type == 2) {
                  type_Simple_Candle();
                }
                if (Candle_type == 3) {
                  type_Advanced_Candle();
                }
                Candle_Update = millis() + random(25, 150);
              }
              break;
            }

          case 4:   // Update for Police
            {
              if (timeOutReached(Candle_Update)) {
                type_Police();
                Candle_Update = millis() + 150;
              }
              break;
            }

          case 5:   // Update for Blink
            {
              if (timeOutReached(Candle_Update)) {
                type_BlinkStrobe();
                Candle_Update = millis() + 100;
              }
              break;
            }

          case 6:   // Update for Strobe
            {
              type_BlinkStrobe();
              break;
            }
          case 7:   // Update for ColorFader
            {
              if (timeOutReached(Candle_Update)) {
                type_ColorFader();
                Candle_Update = millis() + 2000;
              }
              break;
            }
        }

        Candle_pixels->show();

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        UserVar[event->BaseVarIndex] = Candle_red * 65536 + Candle_green * 256 + Candle_blue;
        UserVar[event->BaseVarIndex + 1] = Candle_bright;
        UserVar[event->BaseVarIndex + 2] = Candle_type;

        success = true;
      }

    case PLUGIN_WRITE:
      {
        String tmpString  = string;

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

        if (tmpString.startsWith("CANDLE:")){
          int idx1 = tmpString.indexOf(':');
          int idx2 = tmpString.indexOf(':', idx1+1);
          int idx3 = tmpString.indexOf(':', idx2+1);
          int idx4 = tmpString.indexOf(':', idx3+1);
          String val_Type = tmpString.substring(idx1+1, idx2);
          String val_Color = tmpString.substring(idx2+1, idx3);
          String val_Bright = tmpString.substring(idx3+1, idx4);

          if (val_Type != "") {
             if (val_Type.toInt() > -1 && val_Type.toInt() < 8) {
                Settings.TaskDevicePluginConfig[event->TaskIndex][4] = val_Type.toInt();     // Type
                Candle_type = (SimType)Settings.TaskDevicePluginConfig[event->TaskIndex][4];
                String log = F("CAND : CMD - Type : ");
                log += val_Type;
                addLog(LOG_LEVEL_DEBUG, log);
             }
          }

          if (val_Bright != "") {
             if (val_Bright.toInt() > -1 && val_Bright.toInt() < 256) {
                Settings.TaskDevicePluginConfig[event->TaskIndex][3] = val_Bright.toInt();     // Brightness
                Candle_bright = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
                String log = F("CAND : CMD - Bright : ");
                log += val_Bright;
                addLog(LOG_LEVEL_DEBUG, log);
             }
          }

          if (val_Color != "") {
            long number = strtol( &val_Color[0], NULL, 16);
            // Split RGB to r, g, b values
            byte r = number >> 16;
            byte g = number >> 8 & 0xFF;
            byte b = number & 0xFF;

            Settings.TaskDevicePluginConfig[event->TaskIndex][0] = r;   // R
            Settings.TaskDevicePluginConfig[event->TaskIndex][1] = g;   // G
            Settings.TaskDevicePluginConfig[event->TaskIndex][2] = b;   // B
            Candle_red = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
            Candle_green = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
            Candle_blue = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
            Settings.TaskDevicePluginConfig[event->TaskIndex][5] = 1;
            Candle_color = (ColorType)Settings.TaskDevicePluginConfig[event->TaskIndex][5];   // ColorType (ColorSelected)

            String log = F("CAND : CMD - R ");
            log += r;
            log += F(" G ");
            log += g;
            log += F(" B ");
            log += b;
            addLog(LOG_LEVEL_DEBUG, log);
          } else {
            Settings.TaskDevicePluginConfig[event->TaskIndex][5] = 0;
            Candle_color = (ColorType)Settings.TaskDevicePluginConfig[event->TaskIndex][5];   // ColorType (ColorDefault)
            addLog(LOG_LEVEL_DEBUG, F("CAND : CMD - Color : DEFAULT"));
          }

          //SaveTaskSettings(event->TaskIndex);
          //SaveSettings();

          success = true;
        }

        break;
      }

  }
  return success;
}

void SetPixelsBlack() {
  for (int i = 0; i < NUM_PIXEL; i++) {
    Candle_pixels->setPixelColor(i, Candle_pixels->Color(0, 0, 0));
  }
}

void SetPixelToColor(int PixelIdx) {
  Candle_pixels->setPixelColor(PixelIdx, Candle_pixels->Color(Candle_red, Candle_green, Candle_blue));
}

void type_Off() {
  SetPixelsBlack();
}

void type_Static_Light() {
  for (int i = 0; i < NUM_PIXEL; i++) {
    if (Candle_color == ColorDefault) {
      Candle_pixels->setPixelColor(i, 255, 255, 255);     // Default is white
    } else {
      Candle_pixels->setPixelColor(i, Candle_red, Candle_green, Candle_blue);
    }
  }
}

void type_Simple_Candle() {
  int r, g, b;
  if (Candle_color == ColorDefault) {
    r = 226, g = 042, b =  35;   // Regular (orange) flame
    //r = 158, g =   8, b = 148;   // Purple flame
    //r =  74, g = 150, b =  12;   // Green flame
  } else {
    r = Candle_red, g = Candle_green, b = Candle_blue;
  }

  //  Flicker, based on our initial RGB values
  for (int i = 0; i < NUM_PIXEL; i++) {
    int flicker = random(0, RANDOM_PIXEL);
    int r1 = r - flicker;
    int g1 = g - flicker;
    int b1 = b - flicker;
    if (g1 < 0) g1 = 0;
    if (r1 < 0) r1 = 0;
    if (b1 < 0) b1 = 0;
    Candle_pixels->setPixelColor(i, r1, g1, b1);
  }
}

void type_Advanced_Candle() {
  Candle_Temp[0] = random(1, 4); // 1..4  LEDs in RED
  Candle_Temp[1] = random(1, 4) + Candle_Temp[0]; // 1..3  LEDs in Yellow / Orange
  Candle_Temp[2] = random(0, 2); // 0..1  Choose Yellow = 0 / Orange = 1

  int colorbase[3];
  int color1[3];
  int color2[3];
  int color3[3];

  if (Candle_color == ColorDefault) {
    colorbase[0] = 255; colorbase[1] = 120; colorbase[2] = 0;   // Light Orange #FF7800
    color1[0] = 115; color1[1] = 50; color1[2] = 0;             // Brown      #733200
    color2[0] = 180; color2[1] = 80; color2[2] = 0;             // Orange     #B45000
    color3[0] =  70; color3[1] = 30; color3[2] = 0;              // Dark brown #4A2000
  } else {
    colorbase[0] = Candle_red; colorbase[1] = Candle_green; colorbase[2] = Candle_blue;
    double hsv[3];
    // Calc HSV
    RGBtoHSV(Candle_red, Candle_green, Candle_blue, hsv);
    double newH = hsv[0] - 5;
    if (newH < 0) { newH += 359; }
    double newV = hsv[2] / 2;
    double newV2 = hsv[2] / 4;
    // Calc new RGBs
    HSVtoRGB(newH, hsv[1], hsv[2], color1);
    HSVtoRGB(hsv[0], hsv[1], newV, color2);
    HSVtoRGB(newH, hsv[1], newV2, color3);
  }

  for (int j = 0; j < 4; j++) {
    for (unsigned int i = 1; i < 6; i++){
      if (i <= Candle_Temp[0]) {
        Candle_pixels->setPixelColor(j * 5 + i - 1, colorbase[0], colorbase[1], colorbase[2]);
      }
      if (i > Candle_Temp[0] && i <= Candle_Temp[1]) {
        if (Candle_Temp[2] == 0){
          Candle_pixels->setPixelColor(j * 5 + i - 1, color1[0], color1[1], color1[2]);
        } else {
          Candle_pixels->setPixelColor(j * 5 + i - 1, color2[0], color2[1], color2[2]);
        }
      }
      if (i > Candle_Temp[1]) {
        Candle_pixels->setPixelColor(j * 5 + i - 1, color3[0], color3[1], color3[2]);
      }
    }
  }
}

void type_Police() {
  Candle_Temp[0]++;
  if (Candle_Temp[0] > 3) {
    Candle_Temp[0] = 0;
  }

  for (unsigned int i = 0; i < 4; i++) {
    if (i == Candle_Temp[0])
    {
      for (int j = 0; j < 5; j++) {
        if (Candle_color == ColorDefault) {
          Candle_pixels->setPixelColor(i * 5 + j, 0, 0, 255);
        } else {
          Candle_pixels->setPixelColor(i * 5 + j, Candle_red, Candle_green, Candle_blue);
        }
      }
    } else {
      for (int j = 0; j < 5; j++) {
        Candle_pixels->setPixelColor(i * 5 + j, 0, 0, 0);
      }
    }
  }
}

void type_BlinkStrobe() {
  Candle_Temp[0]++;
  if (Candle_Temp[0] > 1) {
    Candle_Temp[0] = 0;
  }

  for (int i = 0; i < NUM_PIXEL; i++) {
    if (Candle_Temp[0] == 0) {
      Candle_pixels->setPixelColor(i, 0, 0, 0);
    } else {
      if (Candle_color == ColorDefault) {
        Candle_pixels->setPixelColor(i, 255, 255, 255);     // Default is white
      } else {
        Candle_pixels->setPixelColor(i, Candle_red, Candle_green, Candle_blue);
      }
    }
  }
}

void type_ColorFader() {
  int colors[3];
  double hsv[3];
  if (Candle_color != ColorDefault) {
    if (Candle_Temp[0] > 254 && Candle_Temp[1] == 1) {
      Candle_Temp[1] = 0;
    }
    if (Candle_Temp[0] < 55 && Candle_Temp[1] == 0) {
      Candle_Temp[1] = 1;
    }

    if (Candle_Temp[1] > 0) {
      Candle_Temp[0]++;
    } else {
      Candle_Temp[0]--;
    }

    // Calc HSV
    // void RGBtoHSV(byte r, byte g, byte b, double hsv[3])
    RGBtoHSV(Candle_red, Candle_green, Candle_blue, hsv);

    // Calc RGB with new V
    // HSVtoRGB(int hue, int sat, int val, int colors[3])
    // hue: 0-359, sat: 0-255, val (lightness): 0-255
    HSVtoRGB(hsv[0], hsv[1], Candle_Temp[0], colors);

    for (int i = 0; i < NUM_PIXEL; i++) {
      Candle_pixels->setPixelColor(i, colors[0], colors[1], colors[2]);
    }
  } else {
    Candle_Temp[0]++;
    if (Candle_Temp[0] > 359) {
      Candle_Temp[0] = 0;
    }

    // hue: 0-359, sat: 0-255, val (lightness): 0-255
    HSVtoRGB(Candle_Temp[0], 255, 255, colors);

    for (int i = 0; i < NUM_PIXEL; i++) {
      Candle_pixels->setPixelColor(i, colors[0], colors[1], colors[2]);
    }
  }
}

// Convert HSC Color to RGB Color
void HSVtoRGB(int hue, int sat, int val, int colors[3]) {
  // hue: 0-359, sat: 0-255, val (lightness): 0-255
  int r=0, g=0, b=0, base=0;

  if (sat == 0) { // Achromatic color (gray).
    colors[0]=val;
    colors[1]=val;
    colors[2]=val;
  }
  else  {
    base = ((255 - sat) * val)>>8;
    switch(hue/60) {
    case 0:
      r = val;
      g = (((val-base)*hue)/60)+base;
      b = base;
      break;
    case 1:
      r = (((val-base)*(60-(hue%60)))/60)+base;
      g = val;
      b = base;
      break;
    case 2:
      r = base;
      g = val;
      b = (((val-base)*(hue%60))/60)+base;
      break;
    case 3:
      r = base;
      g = (((val-base)*(60-(hue%60)))/60)+base;
      b = val;
      break;
    case 4:
      r = (((val-base)*(hue%60))/60)+base;
      g = base;
      b = val;
      break;
    case 5:
      r = val;
      g = base;
      b = (((val-base)*(60-(hue%60)))/60)+base;
      break;
    }
    colors[0]=r;
    colors[1]=g;
    colors[2]=b;
  }
}

// Convert RGB Color to HSV Color
void RGBtoHSV(byte r, byte g, byte b, double hsv[3]) {
    double rd = (double) r/255;
    double gd = (double) g/255;
    double bd = (double) b/255;
    double maxval = rd;
    if (gd > maxval) { maxval = gd; }
    if (bd > maxval) { maxval = bd; }
    double minval = rd;
    if (gd < minval) { minval = gd; }
    if (bd < minval) { minval = bd; }
    double h = 0, s, v = maxval;
    double d = maxval - minval;

    s = maxval == 0 ? 0 : d / maxval;

    if (maxval == minval) {
        h = 0; // achromatic
    } else {
        if (maxval == rd) {
            h = (gd - bd) / d + (gd < bd ? 6 : 0);
        } else if (maxval == gd) {
            h = (bd - rd) / d + 2;
        } else if (maxval == bd) {
            h = (rd - gd) / d + 4;
        }
        h /= 6;
    }

    hsv[0] = h * 360;
    hsv[1] = s * 255;
    hsv[2] = v * 255;
}
#endif // USES_P042
