#include "_Plugin_Helper.h"
#ifdef USES_P055
//#######################################################################################################
//#################################### Plugin 055: Chiming Mechanism ####################################
//#######################################################################################################

// ESPEasy plugin to strike up to 4 physical bells and gongs with chiming sequences.
// You also can use an antique door bell as a single strikes (not ringing) notification.
// Optional you can use it as hourly chiming clock
// written by Jochen Krapf (jk@nerd2nerd.org)

// List of commands:
// (1) CHIME,<tokens>             Play token direct
// (2) CHIMESAVE,<name>,<tokens>  Save tokens with given name in FFS
// (3) CHIMEPLAY,<name>           Play saved tokens given name out of FFS

// List of tokens:
// (a) '1'...'9', 'A'...'F'       Bell number - 1=1st bell, 2=2nd bell, 4=3rd bell, 8=4th bell, numbers can be added to strike simultaniouly, F=all bells
// (b) '!'                        Double strike prev. token
// (c) '-' or ' '                 Normal Pause
// (d) '='                        Long Pause (3 times normal)
// (e) '.'                        Short Pause (1/3 of normal)
// (f) '|'                        Shortest Pause
// (g) '#'                        Comment - rest of the tokens will be ignored
// Note: If no pause is specified, a normal pause will be inserted "111" -> "1-1-1"

// Usage as Hourly Chime Clock:
// save twelve comma separated tokens with name "hours", enable checkbox "Hourly Chiming Clock Strike" in plugin web interface and enable NTP (advanced settings)
//
// examples:
// Historical coded with 1 bell:              "1,11,111,1111,11111,111111,1111111,11111111,111111111,1111111111,11111111111,111111111111"
// Binary coded with 2 bells (2nd bell=1):    "1112,1121,1122,1211,1212,1221,1222,2111,2112,2121,2122,2211"
// Binary coded with 1 bell (short pause=1):  "1_1_1_11,1_1_11_1,1_1_111,1_11_1_1,1_11_11,1_111_1,1_1111,11_1_1_1,11_1_11,11_11_1,11_111,111_1_1"
// Binary coded with 1 bell (double strike=1):"1111!,111!1,111!1!,11!11,11!11!,11!1!1,11!1!1!,1!111,1!111!,1!11!1,1!11!1!,1!1!11"
//
// CHIMESAVE,hours,1111!,111!1,111!1!,11!11,11!11!,11!1!1,11!1!1!,1!111,1!111!,1!11!1,1!11!1!,1!1!11

// Usage as Alarm Clock:
// save tokens with name "<HH><MM>" and enable NTP (advanced settings)
//
// examples:
// CHIMESAVE,0815,1111!           Daily Alarm at 8:15am
// CHIMESAVE,2015,11121           Daily Alarm at 8:15pm
// CHIMESAVE,2015                 Delete Alarm at 8:15pm

// Electronics:
// Use a power-FET or an ULN2003 to switch on the bells coil with 12 or 24 volts



//#include <*.h>   - no external lib required

#include "src/WebServer/Markup_Buttons.h"

#define PLUGIN_055
#define PLUGIN_ID_055         55
#define PLUGIN_NAME_055       "Notify - Chiming [TESTING]"

#define PLUGIN_055_FIFO_SIZE 64   // must be power of 2
#define PLUGIN_055_FIFO_MASK (PLUGIN_055_FIFO_SIZE-1)

class CPlugin_055_Data
{
public:
  long millisStateEnd;
  long millisChimeTime;
  long millisPauseTime;

  int pin[4];
  byte lowActive;
  byte chimeClock;

  char FIFO[PLUGIN_055_FIFO_SIZE];
  byte FIFO_IndexR;
  byte FIFO_IndexW;

  void Plugin_055_Data()
  {
    millisStateEnd = 0;
    millisChimeTime = 60;
    millisPauseTime = 400;

    for (byte i=0; i<4; i++)
      pin[i] = -1;
    lowActive = false;
    chimeClock = true;

    FIFO_IndexR = 0;
    FIFO_IndexW = 0;
  }
};

static CPlugin_055_Data* Plugin_055_Data = NULL;


boolean Plugin_055(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_055;
        Device[deviceCount].Type = DEVICE_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_NONE;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = true;
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
        string = F(PLUGIN_NAME_055);
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("Driver#1"));
        event->String2 = formatGpioName_output(F("Driver#2"));
        event->String3 = formatGpioName_output(F("Driver#4"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        //default values
        if (PCONFIG(0) <= 0)   //Plugin_055_millisChimeTime
          PCONFIG(0) = 60;
        if (PCONFIG(1) <= 0)   //Plugin_055_millisPauseTime
          PCONFIG(1) = 400;

        // FIXME TD-er: Should we add support for 4 pin definitions?
        addFormPinSelect(formatGpioName_output(F("Driver#8")), F("TDP4"), (int)(Settings.TaskDevicePin[3][event->TaskIndex]));

        addFormSubHeader(F("Timing"));

        addFormNumericBox(F("Chiming/Strike Time (ct)"), F("chimetime"), PCONFIG(0));
        addUnit(F("ms"));

        addFormNumericBox(F("Normal Pause Time (t)"), F("pausetime"), PCONFIG(1));
        addUnit(F("ms"));

        addFormNote(F("'1=1'&rArr;3t, '1-1' or '11'&rArr;1t, '1.1'&rArr;&#8531;t, '1|1'&rArr;&frac12;ct"));


        addFormSubHeader(F("Chiming Clock"));

        addFormCheckBox(F("Hourly Chiming Clock Strike"), F("chimeclock"), PCONFIG(2));
        //addHtml(F("<TR><TD><TD>"));
        addButton(F("'control?cmd=chimeplay,hours'"), F("Test 1&hellip;12"));

        if (PCONFIG(2) && !Settings.UseNTP)
          addFormNote(F("Enable and configure NTP!"));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePin[3][event->TaskIndex] = (int8_t)getFormItemInt(F("TDP4"));

        PCONFIG(0) = getFormItemInt(F("chimetime"));
        PCONFIG(1) = getFormItemInt(F("pausetime"));
        PCONFIG(2) = isFormItemChecked(F("chimeclock"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!Plugin_055_Data)
          Plugin_055_Data = new CPlugin_055_Data();

        Plugin_055_Data->lowActive = Settings.TaskDevicePin1Inversed[event->TaskIndex];
        Plugin_055_Data->millisChimeTime = PCONFIG(0);
        Plugin_055_Data->millisPauseTime = PCONFIG(1);
        Plugin_055_Data->chimeClock = PCONFIG(2);

        String log = F("Chime: GPIO: ");
        for (byte i=0; i<4; i++)
        {
          int pin = Settings.TaskDevicePin[i][event->TaskIndex];
          Plugin_055_Data->pin[i] = pin;
          if (pin >= 0)
          {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, Plugin_055_Data->lowActive);
          }
          log += pin;
          log += ' ';
        }
        if (Plugin_055_Data->lowActive)
          log += F("!");
        addLog(LOG_LEVEL_INFO, log);

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (!Plugin_055_Data)
          break;

        String command = parseString(string, 1);

        if (command == F("chime"))
        {
          String param = parseStringToEndKeepCase(string, 2);
          if (param.length() > 0) {
            Plugin_055_AddStringFIFO(param);
          }
          success = true;
        }
        if (command == F("chimeplay"))
        {
          String name = parseString(string, 2);
          if (name.length() > 0) {
            String param;
            Plugin_055_ReadChime(name, param);
            Plugin_055_AddStringFIFO(param);
          }
          success = true;
        }
        if (command == F("chimesave"))
        {
          String name = parseString(string, 2);
          String param = parseStringToEndKeepCase(string, 3);
          if (name.length() > 0 && param.length() > 0) {
            Plugin_055_WriteChime(name, param);
            Plugin_055_AddStringFIFO("1");
          }
          success = true;
        }

        break;
      }

      case PLUGIN_CLOCK_IN:
        {
          if (!Plugin_055_Data)
            break;

          String tokens = "";
          byte hours = node_time.hour();
          byte minutes = node_time.minute();

          if (Plugin_055_Data->chimeClock)
          {
            char tmpString[8] = {0};

            sprintf_P(tmpString, PSTR("%02d%02d"), hours, minutes);
            if (Plugin_055_ReadChime(tmpString, tokens))
              Plugin_055_AddStringFIFO(tokens);

            if (minutes == 0)
            {
              if (Plugin_055_ReadChime("hours", tokens) == 0)
                tokens = F("1111!,111!1,111!1!,11!11,11!11!,11!1!1,11!1!1!,1!111,1!111!,1!11!1,1!11!1!,1!1!11");   //1..12

              // hours 0..23 -> 1..12
              hours = hours % 12;
              if (hours == 0)
                hours = 12;

              byte index = hours;

              tokens = parseString(tokens, index);
              Plugin_055_AddStringFIFO(tokens);
            }
          }

          success = true;
          break;
        }


    case PLUGIN_FIFTY_PER_SECOND:
    //case PLUGIN_TEN_PER_SECOND:
      {
        if (!Plugin_055_Data)
          break;

        long millisAct = millis();

        if (Plugin_055_Data->millisStateEnd > 0)   // just striking?
        {
          if (timeDiff(millisAct, Plugin_055_Data->millisStateEnd) <= 0)   // end reached?
          {
            for (byte i=0; i<4; i++)
            {
              if (Plugin_055_Data->pin[i] >= 0)
                digitalWrite(Plugin_055_Data->pin[i], Plugin_055_Data->lowActive);
            }
            Plugin_055_Data->millisStateEnd = 0;
          }
        }

        if (Plugin_055_Data->millisStateEnd == 0)   // just finished?
        {
          if (! Plugin_055_IsEmptyFIFO())
          {
            char c = Plugin_055_ReadFIFO();

            String log = F("Chime: Process '");
            log += c;
            log += '\'';
            addLog(LOG_LEVEL_DEBUG, log);

            switch (c)
            {
              case 'a':
              case 'b':
              case 'c':
              case 'd':
              case 'e':
              case 'f':
              case 'A':
              case 'B':
              case 'C':
              case 'D':
              case 'E':
              case 'F':
                c -= 'A' - '0' - 10;
                //vvv

              case '0':   //strikes 1=1st bell, 2=2nd bell, 4=3rd bell, 8=4rd bell
              case '1':
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7':
              case '8':
              case '9':
              {
                byte mask = 1;
                for (byte i=0; i<4; i++)
                {
                  if (Plugin_055_Data->pin[i] >= 0)
                    if (c & mask)
                      digitalWrite(Plugin_055_Data->pin[i], !Plugin_055_Data->lowActive);
                  mask <<= 1;
                }
                Plugin_055_Data->millisStateEnd = millisAct + Plugin_055_Data->millisChimeTime;
                break;
              }
              case '=':   //long pause
              case ' ':
              case ',':
                Plugin_055_Data->millisStateEnd = millisAct + Plugin_055_Data->millisPauseTime*3;
                break;
              case '-':   //single pause
                Plugin_055_Data->millisStateEnd = millisAct + Plugin_055_Data->millisPauseTime;
                break;
              case '.':   //short pause
                Plugin_055_Data->millisStateEnd = millisAct + Plugin_055_Data->millisPauseTime/3;
                break;
              case '|':   //shortest pause
                Plugin_055_Data->millisStateEnd = millisAct + Plugin_055_Data->millisChimeTime/2;
                break;
              case '#':   //comment -> eat till FIFO is empty
                while (Plugin_055_ReadFIFO());
                break;
              default:   //unknown char -> do nothing
                break;
            }
          }

        }
        success = true;
        break;
      }

  }
  return success;
}

// FIFO functions

void Plugin_055_WriteFIFO(char c)
{
  if (Plugin_055_Data->FIFO_IndexR == ((Plugin_055_Data->FIFO_IndexW+1) & PLUGIN_055_FIFO_MASK))   // FIFO full?
    return;

  Plugin_055_Data->FIFO[Plugin_055_Data->FIFO_IndexW] = c;
  Plugin_055_Data->FIFO_IndexW++;
  Plugin_055_Data->FIFO_IndexW &= PLUGIN_055_FIFO_MASK;
}

char Plugin_055_ReadFIFO()
{
  if (Plugin_055_IsEmptyFIFO())
    return '\0';

  char c = Plugin_055_Data->FIFO[Plugin_055_Data->FIFO_IndexR];
  Plugin_055_Data->FIFO_IndexR++;
  Plugin_055_Data->FIFO_IndexR &= PLUGIN_055_FIFO_MASK;

  return c;
}

char Plugin_055_PeekFIFO()
{
  if (Plugin_055_IsEmptyFIFO())
    return '\0';

  return Plugin_055_Data->FIFO[Plugin_055_Data->FIFO_IndexR];
}

boolean Plugin_055_IsEmptyFIFO()
{
  return (Plugin_055_Data->FIFO_IndexR == Plugin_055_Data->FIFO_IndexW);
}

void Plugin_055_AddStringFIFO(const String& param)
{
  if (param.length() == 0)
    return;

  byte i = 0;
  char c = param[i];
  char c_last = '\0';

  while (c != 0)
  {
    if (isDigit(c) && isDigit(c_last))   // "11" is shortcut for "1-1" -> add pause
      Plugin_055_WriteFIFO('-');
    if (c == '!')   //double strike -> add shortest pause and repeat last strike
    {
      Plugin_055_WriteFIFO('|');
      c = c_last;
    }
    Plugin_055_WriteFIFO(c);
    c_last = c;

    c = param[++i];
  }

  Plugin_055_WriteFIFO('=');
}

//File I/O functions

void Plugin_055_WriteChime(const String& name, const String& tokens)
{
  String fileName = F("chime_");
  fileName += name;
  fileName += F(".txt");

  String log = F("Chime: write ");
  log += fileName;
  log += ' ';

  fs::File f = tryOpenFile(fileName, "w");
  if (f)
  {
    f.print(tokens);
    f.close();
    //flashCount();
    log += tokens;
  }

  addLog(LOG_LEVEL_INFO, log);
}

byte Plugin_055_ReadChime(const String& name, String& tokens)
{
  String fileName = F("chime_");
  fileName += name;
  fileName += F(".txt");

  String log = F("Chime: read ");
  log += fileName;
  log += ' ';

  tokens = "";
  fs::File f = tryOpenFile(fileName, "r");
  if (f)
  {
    tokens.reserve(f.size());
    char c;
    while (f.available())
    {
      c = f.read();
      tokens += c;
    }
    f.close();

    log += tokens;
  }

  addLog(LOG_LEVEL_INFO, log);

  return tokens.length();
}

#endif // USES_P055
