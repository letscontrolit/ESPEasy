#ifdef USES_P061

// #######################################################################################################
// #################################### Plugin 061: PCF8574/MCP23017 KeyPad ##############################
// #######################################################################################################

// ESPEasy Plugin to scan a (up to) 9x8 key pad matrix chip MCP23017
// or a (up to) 5x4 key pad matrix chip PCF8574
// written by Jochen Krapf (jk@nerd2nerd.org)

// Connecting KeyPad matrix to MCP23017 chip:
// row 0 = GND   (optional if 9 rows needed)
// row 1 = GPA0
// row 2 = GPA1
// ...
// row 8 = GPA7
//
// column 1 = GPB0
// column 2 = GPB1
// ...
// column 8 = GPB7

// Typical Key Pad:
//      C1  C2  C3
// R1   [1] [2] [3]
// R2   [4] [5] [6]
// R3   [7] [8] [9]
// R4   [*] [0] [#]

// Connecting KeyPad matrix to PCF8574 chip:
// row 0 = GND   (optional if 5 rows needed)
// row 1 = P0
// row 2 = P1
// row 3 = P2
// row 4 = P3
//
// column 1 = P4
// column 2 = P5
// column 3 = P6
// column 4 = P7

// Connecting KeyPad direct to PCF8574 chip:
// common = GND
// key 1 = P0
// key 2 = P1
// ...
// key 8 = P7

// ScanCode;
// 16*col + row
// Pressing the top left key (typically "1") the code is 17 (0x11)
// Pressing the key in rowumn 2 and col 3 (typically "8") the code is 35 (0x23)
// No key - the code 0
// If more than one key is pressed, the scan code is the code with the lowest value


#define PLUGIN_061
#define PLUGIN_ID_061         61
#define PLUGIN_NAME_061       "Keypad - PCF8574 / MCP23017 [TESTING]"
#define PLUGIN_VALUENAME1_061 "ScanCode"

// #include <*.h>   // no include needed
#include "_Plugin_Helper.h"

boolean Plugin_061(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_061;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SWITCH;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_061);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_061));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      byte addr = PCONFIG(0);

      int optionValues[16] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F };
      addFormSelectorI2C(F("i2c_addr"), (PCONFIG(1) == 0) ? 8 : 16, optionValues, addr);

      if (PCONFIG(1) != 0) {
        addFormNote(F("PCF8574 uses address 0x20+; PCF8574<b>A</b> uses address 0x38+"));
      }
      break;
    }


    case PLUGIN_WEBFORM_LOAD:
    {
      String options[3] = { F("MCP23017 (Matrix 9x8)"), F("PCF8574 (Matrix 5x4)"), F("PCF8574 (Direct 8)") };
      addFormSelector(F("Chip (Mode)"), F("chip"), 3, options, NULL, PCONFIG(1));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));

      PCONFIG(1) = getFormItemInt(F("chip"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      switch (PCONFIG(1))
      {
        case 0: MCP23017_KeyPadMatrixInit(PCONFIG(0)); break;
        case 1: PCF8574_KeyPadMatrixInit(PCONFIG(0)); break;
        case 2: PCF8574_KeyPadDirectInit(PCONFIG(0)); break;
      }

      success = true;
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      static byte lastScanCode = 0xFF;
      static byte sentScanCode = 0xFF;
      byte actScanCode         = 0;

      switch (PCONFIG(1))
      {
        case 0: actScanCode = MCP23017_KeyPadMatrixScan(PCONFIG(0)); break;
        case 1: actScanCode = PCF8574_KeyPadMatrixScan(PCONFIG(0)); break;
        case 2: actScanCode = PCF8574_KeyPadDirectScan(PCONFIG(0)); break;
      }

      if (lastScanCode == actScanCode)   // debounced? - two times the same value?
      {
        if (sentScanCode != actScanCode) // any change to last sent data?
        {
          UserVar[event->BaseVarIndex] = (float)actScanCode;
          event->sensorType            = Sensor_VType::SENSOR_TYPE_SWITCH;

          String log = F("KPad : ScanCode=0x");
          log += String(actScanCode, 16);
          addLog(LOG_LEVEL_INFO, log);

          sendData(event);

          sentScanCode = actScanCode;
        }
      }
      else {
        lastScanCode = actScanCode;
      }

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      // work is done in PLUGIN_FIFTY_PER_SECOND
      success = true;
      break;
    }
  }
  return success;
}

// MCP23017 Matrix /////////////////////////////////////////////////////////////

#define MCP23017_IODIRA         0x00 // I/O DIRECTION REGISTER   IO7 IO6 IO5 IO4 IO3 IO2 IO1 IO0 1111 1111
#define MCP23017_IODIRB         0x01 // I/O DIRECTION REGISTER   IO7 IO6 IO5 IO4 IO3 IO2 IO1 IO0 1111 1111
#define MCP23017_IPOLA          0x02 // INPUT POLARITY PORT REGISTER   IP7 IP6 IP5 IP4 IP3 IP2 IP1 IP0 0000 0000
#define MCP23017_IPOLB          0x03 // INPUT POLARITY PORT REGISTER   IP7 IP6 IP5 IP4 IP3 IP2 IP1 IP0 0000 0000
#define MCP23017_GPINTENA       0x04 // INTERRUPT-ON-CHANGE PINS   GPINT7 GPINT6 GPINT5 GPINT4 GPINT3 GPINT2 GPINT1 GPINT0 0000 0000
#define MCP23017_GPINTENB       0x05 // INTERRUPT-ON-CHANGE PINS   GPINT7 GPINT6 GPINT5 GPINT4 GPINT3 GPINT2 GPINT1 GPINT0 0000 0000
#define MCP23017_DEFVALA        0x06 // DEFAULT VALUE REGISTER   DEF7 DEF6 DEF5 DEF4 DEF3 DEF2 DEF1 DEF0 0000 0000
#define MCP23017_DEFVALB        0x07 // DEFAULT VALUE REGISTER   DEF7 DEF6 DEF5 DEF4 DEF3 DEF2 DEF1 DEF0 0000 0000
#define MCP23017_INTCONA        0x08 // INTERRUPT-ON-CHANGE CONTROL REGISTER   IOC7 IOC6 IOC5 IOC4 IOC3 IOC2 IOC1 IOC0 0000 0000
#define MCP23017_INTCONB        0x09 // INTERRUPT-ON-CHANGE CONTROL REGISTER   IOC7 IOC6 IOC5 IOC4 IOC3 IOC2 IOC1 IOC0 0000 0000
#define MCP23017_IOCON          0x0A // I/O EXPANDER CONFIGURATION REGISTER   BANK MIRROR SEQOP DISSLW HAEN ODR INTPOL — 0000 0000 - also on
                                     // addr 0x0B
#define MCP23017_GPPUA          0x0C // GPIO PULL-UP RESISTOR REGISTER   PU7 PU6 PU5 PU4 PU3 PU2 PU1 PU0 0000 0000
#define MCP23017_GPPUB          0x0D // GPIO PULL-UP RESISTOR REGISTER   PU7 PU6 PU5 PU4 PU3 PU2 PU1 PU0 0000 0000
#define MCP23017_INTFA          0x0E // INTERRUPT FLAG REGISTER   INT7 INT6 INT5 INT4 INT3 INT2 INT1 INTO 0000 0000
#define MCP23017_INTFB          0x0F // INTERRUPT FLAG REGISTER   INT7 INT6 INT5 INT4 INT3 INT2 INT1 INTO 0000 0000
#define MCP23017_INTCAPA        0x10 // INTERRUPT CAPTURED VALUE FOR PORT REGISTER   ICP7 ICP6 ICP5 ICP4 ICP3 ICP2 ICP1 ICP0 0000 0000
#define MCP23017_INTCAPB        0x11 // INTERRUPT CAPTURED VALUE FOR PORT REGISTER   ICP7 ICP6 ICP5 ICP4 ICP3 ICP2 ICP1 ICP0 0000 0000
#define MCP23017_GPIOA          0x12 // GENERAL PURPOSE I/O PORT REGISTER   GP7 GP6 GP5 GP4 GP3 GP2 GP1 GP0 0000 0000
#define MCP23017_GPIOB          0x13 // GENERAL PURPOSE I/O PORT REGISTER   GP7 GP6 GP5 GP4 GP3 GP2 GP1 GP0 0000 0000
#define MCP23017_OLATA          0x14 // OUTPUT LATCH REGISTER   OL7 OL6 OL5 OL4 OL3 OL2 OL1 OL0 0000 0000
#define MCP23017_OLATB          0x15 // OUTPUT LATCH REGISTER   OL7 OL6 OL5 OL4 OL3 OL2 OL1 OL0 0000 0000


void MCP23017_setReg(byte addr, byte reg, byte data)
{
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

byte MCP23017_getReg(byte addr, byte reg)
{
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(addr, (uint8_t)0x1);

  if (Wire.available())
  {
    return Wire.read();
  }
  return 0xFF;
}

void MCP23017_KeyPadMatrixInit(byte addr)
{
  MCP23017_setReg(addr, MCP23017_IODIRA, 0x00); // port A to output
  MCP23017_setReg(addr, MCP23017_GPIOA,  0x00); // port A to low
  MCP23017_setReg(addr, MCP23017_IODIRB, 0xFF); // port B to input
  MCP23017_setReg(addr, MCP23017_GPPUA,  0xFF); // port A pullup on
  MCP23017_setReg(addr, MCP23017_GPPUB,  0xFF); // port B pullup on
}

byte MCP23017_KeyPadMatrixScan(byte addr)
{
  byte rowMask = 1;
  byte colData;

  colData = MCP23017_getReg(addr, MCP23017_GPIOB);

  if (colData == 0xFF) { // no key pressed?
    return 0;            // no key pressed!
  }

  for (byte row = 0; row <= 8; row++)
  {
    if (row == 0) {
      MCP23017_setReg(addr, MCP23017_IODIRA, 0xFF); // no bit of port A to output
    }
    else
    {
      MCP23017_setReg(addr, MCP23017_IODIRA, ~rowMask); // one bit of port A to output 0
      rowMask <<= 1;
    }

    colData = MCP23017_getReg(addr, MCP23017_GPIOB);

    if (colData != 0xFF) // any key pressed?
    {
      byte colMask = 1;

      for (byte col = 1; col <= 8; col++)
      {
        if ((colData & colMask) == 0)                   // this key pressed?
        {
          MCP23017_setReg(addr, MCP23017_IODIRA, 0x00); // port A to output 0
          return (row << 4) | col;
        }
        colMask <<= 1;
      }
    }
  }

  MCP23017_setReg(addr, MCP23017_IODIRA, 0x00); // port A to output 0
  return 0;                                     // no key pressed!
}

// PCF8574 Matrix //////////////////////////////////////////////////////////////

void PCF8574_setReg(byte addr, byte data)
{
  Wire.beginTransmission(addr);
  Wire.write(data);
  Wire.endTransmission();
}

byte PCF8574_getReg(byte addr)
{
  Wire.requestFrom(addr, (uint8_t)0x1);

  if (Wire.available())
  {
    return Wire.read();
  }
  return 0xFF;
}

void PCF8574_KeyPadMatrixInit(byte addr)
{
  PCF8574_setReg(addr, 0xF0); // low nibble to output 0
}

byte PCF8574_KeyPadMatrixScan(byte addr)
{
  byte rowMask = 1;
  byte colData;

  colData = PCF8574_getReg(addr) & 0xF0;

  if (colData == 0xF0) { // no key pressed?
    return 0;            // no key pressed!
  }

  for (byte row = 0; row <= 4; row++)
  {
    if (row == 0) {
      PCF8574_setReg(addr, 0xFF); // no bit of port A to output
    }
    else
    {
      PCF8574_setReg(addr, ~rowMask); // one bit of port A to output 0
      rowMask <<= 1;
    }

    colData = PCF8574_getReg(addr) & 0xF0;

    if (colData != 0xF0) // any key pressed?
    {
      byte colMask = 0x10;

      for (byte col = 1; col <= 4; col++)
      {
        if ((colData & colMask) == 0) // this key pressed?
        {
          PCF8574_setReg(addr, 0xF0); // low nibble to output 0
          return (row << 4) | col;
        }
        colMask <<= 1;
      }
    }
  }

  PCF8574_setReg(addr, 0xF0); // low nibble to output 0
  return 0;                   // no key pressed!
}

// PCF8574 Direct //////////////////////////////////////////////////////////////

void PCF8574_KeyPadDirectInit(byte addr)
{
  PCF8574_setReg(addr, 0xFF); // all to input
}

byte PCF8574_KeyPadDirectScan(byte addr)
{
  byte colData;

  colData = PCF8574_getReg(addr);

  if (colData == 0xFF) { // no key pressed?
    return 0;            // no key pressed!
  }
  byte colMask = 0x01;

  for (byte col = 1; col <= 8; col++)
  {
    if ((colData & colMask) == 0) // this key pressed?
    {
      return col;
    }
    colMask <<= 1;
  }

  return 0; // no key pressed!
}

#endif // USES_P061
