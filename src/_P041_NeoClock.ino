#include "_Plugin_Helper.h"
#ifdef USES_P041
//#######################################################################################################
//#################################### Plugin 041: NeoPixel clock #######################################
//#######################################################################################################
#include <Adafruit_NeoPixel.h>


#define NUM_LEDS      114

uint8_t Plugin_041_red = 0;
uint8_t Plugin_041_green = 0;
uint8_t Plugin_041_blue = 0;

Adafruit_NeoPixel *Plugin_041_pixels;

#define PLUGIN_041
#define PLUGIN_ID_041         41
#define PLUGIN_NAME_041       "Output - NeoPixel (Word Clock)"
#define PLUGIN_VALUENAME1_041 "Clock"
boolean Plugin_041(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_041;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_NONE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].SendDataOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_041);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_041));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("Data"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
      	addFormNumericBox(F("Red"), F("red"), PCONFIG(0), 0, 255);
      	addFormNumericBox(F("Green"), F("green"), PCONFIG(1), 0, 255);
      	addFormNumericBox(F("Blue"), F("blue"), PCONFIG(2), 0, 255);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("red"));
        PCONFIG(1) = getFormItemInt(F("green"));
        PCONFIG(2) = getFormItemInt(F("blue"));
        Plugin_041_red = PCONFIG(0);
        Plugin_041_green = PCONFIG(1);
        Plugin_041_blue = PCONFIG(2);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (Plugin_041_pixels == nullptr)
        {
          Plugin_041_pixels = new (std::nothrow) Adafruit_NeoPixel(NUM_LEDS, CONFIG_PIN1, NEO_GRB + NEO_KHZ800);
          if (Plugin_041_pixels != nullptr) {
            Plugin_041_pixels->begin(); // This initializes the NeoPixel library.
          }
        }
        Plugin_041_red = PCONFIG(0);
        Plugin_041_green = PCONFIG(1);
        Plugin_041_blue = PCONFIG(2);
        success = Plugin_041_pixels != nullptr;
        break;
      }

    case PLUGIN_EXIT:
      {
        if (Plugin_041_pixels != nullptr) {
          delete Plugin_041_pixels;
          Plugin_041_pixels = nullptr;
        }
        break;
      }

    case PLUGIN_CLOCK_IN:
      {
        Plugin_041_update();
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        //int ldrVal = map(analogRead(A0), 0, 1023, 15, 245);
        //serialPrint("LDR value: ");
        //serialPrintln(ldrVal);
        //Plugin_041_pixels->setBrightness(255-ldrVal);
        //Plugin_041_pixels->show(); // This sends the updated pixel color to the hardware.
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String cmd = parseString(string, 1);
        if (cmd.equalsIgnoreCase(F("NeoClockColor")))
        {
          Plugin_041_red = event->Par1;
          Plugin_041_green = event->Par2;
          Plugin_041_blue = event->Par3;
          Plugin_041_update();
          success = true;
        }

        if (cmd.equalsIgnoreCase(F("NeoTestAll")))
        {
          for (int i = 0; i < NUM_LEDS; i++)
            Plugin_041_pixels->setPixelColor(i, Plugin_041_pixels->Color(event->Par1, event->Par2, event->Par3));
          Plugin_041_pixels->show(); // This sends the updated pixel color to the hardware.
          success = true;
        }

        if (cmd.equalsIgnoreCase(F("NeoTestLoop")))
        {
          for (int i = 0; i < NUM_LEDS; i++)
          {
            resetAndBlack();
            Plugin_041_pixels->setPixelColor(i, Plugin_041_pixels->Color(event->Par1, event->Par2, event->Par3));
            Plugin_041_pixels->show(); // This sends the updated pixel color to the hardware.
            delay(200);
          }
          success = true;
        }

        break;
      }

  }
  return success;
}

void Plugin_041_update()
{
  uint8_t Hours = node_time.hour();
  uint8_t Minutes = node_time.minute();
  resetAndBlack();
  timeToStrip(Hours, Minutes);
  Plugin_041_pixels->show(); // This sends the updated pixel color to the hardware.
}


void resetAndBlack() {
  for (int i = 0; i < NUM_LEDS; i++) {
    Plugin_041_pixels->setPixelColor(i, Plugin_041_pixels->Color(0, 0, 0));
  }
}

void pushToStrip(int ledId) {
  Plugin_041_pixels->setPixelColor(ledId, Plugin_041_pixels->Color(Plugin_041_red, Plugin_041_green, Plugin_041_blue));
}

void timeToStrip(uint8_t hours, uint8_t minutes)
{
  pushIT_IS();
  //show minutes
  if (minutes >= 5 && minutes < 10) {
    pushFIVE1();
    pushAFTER();
  } else if (minutes >= 10 && minutes < 15) {
    pushTEN1();
    pushAFTER();
  } else if (minutes >= 15 && minutes < 20) {
    pushQUATER();
    pushAFTER();
  } else if (minutes >= 20 && minutes < 25) {
    pushTEN1();
    pushFOR();
    pushHALF();
  } else if (minutes >= 25 && minutes < 30) {
    pushFIVE1();
    pushFOR();
    pushHALF();
  } else if (minutes >= 30 && minutes < 35) {
    pushHALF();
  } else if (minutes >= 35 && minutes < 40) {
    pushFIVE1();
    pushAFTER();
    pushHALF();
  } else if (minutes >= 40 && minutes < 45) {
    pushTEN1();
    pushAFTER();
    pushHALF();
  } else if (minutes >= 45 && minutes < 50) {
    pushQUATER();
    pushFOR();
  } else if (minutes >= 50 && minutes < 55) {
    pushTEN1();
    pushFOR();
  } else if (minutes >= 55 && minutes < 60) {
    pushFIVE1();
    pushFOR();
  }

  int singleMinutes = minutes % 5;
  switch (singleMinutes) {
    case 1:
      pushM_ONE();
      break;
    case 2:
      pushM_ONE();
      pushM_TWO();
      break;
    case 3:
      pushM_ONE();
      pushM_TWO();
      pushM_THREE();
      break;
    case 4:
      pushM_ONE();
      pushM_TWO();
      pushM_THREE();
      pushM_FOUR();
      break;
  }
  if (hours >= 12) {
    hours -= 12;
  }
  if (hours == 12) {
    hours = 0;
  }
  if (minutes >= 20) {
    hours++;
  }

  //show hours
  switch (hours) {
    case 1:
      pushONE();
      break;
    case 2:
      pushTWO();
      break;
    case 3:
      pushTHREE();
      break;
    case 4:
      pushFOUR();
      break;
    case 5:
      pushFIVE2();
      break;
    case 6:
      pushSIX();
      break;
    case 7:
      pushSEVEN();
      break;
    case 8:
      pushEIGHT();
      break;
    case 9:
      pushNINE();
      break;
    case 10:
      pushTEN();
      break;
    case 11:
      pushELEVEN();
      break;
    case 0:
    case 12:
      pushTWELVE();
      break;
  }
  //show HOUR
  if (minutes < 5) {
    pushHOURE();
  }
}

void pushToStrip(const int* ids, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    pushToStrip(ids[i]);
  }
}


void pushM_ONE() {
  pushToStrip(0);
}
void pushM_TWO() {
  pushToStrip(12);
}
void pushM_THREE() {
  pushToStrip(101);
}
void pushM_FOUR() {
  pushToStrip(113);
}
void pushIT_IS()  {
  constexpr int ids[] = {1, 2, 3, 5, 6};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushAFTER() {
  constexpr int ids[] = {36, 37, 38, 39};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushQUATER() {
  constexpr int ids[] = {30, 31, 32, 33, 34};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushFOR() {
  constexpr int ids[] = {41, 42, 43, 44};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushHALF() {
  constexpr int ids[] = {50, 51, 52, 53};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushONE()  {
  constexpr int ids[] = {63, 64, 65};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushTWO() {
  constexpr int ids[] = {64, 65, 66, 67};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushTHREE() {
  constexpr int ids[] = {109, 110, 111, 112};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushFOUR() {
  constexpr int ids[] = {57, 58, 59, 60};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushFIVE1() {
  constexpr int ids[] = {8, 9, 10, 11};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushFIVE2() {
  constexpr int ids[] = {92, 93, 94, 95};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushSIX() {
  constexpr int ids[] = {69, 88, 91};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushSEVEN() {
  constexpr int ids[] = {69, 70, 71, 72, 73};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushEIGHT() {
  constexpr int ids[] = {97, 98, 99, 100};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushNINE() {
  constexpr int ids[] = {73, 74, 75, 76, 77};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushTEN() {
  constexpr int ids[] = {54, 59, 76, 81};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushTEN1() {
  constexpr int ids[] = {25, 26, 27, 28};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushELEVEN() {
  constexpr int ids[] = {107, 108, 109};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushTWELVE() {
  constexpr int ids[] = {82, 83, 84, 85, 86, 87};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}
void pushTWENTY() {
  constexpr int ids[] = {16, 17, 18, 19, 20, 21, 22};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}

void pushHOURE() {
  constexpr int ids[] = {102,103,104};
  constexpr size_t count = NR_ELEMENTS(ids);
  pushToStrip(ids, count);
}

#endif // USES_P041
