// Tiny85 as I2C Watchdog / Busmonitor
// Version 0.1, 2016-01-08 

// Uses a customized TinyWire Slave library that does not hang on persistent SDA low conditions
//   - With this library, the MCU main sketch can still monitor I2C bus signal levels,
//       instead of hanging within the ISR routine
//   - Without this custom library, the MCU would not be able to reset the target if this happens
//
// This sketch also uses customized Software Serial code:
//   - Smaller size than the full librarym but TX only
//   - TX without IRQ blocking as it interferes sometimes with I2C traffic
//   - Serial debug can get garbled sometimes due to non IRQ blocking (but I2C is more important)
//==============================================================================================================
// ATMEL ATTINY85 GENERIC PINOUTS
//                          o-\/-+ 
//                  Reset  1|    |8  VCC 
//              D3/A3 PB3  2|    |7  PB2 D2/A1/I2C-SCL
//              D4/A2 PB4  3|    |6  PB1 D1/PWM1
//                    GND  4|    |5  PB0 D0/PWM0/I2C-SDA
//                          +----+ 
//==============================================================================================================

//==============================================================================================================
// ATMEL ATTINY85 PINOUTS used for watchdog purpose
//                          o-\/-+ 
//                  Reset  1|    |8  VCC 
//      Factory Reset Pin  2|    |7  I2C-SCL to ESP
//      (debug) Serial TX  3|    |6  Reset output to ESP
//                    GND  4|    |5  I2C-SDA to ESP
//                          +----+ 
//==============================================================================================================

// I2C command structure uses one or two bytes:
// 
//   <cmd>,[value]
//
// Commands below 0x80 are used to write data to register in settings struct
//
// Commands starting at 0x80 are special commands:
// Command 0x80 = store settings to EEPROM
// Command 0x81 = reset to factory default settings
// Command 0x82 = set I2C address, factory default = 0x26
// Command 0x83 = set pointer to settings struct for I2C readings
// Command 0x84 = reset statistics counters
// Command 0x85 = simulate (test) watchdog timeout
// Command 0xA5 = reset watchdog timer value to 0
//

#define SWSERIALDEBUG true
#define SWSERIAL false

#if SWSERIALDEBUG
#define SERIAL_TX_PIN 4
#endif

#if SWSERIAL
#include <SoftwareSerial.h>
//SoftwareSerial Serial(3,4);         // RX, TX
#endif

#include "TinyWireS_Custom.h"
#include <EEPROM.h>

#define PROJECT_PID               2015123101L
#define VERSION                             1
#define BUILD                               1

#define RESET_CAUSE_TIMEOUT                 1

struct SettingsStruct
{
  unsigned long PID;                        // (B0-3) 4 byte ID, wipes eeprom if missing
  int           Version;                    // (B4/5) Version, change if struct changes, wipes eeprom
  byte          TimeOut;                    // (B6)   Timeout in seconds
  byte          Action;                     // (B7)   Action on timeout, 1 = reset target
  byte          ResetPin;                   // (B8)   Reset output pin to toggle
  byte          Sleep;                      // (B9)   After reset, do nothing for x seconds
  byte          I2CAddress;                 // (B10)  Listen to this I2C Address (0x26 default)
  byte          I2CLoadThreshold;           // (B11)  Threshold for I2C bus load
  byte          ResetCounter;               // (B12)  Seconds counter for WD reset status
  byte          SleepCounter;               // (B13)  Seconds counter for sleep time
  byte          TargetResetCount;           // (B14)  Counts number of target resets
  byte          I2CBusInitCount;            // (B15)  Counts I2C bus init calls
  byte          I2CBusErrorCount;           // (B16)  Counts I2C bus threshold exceptions
  byte          Status;                     // (B17)  Status byte
  byte          LastResetCause;             // (B18)  Last reset cause code
} 
Settings;

unsigned long timer1Sec=0;                  // Timer triggers each second for WD reset count
unsigned long timer100mSec=0;               // Timer triggers 10 times/second, check I2C bus levels
volatile byte I2CBuffer[16];                // Buffer to store incoming I2C message
volatile byte I2CReceived=0;                // flag, will be set if I2C message received
unsigned long I2CActive;                    // Count active signal on I2C bus
unsigned long I2CIdle;                      // Count inactive signal on I2C bus
byte settingsPointer = 0;                   // pointer to specifig location in the settings struct

void(*Reset)(void)=0;
volatile uint8_t *receivePortRegister;

/********************************************************************************************\
 * Main setup routine 
 \*********************************************************************************************/
void setup()
{
#if SWSERIAL
  Serial.begin(9600);
#endif

#if SWSERIALDEBUG
  SoftwareSerial_init(SERIAL_TX_PIN);
#endif

  LoadSettings();
  if (Settings.Version != VERSION || Settings.PID != PROJECT_PID)
  {
#if SWSERIAL
    Serial.print(F("\nPID:"));
    Serial.println(Settings.PID);
    Serial.print(F("Version:"));
    Serial.println(Settings.Version);
    Serial.println(F("INIT : Incorrect PID or version!"));
#endif
    tws_delay(1000);
    ResetFactory();
  }

#if SWSERIAL
  Serial.println(F("\nTiny WD Boot...\n"));
  Serial.print(F("Free RAM  : "));
  Serial.println(freeRam());
  Serial.print(F("Timeout   : "));
  Serial.println(Settings.TimeOut);
  Serial.print(F("Action    : "));
  Serial.println(Settings.Action);
  Serial.print(F("Resetpin  : "));
  Serial.println(Settings.ResetPin);
  Serial.print(F("I2CAddress: "));
  Serial.println(Settings.I2CAddress, HEX);
#endif

#if SWSERIALDEBUG
  SoftwareSerial_println("Boot");
#endif

  pinMode(3, INPUT); 
  pinMode(4, OUTPUT);

  // if pin 3 held low for > 10 seconds after boot, perform hard reset!
  byte factoryResetCount = 0;
  while (digitalRead(3) == 0)
  {
    factoryResetCount++;
    tws_delay(100);
    if (factoryResetCount > 100)
    {
#if SWSERIALDEBUG
      SoftwareSerial_println("Hard Reset");
#endif
      ResetFactory();
    }
  }

  TinyWireS.begin(Settings.I2CAddress);
  TinyWireS.onRequest(requestEvent);
  TinyWireS.onReceive(receiveEvent);

  uint8_t port = digitalPinToPort(0);
  receivePortRegister = portInputRegister(port);

  timer1Sec = millis() + 1000;
  timer100mSec = millis() + 100;

  Settings.ResetCounter = 0;
  Settings.SleepCounter = 0;
  Settings.Status = 0;
}


/********************************************************************************************\
 * Main program loop 
 \*********************************************************************************************/
void loop()
{

  if (I2CReceived)
    I2CCommand();

  if((*receivePortRegister & 0x5) != 5)
    I2CActive++;
  else
    I2CIdle++;

  if (millis() > timer100mSec)
  {
    timer100mSec = millis() + 100;
    checkI2Cbus();
  }

  if (millis() > timer1Sec)
  {
    timer1Sec = millis() + 1000;

    Settings.I2CBusInitCount = TinyWireS.initCount();

    if (Settings.SleepCounter > 0)
    {
#if SWSERIAL
      Serial.print(F("Sleeping: "));
      Serial.println(int(Settings.SleepCounter));
#endif
      Settings.SleepCounter--;
    }
    else
    {
#if SWSERIAL
      Serial.print(F("ResetCounter: "));
      Serial.println(Settings.ResetCounter);
#endif
#if SWSERIALDEBUG
      SoftwareSerial_print("RC ");
      SoftwareSerial_print(int2str(Settings.ResetCounter));
      SoftwareSerial_print(" I ");
      SoftwareSerial_println(int2str(TinyWireS.initCount()));
#endif

      Settings.ResetCounter++;
      if ((Settings.TimeOut != 0) && (Settings.ResetCounter > Settings.TimeOut))
        timeOutAction();
    }
  }
  TinyWireS_stop_check();
}


/********************************************************************************************\
 * Check I2C bus load 
 \*********************************************************************************************/
void checkI2Cbus()
{
  byte ratio = (100 * I2CActive) / (I2CIdle + I2CActive);
  if (ratio > 0)
  {
#if SWSERIAL
    Serial.print(F("bus act: "));
    Serial.print(I2CActive);
    Serial.print(F(" idle: "));
    Serial.print(I2CIdle);
    Serial.print(F(" ratio: "));
    Serial.print(ratio);
    Serial.print(F(" I2C inits: "));
    Serial.println(TinyWireS.initCount());
#endif
  }

  I2CActive=0;
  I2CIdle=0;

  byte portState = *receivePortRegister & 0x5;

  if (ratio > Settings.I2CLoadThreshold)
  {
#if SWSERIAL
    Serial.print(F("I2C bus: "));
    Serial.println(portState);
    Serial.println(F("Reset I2C due to I2C threshold"));
#endif
#if SWSERIALDEBUG
    SoftwareSerial_println("I2C Err");
#endif
    Settings.I2CBusErrorCount++;
    TinyWireS.begin(Settings.I2CAddress);
    TinyWireS.onRequest(requestEvent);
    TinyWireS.onReceive(receiveEvent);
  }    
}


/********************************************************************************************\
 * Check I2C incoming command and perform actions 
 \*********************************************************************************************/
void I2CCommand()
{
#if SWSERIAL
  Serial.print(F("I2C bytes: "));
  Serial.print(I2CReceived);
  Serial.print(F( " Inits: "));
  Serial.print(TinyWireS.initCount());
  Serial.print(F( " cmd: "));
#endif

  if ((I2CBuffer[0] > 5) && (I2CBuffer[0] < sizeof(SettingsStruct)))
  {
    byte *pointerToByteToSave = (byte*)&Settings + I2CBuffer[0];
    *pointerToByteToSave = I2CBuffer[1];
  }
  else
  {
    switch(I2CBuffer[0])
    { 
    case 0x80:
      {
        SaveSettings();
        break;
      }

    case 0x81:
      {
        ResetFactory();
        break;
      }

    case 0x82:
      {
        Settings.I2CAddress = I2CBuffer[1];
        TinyWireS.begin(Settings.I2CAddress);
        TinyWireS.onRequest(requestEvent);
        TinyWireS.onReceive(receiveEvent);
        break;
      }

    case 0x83:
      {
        settingsPointer = I2CBuffer[1];
        break;
      }

    case 0x84:
      {
        Settings.TargetResetCount = 0;
        Settings.I2CBusInitCount = 0;
        Settings.I2CBusErrorCount = 0;
        break;
      }

    case 0x85:
      {
        timeOutAction();
        break;
      }

    case 0xA5:
      {
        Settings.Status &= 0xFE; // reset bit 0, target reset state indicator
        Settings.ResetCounter = 0;
        break;
      }

    }  // switch
  } // else

  I2CReceived = 0;
}


/********************************************************************************************\
 * Perform preprogrammed actions due to timeout on watchdog feed 
 \*********************************************************************************************/
void timeOutAction()
{
  switch(Settings.Action)
  {
  case 1:
    {
#if SWSERIAL
      Serial.println(F("Resetting target!"));
#endif
      Settings.LastResetCause = RESET_CAUSE_TIMEOUT;
      resetTarget();
      break;
    }
  }
}


/********************************************************************************************\
 * Reply to I2C read requests 
 \*********************************************************************************************/
void requestEvent()
{
  byte *pointerToByteToRead = (byte*)&Settings + settingsPointer;
  TinyWireS.send(*pointerToByteToRead);
}

/********************************************************************************************\
 * Receive and store incoming I2C writes 
 \*********************************************************************************************/
void receiveEvent(uint8_t howMany)
{

  if (howMany > 16)
    return;

  I2CReceived = howMany;

  for(byte x=0; x < howMany; x++)
    I2CBuffer[x] = TinyWireS.receive();
}


/********************************************************************************************\
 * Reset target device using digital output pin, should connect to target's reset input pin
 * Signal is active low
 \*********************************************************************************************/
void resetTarget()
{
  Settings.TargetResetCount++;
  Settings.Status |= 0x01; // set bit 0, reset state
  pinMode(Settings.ResetPin,OUTPUT);
  digitalWrite(Settings.ResetPin,LOW);
  tws_delay(500);
  digitalWrite(Settings.ResetPin,HIGH);
  pinMode(Settings.ResetPin,INPUT);
  Settings.ResetCounter=0;
  Settings.SleepCounter=Settings.Sleep;
}


/*********************************************************************************************\
 * Save settings to EEPROM memory.
 \*********************************************************************************************/
void SaveSettings(void)  
{
  char ByteToSave,*pointerToByteToSave=pointerToByteToSave=(char*)&Settings;

  for(int x=0; x<sizeof(struct SettingsStruct) ;x++)
  {
    EEPROM.write(x,*pointerToByteToSave); 
    pointerToByteToSave++;
  }  
}


/*********************************************************************************************\
 * Load settings from EEPROM memory.
 \*********************************************************************************************/
boolean LoadSettings()
{
  byte x;

  char ByteToSave,*pointerToByteToRead=(char*)&Settings;

  for(int x=0; x<sizeof(struct SettingsStruct);x++)
  {
    *pointerToByteToRead=EEPROM.read(x);
    pointerToByteToRead++;
  }
}


/********************************************************************************************\
 * Reset all settings to factory defaults
 \*********************************************************************************************/
void ResetFactory(void)
{
  Settings.PID              = PROJECT_PID;
  Settings.Version          = VERSION;
  Settings.TimeOut          = 70;
  Settings.Action           = 1;
  Settings.ResetPin         = 1;
  Settings.Sleep            = 30;
  Settings.I2CAddress       = 0x26;
  Settings.I2CLoadThreshold = 50;
  Settings.ResetCounter     = 0;
  Settings.SleepCounter     = 0;
  Settings.TargetResetCount = 0;
  Settings.I2CBusInitCount  = 0;
  Settings.I2CBusErrorCount = 0;
  Settings.Status           = 0;
  Settings.LastResetCause   = 0;
  SaveSettings();
  tws_delay(1000);
  Reset();
}


/********************************************************************************************\
 * Check free RAM 
 \*********************************************************************************************/
int freeRam() {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

char OutputLine[12];  // made global for both int2str and int2strhex, to save RAM.

/**********************************************************************************************\
 * Converteert een unsigned long naar een string met decimale integer.
 \*********************************************************************************************/
char* int2str(unsigned long x)
{
  //static char OutputLine[12];
  char* OutputLinePosPtr=&OutputLine[10];
  int y;

  *OutputLinePosPtr=0;

  if(x==0)
  {
    *--OutputLinePosPtr='0';
  }
  else
  {  
    while(x>0)
    {
      *--OutputLinePosPtr='0'+(x%10);
      x/=10;
    }
  }    
  return OutputLinePosPtr;
}

/**********************************************************************************************\
 * Converteert een unsigned long naar een hexadecimale string.
 \*********************************************************************************************/
char* int2strhex(unsigned long x)
{
  //static char OutputLine[12];
  char* OutputLinePosPtr=&OutputLine[10];
  int y;

  *OutputLinePosPtr=0;

  if(x==0)
  {
    *--OutputLinePosPtr='0';
  }
  else
  {  
    while(x>0)
    {
      y=x&0xf;

      if(y<10)
        *--OutputLinePosPtr='0'+y;
      else
        *--OutputLinePosPtr='A'+(y-10);

      x=x>>4;
      ;
    }
    *--OutputLinePosPtr='x';
    *--OutputLinePosPtr='0';
  }
  return OutputLinePosPtr;
}




