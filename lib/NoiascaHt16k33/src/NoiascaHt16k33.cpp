/* *******************************************************************
  HT16K33 Library
  
  by (c) noiasca
  
  2024-09-13 fixed decimal point for 7 segment variants
  2022-02-28 fixed typo interval
  2021-11-27 corrected TWCR to 328/2560 only
 * ******************************************************************/
#include "NoiascaHt16k33.h"

Noiasca_ht16k33::Noiasca_ht16k33()
{}


uint8_t Noiasca_ht16k33::begin(uint8_t i2c_addr,  uint8_t numDevices  /* = 1 */) {
  uint8_t result = 0;    // 0 = success
  _i2c_addr = i2c_addr;
  _numDevices = numDevices;
  if (_numDevices > 8) _numDevices = 1;
#if HT16K33_DEBUG > 2
  Serial.println(F("D012 begin base"));
  //Serial.print(F("D013 numDevices=")); Serial.println(numDevices);
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
  if (TWCR == 0) {
    Wire.begin();                                // only call when not started before
#if HT16K33_DEBUG >= 2
  Serial.println(F("W019 add Wire.begin() to your setup"));
#endif
  }
#endif
  for (uint8_t i = _i2c_addr; i < _i2c_addr + _numDevices; i++)
  {
    Wire.beginTransmission(i);
    Wire.write(HT16K33_OSCILATOR_ON);            // turn on oscillator
    if (Wire.endTransmission())
    {
      result = HT16K33_ERR_ADDR;
#if HT16K33_DEBUG > 0
      Serial.print(F("E039 I2C error for adress 0x"));
      Serial.println(i, HEX);
#endif
    }
    blinkRate(HT16K33_BLINK_OFF);
    setBrightness(15);                           // max brightness
  }
  return result;
}


void Noiasca_ht16k33::blinkRate(uint8_t b) {
  for (uint8_t i = 0; i < _numDevices; i++)
  {
    Wire.beginTransmission(_i2c_addr + i);
    if (b > 3) b = 0;                            // turn blink off if not sure
    Wire.write(HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (b << 1));
    Wire.endTransmission();
  }
}


void Noiasca_ht16k33::clear(void) {                                 // Clears displaybuffer and Display (I2C)
#ifdef HT16K33_BUFFERSIZE
  for (uint8_t i = 0; i < HT16K33_BUFFERSIZE; i++) {
    displaybuffer[i] = 0;
  }
#endif
  for (uint8_t i = 0; i < _numDevices; i++)
  {
    Wire.beginTransmission(_i2c_addr + i);
    for (uint8_t j = 0; j < 16; j++) {                               // for all 8 positions per device (16 adresses)
      Wire.write(0);
    }
    Wire.endTransmission();
  }
  _currentPosition = 0;
}


/*
   check if all ICs are responding
   return true on success
*/
bool Noiasca_ht16k33::isConnected()                                 
{
  for (uint8_t i = _i2c_addr; i < _i2c_addr + _numDevices; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() != 0) // 0 if something ack's at this address
    {
#if HT16K33_DEBUG >= 1
      Serial.print(F("E090 not responding IC 0x"));
      Serial.println(i, HEX);
#endif
      return false; // exit on first not responding display
    }
  }
  return true;
}


/*
   turn the display off
*/
void Noiasca_ht16k33::off(void)
{
  for (uint8_t i = 0; i < _numDevices; i++)
  {
    Wire.beginTransmission(_i2c_addr + i);
    Wire.write(HT16K33_BLINK_CMD);
    Wire.endTransmission();
  } 
}


/*
   turn the display on
*/
void Noiasca_ht16k33::on(void)
{
   blinkRate(HT16K33_BLINK_OFF);
}


/*
   set the brightness of the display 
*/
void Noiasca_ht16k33::setBrightness(uint8_t b) {
  if (b > 15) b = 15;
  for (uint8_t i = 0; i < _numDevices; i++) {
    Wire.beginTransmission(_i2c_addr + i);
    Wire.write(HT16K33_CMD_BRIGHTNESS | b);
    Wire.endTransmission();
  }
}


/*
  set the cursor for the next writing position
*/
void Noiasca_ht16k33::setCursor(uint8_t newPosition) {               // Set the cursor for the next writing position
  _currentPosition = newPosition;
}


/*
  get the cursor for the next writing position
*/
uint8_t Noiasca_ht16k33::getCursor() {                              // Get the cursor for the next writing position
  return _currentPosition;
}


/*
  set the number of digits per device
*/
void Noiasca_ht16k33::setDigits(uint8_t newDigits)
{
  _numDigits = newDigits;
}


#ifdef HT16K33_BUFFERSIZE
void Noiasca_ht16k33::writeChar(uint8_t device, uint16_t digit) {     // I2C Ausgabe eines Zeichen aus Displaybuffer
  device = digit / _numDigits;
  uint8_t reg = (digit % _numDigits) * 2;
  uint8_t i2c_addr = _i2c_addr + device;
  Wire.beginTransmission(i2c_addr);
  Wire.write(reg);                         // Register  (uint8_t)digit * 2
  Wire.write(displaybuffer[digit] & 0xFF); // nibble
  Wire.write(displaybuffer[digit] >> 8);   // nibble
  Wire.endTransmission();
#if HT16K33_DEBUG > 2
  Serial.print(F("   digit=")); Serial.println(digit);
  Serial.print(F("  device=")); Serial.println(device);
  Serial.print(F("i2c_addr=")); Serial.println(i2c_addr, HEX);
  Serial.print(F("     reg=")); Serial.println(reg);
  Serial.print(F("    char=")); Serial.println(displaybuffer[digit], BIN);
#endif
}
#endif


/*
  I2C lowLevel write of a bitmask to the IC to a specific position
*/
void Noiasca_ht16k33::writeLowLevel(uint8_t position, uint16_t bitmask) {
  uint8_t device = position / _numDigits;
  uint8_t reg = (position % _numDigits) * 2;
  uint8_t i2c_addr = _i2c_addr + device;
  Wire.beginTransmission(i2c_addr);
  Wire.write(reg);                         // Register  (uint8_t)digit * 2
  Wire.write(bitmask & 0xFF); // nibble
  Wire.write(bitmask >> 8);   // nibble
  Wire.endTransmission();
#if HT16K33_DEBUG > 2
  Serial.print(F("position=")); Serial.println(position);
  Serial.print(F("  device=")); Serial.println(device);
  Serial.print(F("i2c_addr=")); Serial.println(i2c_addr, HEX);
  Serial.print(F("     reg=")); Serial.println(reg);
  Serial.print(F(" bitmask=")); Serial.println(bitmask, BIN);
#endif
}


/* *******************************************************************
   generic module 7 SEGMENT OBJECT
 * ******************************************************************/
Noiasca_ht16k33_hw_7::Noiasca_ht16k33_hw_7 ()
{
  //public:
}

uint16_t Noiasca_ht16k33_hw_7::getCharacterBitmap(uint8_t value) {
  
  if (value > 31 && value < FONTFILE7_MAX_CHAR)             // write printable ASCII characters to display
  {
    return pgm_read_byte_near(charTable + value - 32);      // the table starts with the first printable character at 32
  }
  return 0u;
}


// general write method for 7 segment
size_t Noiasca_ht16k33_hw_7::write(uint8_t value) {
  if (value == '.' || value == ',' || value == ':' || value == ';')  // dots need a special handling
  {
#ifdef HT16K33_BUFFERSIZE
    displaybuffer[_lastPosition] = displaybuffer[_lastPosition] | SEG_DP;
#endif
    _lastBitmap = _lastBitmap | SEG_DP;
    writeLowLevel(_lastPosition, _lastBitmap);
  }
  else if (value > 31 && value < FONTFILE7_MAX_CHAR)                   // write printable ASCII characters to display
  {
    _lastBitmap = pgm_read_byte_near(charTable + value - 32);          // the table starts with the first printable character at 32
#ifdef HT16K33_BUFFERSIZE    
    displaybuffer[_currentPosition] = _lastBitmap;
#endif
    _lastPosition = _currentPosition;
    writeLowLevel(_currentPosition, _lastBitmap);
    _currentPosition++;
    if (_hasColonDigit && _currentPosition == 2) _currentPosition = 3;                                    // colon     MISSING: auch für die vielfachen Positionen!!!
    if (_currentPosition >= _numDigits * _numDevices) _currentPosition = 0;
  }
  return 1; // assume sucess
}


// activates/deactivtes a separate colon digit
void Noiasca_ht16k33_hw_7::setColonDigit(uint8_t activate)
{
  _hasColonDigit = activate;
}  


/* *******************************************************************
   7 SEGMENT 4 Digits no digitalpoint, and colon digit
 * ******************************************************************/

Noiasca_ht16k33_hw_7_4_c::Noiasca_ht16k33_hw_7_4_c()
{
  _numDigits = 5;            // In fact the display has 5 digits but only 0 1 3 4 are visible. 2 is used for the colon Digit
  _hasColonDigit = 1;
}


void Noiasca_ht16k33_hw_7_4_c::showColon(bool colon)
{
  for (uint8_t i = 0; i < _numDevices; i++) {
    showColon(i, colon);
  }
}


void Noiasca_ht16k33_hw_7_4_c::showColon(uint8_t device, bool colon)
{
  Wire.beginTransmission(_i2c_addr + device);
  Wire.write((uint8_t)0x04); // start at address $02
  if (colon)
    Wire.write((uint8_t)0x2);
  else
    Wire.write((uint8_t)0);
  Wire.write((uint8_t)0);
  Wire.endTransmission();
}


/* *******************************************************************
   14 SEGMENT generic
 * ******************************************************************/
Noiasca_ht16k33_hw_14::Noiasca_ht16k33_hw_14()
{}

uint16_t Noiasca_ht16k33_hw_14::getCharacterBitmap(uint8_t value) {
  
  if (value > 31 && value < FONTFILE14_MAX_CHAR)                // write printable ASCII characters to display
  {
    return pgm_read_word_near(charTable14 + value - 32);      // the table starts with the first printable character at 32
  }
  return 0u;
}

size_t Noiasca_ht16k33_hw_14::write(uint8_t value) {
  if (value == 35) value = 127;                  // MISSING: I don't remember why this is of any sence. This mapping costs 6 byte. DEPRECATED
  //if (value==167 || value == 248) value = 42;  // degree
  //if (value==219) value = 127                  // all segements
  
  if (value == '.' || value == ',' || value == ':' || value == ';')  // dots need a special handling
  {
#ifdef HT16K33_BUFFERSIZE
    displaybuffer[_lastPosition] = displaybuffer[_lastPosition] | SEG14_DP;
#endif
    _lastBitmap = _lastBitmap | SEG14_DP;
    writeLowLevel(_lastPosition, _lastBitmap);
  }
  else if (value > 31 && value < FONTFILE14_MAX_CHAR)                // write printable ASCII characters to display
  {
    _lastBitmap = pgm_read_word_near(charTable14 + value - 32);      // the table starts with the first printable character at 32
#ifdef HT16K33_BUFFERSIZE
    displaybuffer[_currentPosition] = _lastBitmap;
#endif
    _lastPosition = _currentPosition;
    writeLowLevel(_currentPosition, _lastBitmap);
    _currentPosition++;
    if (_currentPosition >= _numDigits * _numDevices) _currentPosition = 0;
  }
  // Serial.write(value);
  // Serial.print(F("Noiasca write: "));
  // Serial.println(value);
  return 1;        // assume sucess
}


/* *******************************************************************
   14 segment 4 digits, digitalpoint
 * ******************************************************************/
Noiasca_ht16k33_hw_14_4::Noiasca_ht16k33_hw_14_4()
{
  _numDigits = 4;
}


/* *******************************************************************
   14 segment extend class with scroll support
 * ******************************************************************/
Noiasca_ht16k33_hw_14_ext::Noiasca_ht16k33_hw_14_ext()
{}


uint32_t Noiasca_ht16k33_hw_14_ext::scroll()      // the "tick" method for scrolling text
{
  if (millis() - previousMillis > interval)
  {
    const uint8_t textLength = strlen(myText);
    //Serial.print(F("textLength=")); Serial.println(textLength);
    const uint8_t displayLength = _numDigits * _numDevices;               // das soll woanders her... ein getter???
    if (millis() - previousMillis > interval)
    {
      char buffer[displayLength + 2];
      if (counter < displayLength && counter < textLength + 1)        // Step 1: print text at right end of display - scroll from right to left
      {
        clear();
        setCursor(displayLength - counter);
        strlcpy(buffer, myText, counter + 1);
        print(buffer);
#if HT16K33_DEBUG > 2
        Serial.print(counter); Serial.print(" "); Serial.println(buffer);
#endif
      }
      else if (counter < textLength + 1)                               // Step 3: print text on full display
      {
        strlcpy(buffer, myText + (counter - displayLength), displayLength + 1);
        clear();
        print(buffer);
#if HT16K33_DEBUG > 2        
        Serial.print(counter); Serial.print(" "); Serial.println(buffer);
#endif
      }
      if (counter > textLength)                                      //Step 3: A extra delay at the end of the text
      {
        if (millis() - previousMillis > wait)
        {
          counter = 1;
          rounds++;
          previousMillis = millis();
        }
      }
      else
      {
        counter++;
        previousMillis = millis();
      }
    }
  }
  return rounds;
}


void Noiasca_ht16k33_hw_14_ext::setScrollInterval(uint16_t newInterval)
{
  if (newInterval < 100) interval = 100; else interval = newInterval;
}


void Noiasca_ht16k33_hw_14_ext::setScrollWait(uint16_t newWait)
{
  if (newWait >= interval) wait = newWait;
}


void Noiasca_ht16k33_hw_14_ext::setScrolltext(const char *newText)
{
  strlcpy(myText, newText, scrollBufferSize);
  rounds = 0;
#if HT16K33_DEBUG > 2
  Serial.println(myText);
#endif
}


void Noiasca_ht16k33_hw_14_ext::setScrolltext(int32_t myNumber)
{
  char newText[12];
  ltoa(myNumber, newText, 10);
  strlcpy(myText, newText, scrollBufferSize);
  rounds = 0;
#if HT16K33_DEBUG > 2
  Serial.println(myText);
#endif
}


void Noiasca_ht16k33_hw_14_ext::setScrolltext(double myNumber)       // this will scroll a float
{
  char newText[8];                     // Buffer big enough for 7-character float
  dtostrf(myNumber, 6, 2, newText);    // Leave room for too large numbers!
  strlcpy(myText, newText, scrollBufferSize - 1);
  rounds = 0;                          // resets enables to keep external track of how often the text was scrolled already
#if HT16K33_DEBUG > 2
  Serial.println(myText);
#endif
}
