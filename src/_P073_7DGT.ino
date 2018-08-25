#ifdef USES_P073
//#######################################################################################################
//###################   Plugin 073 - 7-segment display plugin TM1637/MAX7219       ######################
//#######################################################################################################
//
// Chips/displays supported:
//  0 - TM1637     -- 2 pins - 4 digits and colon in the middle (XX:XX)
//  1 - TM1637     -- 2 pins - 4 digits and dot on each digit (X.X.X.X.)
//  2 - TM1637     -- 2 pins - 6 digits and dot on each digit (X.X.X.X.X.X.)
//  3 - MAX7219/21 -- 3 pins - 8 digits and dot on each digit (X.X.X.X.)
//
// Plugin can be setup as:
//  - Manual        -- display is manually updated sending commands
//                     "7dn,<number>"      (number can be negative or positive, even with decimal)
//                     "7dt,<temperature>" (temperature can be negative or positive and containing decimals)
//  - Clock-Blink   -- display is automatically updated with current time and blinking dot/lines
//  - Clock-NoBlink -- display is automatically updated with current time and steady dot/lines
//  - Date          -- display is automatically updated with current date
//
// Generic commands:
//  - "7don"      -- turn ON the display
//  - "7doff"     -- turn OFF the display
//  - "7db,<0-15> -- set brightness to specific value between 0 and 15
//


#define PLUGIN_073
#define PLUGIN_ID_073        73
#define PLUGIN_NAME_073      "Display - 7-segment display [TESTING]"
#define PLUGIN_073_DEBUG     false    //activate extra log info in the debug

//---------------------------------------------------
// Class used by plugin
//---------------------------------------------------
class p073_7dgt
{
  public:
    uint8_t pin1, pin2, pin3;
    byte type;
    byte output;
    byte brightness;
    boolean timesep;
};
p073_7dgt *Plugin_073_7dgt = NULL;
//---------------------------------------------------

uint8_t p073_showbuffer[8];
byte    p073_spidata[2];
byte    p073_dotpos;
bool    p073_shift;

#define TM1637_POWER_ON   B10001000
#define TM1637_POWER_OFF  B10000000
#define TM1637_CLOCKDELAY 40
#define TM1637_4DIGIT     4
#define TM1637_6DIGIT     2

// each char table is specific for each display and maps all numbers/symbols needed:
//   - pos 0-9  - Numbers from 0 to 9
//   - pos 10   - Space " "
//   - pos 11   - minus symbol "-"
//   - pos 12   - degree symbol "°"
const PROGMEM byte CharTableTM1637  [13] = {B00111111,B00000110,B01011011,B01001111,B01100110,B01101101,B01111101,B00000111,B01111111,B01101111,B00000000,B01000000,B01100011};
const PROGMEM byte CharTableMAX7219 [13] = {B01111110,B00110000,B01101101,B01111001,B00110011,B01011011,B01011111,B01110000,B01111111,B01111011,B00000000,B00000001,B01100011};

boolean Plugin_073(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_073;
        Device[deviceCount].Type = DEVICE_TYPE_TRIPLE;
        Device[deviceCount].VType = SENSOR_TYPE_NONE;
        Device[deviceCount].Ports = 0;
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
        string = F(PLUGIN_NAME_073);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormNote(F("TM1637:  1st=CLK-Pin, 2nd=DIO-Pin"));
        addFormNote(F("MAX7219: 1st=DIN-Pin, 2nd=CLK-Pin, 3rd=CS-Pin"));
        String displtype[5] = { F("TM1637 - 4 digit (colon)"), F("TM1637 - 4 digit (dots)"), F("TM1637 - 6 digit"), F("MAX7219 - 8 digit")};
        addFormSelector(F("Display Type"), F("plugin_073_displtype"), 4, displtype, NULL, Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        String displout[4] = { F("Manual"), F("Clock - Blink"), F("Clock - No Blink"), F("Date")  };
        addFormSelector(F("Display Output"), F("plugin_073_displout"), 4, displout, NULL, Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        addFormNumericBox(F("Brightness"), F("plugin_073_brightness"), Settings.TaskDevicePluginConfig[event->TaskIndex][2], 0, 15);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_073_displtype"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_073_displout"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_073_brightness"));
        if (Plugin_073_7dgt) {
          // dat_pin:
          Plugin_073_7dgt->pin1 = Settings.TaskDevicePin1[event->TaskIndex];
          // clk_pin:
          Plugin_073_7dgt->pin2 = Settings.TaskDevicePin2[event->TaskIndex];
          // cs_pin:
          Plugin_073_7dgt->pin3 = Settings.TaskDevicePin3[event->TaskIndex];
          Plugin_073_7dgt->type = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
          Plugin_073_7dgt->output = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
          Plugin_073_7dgt->brightness = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
          Plugin_073_7dgt->timesep = true;
          switch (Settings.TaskDevicePluginConfig[event->TaskIndex][0])
          {
            case 0:    // set brightness of TM1637
            case 1:
            case 2:
              {
                int tm1637_bright = Settings.TaskDevicePluginConfig[event->TaskIndex][2] / 2;
                tm1637_SetPowerBrightness(tm1637_bright, true);
                if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == 0)
                  tm1637_ClearDisplay();
                break;
              }
            case 3:    // set brightness of MAX7219
              {
                max7219_SetPowerBrightness(Settings.TaskDevicePluginConfig[event->TaskIndex][2], true);
                if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == 0)
                  max7219_ClearDisplay();
                break;
              }
          }
        }
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!Plugin_073_7dgt) {
          Plugin_073_7dgt = new p073_7dgt;
            Plugin_073_7dgt->pin1 = Settings.TaskDevicePin1[event->TaskIndex];
            Plugin_073_7dgt->pin2 = Settings.TaskDevicePin2[event->TaskIndex];
            Plugin_073_7dgt->pin3 = Settings.TaskDevicePin3[event->TaskIndex];
            Plugin_073_7dgt->type = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
            Plugin_073_7dgt->output = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
            Plugin_073_7dgt->brightness = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
          switch (Settings.TaskDevicePluginConfig[event->TaskIndex][0])
          {
            case 0:
            case 1:
            case 2:
              {
                tm1637_InitDisplay();
                int tm1637_bright = Settings.TaskDevicePluginConfig[event->TaskIndex][2] / 2;
                tm1637_SetPowerBrightness(tm1637_bright, true);
                break;
              }
            case 3:
              {
                max7219_InitDisplay();
                max7219_SetPowerBrightness(Settings.TaskDevicePluginConfig[event->TaskIndex][2], true);
                break;
              }
          }
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (!Plugin_073_7dgt)
          break;

        String tmpString  = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex)
          tmpString = tmpString.substring(0, argIndex);

        String tmpStr = string;
        int comma1 = tmpStr.indexOf(',');

        if (tmpString.equalsIgnoreCase(F("7dn"))) {
          if (Plugin_073_7dgt->output != 0)
            break;
          String log = F("7DGT : Show Number=");
          log += event->Par1;
          addLog(LOG_LEVEL_INFO, log);
          switch (Plugin_073_7dgt->type)
          {
            case 0:
              if (event->Par1 > -1000 && event->Par1 < 10000)
                p073_FillBufferWithNumber(String(int(event->Par1)));
              else
                p073_FillBufferWithDash();
              tm1637_ShowBuffer(TM1637_4DIGIT);
              break;
            case 1:
            {
              if (event->Par1 > -1000 && event->Par1 < 10000)
                p073_FillBufferWithNumber(tmpStr.substring(comma1+1).c_str());
              else
                p073_FillBufferWithDash();
              tm1637_ShowBuffer(TM1637_4DIGIT);
              break;
            }
            case 2:
            {
              if (event->Par1 > -100000 && event->Par1 < 1000000)
                p073_FillBufferWithNumber(tmpStr.substring(comma1+1).c_str());
              else
                p073_FillBufferWithDash();
              tm1637_SwapDigitInBuffer();     // only needed for 6-digits displays
              tm1637_ShowBuffer(TM1637_6DIGIT);
              break;
            }
            case 3:
            {
              if (comma1 > 0) {
                if (event->Par1 > -10000000 && event->Par1 < 100000000) {
                  p073_FillBufferWithNumber(tmpStr.substring(comma1+1).c_str());
                }
                else
                  p073_FillBufferWithDash();
                max7219_ShowBuffer();
              }
              break;
            }
          }
          success = true;
        } else if (tmpString.equalsIgnoreCase(F("7dt"))) {
          if (Plugin_073_7dgt->output != 0)
            break;
          double p073_temptemp = 0;
          bool p073_tempflagdot = false;
          if (comma1 > 0)
            p073_temptemp  = atof(tmpStr.substring(comma1+1).c_str());
          String log = F("7DGT : Show Temperature=");
          log += p073_temptemp;
          addLog(LOG_LEVEL_INFO, log);
          switch (Plugin_073_7dgt->type)
          {
            case 0:
            case 1:
            {
              if (p073_temptemp > 999 || p073_temptemp < -99.9)
                p073_FillBufferWithDash();
              else {
                if (p073_temptemp < 100 && p073_temptemp > -10) {
                  p073_temptemp = int(p073_temptemp*10);
                  p073_tempflagdot = true;
                }
                p073_FillBufferWithTemp(p073_temptemp);
                if (p073_temptemp == 0 && p073_tempflagdot)
                  p073_showbuffer[5] = 0;
              }
              tm1637_ShowTimeTemp4(p073_tempflagdot, 4);
              break;
            }
            case 2:
            {
              if (p073_temptemp > 999 || p073_temptemp < -99.9)
                p073_FillBufferWithDash();
              else {
                if (p073_temptemp < 100 && p073_temptemp > -10) {
                  p073_temptemp = int(p073_temptemp*10);
                  p073_tempflagdot = true;
                }
                p073_FillBufferWithTemp(p073_temptemp);
                if (p073_temptemp == 0 && p073_tempflagdot)
                  p073_showbuffer[5] = 0;
              }
              tm1637_ShowTemp6(p073_tempflagdot);
              break;
            }
            case 3:
            {
              p073_temptemp = int(p073_temptemp*10);
              p073_FillBufferWithTemp(p073_temptemp);
              if (p073_temptemp == 0)
                p073_showbuffer[5] = 0;
              max7219_ShowTemp();
              break;
            }
          }
          success = true;
        } else {
          bool p073_validcmd = false;
          bool p073_displayon;
          if (tmpString.equalsIgnoreCase(F("7don"))) {
            String log = F("7DGT : Display ON");
            addLog(LOG_LEVEL_INFO, log);
            p073_displayon = true;
            p073_validcmd = true;
          }
          else if (tmpString.equalsIgnoreCase(F("7doff"))) {
            String log = F("7DGT : Display OFF");
            addLog(LOG_LEVEL_INFO, log);
            p073_displayon = false;
            p073_validcmd = true;
          }
          else if (tmpString.equalsIgnoreCase(F("7db"))) {
            if (event->Par1 >= 0 && event->Par1 < 16) {
              String log = F("7DGT : Brightness=");
              log += event->Par1;
              addLog(LOG_LEVEL_INFO, log);
              Plugin_073_7dgt->brightness = event->Par1;
              p073_displayon = true;
              p073_validcmd = true;
            }
          }
          if (p073_validcmd) {
            success = true;
            switch (Plugin_073_7dgt->type)
            {
              case 0:
              case 1:
              case 2:
              { int tm1637_bright = Plugin_073_7dgt->brightness / 2;
                tm1637_SetPowerBrightness(tm1637_bright, p073_displayon);
                break; }
              case 3:
              { max7219_SetPowerBrightness(Plugin_073_7dgt->brightness, p073_displayon);
                break; }
            }
          }
        }
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        if (Plugin_073_7dgt->output == 0)
          break;

        if (Plugin_073_7dgt->output == 1)
          { Plugin_073_7dgt->timesep = !Plugin_073_7dgt->timesep; }
        else
          { Plugin_073_7dgt->timesep = true; }

        if (Plugin_073_7dgt->output == 3)
          p073_FillBufferWithDate();
        else
          p073_FillBufferWithTime();

        switch (Plugin_073_7dgt->type)
        {
          case 0:
          case 1:
          {
            tm1637_ShowTimeTemp4(Plugin_073_7dgt->timesep, 0);
            break;
          }
          case 2:
          {
            if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == 3)
              tm1637_ShowDate6(Plugin_073_7dgt->timesep);
            else
              tm1637_ShowTime6(Plugin_073_7dgt->timesep);
            break;
          }
          case 3:
          {
            if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == 3)
              max7219_ShowDate();
            else
              max7219_ShowTime(Plugin_073_7dgt->timesep);
            break;
          }
        }
      }

  }
  return success;
}

void p073_FillBufferWithTime(void)
{
  memset(p073_showbuffer,0,sizeof(p073_showbuffer));
  byte sevendgt_hours = hour();
  byte sevendgt_minutes = minute();
  byte sevendgt_seconds = second();
  uint8_t p073_digit1, p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_hours / 10);
  p073_digit2 = sevendgt_hours - p073_digit1*10;
  p073_showbuffer[0] = p073_digit1; p073_showbuffer[1] = p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_minutes / 10);
  p073_digit2 = sevendgt_minutes - p073_digit1*10;
  p073_showbuffer[2] = p073_digit1; p073_showbuffer[3] = p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_seconds / 10);
  p073_digit2 = sevendgt_seconds - p073_digit1*10;
  p073_showbuffer[4] = p073_digit1; p073_showbuffer[5] = p073_digit2;
}

void p073_FillBufferWithDate(void)
{
  memset(p073_showbuffer,0,sizeof(p073_showbuffer));
  byte sevendgt_day = day();
  byte sevendgt_month = month();
  byte sevendgt_year1 = uint8_t(year()/100);
  byte sevendgt_year2 = uint8_t(year()-(sevendgt_year1*100));
  uint8_t p073_digit1, p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_day / 10);
  p073_digit2 = sevendgt_day - p073_digit1*10;
  p073_showbuffer[0] = p073_digit1; p073_showbuffer[1] = p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_month / 10);
  p073_digit2 = sevendgt_month - p073_digit1*10;
  p073_showbuffer[2] = p073_digit1; p073_showbuffer[3] = p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_year1 / 10);
  p073_digit2 = sevendgt_year1 - p073_digit1*10;
  p073_showbuffer[4] = p073_digit1; p073_showbuffer[5] = p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_year2 / 10);
  p073_digit2 = sevendgt_year2 - p073_digit1*10;
  p073_showbuffer[6] = p073_digit1; p073_showbuffer[7] = p073_digit2;
}

void p073_FillBufferWithNumber(String number)
{
  memset(p073_showbuffer,10,sizeof(p073_showbuffer));
  byte p073_numlenght = number.length();
  byte p073_dispdigit;
  byte p073_index = 7;
  p073_dotpos = -1;     // -1 means no dot to display
  for (int i=p073_numlenght;i>0;i--) {
    char p073_tmpchar = number.charAt(i-1);
    p073_dispdigit = 10;           // default is space
    if (p073_tmpchar > 47 && p073_tmpchar < 58)
      p073_dispdigit = p073_tmpchar-48;
    else if (p073_tmpchar == 32)  // space
      p073_dispdigit = 10;
    else if (p073_tmpchar == 45)  // minus
      p073_dispdigit = 11;
    if (p073_tmpchar == 46)  // dot
      p073_dotpos = p073_index;
    else {
      p073_showbuffer[p073_index] = p073_dispdigit;
      p073_index--; }
  }
}

void p073_FillBufferWithTemp(long temperature)
{
  memset(p073_showbuffer,10,sizeof(p073_showbuffer));
  char p073_digit[8];
  sprintf(p073_digit, "%7d", static_cast<int>(temperature));
  int p073_numlenght = strlen(p073_digit);
  byte p073_dispdigit;
  for (int i=0;i<p073_numlenght;i++) {
    if (p073_digit[i] > 47 && p073_digit[i] < 58)
      p073_dispdigit = p073_digit[i]-48;
    else if (p073_digit[i] == 32)  // space
      p073_dispdigit = 10;
    else if (p073_digit[i] == 45)  // minus
      p073_dispdigit = 11;
    p073_showbuffer[i] = p073_dispdigit;
  }
  p073_showbuffer[7] = 12;  // degree "°"
}

void p073_FillBufferWithDash(void)  // in case of error show all dashes
{
  memset(p073_showbuffer,11,sizeof(p073_showbuffer));
}

//===================================
//---- TM1637 specific functions ----
//===================================

#define CLK_HIGH()  digitalWrite(Plugin_073_7dgt->pin2, HIGH)
#define CLK_LOW()   digitalWrite(Plugin_073_7dgt->pin2, LOW)
#define DIO_HIGH()  pinMode(Plugin_073_7dgt->pin1, INPUT)
#define DIO_LOW()   pinMode(Plugin_073_7dgt->pin1, OUTPUT)

void tm1637_i2cStart (void)
{
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : Comm Start");
    addLog(LOG_LEVEL_INFO, log);
  }
  DIO_HIGH();
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_LOW();
}

void tm1637_i2cStop (void)
{
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : Comm Stop");
    addLog(LOG_LEVEL_INFO, log);
  }
  CLK_LOW();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_LOW();
  delayMicroseconds(TM1637_CLOCKDELAY);
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_HIGH();
}

void tm1637_i2cAck (void)
{
  bool dummyAck = false;
  CLK_LOW();
  pinMode(Plugin_073_7dgt->pin1, INPUT_PULLUP);
  //DIO_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  //while(digitalRead(Plugin_073_7dgt->pin1));
  dummyAck = digitalRead(Plugin_073_7dgt->pin1);
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : Comm ACK=");
    if (dummyAck == 0) { log += F("TRUE"); } else { log += F("FALSE"); }
    addLog(LOG_LEVEL_INFO, log);
  }
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  CLK_LOW();
  pinMode(Plugin_073_7dgt->pin1, OUTPUT);
}

void tm1637_i2cWrite (uint8_t bytetoprint)
{
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : WriteByte");
    addLog(LOG_LEVEL_INFO, log);
  }
  uint8_t i;
  for(i=0; i<8; i++)
  {
    CLK_LOW();
    (bytetoprint & B00000001)? DIO_HIGH() : DIO_LOW();
    delayMicroseconds(TM1637_CLOCKDELAY);
    bytetoprint = bytetoprint >> 1;
    CLK_HIGH();
    delayMicroseconds(TM1637_CLOCKDELAY);
  }
}

void tm1637_ClearDisplay (void)
{
  tm1637_i2cStart();
  tm1637_i2cWrite(0xC0);   tm1637_i2cAck();
  for (uint8_t i=0; i<6; i++)
    tm1637_i2cWrite(0);      tm1637_i2cAck();
  tm1637_i2cStop();
}

void tm1637_SetPowerBrightness (uint8_t brightlvl, bool poweron)
{
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : Set BRIGHT");
    addLog(LOG_LEVEL_INFO, log);
  }
  uint8_t brightvalue = (brightlvl & 0b111);
  if (poweron)
    brightvalue = TM1637_POWER_ON  | brightvalue;
  else
    brightvalue = TM1637_POWER_OFF | brightvalue;
  tm1637_i2cStart();
  tm1637_i2cWrite(brightvalue);
  tm1637_i2cAck();
  tm1637_i2cStop();
}

void tm1637_InitDisplay(void)
{
  pinMode(Plugin_073_7dgt->pin2, OUTPUT); // clk_pin
  pinMode(Plugin_073_7dgt->pin1, OUTPUT); // dat_pin
  CLK_HIGH();
  DIO_HIGH();
//  pinMode(Plugin_073_7dgt->pin1, INPUT_PULLUP);
//  pinMode(Plugin_073_7dgt->pin2, OUTPUT);
  tm1637_i2cStart();
  tm1637_i2cWrite(0x40);
  tm1637_i2cAck();
  tm1637_i2cStop();
  tm1637_ClearDisplay();
}

void tm1637_ShowTime6(bool sep)
{
  byte p073_datashowpos1;
  tm1637_i2cStart();
  tm1637_i2cWrite(0xC0);                                  tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[2]));   tm1637_i2cAck();
  // add bit for colon on second digit if required
    p073_datashowpos1 = get_byte_CharTableTM1637(p073_showbuffer[1]);
    if (sep) p073_datashowpos1 |= 0b10000000;
  tm1637_i2cWrite(p073_datashowpos1);                     tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[0]));   tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[5]));   tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[4]));   tm1637_i2cAck();
  // add bit for colon on fourth digit if required
    p073_datashowpos1 = get_byte_CharTableTM1637(p073_showbuffer[3]);
    if (sep) p073_datashowpos1 |= 0b10000000;
  tm1637_i2cWrite(p073_datashowpos1);                     tm1637_i2cAck();
  tm1637_i2cStop();
}

void tm1637_ShowDate6(bool sep)
{
  byte p073_datashowpos1;
  tm1637_i2cStart();
  tm1637_i2cWrite(0xC0);                                  tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[2]));   tm1637_i2cAck();
  // add bit for colon on second digit if required
    p073_datashowpos1 = get_byte_CharTableTM1637(p073_showbuffer[1]);
    if (sep) p073_datashowpos1 |= 0b10000000;
  tm1637_i2cWrite(p073_datashowpos1);                     tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[0]));   tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[7]));   tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[6]));   tm1637_i2cAck();
  // add bit for colon on fourth digit if required
    p073_datashowpos1 = get_byte_CharTableTM1637(p073_showbuffer[3]);
    if (sep) p073_datashowpos1 |= 0b10000000;
  tm1637_i2cWrite(p073_datashowpos1);                     tm1637_i2cAck();
  tm1637_i2cStop();
}

void tm1637_ShowTemp6(bool sep)
{
  tm1637_i2cStart();
  tm1637_i2cWrite(0xC0);                                  tm1637_i2cAck();
  // add bit for colon on second digit if required
    byte p073_datashowpos1 = get_byte_CharTableTM1637(p073_showbuffer[5]);
    if (sep) p073_datashowpos1 |= 0b10000000;
  tm1637_i2cWrite(p073_datashowpos1);                     tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[4]));   tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(10));                   tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(10));                   tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[7]));   tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[6]));   tm1637_i2cAck();
  tm1637_i2cStop();
}

void tm1637_ShowTimeTemp4(bool sep, byte bufoffset)
{
  tm1637_i2cStart();
  tm1637_i2cWrite(0xC0);                                            tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[0+bufoffset]));   tm1637_i2cAck();
  // add bit for colon on second digit if required
    byte p073_datashowpos1 = get_byte_CharTableTM1637(p073_showbuffer[1+bufoffset]);
    if (sep) p073_datashowpos1 |= 0b10000000;
  tm1637_i2cWrite(p073_datashowpos1);                               tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[2+bufoffset]));   tm1637_i2cAck();
  tm1637_i2cWrite(get_byte_CharTableTM1637(p073_showbuffer[3+bufoffset]));   tm1637_i2cAck();
  tm1637_i2cStop();
}

void tm1637_SwapDigitInBuffer() {
  uint8_t p073_temp;
  p073_temp = p073_showbuffer[4]; p073_showbuffer[4] = p073_showbuffer[2]; p073_showbuffer[2] = p073_temp;
  p073_temp = p073_showbuffer[5]; p073_showbuffer[5] = p073_showbuffer[7]; p073_showbuffer[7] = p073_temp;
  switch (p073_dotpos)
  {
    case 2: { p073_dotpos = 4; break; }
    case 4: { p073_dotpos = 2; break; }
    case 5: { p073_dotpos = 7; break; }
    case 7: { p073_dotpos = 5; break; }
  }
}

void tm1637_ShowBuffer(byte digits)
{
  byte p073_datashowpos1;
  tm1637_i2cStart();
  tm1637_i2cWrite(0xC0);                            tm1637_i2cAck();
    for(int i=digits;i<8;i++) {
    p073_datashowpos1 = get_byte_CharTableTM1637(p073_showbuffer[i]);
    if (p073_dotpos == i) p073_datashowpos1 |= 0b10000000;
    tm1637_i2cWrite(p073_datashowpos1);             tm1637_i2cAck();
  }
  tm1637_i2cStop();
}

//====================================
//---- MAX7219 specific functions ----
//====================================

#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

void max7219_spiTransfer (byte opcode, byte data)
{
  p073_spidata[0]=(byte)0;  p073_spidata[1]=(byte)0;
  p073_spidata[1]=opcode;   p073_spidata[0]=data;
  digitalWrite(Plugin_073_7dgt->pin3,LOW);
  shiftOut(Plugin_073_7dgt->pin1,Plugin_073_7dgt->pin2,MSBFIRST,p073_spidata[1]);
  shiftOut(Plugin_073_7dgt->pin1,Plugin_073_7dgt->pin2,MSBFIRST,p073_spidata[0]);
  digitalWrite(Plugin_073_7dgt->pin3,HIGH); // cs_pin
}

void max7219_ClearDisplay (void)
{
  for(int i=0;i<8;i++) {
    max7219_spiTransfer(i+1, 0);
  }
}

void max7219_SetPowerBrightness (uint8_t brightlvl, bool poweron)
{
  max7219_spiTransfer(OP_INTENSITY, brightlvl);
  if (poweron)
    max7219_spiTransfer(OP_SHUTDOWN, 1);
  else
    max7219_spiTransfer(OP_SHUTDOWN, 0);
}

void max7219_SetDigit(int dgtpos, byte dgtvalue, boolean showdot)
{
  byte p073_tempvalue;
  p073_tempvalue = pgm_read_byte(&CharTableMAX7219[dgtvalue]);
  if(showdot)
    p073_tempvalue |= 0b10000000;
  max7219_spiTransfer(dgtpos+1, p073_tempvalue);
}

void max7219_InitDisplay(void)
{
  pinMode(Plugin_073_7dgt->pin1, OUTPUT); // dat_pin
  pinMode(Plugin_073_7dgt->pin2, OUTPUT); // clk_pin
  pinMode(Plugin_073_7dgt->pin3, OUTPUT); // cs_pin
  digitalWrite(Plugin_073_7dgt->pin3, HIGH); // cs_pin
  max7219_spiTransfer(OP_DISPLAYTEST, 0);
  max7219_spiTransfer(OP_SCANLIMIT, 7);    // scanlimit setup to max at Init
  max7219_spiTransfer(OP_DECODEMODE, 0);
  max7219_ClearDisplay();
  max7219_SetPowerBrightness(0, false);
}

void max7219_ShowTime(bool sep)
{
  max7219_SetDigit(0, p073_showbuffer[5], false);
  max7219_SetDigit(1, p073_showbuffer[4], false);
  max7219_SetDigit(3, p073_showbuffer[3], false);
  max7219_SetDigit(4, p073_showbuffer[2], false);
  max7219_SetDigit(6, p073_showbuffer[1], false);
  max7219_SetDigit(7, p073_showbuffer[0], false);
  if (sep) {
    max7219_SetDigit(2, 11, false);
    max7219_SetDigit(5, 11, false);
  }
  else {
    max7219_SetDigit(2, 10, false);
    max7219_SetDigit(5, 10, false);
  }
}

void max7219_ShowTemp(void)
{
  max7219_SetDigit(0, 10, false);
  byte dotflags[8] = {false,false,false,false,false,true,false,false};
  for(int i=1;i<8;i++)
    max7219_SetDigit(i, p073_showbuffer[8-i], dotflags[8-i]);
}

void max7219_ShowDate(void)
{
  byte dotflags[8] = {false,true,false,true,false,false,false,false};
  for(int i=0;i<8;i++)
    max7219_SetDigit(i, p073_showbuffer[7-i], dotflags[7-i]);
}

void max7219_ShowBuffer(void)
{
  byte dotflags[8] = {false,false,false,false,false,false,false,false};
  if (p073_dotpos >= 0) dotflags[p073_dotpos] = true;
  for(int i=0;i<8;i++)
    max7219_SetDigit(i, p073_showbuffer[7-i], dotflags[7-i]);
}

uint8_t get_byte_CharTableTM1637(uint8_t index)
{
  return pgm_read_byte(&CharTableTM1637[index]);
}

#endif // USES_P073
