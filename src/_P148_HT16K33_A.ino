//######################################## HT16K33 ####################################################
//#######################################################################################################

// ESPEasy Plugin to control a 16x8 LED matrix chip HT16K33
// written by Jochen Krapf (jk@nerd2nerd.org)

// List of commands:
// (1) ???,<param>
// (2) DMX,<param>,<param>,<param>, ...

// List of DMX params:
// (a) <value>
//     DMX-value (0...255) to write to the next channel address (1...512) starting with 1

// Examples:
// DMX,123"   Set channel 1 to value 123


//#include <*.h>   //no lib needed

#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_148
#define PLUGIN_ID_148         148
#define PLUGIN_NAME_148       "LED - HT16K33"


class CHT16K33 {
 public:
  CHT16K33(void) {};

  void Init(uint8_t addr)
  {
    _addr = addr;

    // System Setup Register
    Wire.beginTransmission(_addr);
    Wire.write(0x21);  // oscillator on
    Wire.endTransmission();

    // Display Setup Register
    Wire.beginTransmission(_addr);
    Wire.write(0x81);  // blink off; display on
    Wire.endTransmission();

    SetBrightness(15);
    ClearRowBuffer();
    TransmitRowBuffer();
  };

  void SetBrightness(uint8_t b)
  {
    if (b > 15)
      b = 15;
    // Digital Dimming Data Input
    Wire.beginTransmission(_addr);
    Wire.write(0xE0 | b);  // brightness
    Wire.endTransmission();
  };

  void TransmitRowBuffer(void)
  {
    // Display Memory
    Wire.beginTransmission(_addr);
    Wire.write(0); // start data at address 0
    for (byte i=0; i<8; i++)
    {
      Wire.write(_rowBuffer[i] & 0xFF);
      Wire.write(_rowBuffer[i] >> 8);
    }
    Wire.endTransmission();
  };

  void ClearRowBuffer(void)
  {
    for (byte i=0; i<8; i++)
      _rowBuffer[i] = 0;
  };

  void SetRow(uint8_t com, uint16_t data)
  {
    if (com < 8)
      _rowBuffer[com] = data;
  };

  uint16_t GetRow(uint8_t com)
  {
    if (com < 8)
      return _rowBuffer[com];
    else
      return 0;
  };

  void SetDigit(uint8_t com, uint8_t c)
  {
    uint16_t value = 0;

    if (c <= 0xF)
      value = _digits[c];

    switch (c)
    {
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
        value = c + 10 - 'A';
        value = _digits[value & 0xF];
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        value = c - '0';
        value = _digits[value & 0xF];
        break;
      case ' ':
        value = 0;
        break;
      case ':':
        value = 0x02;   //special for China 4 x 7-seg big display
        break;
      case '-':
        value = 0x40;
        break;
    }

    SetRow(com, value);
  }

uint8_t ReadKeys(void)
  {
    // Display Memory
    Wire.beginTransmission(_addr);
    Wire.write(0x40); // start data at address 0x40
    Wire.endTransmission();

    Wire.requestFrom(_addr, (uint8_t)6);
    if (Wire.available() == 6)
    {
      for (byte i=0; i<3; i++)
      {
        _keyBuffer[i] = Wire.read() | (Wire.read() << 8);
      }
      Wire.endTransmission();
    }

    for (byte i=0; i<3; i++)
    {
      byte mask = 1;
      for (byte k=0; k<12; k++)
      {
        if (_keyBuffer[i] & mask)
        {
          _keydown = 16*(i+1) + (k+1);
          return _keydown;
        }
        mask <<= 1;
      }
    }
    _keydown = 0;
    return _keydown;
  };

protected:
  uint8_t _addr;
  uint16_t _rowBuffer[8];
  uint16_t _keyBuffer[3];
  byte _keydown;

  static const uint8_t _digits[16];
};

CHT16K33* Plugin_148_M = NULL;

const uint8_t CHT16K33::_digits[16] =
{
  0x3F,   // 0
  0x06,   // 1
  0x5B,   // 2
  0x4F,   // 3
  0x66,   // 4
  0x6D,   // 5
  0x7D,   // 6
  0x07,   // 7
  0x7F,   // 8
  0x6F,   // 9
  0x77,   // A
  0x7C,   // B
  0x39,   // C
  0x5E,   // D
  0x79,   // E
  0x71,   // F
};


boolean Plugin_148(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_148;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_148);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte addr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        int optionValues[8] = { 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77 };
        addFormSelectorI2C(string, F("i2c_addr"), 8, optionValues, addr);

        addFormCheckBox(string, F("Scan Keys"), F("usekeys"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);

        //Settings.TaskDevicePin1[event->TaskIndex] = 2;
        //Settings.TaskDevicePluginConfig[event->TaskIndex][0] = Plugin_148_DMXSize;
        //addFormNote(string, F("Only GPIO-2 (D4) can be used as TX1!"));
        //addFormNumericBox(string, F("Channels"), F("channels"), Plugin_148_DMXSize, 1, 512);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("i2c_addr"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = isFormItemChecked(F("usekeys"));

        //Plugin_148_DMXSize = getFormItemInt(F("channels"));
        //Limit (Plugin_148_DMXSize, 1, 512);
        //Settings.TaskDevicePluginConfig[event->TaskIndex][0] = Plugin_148_DMXSize;
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        byte addr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        if (!Plugin_148_M)
          Plugin_148_M = new CHT16K33;

        Plugin_148_M->Init(addr);

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (!Plugin_148_M)
          return false;

        string.toLowerCase();
        String command = parseString(string, 1);

        if (command == F("mprint"))
        {
          int paramPos = getParamStartPos(string, 2);
          String text = string.substring(paramPos);
          byte seg = 0;

          Plugin_148_M->ClearRowBuffer();
          while (text[seg] && seg < 8)
          {
            uint16_t value = 0;
            char c = text[seg];
            Plugin_148_M->SetDigit(seg, c);
            seg++;
          }
          Plugin_148_M->TransmitRowBuffer();
          success = true;
        }

        else if (command == F("m") || command == F("mx") || command == F("num"))
        {
          String param;
          String paramKey;
          String paramVal;
          byte paramIdx = 2;
          uint8_t seg = 0;
          uint16_t value = 0;

          string.replace("  ", " ");
          string.replace(" =", "=");
          string.replace("= ", "=");

          param = parseString(string, paramIdx++);
          if (param.length())
          {
            while (param.length())
            {
              addLog(LOG_LEVEL_DEBUG_MORE, param);

              if (param == F("log"))
              {
                String log = F("MX   : ");
                for (byte i = 0; i < 8; i++)
                {
                  log += String(Plugin_148_M->GetRow(i), 16);
                  log += F("h, ");
                }
                addLog(LOG_LEVEL_INFO, log);
                success = true;
              }

              else if (param == F("test"))
              {
                for (byte i = 0; i < 8; i++)
                  Plugin_148_M->SetRow(i, 1 << i);
                success = true;
              }

              else if (param == F("clear"))
              {
                Plugin_148_M->ClearRowBuffer();
                success = true;
              }

              else
              {
                int index = param.indexOf('=');
                if (index > 0)   //syntax: "<seg>=<value>"
                {
                  paramKey = param.substring(0, index);
                  paramVal = param.substring(index+1);
                  seg = paramKey.toInt();
                }
                else   //syntax: "<value>"
                {
                  paramVal = param;
                }

                if (command == F("num"))
                {
                  value = paramVal.toInt();
                  if (value < 16)
                    Plugin_148_M->SetDigit(seg, value);
                  else
                    Plugin_148_M->SetRow(seg, value);
                }
                else if (command == F("mx"))
                {
                  char* ep;
                  value = strtol(paramVal.c_str(), &ep, 16);
                  Plugin_148_M->SetRow(seg, value);
                }
                else
                {
                  value = paramVal.toInt();
                  Plugin_148_M->SetRow(seg, value);
                }

                success = true;
                seg++;
              }

              param = parseString(string, paramIdx++);
            }
          }
          else
          {
            //??? no params
          }

          if (success)
            Plugin_148_M->TransmitRowBuffer();
          success = true;
        }

        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_148_M && Settings.TaskDevicePluginConfig[event->TaskIndex][1])
        {

        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (Plugin_148_M && Settings.TaskDevicePluginConfig[event->TaskIndex][1])
        {
          uint8_t key = Plugin_148_M->ReadKeys();
          UserVar[event->BaseVarIndex] = (float)key;

          if (1)
          {
            event->sensorType = SENSOR_TYPE_SWITCH;

            String log = F("M    : key=");
            log += key;
            addLog(LOG_LEVEL_INFO, log);

            sendData(event);
          }

        }
        success = true;
        break;
      }

  }
  return success;
}

#endif
