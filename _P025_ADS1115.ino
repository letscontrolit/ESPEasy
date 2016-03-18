//#######################################################################################################
//#################################### Plugin 025: ADS1115 I2C 0x48)  ###############################################
//#######################################################################################################

// MyMessage *msgAnalog025; // Mysensors

#define PLUGIN_025
#define PLUGIN_ID_025 25
#define PLUGIN_NAME_025 "Analog input - ADS1115"
#define PLUGIN_VALUENAME1_025 "Analog"

boolean Plugin_025_init = false;
// byte Plugin_Switch_Pin = 0;

static uint16_t readRegister025(uint8_t i2cAddress, uint8_t reg) {
  Wire.beginTransmission(i2cAddress);
  Wire.write((0x00));
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress, (uint8_t)2);
  return ((Wire.read() << 8) | Wire.read());  
}

boolean Plugin_025(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte portValue = 0;
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_025;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 4;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_025);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_025));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        #define ADS1115_GAIN_OPTION 6

        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[ADS1115_GAIN_OPTION];
        uint optionValues[ADS1115_GAIN_OPTION];
        optionValues[0] = (0x00);
        options[0] = F("2/3x gain 6.144V 0.1875mV");
        optionValues[1] = (0x02);
        options[1] = F("1x gain 4.096V 0.125mV");
        optionValues[2] = (0x04);
        options[2] = F("2x gain 2.048V 0.0625mV");
        optionValues[3] = (0x06);
        options[3] = F("4x gain 1.024V 0.03125mV");
        optionValues[4] = (0x08);
        options[4] = F("8x gain 0.512V 0.015625mV");
        optionValues[5] = (0x0A);
        options[5] = F("16x gain 0.256V 0.0078125V");

        string += F("<TR><TD>Gain:<TD><select name='plugin_025_gain'>");
        for (byte x = 0; x < ADS1115_GAIN_OPTION; x++)
        {
          string += F("<option value='");
          string += optionValues[x];
          string += "'";
          if (choice == optionValues[x])
            string += F(" selected");
          string += ">";
          string += options[x];
          string += F("</option>");
        }
        string += F("</select>");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_025_gain");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        Plugin_025_init = false; // Force device setup next time
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_025_init = true;
//        Plugin_Switch_Pin = Settings.TaskDevicePin1[event->TaskIndex];
//        if (!msgAnalog025)  //Mysensors
//          msgDust025 = new MyMessage(event->BaseVarIndex, V_LEVEL); //Mysensors
//        present(event->BaseVarIndex, S_DUST); //Mysensors
//        Serial.print("Present ADS1115: "); // Mysensors
//        Serial.println(event->BaseVarIndex); // Mysensors
//        if (Settings.TaskDevicePin1[event->TaskIndex] != -1)
//        {
//            pinMode(Plugin_Switch_Pin, OUTPUT);
//            digitalWrite(Plugin_Switch_Pin, HIGH);
//        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        uint8_t m_gain = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
//        noInterrupts();
        int value;
        value = 0;
//        if (Settings.TaskDevicePin1[event->TaskIndex] != -1)
//        {
//          Plugin_Switch_Pin = Settings.TaskDevicePin1[event->TaskIndex];
//          digitalWrite(Plugin_Switch_Pin, LOW);
//          delayMicroseconds(280);
//        }
        byte unit = (Settings.TaskDevicePort[event->TaskIndex] - 1) / 4;
        byte port = Settings.TaskDevicePort[event->TaskIndex] - (unit * 4);
        uint8_t address = 0x48 + unit;
        // get the current pin value

 uint16_t config = (0x0003)    |  // Disable the comparator (default val)
                   (0x0000)    |  // Non-latching (default val)
                   (0x0000)    |  // Alert/Rdy active low   (default val)
                   (0x0000)    |  // Traditional comparator (default val)
                   (0x0080)    |  // 1600 samples per second (default)
                   (0x0100) ;      // Single-shot mode (default)

  // m_Gain = (0x0000);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // m_Gain = (0x0200);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // m_Gain = (0x0400);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // m_Gain = (0x0600);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // m_Gain = (0x0800);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // m_Gain = (0x0A00);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  //      config |= m_gain;
  //      config |= (0x0000);
        switch (m_gain)
        {
        case (0x00):
          config |= (0x0000);
          break;
        case (0x02):
          config |= (0x0200);
          break;
        case (0x04):
          config |= (0x0400);
          break;
        case (0x06):
          config |= (0x0600);
          break;
        case (0x08):
          config |= (0x0800);
          break;
        case (0x0A):
          config |= (0x0A00);
          break;
        }
        switch (port)
        {
        case (0):
          config |= (0x4000);
          break;
        case (1):
          config |= (0x5000);
          break;
        case (2):
          config |= (0x6000);
          break;
        case (3):
          config |= (0x7000);
          break;
        }
        config |= (0x8000);
        Wire.beginTransmission(address);
        Wire.write((uint8_t)(0x01));
        Wire.write((uint8_t)(config>>8));
        Wire.write((uint8_t)(config & 0xFF));
        Wire.endTransmission();
        delay(8);
        UserVar[event->BaseVarIndex] = (float) readRegister025((address), (0x00)) ;  
        String log = F("ADS1115  : Analog value: ");
        log += UserVar[event->BaseVarIndex];
//        send(msgDust025->set(UserVar[event->BaseVarIndex], 1));  // Mysensors
        addLog(LOG_LEVEL_INFO,log);
        success = true;
//        if (Settings.TaskDevicePin1[event->TaskIndex] != -1)
//        {
//          delayMicroseconds(40);
//          digitalWrite(Plugin_Switch_Pin, HIGH);
//        }
//        interrupts();
        break;
      }
  }
  return success;
}
