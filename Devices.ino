/*********************************************************************************************\
 * Pulse Counter
\*********************************************************************************************/
void pulse_interrupt1()
{
  pulseCounter1++;
}

void pulseinit(byte Par1)
{
  // Init IO pins
  Serial.println("PULSE: Init");
  attachInterrupt(Par1, pulse_interrupt1, FALLING);
}

/*********************************************************************************************\
 * Analog port
\*********************************************************************************************/
boolean analog(byte Par1)
{
  boolean success = false;
  int value = analogRead(0);
  UserVar[Par1 - 1] = (float)value;
  Serial.print("ADC  : Analog value: ");
  Serial.println(value);
  return success;
}

/*********************************************************************************************\
 * Lux reader BH1750
\*********************************************************************************************/
#define BH1750_ADDRESS    0x23
boolean luxinit = false;

boolean lux(byte Par1)
{
  boolean success = false;
  if (!luxinit)
  {
    Wire.beginTransmission(BH1750_ADDRESS);
    Wire.write(0x10);                             // 1 lx resolution
    Wire.endTransmission();
    luxinit = true;
  }
  Wire.requestFrom(BH1750_ADDRESS, 2);
  byte b1 = Wire.read();
  byte b2 = Wire.read();
  unsigned int val = 0;
  val = ((b1 << 8) | b2) / 1.2;
  val = val + 15;
  UserVar[Par1 - 1] = (float)val;
  Serial.print("LUX  : Light intensity: ");
  Serial.println(UserVar[Par1 - 1]);
  success = true;
  return success;
}

/*********************************************************************************************\
 * RDIF Wiegand 26
\*********************************************************************************************/
#define RFID_WGSIZE 26
volatile byte RFID_bitCount = 0;	           // Count the number of bits received.
volatile unsigned long RFID_keyBuffer = 0;   // A 32-bit-long keyBuffer into which the number is stored.
byte RFID_bitCountPrev = 0;                  // to detect noise

/*********************************************************************/
void RFID_interrupt1()
/*********************************************************************/
{
  // We've received a 1 bit. (bit 0 = high, bit 1 = low)
  RFID_keyBuffer = RFID_keyBuffer << 1;     // Left shift the number (effectively multiplying by 2)
  RFID_keyBuffer += 1;    		 // Add the 1 (not necessary for the zeroes)
  RFID_bitCount++;   			 // Increment the bit count
}

/*********************************************************************/
void RFID_interrupt2()
/*********************************************************************/
{
  // We've received a 0 bit. (bit 0 = low, bit 1 = high)
  RFID_keyBuffer = RFID_keyBuffer << 1;     // Left shift the number (effectively multiplying by 2)
  RFID_bitCount++;       		 // Increment the bit count
}

void rfidinit(byte Par1, byte Par2)
{
  // Init IO pins
  Serial.println("RFID : Init");
  attachInterrupt(Par1, RFID_interrupt1, FALLING);
  attachInterrupt(Par2, RFID_interrupt2, FALLING);
}

unsigned long rfid()
{
  if ((RFID_bitCount != RFID_WGSIZE) && (RFID_bitCount == RFID_bitCountPrev))
  {
    // must be noise
    RFID_bitCount = 0;
    RFID_keyBuffer = 0;
  }

  if (RFID_bitCount == RFID_WGSIZE)
  {
    RFID_bitCount = 0;          // Read in the current key and reset everything so that the interrupts can

    RFID_keyBuffer = RFID_keyBuffer >> 1;          // Strip leading and trailing parity bits from the keyBuffer
    RFID_keyBuffer &= 0xFFFFFF;
    return RFID_keyBuffer;
  }

  RFID_bitCountPrev = RFID_bitCount; // store this value for next check, detect noise

  return 0;
}

/*********************************************************************************************\
 * PCF8591
\*********************************************************************************************/
boolean pcf8591(byte Par1, byte Par2)
{
  boolean success = false;
  static byte portValue = 0;

  byte unit = (Par1 - 1) / 4;
  byte port = Par1 - (unit * 4);
  uint8_t address = 0x48 + unit;

  // get the current pin value
  Wire.beginTransmission(address);
  Wire.write(port - 1);
  Wire.endTransmission();

  Wire.requestFrom(address, (uint8_t)0x2);
  if (Wire.available())
  {
    Wire.read(); // Read older value first (stored in chip)
    UserVar[Par2 - 1] = (float)Wire.read(); // now read actual value and store into Nodo var
    Serial.print("PCF  : Analog Value : ");
    Serial.println(UserVar[Par1 - 1]);
    success = true;
  }
  return success;
}
/*********************************************************************************************\
 * MCP23017
\*********************************************************************************************/
boolean mcp23017(byte Par1, byte Par2)
{
  Serial.println("MCP23017");
  boolean success = false;
  byte portvalue = 0;
  byte unit = (Par1 - 1) / 16;
  byte port = Par1 - (unit * 16);
  uint8_t address = 0x20 + unit;
  byte IOBankConfigReg = 0;
  byte IOBankValueReg = 0x12;
  if (port > 8)
  {
    port = port - 8;
    IOBankConfigReg++;
    IOBankValueReg++;
  }
  // turn this port into output, first read current config
  Wire.beginTransmission(address);
  Wire.write(IOBankConfigReg); // IO config register
  Wire.endTransmission();
  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    portvalue = Wire.read();
    portvalue &= ~(1 << (port - 1)); // change pin from (default) input to output

    // write new IO config
    Wire.beginTransmission(address);
    Wire.write(IOBankConfigReg); // IO config register
    Wire.write(portvalue);
    Wire.endTransmission();
  }
  // get the current pin status
  Wire.beginTransmission(address);
  Wire.write(IOBankValueReg); // IO data register
  Wire.endTransmission();
  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    portvalue = Wire.read();
    if (Par2 == 1)
      portvalue |= (1 << (port - 1));
    else
      portvalue &= ~(1 << (port - 1));

    // write back new data
    Wire.beginTransmission(address);
    Wire.write(IOBankValueReg);
    Wire.write(portvalue);
    Wire.endTransmission();
    success = true;
  }
}

/*********************************************************************************************\
 * DALLAS
\*********************************************************************************************/
uint8_t DallasPin;

uint8_t DS_read(void)
{
  uint8_t bitMask;
  uint8_t r = 0;
  uint8_t BitRead;

  for (bitMask = 0x01; bitMask; bitMask <<= 1)
  {
    pinMode(DallasPin, OUTPUT);
    digitalWrite(DallasPin, LOW);
    delayMicroseconds(3);

    pinMode(DallasPin, INPUT); // let pin float, pull up will raise
    delayMicroseconds(10);
    BitRead = digitalRead(DallasPin);
    delayMicroseconds(53);

    if (BitRead)
      r |= bitMask;
  }
  return r;
}

void DS_write(uint8_t ByteToWrite)
{
  uint8_t bitMask;

  pinMode(DallasPin, OUTPUT);
  for (bitMask = 0x01; bitMask; bitMask <<= 1)
  { // BitWrite
    digitalWrite(DallasPin, LOW);
    if (((bitMask & ByteToWrite) ? 1 : 0) & 1)
    {
      delayMicroseconds(5);// Dallas spec.= 5..15 uSec.
      digitalWrite(DallasPin, HIGH);
      delayMicroseconds(55);// Dallas spec.= 60uSec.
    }
    else
    {
      delayMicroseconds(55);// Dallas spec.= 60uSec.
      digitalWrite(DallasPin, HIGH);
      delayMicroseconds(5);// Dallas spec.= 5..15 uSec.
    }
  }
}

uint8_t DS_reset()
{
  uint8_t r;
  uint8_t retries = 125;

  pinMode(DallasPin, INPUT);
  do  {  // wait until the wire is high... just in case
    if (--retries == 0) return 0;
    delayMicroseconds(2);
  } while ( !digitalRead(DallasPin));

  pinMode(DallasPin, OUTPUT); digitalWrite(DallasPin, LOW);
  delayMicroseconds(492); // Dallas spec. = Min. 480uSec. Arduino 500uSec.
  pinMode(DallasPin, INPUT); //Float
  delayMicroseconds(40);
  r = !digitalRead(DallasPin);
  delayMicroseconds(420);
  return r;
}

boolean dallas(byte Par1, byte Par2)
{
  static byte Call_Status = 0x00; // Each bit represents one relative port. 0=not called before, 1=already called before.
  boolean success = false;
  int DSTemp;                           // Temperature in 16-bit Dallas format.
  byte ScratchPad[12];                  // Scratchpad buffer Dallas sensor.
  byte var = Par2;               // Variable to be set.
  byte RelativePort = Par1 - 1;

  DallasPin = PIN_WIRED_OUT_1 + Par1 - 1;

  noInterrupts();
  while (!(bitRead(Call_Status, RelativePort)))
  {
    // if this is the very first call to the sensor on this port, reset it to wake it up
    boolean present = DS_reset();
    bitSet(Call_Status, RelativePort);
  }
  boolean present = DS_reset(); DS_write(0xCC /* rom skip */); DS_write(0x44 /* start conversion */);
  interrupts();

  if (present)
  {
    delay(800);     // neccesary delay

    noInterrupts();
    DS_reset(); DS_write(0xCC /* rom skip */); DS_write(0xBE /* Read Scratchpad */);

    digitalWrite(DallasPin, LOW);
    pinMode(DallasPin, INPUT);

    for (byte i = 0; i < 9; i++)            // copy 8 bytes
      ScratchPad[i] = DS_read();
    interrupts();

    DSTemp = (ScratchPad[1] << 8) + ScratchPad[0];
    UserVar[var - 1] = (float(DSTemp) * 0.0625);
    Serial.print("DS   : Temperature: ");
    Serial.println(UserVar[var - 1]);
    success = true;
  }
  return success;
}

/*********************************************************************************************\
 * DHT 11
\*********************************************************************************************/
uint8_t DHT_Pin;

byte read_dht_dat(void)
{
  byte i = 0;
  byte result = 0;
  noInterrupts();
  for (i = 0; i < 8; i++)
  {
    while (!digitalRead(DHT_Pin)); // wait for 50us
    delayMicroseconds(30);
    if (digitalRead(DHT_Pin))
      result |= (1 << (7 - i));
    while (digitalRead(DHT_Pin)); // wait '1' finish
  }
  interrupts();
  return result;
}

boolean dht(byte type, byte Par1, byte Par2)
{
  boolean success = false;
  DHT_Pin = PIN_WIRED_OUT_1 + Par1 - 1;
  byte dht_dat[5];
  byte dht_in;
  byte i;
  byte Retry = 0;

  do
  {
    pinMode(DHT_Pin, OUTPUT);
    // DHT start condition, pull-down i/o pin for 18ms
    digitalWrite(DHT_Pin, LOW);              // Pull low
    delay(18);
    digitalWrite(DHT_Pin, HIGH);             // Pull high
    delayMicroseconds(40);
    pinMode(DHT_Pin, INPUT);                 // change pin to input
    delayMicroseconds(40);

    dht_in = digitalRead(DHT_Pin);
    if (!dht_in)
    {
      delayMicroseconds(80);
      dht_in = digitalRead(DHT_Pin);
      if (dht_in)
      {
        delayMicroseconds(40);                     // now ready for data reception
        for (i = 0; i < 5; i++)
          dht_dat[i] = read_dht_dat();

        // Checksum calculation is a Rollover Checksum by design!
        byte dht_check_sum = dht_dat[0] + dht_dat[1] + dht_dat[2] + dht_dat[3]; // check check_sum

        if (dht_dat[4] == dht_check_sum)
        {

          if (type == 11)
          {
            UserVar[Par2 - 1] = float(dht_dat[2]); // Temperature
            UserVar[Par2   ] = float(dht_dat[0]); // Humidity

          }
          if (type == 22)
          {
            if (dht_dat[2] & 0x80) // negative temperature
              UserVar[Par2 - 1] = -0.1 * word(dht_dat[2] & 0x7F, dht_dat[3]);
            else
              UserVar[Par2 - 1] = 0.1 * word(dht_dat[2], dht_dat[3]);
            UserVar[Par2] = word(dht_dat[0], dht_dat[1]) * 0.1; // Humidity
          }
          Serial.print("DHT  : Temperature: ");
          Serial.println(UserVar[Par2 - 1]);
          Serial.print("DHT  : Humidity: ");
          Serial.println(UserVar[Par2]);
          success = true;
        }
      }
    }
    if (!success)
    {
      delay(2000);
    }
  } while (!success && ++Retry < 3);
}

/*********************************************************************************************\
 * BMP085
\*********************************************************************************************/

#define BMP085_I2CADDR           0x77
#define BMP085_ULTRAHIGHRES         3
#define BMP085_CAL_AC1           0xAA  // R   Calibration data (16 bits)
#define BMP085_CAL_AC2           0xAC  // R   Calibration data (16 bits)
#define BMP085_CAL_AC3           0xAE  // R   Calibration data (16 bits)    
#define BMP085_CAL_AC4           0xB0  // R   Calibration data (16 bits)
#define BMP085_CAL_AC5           0xB2  // R   Calibration data (16 bits)
#define BMP085_CAL_AC6           0xB4  // R   Calibration data (16 bits)
#define BMP085_CAL_B1            0xB6  // R   Calibration data (16 bits)
#define BMP085_CAL_B2            0xB8  // R   Calibration data (16 bits)
#define BMP085_CAL_MB            0xBA  // R   Calibration data (16 bits)
#define BMP085_CAL_MC            0xBC  // R   Calibration data (16 bits)
#define BMP085_CAL_MD            0xBE  // R   Calibration data (16 bits)
#define BMP085_CONTROL           0xF4
#define BMP085_TEMPDATA          0xF6
#define BMP085_PRESSUREDATA      0xF6
#define BMP085_READTEMPCMD       0x2E
#define BMP085_READPRESSURECMD   0x34

uint8_t oversampling = BMP085_ULTRAHIGHRES;
int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
uint16_t ac4, ac5, ac6;

//*********************************************************************
boolean bmp085_begin()
//*********************************************************************
{
  if (bmp085_read8(0xD0) != 0x55) return false;

  ac1 = bmp085_read16(BMP085_CAL_AC1);
  ac2 = bmp085_read16(BMP085_CAL_AC2);
  ac3 = bmp085_read16(BMP085_CAL_AC3);
  ac4 = bmp085_read16(BMP085_CAL_AC4);
  ac5 = bmp085_read16(BMP085_CAL_AC5);
  ac6 = bmp085_read16(BMP085_CAL_AC6);

  b1 = bmp085_read16(BMP085_CAL_B1);
  b2 = bmp085_read16(BMP085_CAL_B2);

  mb = bmp085_read16(BMP085_CAL_MB);
  mc = bmp085_read16(BMP085_CAL_MC);
  md = bmp085_read16(BMP085_CAL_MD);
  return true;
}

//*********************************************************************
uint16_t bmp085_readRawTemperature(void)
//*********************************************************************
{
  bmp085_write8(BMP085_CONTROL, BMP085_READTEMPCMD);
  delay(5);
  return bmp085_read16(BMP085_TEMPDATA);
}

//*********************************************************************
uint32_t bmp085_readRawPressure(void)
//*********************************************************************
{
  uint32_t raw;

  bmp085_write8(BMP085_CONTROL, BMP085_READPRESSURECMD + (oversampling << 6));

  delay(26);

  raw = bmp085_read16(BMP085_PRESSUREDATA);
  raw <<= 8;
  raw |= bmp085_read8(BMP085_PRESSUREDATA + 2);
  raw >>= (8 - oversampling);

  return raw;
}

//*********************************************************************
int32_t bmp085_readPressure(void)
//*********************************************************************
{
  int32_t UT, UP, B3, B5, B6, X1, X2, X3, p;
  uint32_t B4, B7;

  UT = bmp085_readRawTemperature();
  UP = bmp085_readRawPressure();

  // do temperature calculations
  X1 = (UT - (int32_t)(ac6)) * ((int32_t)(ac5)) / pow(2, 15);
  X2 = ((int32_t)mc * pow(2, 11)) / (X1 + (int32_t)md);
  B5 = X1 + X2;

  // do pressure calcs
  B6 = B5 - 4000;
  X1 = ((int32_t)b2 * ( (B6 * B6) >> 12 )) >> 11;
  X2 = ((int32_t)ac2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = ((((int32_t)ac1 * 4 + X3) << oversampling) + 2) / 4;

  X1 = ((int32_t)ac3 * B6) >> 13;
  X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
  B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> oversampling );

  if (B7 < 0x80000000)
  {
    p = (B7 * 2) / B4;
  }
  else
  {
    p = (B7 / B4) * 2;
  }
  X1 = (p >> 8) * (p >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * p) >> 16;

  p = p + ((X1 + X2 + (int32_t)3791) >> 4);
  return p;
}

//*********************************************************************
float bmp085_readTemperature(void)
//*********************************************************************
{
  int32_t UT, X1, X2, B5;     // following ds convention
  float temp;

  UT = bmp085_readRawTemperature();

  // step 1
  X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) / pow(2, 15);
  X2 = ((int32_t)mc * pow(2, 11)) / (X1 + (int32_t)md);
  B5 = X1 + X2;
  temp = (B5 + 8) / pow(2, 4);
  temp /= 10;

  return temp;
}

//*********************************************************************
uint8_t bmp085_read8(uint8_t a)
//*********************************************************************
{
  uint8_t ret;

  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device
  Wire.write(a); // sends register address to read from
  Wire.endTransmission(); // end transmission

  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device
  Wire.requestFrom(BMP085_I2CADDR, 1);// send data n-bytes read
  ret = Wire.read(); // receive DATA
  Wire.endTransmission(); // end transmission

  return ret;
}

//*********************************************************************
uint16_t bmp085_read16(uint8_t a)
//*********************************************************************
{
  uint16_t ret;

  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device
  Wire.write(a); // sends register address to read from
  Wire.endTransmission(); // end transmission

  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device
  Wire.requestFrom(BMP085_I2CADDR, 2);// send data n-bytes read
  ret = Wire.read(); // receive DATA
  ret <<= 8;
  ret |= Wire.read(); // receive DATA
  Wire.endTransmission(); // end transmission

  return ret;
}

//*********************************************************************
void bmp085_write8(uint8_t a, uint8_t d)
//*********************************************************************
{
  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device
  Wire.write(a); // sends register address to read from
  Wire.write(d);  // write data
  Wire.endTransmission(); // end transmission
}


boolean bmp085init = false;

boolean bmp085(byte Par1)
{
  if (!bmp085init)
  {
    Serial.println("BMP  : Init");
    if (bmp085_begin())
      bmp085init = true;
  }
  else
  {
    boolean success = false;
    UserVar[Par1 - 1] = bmp085_readTemperature();
    UserVar[Par1   ] = ((float)bmp085_readPressure()) / 100;
    Serial.print("BMP  : Temperature: ");
    Serial.println(UserVar[Par1 - 1]);
    Serial.print("BMP  : Barometric Pressure: ");
    Serial.println(UserVar[Par1]);
    success = true;
  }
}

/*********************************************************************************************\
 * LCD I2C Display
\*********************************************************************************************/
#define LCD_I2C_ADDRESS 0x27

#define PLUGIN_021_ROWS  4
#define PLUGIN_021_COLS 20

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100  // Enable bit
#define Rw B00000010  // Read/Write bit
#define Rs B00000001  // Register select bit

void LCD_I2C_init();
void LCD_I2C_printline(byte row, byte col, char* message);
inline size_t LCD_I2C_write(uint8_t value);
void LCD_I2C_display();
void LCD_I2C_clear();
void LCD_I2C_home();
void LCD_I2C_setCursor(uint8_t col, uint8_t row);
inline void LCD_I2C_command(uint8_t value);
void LCD_I2C_send(uint8_t value, uint8_t mode);
void LCD_I2C_write4bits(uint8_t value);
void LCD_I2C_expanderWrite(uint8_t _data);                                        
void LCD_I2C_pulseEnable(uint8_t _data);

uint8_t _displayfunction;
uint8_t _displaycontrol;
uint8_t _displaymode;
uint8_t _numlines;
uint8_t _backlightval=LCD_BACKLIGHT;

boolean lcdinit=false;

boolean lcd(byte Par1, byte Par2, char *text)
{
  Serial.print("LCD  : ");
  Serial.println(text);
  if (!lcdinit)
    {
      LCD_I2C_init();
      lcdinit=true;
    }
  
     if (Par1 >= 0 && Par1 <= PLUGIN_021_ROWS)
       {
         LCD_I2C_printline(Par1-1, Par2-1, text);
       }
     Wire.endTransmission(true);
}
/*********************************************************************/
void LCD_I2C_init()
/*********************************************************************/
{
     _displayfunction = LCD_2LINE;
     _numlines = PLUGIN_021_ROWS;
     delay(50); 
     // Now we pull both RS and R/W low to begin commands
     LCD_I2C_expanderWrite(_backlightval);	// reset expander and turn backlight off (Bit 8 =1)
     delay(1000);

     //put the LCD into 4 bit mode, this is according to the hitachi HD44780 datasheet, figure 24, pg 46
     LCD_I2C_write4bits(0x03 << 4);        // we start in 8bit mode, try to set 4 bit mode
     delayMicroseconds(4500);              // wait min 4.1ms
     LCD_I2C_write4bits(0x03 << 4);        // second try
     delayMicroseconds(4500);              // wait min 4.1ms
     LCD_I2C_write4bits(0x03 << 4);        // third go!
     delayMicroseconds(150);
     LCD_I2C_write4bits(0x02 << 4);        // finally, set to 4-bit interface
     LCD_I2C_command(LCD_FUNCTIONSET | _displayfunction);              // set # lines, font size, etc.
     _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;   // turn the display on with no cursor or blinking default
     LCD_I2C_display();
     LCD_I2C_clear();                                                  // clear it off
     _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;           // Initialize to default text direction (for roman languages)
     LCD_I2C_command(LCD_ENTRYMODESET | _displaymode);                 // set the entry mode
     LCD_I2C_home();
}

/*********************************************************************/
void LCD_I2C_printline(byte row, byte col, char* message)
/*********************************************************************/
{
 LCD_I2C_setCursor(col,row);
 byte maxcol = PLUGIN_021_COLS-col;

 //clear line if empty message
 if (message[0]==0)
   for (byte x=0; x<PLUGIN_021_COLS; x++) LCD_I2C_write(' ');
 else
   for (byte x=0; x < maxcol; x++)
     {
       if (message[x] != 0) LCD_I2C_write(message[x]);
       else break;
     }
}

/*********************************************************************/
inline size_t LCD_I2C_write(uint8_t value)
/*********************************************************************/
{
LCD_I2C_send(value, Rs);
return 0;
}

/*********************************************************************/
void LCD_I2C_display() {
/*********************************************************************/
 _displaycontrol |= LCD_DISPLAYON;
 LCD_I2C_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

/*********************************************************************/
void LCD_I2C_clear(){
/*********************************************************************/
 LCD_I2C_command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
 delayMicroseconds(2000);  // this command takes a long time!
}

/*********************************************************************/
void LCD_I2C_home(){
/*********************************************************************/
 LCD_I2C_command(LCD_RETURNHOME);  // set cursor position to zero
 delayMicroseconds(2000);  // this command takes a long time!
}

/*********************************************************************/
void LCD_I2C_setCursor(uint8_t col, uint8_t row){
/*********************************************************************/
 int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
 if ( row > _numlines ) {
row = _numlines-1;    // we count rows starting w/0
 }
 LCD_I2C_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

/*********************************************************************/
inline void LCD_I2C_command(uint8_t value) {
/*********************************************************************/
 LCD_I2C_send(value, 0);
}

/*********************************************************************/
void LCD_I2C_send(uint8_t value, uint8_t mode) {
/*********************************************************************/
 uint8_t highnib=value&0xf0;
 uint8_t lownib=(value<<4)&0xf0;
 LCD_I2C_write4bits((highnib)|mode);
 LCD_I2C_write4bits((lownib)|mode); 
}

/*********************************************************************/
void LCD_I2C_write4bits(uint8_t value) {
/*********************************************************************/
 LCD_I2C_expanderWrite(value);
 LCD_I2C_pulseEnable(value);
}

/*********************************************************************/
void LCD_I2C_expanderWrite(uint8_t _data){                                        
/*********************************************************************/
 Wire.beginTransmission(LCD_I2C_ADDRESS);
 Wire.write((int)(_data) | _backlightval);
 Wire.endTransmission(false);
 yield();   
}

/*********************************************************************/
void LCD_I2C_pulseEnable(uint8_t _data){
/*********************************************************************/
 LCD_I2C_expanderWrite(_data | En);	// En high
 delayMicroseconds(1);	 // enable pulse must be >450ns

 LCD_I2C_expanderWrite(_data & ~En);	// En low
 delayMicroseconds(50);	 // commands need > 37us to settle
} 

