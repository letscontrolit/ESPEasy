#include "_Plugin_Helper.h"
#ifdef USES_P063
//#######################################################################################################
//#################################### Plugin 063: TTP229 KeyPad ########################################
//#######################################################################################################

// ESPEasy Plugin to scan a 16 key touch pad chip TTP229
// written by Jochen Krapf (jk@nerd2nerd.org)

// Important: There are several types of TTP299 chips with different features available. They are named all TTP229 but differ in the letter(s) followed.
// On the china boards (found on eBay and AliExpress) the TTP229-B is used which has NO! I2C-interface. It uses a proprietary serial protocol with clock (SCL) and bidirectional data (SDO)

// ScanCode;
// Value 1...16 for the key number
// No key - the code 0
// If more than one key is pressed, the scan code is the code with the lowest value

// If ScanCode is unchecked the value is the KeyMap 1.Key=1, 2.Key=2, 3.Key=4, 4.Key=8 ... 16.Key=32768
// If more than one key is pressed, the value is sum of all KeyMap-values

// Electronics:
// Connect SCL to 1st GPIO and SDO to 2nd GPIO. Use 3.3 volt for VCC.
// Set the jumper for 16 key mode (TP2=jumper3). Additional set jumper for multi-key (TP3=jumper4, TP4=jumper5).
// Schematics: https://www.openimpulse.com/blog/wp-content/uploads/wpsc/downloadables/TTP229B-Schematic-Diagram.pdf
// Datasheet: http://www.datasheet4u.com/download_new.php?id=996751

#define PLUGIN_063
#define PLUGIN_ID_063         63
#define PLUGIN_NAME_063       "Keypad - TTP229 Touch"
#define PLUGIN_VALUENAME1_063 "ScanCode"



uint16_t readTTP229(int16_t pinSCL, int16_t pinSDO)
{
  uint16_t value = 0;
  uint16_t mask = 1;

  pinMode(pinSDO, OUTPUT);
  digitalWrite(pinSDO, HIGH);
  delayMicroseconds(100);

  digitalWrite(pinSDO, LOW);
  delayMicroseconds(10);

  pinMode(pinSDO, INPUT);
  for (byte i = 0; i < 16; i++)
  {
    digitalWrite(pinSCL, HIGH);
    delayMicroseconds(1);
    digitalWrite(pinSCL, LOW);
    if (!digitalRead(pinSDO))
      value |= mask;
    delayMicroseconds(1);
    mask <<= 1;
  }

  return value;
}


boolean Plugin_063(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_063;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SWITCH;
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
        string = F(PLUGIN_NAME_063);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_063));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output("SCL");
        event->String2 = formatGpioName_bidirectional("SDO");
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormCheckBox(F("ScanCode"), F("scancode"), PCONFIG(1));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(1) = isFormItemChecked(F("scancode"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        portStatusStruct newStatus;

        int16_t pinSCL = CONFIG_PIN1;
        int16_t pinSDO = CONFIG_PIN2;

        String log = F("Tkey : GPIO: ");
        log += pinSCL;
        log += ' ';
        log += pinSDO;
        addLog(LOG_LEVEL_INFO, log);

        if (pinSCL >= 0 && pinSDO >= 0)
        {
          pinMode(pinSCL, OUTPUT);
          digitalWrite(pinSCL, LOW);
          uint32_t key = createKey(PLUGIN_ID_063,pinSCL);
          // WARNING: operator [] creates an entry in the map if key does not exist
          newStatus = globalMapPortStatus[key];
          newStatus.task++; // add this GPIO/port as a task
          newStatus.mode = PIN_MODE_OUTPUT;
          newStatus.state = 0;
          savePortStatus(key,newStatus);
          //setPinState(PLUGIN_ID_063, pinSCL, PIN_MODE_OUTPUT, 0);

          pinMode(pinSDO, OUTPUT);
          digitalWrite(pinSDO, LOW);
          key = createKey(PLUGIN_ID_063,pinSDO);
          // WARNING: operator [] creates an entry in the map if key does not exist
          newStatus = globalMapPortStatus[key];
          newStatus.task++; // add this GPIO/port as a task
          newStatus.mode = PIN_MODE_INPUT;
          newStatus.state = 0;
          savePortStatus(key,newStatus);
          //setPinState(PLUGIN_ID_063, pinSDO, PIN_MODE_INPUT, 0);
        }

        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        static uint16_t keyLast = 0;
        int16_t pinSCL = CONFIG_PIN1;
        int16_t pinSDO = CONFIG_PIN2;

        if (pinSCL >= 0 && pinSDO >= 0)
        {
          uint16_t key = readTTP229(pinSCL, pinSDO);

          if (key && PCONFIG(1))
          {
            uint16_t colMask = 0x01;
            for (byte col = 1; col <= 16; col++)
            {
              if (key & colMask)   // this key pressed?
              {
                key = col;
                break;
              }
              colMask <<= 1;
            }
          }

          if (keyLast != key)
          {
            keyLast = key;
            UserVar[event->BaseVarIndex] = (float)key;
            event->sensorType = Sensor_VType::SENSOR_TYPE_SWITCH;

            String log = F("Tkey : ");
            if (PCONFIG(1))
              log = F("ScanCode=0x");
            else
              log = F("KeyMap=0x");
            log += String(key, 16);
            addLog(LOG_LEVEL_INFO, log);

            sendData(event);
          }
        }

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // work is done in PLUGIN_TEN_PER_SECOND
        success = true;
        break;
      }

  }
  return success;
}
#endif // USES_P063
