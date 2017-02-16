#ifdef PLUGIN_BUILD_DEV

//#######################################################################################################
//######################### Plugin 104: SRF02 Ultrasonic range finder sensor ############################
//#######################################################################################################

#define PLUGIN_104
#define PLUGIN_ID_104                         104
#define PLUGIN_NAME_104                       "Ultrasonic range finder - SRF02 [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_104                 "Distance"

#define SRF02_ADDRESS                         (0x70)  // default address (0x70 = datasheet address 0xE0)

#define SRF02_REG_COMMAND                     (0x00)  // a read on this register returns the software revision
#define SRF02_REG_UNUSED                      (0x01)
#define SRF02_REG_RANGE_HIGH_BYTE             (0x02)
#define SRF02_REG_RANGE_LOW_BYTE              (0x03)
#define SRF02_REG_AUTOTUNE_MINIMUM_HIGH_BYTE  (0x04)
#define SRF02_REG_AUTOTUNE_MINIMUM_LOW_BYTE   (0x05)

#define SRF02_CMD_REAL_RANGING_MODE_INCH      (0x50)
#define SRF02_CMD_REAL_RANGING_MODE_CM        (0x51)
#define SRF02_CMD_REAL_RANGING_MODE_US        (0x52)

#define SRF02_CMD_FAKE_RANGING_MODE_INCH      (0x56)
#define SRF02_CMD_FAKE_RANGING_MODE_CM        (0x57)
#define SRF02_CMD_FAKE_RANGING_MODE_US        (0x58)

#define SRF02_CMD_FORCE_AUTOTUNE_RESTART      (0x5C)

#define SRF02_CMD_I2C_CHANGE_SEQ_1            (0xA0)
#define SRF02_CMD_I2C_CHANGE_SEQ_2            (0xA5)
#define SRF02_CMD_I2C_CHANGE_SEQ_3            (0xAA)


uint8_t SRF02_i2caddr;

boolean Plugin_104(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_104;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_104);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_104));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        String options[1];
        options[0] = F("Distance");

        int optionValues[1];
        optionValues[0] = 0;

        string += F("<TR><TD>Report:<TD><select name='plugin_104_value'>");
        string += F("<option value='");
        string += optionValues[0];
        string += "'";
        if (choice == optionValues[0])
          string += F(" selected");
        string += ">";
        string += options[0];
        string += F("</option>");
        string += F("</select>");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_104_value");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_104_begin();
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        float value;
        value = Plugin_104_getDistance();
        UserVar[event->BaseVarIndex] = value;
        String log = F("SRF02 : value : ");
        log += value;
        log += F("mm");
        addLog(LOG_LEVEL_INFO,log);
        success = true;
        break;
      }
  }
  return success;
}

//**************************************************************************/
// I2C single byte write
//**************************************************************************/
void Plugin_104_wireWriteByte(uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(SRF02_i2caddr);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

//**************************************************************************/
// I2C two byte read
//**************************************************************************/
void Plugin_104_wireReadTwoBytes(uint8_t reg, uint16_t *value)
{
  Wire.beginTransmission(SRF02_i2caddr);
  Wire.write(reg);
  Wire.endTransmission();

  delayMicroseconds(10);

  Wire.requestFrom(SRF02_i2caddr, (uint8_t)2);
  *value = ((Wire.read() << 8) | Wire.read());
}

//**************************************************************************/
// Sensor setup
//**************************************************************************/
void Plugin_104_begin(void)
{
  SRF02_i2caddr = SRF02_ADDRESS;
}

//**************************************************************************/
// Report distance
//**************************************************************************/
float Plugin_104_getDistance()
{
  uint16_t value;

  Plugin_104_wireWriteByte(SRF02_REG_COMMAND, SRF02_CMD_REAL_RANGING_MODE_US);
  delay(70);                                                                    // transmit -> receive turnaround time (up to 65ms)
  Plugin_104_wireReadTwoBytes(SRF02_REG_RANGE_HIGH_BYTE, &value);

  return (float)value/(float)5.82750583;                                        // distance in [mm]  (2*1000/343.2)
}

#endif
