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
#define PLUGIN_NAME_148       "HT16K33"

byte* Plugin_148_DMXBuffer = 0;
int Plugin_148_DMXSize = 32;

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
    Clear();
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

  void Transmit(void)
  {
    // Display Memory
    Wire.beginTransmission(_addr);
    Wire.write(0); // start data at address 0
    for (byte i=0; i<8; i++)
    {
      Wire.write(_buffer[i] & 0xFF);
      Wire.write(_buffer[i] >> 8);
    }
    Wire.endTransmission();
  };

  void Keys(void)
  {
    // Display Memory
    Wire.beginTransmission(_addr);
    Wire.write(0x40); // start data at address 0x40
    Wire.endTransmission();

    Wire.requestFrom(_addr, 6);
    if (Wire.available() == 6)
    {
      for (byte i=0; i<3; i++)
      {
        _keys[i] = Wire.read() | (Wire.read() << 8);
      }
      Wire.endTransmission();
    }

    for (byte i=0; i<3; i++)
    {
      byte mask = 1;
      for (byte k=0; k<12; k++)
      {
        if (_keys[i] & mask)
        {
          _keydown = 16*(i+1) + (k+1);
          return;
        }
        mask <<= 1;
      }
    }
    _keydown = 0;
  };

  void Clear(void)
  {
    for (byte i=0; i<8; i++)
      _buffer[i] = 0;
  };

  void SetRow(uint8_t row, uint16_t data)
  {
    if (row < 8)
      _buffer[row] = data;
  };

  uint16_t GetRow(uint8_t row)
  {
    if (row < 8)
      return _buffer[row];
    else
      return 0;
  };

  uint8_t _addr;
  uint16_t _buffer[8];
  uint16_t _keys[3];
  byte _keydown;
};

CHT16K33* Plugin_148_M = NULL;

PROGMEM static const uint8_t diits[] =
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
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].TimerOptional = false;
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



        Settings.TaskDevicePin1[event->TaskIndex] = 2;
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = Plugin_148_DMXSize;
        addFormNote(string, F("Only GPIO-2 (D4) can be used as TX1!"));
        addFormNumericBox(string, F("Channels"), F("channels"), Plugin_148_DMXSize, 1, 512);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("i2c_addr"));

        Plugin_148_DMXSize = getFormItemInt(F("channels"));
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

        if (command == F("num"))
        {
          String number = parseString(string, 2);
          byte row = 0;

          Plugin_148_M->Clear();
          while (number[row] && row < 8)
          {
            int value = number[row];
            if (value >= 'A')
              value -= 'A' - 10;
            else
              value -= '0';
            value = diits[value & 0xF];
            Plugin_148_M->SetRow(row, value);
            row++;
          }
          Plugin_148_M->Transmit();
        }

        else if (command == F("mx") || command == F("seg7"))
        {
          String param;
          String paramKey;
          String paramVal;
          byte paramIdx = 2;
          int row = 0;
          int value = 0;

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
                for (int i = 0; i < 8; i++)
                {
                  log += String(Plugin_148_M->GetRow(i), 16);
                  log += F("h, ");
                }
                addLog(LOG_LEVEL_INFO, log);
                success = true;
              }

              else if (param == F("test"))
              {
                for (int i = 0; i < 8; i++)
                  Plugin_148_M->SetRow(i, 1 << i);
                success = true;
              }

              else if (param == F("clear"))
              {
                Plugin_148_M->Clear();
                success = true;
              }

              else
              {
                int index = param.indexOf('=');
                if (index > 0)   //syntax: "<row>=<value>"
                {
                  paramKey = param.substring(0, index);
                  paramVal = param.substring(index+1);
                  row = paramKey.toInt();
                }
                else   //syntax: "<value>"
                {
                  paramVal = param;
                }

                value = paramVal.toInt();

                if (command == F("seg7"))
                {
                  if (value < 16)
                    value = diits[value];
                  else
                    value = 0;
                }

                if (row >= 0 && row < 8)
                  Plugin_148_M->SetRow(row, value);
                success = true;
                row++;
              }

              param = parseString(string, paramIdx++);
            }
          }
          else
          {
            //??? no params
          }

          if (success)
            Plugin_148_M->Transmit();
          success = true;
        }

        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_148_DMXBuffer)
        {

        }
        break;
      }

    case PLUGIN_READ:
      {
        //no values
        success = true;
        break;
      }

  }
  return success;
}

#endif
