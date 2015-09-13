//#######################################################################################################
//#################################### Plugin 004: TempSensor Dallas DS18B20  ###########################
//#######################################################################################################

#define PLUGIN_004
#define PLUGIN_ID_004        4

uint8_t Plugin_004_DallasPin;

boolean Plugin_004(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static unsigned int Call_Status = 0x00; // Each bit represents one relative port. 0=not called before, 1=already called before.

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_004;
        strcpy(Device[deviceCount].Name, "Temperature DS18b20");
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        strcpy(Device[deviceCount].ValueNames[0], "Temperature");
        break;
      }

    case PLUGIN_COMMAND:
      {
        int DSTemp;                           // Temperature in 16-bit Dallas format.
        byte ScratchPad[12];                  // Scratchpad buffer Dallas sensor.
        byte RelativePort = Settings.TaskDevicePin1[event->TaskIndex];

        Plugin_004_DallasPin = Settings.TaskDevicePin1[event->TaskIndex];

        noInterrupts();
        while (!(bitRead(Call_Status, RelativePort)))
        {
          // if this is the very first call to the sensor on this port, reset it to wake it up
          boolean present = Plugin_004_DS_reset();
          bitSet(Call_Status, RelativePort);
        }
        boolean present = Plugin_004_DS_reset(); Plugin_004_DS_write(0xCC /* rom skip */); Plugin_004_DS_write(0x44 /* start conversion */);
        interrupts();

        if (present)
        {
          delay(800);     // neccesary delay

          noInterrupts();
          Plugin_004_DS_reset(); Plugin_004_DS_write(0xCC /* rom skip */); Plugin_004_DS_write(0xBE /* Read Scratchpad */);

          digitalWrite(Plugin_004_DallasPin, LOW);
          pinMode(Plugin_004_DallasPin, INPUT);

          for (byte i = 0; i < 9; i++)            // copy 8 bytes
            ScratchPad[i] = Plugin_004_DS_read();
          interrupts();

          DSTemp = (ScratchPad[1] << 8) + ScratchPad[0];
          UserVar[event->BaseVarIndex] = (float(DSTemp) * 0.0625);
          Serial.print("DS   : Temperature: ");
          Serial.println(UserVar[event->BaseVarIndex]);
          success = true;
        }
        break;
      }

  }
  return success;
}

uint8_t Plugin_004_DS_read(void)
{
  uint8_t bitMask;
  uint8_t r = 0;
  uint8_t BitRead;

  for (bitMask = 0x01; bitMask; bitMask <<= 1)
  {
    pinMode(Plugin_004_DallasPin, OUTPUT);
    digitalWrite(Plugin_004_DallasPin, LOW);
    delayMicroseconds(3);

    pinMode(Plugin_004_DallasPin, INPUT); // let pin float, pull up will raise
    delayMicroseconds(10);
    BitRead = digitalRead(Plugin_004_DallasPin);
    delayMicroseconds(53);

    if (BitRead)
      r |= bitMask;
  }
  return r;
}

void Plugin_004_DS_write(uint8_t ByteToWrite)
{
  uint8_t bitMask;

  pinMode(Plugin_004_DallasPin, OUTPUT);
  for (bitMask = 0x01; bitMask; bitMask <<= 1)
  { // BitWrite
    digitalWrite(Plugin_004_DallasPin, LOW);
    if (((bitMask & ByteToWrite) ? 1 : 0) & 1)
    {
      delayMicroseconds(5);// Dallas spec.= 5..15 uSec.
      digitalWrite(Plugin_004_DallasPin, HIGH);
      delayMicroseconds(55);// Dallas spec.= 60uSec.
    }
    else
    {
      delayMicroseconds(55);// Dallas spec.= 60uSec.
      digitalWrite(Plugin_004_DallasPin, HIGH);
      delayMicroseconds(5);// Dallas spec.= 5..15 uSec.
    }
  }
}

uint8_t Plugin_004_DS_reset()
{
  uint8_t r;
  uint8_t retries = 125;

  pinMode(Plugin_004_DallasPin, INPUT);
  do  {  // wait until the wire is high... just in case
    if (--retries == 0) return 0;
    delayMicroseconds(2);
  } while ( !digitalRead(Plugin_004_DallasPin));

  pinMode(Plugin_004_DallasPin, OUTPUT); digitalWrite(Plugin_004_DallasPin, LOW);
  delayMicroseconds(492); // Dallas spec. = Min. 480uSec. Arduino 500uSec.
  pinMode(Plugin_004_DallasPin, INPUT); //Float
  delayMicroseconds(40);
  r = !digitalRead(Plugin_004_DallasPin);
  delayMicroseconds(420);
  return r;
}

