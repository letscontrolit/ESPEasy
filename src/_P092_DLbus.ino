#ifdef USES_P092

//#######################################################################################################
//########################### Plugin 092: DL-bus from Technische Alternative ############################
//#######################################################################################################

/**************************************************\
   This plug-in reads and decodes the DL-Bus.
   The DL-Bus is used in heating control units e.g. sold by Technische Alternative (www.ta.co.at).

   The idea for this plug-in is based on Martin Kropf's project UVR31_RF24 (https://github.com/martinkropf/UVR31_RF24)

   The plug-in is tested and workis fine for the ESR21 device.
   The plug-in should be also able to decode the information from the
   UVR31, UVR1611 and UVR 61-3 devices.

   The selected input needs a voltage divider as follows because the DL-Bus runs on 12 volts.
   DLbus@12V - 8k6 - input@3.3V - 3k3 - ground
\**************************************************/

#define PLUGIN_092
#define PLUGIN_ID_092         92
//#define PLUGIN_092_DEBUG    // additional debug messages in the log
#define PLUGIN_NAME_092       "Heating - DL-Bus (Technische Alternative)"
#define PLUGIN_VALUENAME1_092 "Value"

#define P092_DLbus_OptionCount 8
#define P092_DLbus_ValueCount 1
#define P092_DLbus_DeviceCount 5

void P092_Pin_changed() ICACHE_RAM_ATTR;
volatile byte P092_DLB_Pin;
volatile boolean P092_receiving = false;    // receiving flag
boolean P092_init = false;
boolean P092_ReceivedOK = false;
unsigned long P092_LastReceived = 0;
byte P092_MaxIdx[P092_DLbus_OptionCount];
int P092_ValueType[P092_DLbus_ValueCount];
int P092_ValueIdx[P092_DLbus_ValueCount];
struct DataStruct
{
  byte          DataBytes;
  byte          DeviceByte0;
  byte          DeviceByte1;
  byte          DeviceBytes;
  byte          DontCareBytes;
  byte          TimeStampBytes;
  byte          MaxSensors;
  byte          MaxExtSensors;
  byte          OutputBytes;
  byte          SpeedBytes;
  byte          AnalogBytes;
  byte          VolumeBytes;
  byte          MaxHeatMeters;
  byte          CurrentHmBytes;
  byte          MWhBytes;

  uint16_t      MinPulseWidth;
  uint16_t      MaxPulseWidth;
  uint16_t      MinDoublePulseWidth;
  uint16_t      MaxDoublePulseWidth;
  byte          IdxSensor;
  byte          IdxExtSensor;
  byte          IdxOutput;
  byte          IdxDrehzahl;
  byte          IdxAnalog;
  byte          IdxHmRegister;
  byte          IdxVolume;
  byte          IdxHeatMeter1;
  byte          IdxkWh1;
  byte          IdxMWh1;
  byte          IdxHeatMeter2;
  byte          IdxkWh2;
  byte          IdxMWh2;
  byte          IdxHeatMeter3;
  byte          IdxkWh3;
  byte          IdxMWh3;
  byte          IdxCRC;
} DataSettings;

typedef struct {
  byte Idx;
  byte mode;
  float value;
} data_t;
data_t ReadData;

int P092_OptionTypes[P092_DLbus_OptionCount] = {
  // Index der Variablen
  0,  //F("None")
  1,  //F("Sensor")
  2,  //F("Ext. sensor")
  3,  //F("Digital output")
  4,  //F("Speed step")
  5,  //F("Analog output")
  6,  //F("Heat power (kW)")
  7   //F("Heat meter (MWh)")
};

int P092_OptionValueDecimals[P092_DLbus_OptionCount] = {
  // Dezimalstellen der Variablen
  0,  //F("None")
  1,  //[0,1°C]     F("Sensor")
  1,  //[0,1°C]     F("Ext. sensor")
  0,  //            F("Digital output")
  0,  //            F("Speed step")
  1,  //[0,1V]      F("Analog output")
  1,  //[0,1kW]     F("Heat power (kW)") Attention: UVR1611 in 0,01kW
  4   //[0,0001MWh] F("Heat meter (MWh)")
};

// decoding the manchester code
// pulse width @ 488hz: 1000ms/488 = 2,048ms = 2048µs
// 2048µs / 2 = 1024µs (2 pulses for one bit)
// pulse width @ 50hz: 1000ms/50 = 20ms = 20000µs
// 20000µs / 2 = 10000µs (2 pulses for one bit)
#define P092_pulse_width_488        1024   // µs
#define P092_pulse_width_50         10000  // µs

// % tolerance for variances at the pulse width
#define P092_percentage_variance    10
// 1001 or 0110 are two sequential pulses without transition
#define P092_double_pulse_width_488 (P092_pulse_width_488 * 2)
#define P092_double_pulse_width_50  (P092_pulse_width_50 * 2)

// calculating the tolerance limits for variances
#define P092_min_width_488          (P092_pulse_width_488 - (P092_pulse_width_488 *  P092_percentage_variance / 100))
#define P092_max_width_488          (P092_pulse_width_488 + (P092_pulse_width_488 * P092_percentage_variance / 100))
#define P092_double_min_width_488   (P092_double_pulse_width_488 - (P092_pulse_width_488 * P092_percentage_variance / 100))
#define P092_double_max_width_488   (P092_double_pulse_width_488 + (P092_pulse_width_488 * P092_percentage_variance / 100))
#define P092_min_width_50           (P092_pulse_width_50 - (P092_pulse_width_50 *  P092_percentage_variance / 100))
#define P092_max_width_50           (P092_pulse_width_50 + (P092_pulse_width_50 * P092_percentage_variance / 100))
#define P092_double_min_width_50    (P092_double_pulse_width_50 - (P092_pulse_width_50 * P092_percentage_variance / 100))
#define P092_double_max_width_50    (P092_double_pulse_width_50 + (P092_pulse_width_50 * P092_percentage_variance / 100))

boolean Plugin_092(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  int OptionIdx, CurIdx;
  const String plugin_092_Values[P092_DLbus_ValueCount] = {
    F("p092_Value")
  };
  const String plugin_092_Indizes[P092_DLbus_ValueCount] = {
    F("p092_Idx")
  };


  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_092;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = P092_DLbus_ValueCount;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        Device[deviceCount].DecimalsOnly = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_092);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_092));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        const String plugin_092_ValueNames[P092_DLbus_ValueCount] = {
          F(PLUGIN_VALUENAME1_092)
        };
        const String Options[P092_DLbus_OptionCount] = {
          F("None"),
          F("Sensor"),
          F("Ext. sensor"),
          F("Digital output"),
          F("Speed step"),
          F("Analog output"),
          F("Heat power (kW)"),
          F("Heat meter (MWh)")
        };
        const String Devices[P092_DLbus_DeviceCount] = { F("ESR21"), F("UVR31"), F("UVR1611"), F("UVR 61-3 (bis V8.2)"), F("UVR 61-3 (ab V8.3)") };
        int DevTypes[P092_DLbus_DeviceCount] = { 21, 31, 1611, 6132, 6133 };

        addFormSelector(F("DL-Bus Type"), F("p092_dlbtype"), P092_DLbus_DeviceCount, Devices, DevTypes, NULL, PCONFIG(0), true );

        // Calculation of the max indices for each sensor type
        switch (PCONFIG(0)) {
          case 21:  //ESR21
            P092_MaxIdx[0] = 0; // None
            P092_MaxIdx[1] = 3; // Sensor
            P092_MaxIdx[2] = 6; // Ext. sensor
            P092_MaxIdx[3] = 1; // Digital output
            P092_MaxIdx[4] = 1; // Speed step
            P092_MaxIdx[5] = 1; // Analog output
            P092_MaxIdx[6] = 1; // Heat power (kW)
            P092_MaxIdx[7] = 1; // Heat meter (MWh)
            break;
          case 31:  //UVR31
            P092_MaxIdx[0] = 0; // None
            P092_MaxIdx[1] = 3; // Sensor
            P092_MaxIdx[2] = 0; // Ext. sensor
            P092_MaxIdx[3] = 1; // Digital output
            P092_MaxIdx[4] = 0; // Speed step
            P092_MaxIdx[5] = 0; // Analog output
            P092_MaxIdx[6] = 0; // Heat power (kW)
            P092_MaxIdx[7] = 0; // Heat meter (MWh)
            break;
          case 1611:  //UVR1611
            P092_MaxIdx[0] = 0; // None
            P092_MaxIdx[1] = 16; // Sensor
            P092_MaxIdx[2] = 0; // Ext. sensor
            P092_MaxIdx[3] = 13; // Digital output
            P092_MaxIdx[4] = 4; // Speed step
            P092_MaxIdx[5] = 0; // Analog output
            P092_MaxIdx[6] = 2; // Heat power (kW)
            P092_MaxIdx[7] = 2; // Heat meter (MWh)
            break;
          case 6132:  //UVR 61-3 (bis V8.2)
            P092_MaxIdx[0] = 0; // None
            P092_MaxIdx[1] = 6; // Sensor
            P092_MaxIdx[2] = 0; // Ext. sensor
            P092_MaxIdx[3] = 8; // Digital output
            P092_MaxIdx[4] = 1; // Speed step
            P092_MaxIdx[5] = 1; // Analog output
            P092_MaxIdx[6] = 1; // Heat power (kW)
            P092_MaxIdx[7] = 1; // Heat meter (MWh)
            break;
          case 6133:  //UVR 61-3 (ab V8.3)
            P092_MaxIdx[0] = 0; // None
            P092_MaxIdx[1] = 6; // Sensor
            P092_MaxIdx[2] = 9; // Ext. sensor
            P092_MaxIdx[3] = 3; // Digital output
            P092_MaxIdx[4] = 1; // Speed step
            P092_MaxIdx[5] = 2; // Analog output
            P092_MaxIdx[6] = 3; // Heat power (kW)
            P092_MaxIdx[7] = 3; // Heat meter (MWh)
            break;
        }

        addFormSubHeader(F("Inputs"));

        for (int i = 0; i < P092_DLbus_ValueCount; i++) {
          P092_ValueType[i] = PCONFIG(i + 1) >> 8;
          P092_ValueIdx[i] = PCONFIG(i + 1) & 0x00FF;

          addFormSelector(plugin_092_ValueNames[i], plugin_092_Values[i], P092_DLbus_OptionCount, Options, P092_OptionTypes, NULL, P092_ValueType[i], true);
          if (P092_MaxIdx[P092_ValueType[i]] > 1) {
            CurIdx = P092_ValueIdx[i];
            if (CurIdx < 1) {
              CurIdx = 1;
            }
            if (CurIdx > P092_MaxIdx[P092_ValueType[i]]) {
              CurIdx = P092_MaxIdx[P092_ValueType[i]];
            }
	          addHtml(F(" Index: "));
            addNumericBox(plugin_092_Indizes[i], CurIdx, 1, P092_MaxIdx[P092_ValueType[i]]);
          }
        }

        UserVar[event->BaseVarIndex] = NAN;

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("p092_dlbtype"));
        Plugin_092_SetIndices(PCONFIG(0));

        for (int i = 0; i < P092_DLbus_ValueCount; i++) {
          OptionIdx = getFormItemInt(plugin_092_Values[i]);
          CurIdx = getFormItemInt(plugin_092_Indizes[i]);
          if (CurIdx < 1) {
            CurIdx = 1;
          }
          PCONFIG(i + 1) = (OptionIdx << 8) + CurIdx;
          ExtraTaskSettings.TaskDeviceValueDecimals[event->BaseVarIndex + i] = P092_OptionValueDecimals[OptionIdx];
        }

#ifdef PLUGIN_092_DEBUG
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	        P092_DLB_Pin = CONFIG_PIN1;

	        String log = F("PLUGIN_WEBFORM_SAVE :");
	        log += F(" DLB_Pin:");
	        log += P092_DLB_Pin;
	        log += F(" MinPulseWidth:");
	        log += DataSettings.MinPulseWidth;
	        log += F(" MaxPulseWidth:");
	        log += DataSettings.MaxPulseWidth;
	        log += F(" MinDoublePulseWidth:");
	        log += DataSettings.MinDoublePulseWidth;
	        log += F(" MaxDoublePulseWidth:");
	        log += DataSettings.MaxDoublePulseWidth;
	        log += F(" IdxSensor:");
	        log += DataSettings.IdxSensor;
	        log += F(" IdxExtSensor:");
	        log += DataSettings.IdxExtSensor;
	        log += F(" IdxOutput:");
	        log += DataSettings.IdxOutput;
	        if (DataSettings.SpeedBytes > 0) {
	          log += F(" IdxDrehzahl:");
	          log += DataSettings.IdxDrehzahl;
	        }
	        if (DataSettings.AnalogBytes > 0) {
	          log += F(" IdxAnalog:");
	          log += DataSettings.IdxAnalog;
	        }
	        if (DataSettings.MaxHeatMeters > 0) {
	          log += F(" IdxHmRegister:");
	          log += DataSettings.IdxHmRegister;
	        }
	        if (DataSettings.VolumeBytes > 0) {
	          log += F(" IdxVolume:");
	          log += DataSettings.IdxVolume;
	        }
	        if (DataSettings.MaxHeatMeters > 0) {
	          log += F(" IdxHM1:");
	          log += DataSettings.IdxHeatMeter1;
	          log += F(" IdxkWh1:");
	          log += DataSettings.IdxkWh1;
	          log += F(" IdxMWh1:");
	          log += DataSettings.IdxMWh1;
	        }
	        if (DataSettings.MaxHeatMeters > 1) {
	          log += F(" IdxHM2:");
	          log += DataSettings.IdxHeatMeter2;
	          log += F(" IdxkWh2:");
	          log += DataSettings.IdxkWh2;
	          log += F(" IdxMWh2:");
	          log += DataSettings.IdxMWh2;
	        }
	        if (DataSettings.MaxHeatMeters > 2) {
	          log += F(" IdxHM3:");
	          log += DataSettings.IdxHeatMeter3;
	          log += F(" IdxkWh3:");
	          log += DataSettings.IdxkWh3;
	          log += F(" IdxMWh3:");
	          log += DataSettings.IdxMWh3;
	        }
	        log += F(" IdxCRC:");
	        log += DataSettings.IdxCRC;
	        addLog(LOG_LEVEL_INFO, log);
				}
#endif  // PLUGIN_092_DEBUG
        UserVar[event->BaseVarIndex] = NAN;
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (P092_init) {
          addLog(LOG_LEVEL_ERROR, F("P092_init -> Already done!"));
          return true;
        }
        P092_init = true;
        addLog(LOG_LEVEL_ERROR, F("P092_init: attachInterrupt"));
        P092_DLB_Pin = CONFIG_PIN1;
        pinMode(P092_DLB_Pin, INPUT_PULLUP);
        P092_receiving = false;
        // on a CHANGE on the data pin P092_Pin_changed is called
        attachInterrupt(digitalPinToInterrupt(P092_DLB_Pin), P092_Pin_changed, CHANGE);
        UserVar[event->BaseVarIndex] = NAN;
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        addLog(LOG_LEVEL_ERROR, F("PLUGIN_092_READ"));
        if (P092_DLB_Pin != CONFIG_PIN1) {
          String log = F("## Error DL-Bus: Device Pin setting not correct!");
          log += F(" DLB_Pin:");
          log += P092_DLB_Pin;
          log += F(" Setting:");
          log += CONFIG_PIN1;
          addLog(LOG_LEVEL_ERROR, log);
          return false;
        }
        if (WiFi.status() != WL_CONNECTED) {
          addLog(LOG_LEVEL_ERROR, F("## Error DL-Bus: WiFi not connected!"));
          return false;
        }
        if (P092_receiving) {
          addLog(LOG_LEVEL_ERROR, F("## Error DL-Bus: Still P092_receiving!"));
          return false;
        }
        if (P092_init == false) {
          addLog(LOG_LEVEL_ERROR, F("## Error DL-Bus: Not initialized!"));
          return false;
        }
        Plugin_092_SetIndices(PCONFIG(0));

        if ((P092_ReceivedOK == false) || ((millis()-P092_LastReceived)>(Settings.TaskDeviceTimer[event->TaskIndex] * 1000/2))) {
          success = Plugin_092_Receiving();
          if (success)
            success = P092_Processing();
          if (success)
            success = P092_CheckCRC();
          if (success)
            P092_LastReceived = millis();
          P092_ReceivedOK = success;
        }
        else
          success = true;
        if (success)
        {
          for (int i = 0; i < P092_DLbus_ValueCount; i++) {
            OptionIdx = PCONFIG(i + 1) >> 8;
            CurIdx = PCONFIG(i + 1) & 0x00FF;
            if (P092_GetData(OptionIdx, CurIdx)) {
              UserVar[event->BaseVarIndex + i] = ReadData.value;
            }
            else {
              addLog(LOG_LEVEL_ERROR, F("Error: No reading!"));
            }
          }
        }
        else
          addLog(LOG_LEVEL_ERROR, F("## Error DL-Bus: No reading!"));
        break;
      }
  }
  return success;
}

void Plugin_092_SetIndices(int DeviceIndex) {
  // Set the indices for the DL bus packet
  switch (DeviceIndex) {
    case 21:  //ESR21
      DataSettings.DataBytes = 31;
      DataSettings.MinPulseWidth = P092_min_width_488;
      DataSettings.MaxPulseWidth = P092_max_width_488;
      DataSettings.MinDoublePulseWidth = P092_double_min_width_488;
      DataSettings.MaxDoublePulseWidth = P092_double_max_width_488;

      DataSettings.DeviceByte0 = 0x70;
      DataSettings.DeviceByte1 = 0x8F;
      DataSettings.DeviceBytes = 2;
      DataSettings.DontCareBytes = 0;
      DataSettings.TimeStampBytes = 0;
      DataSettings.MaxSensors = 3;
      DataSettings.MaxExtSensors = 6;
      DataSettings.OutputBytes = 1;
      DataSettings.SpeedBytes = 1;
      DataSettings.AnalogBytes = 1;
      DataSettings.VolumeBytes = 0;
      DataSettings.MaxHeatMeters = 1;
      DataSettings.CurrentHmBytes = 2;
      DataSettings.MWhBytes = 2;
      DataSettings.IdxCRC = 30;

      P092_OptionValueDecimals[6] = 1; //[0,1kW]     F("Heat power (kW)")
      break;
    case 31:  //UVR31
      DataSettings.DataBytes = 8;
      DataSettings.MinPulseWidth = P092_min_width_50;
      DataSettings.MaxPulseWidth = P092_max_width_50;
      DataSettings.MinDoublePulseWidth = P092_double_min_width_50;
      DataSettings.MaxDoublePulseWidth = P092_double_max_width_50;

      DataSettings.DeviceByte0 = 0x30;
      DataSettings.DeviceByte1 = 0;
      DataSettings.DeviceBytes = 1;
      DataSettings.DontCareBytes = 0;
      DataSettings.TimeStampBytes = 0;
      DataSettings.MaxSensors = 3;
      DataSettings.MaxExtSensors = 0;
      DataSettings.OutputBytes = 1;
      DataSettings.SpeedBytes = 0;
      DataSettings.AnalogBytes = 0;
      DataSettings.VolumeBytes = 0;
      DataSettings.MaxHeatMeters = 0;
      DataSettings.CurrentHmBytes = 0;
      DataSettings.MWhBytes = 0;
      DataSettings.IdxCRC = 0;
      break;
    case 1611:  //UVR1611
      DataSettings.DataBytes = 64;
      DataSettings.MinPulseWidth = P092_min_width_488;
      DataSettings.MaxPulseWidth = P092_max_width_488;
      DataSettings.MinDoublePulseWidth = P092_double_min_width_488;
      DataSettings.MaxDoublePulseWidth = P092_double_max_width_488;

      DataSettings.DeviceByte0 = 0x80;
      DataSettings.DeviceByte1 = 0x7F;
      DataSettings.DeviceBytes = 2;
      DataSettings.DontCareBytes = 1;
      DataSettings.TimeStampBytes = 5;
      DataSettings.MaxSensors = 16;
      DataSettings.MaxExtSensors = 0;
      DataSettings.OutputBytes = 2;
      DataSettings.SpeedBytes = 4;
      DataSettings.AnalogBytes = 0;
      DataSettings.VolumeBytes = 0;
      DataSettings.MaxHeatMeters = 2;
      DataSettings.CurrentHmBytes = 4;
      DataSettings.MWhBytes = 2;
      DataSettings.IdxCRC = 63;

      P092_OptionValueDecimals[6] = 2; //[0,01kW]     F("Heat power (kW)")
      break;
    case 6132:  //UVR 61-3 (up to V8.2)
      DataSettings.DataBytes = 35;
      DataSettings.MinPulseWidth = P092_min_width_488;
      DataSettings.MaxPulseWidth = P092_max_width_488;
      DataSettings.MinDoublePulseWidth = P092_double_min_width_488;
      DataSettings.MaxDoublePulseWidth = P092_double_max_width_488;

      DataSettings.DeviceByte0 = 0x90;
      DataSettings.DeviceByte1 = 0x6F;
      DataSettings.DeviceBytes = 2;
      DataSettings.DontCareBytes = 1;
      DataSettings.TimeStampBytes = 5;
      DataSettings.MaxSensors = 6;
      DataSettings.MaxExtSensors = 0;
      DataSettings.OutputBytes = 1;
      DataSettings.SpeedBytes = 1;
      DataSettings.AnalogBytes = 1;
      DataSettings.VolumeBytes = 2;
      DataSettings.MaxHeatMeters = 1;
      DataSettings.CurrentHmBytes = 2;
      DataSettings.MWhBytes = 4;
      DataSettings.IdxCRC = 34;

      P092_OptionValueDecimals[6] = 1; //[0,1kW]     F("Heat power (kW)")
      break;
    case 6133:  //UVR 61-3 (from V8.3)
      DataSettings.DataBytes = 62;
      DataSettings.MinPulseWidth = P092_min_width_488;
      DataSettings.MaxPulseWidth = P092_max_width_488;
      DataSettings.MinDoublePulseWidth = P092_double_min_width_488;
      DataSettings.MaxDoublePulseWidth = P092_double_max_width_488;

      DataSettings.DeviceByte0 = 0x90;
      DataSettings.DeviceByte1 = 0x9F;
      DataSettings.DeviceBytes = 2;
      DataSettings.DontCareBytes = 1;
      DataSettings.TimeStampBytes = 5;
      DataSettings.MaxSensors = 6;
      DataSettings.MaxExtSensors = 9;
      DataSettings.OutputBytes = 1;
      DataSettings.SpeedBytes = 1;
      DataSettings.AnalogBytes = 2;
      DataSettings.VolumeBytes = 0;
      DataSettings.MaxHeatMeters = 3;
      DataSettings.CurrentHmBytes = 2;
      DataSettings.MWhBytes = 2;
      DataSettings.IdxCRC = 61;

      P092_OptionValueDecimals[6] = 1; //[0,1kW]     F("Heat power (kW)")
      break;
  }
  DataSettings.IdxSensor               = DataSettings.DeviceBytes + DataSettings.DontCareBytes + DataSettings.TimeStampBytes;
  DataSettings.IdxExtSensor            = DataSettings.IdxSensor + 2 * DataSettings.MaxSensors;
  DataSettings.IdxOutput               = DataSettings.IdxExtSensor + 2 * DataSettings.MaxExtSensors;
  DataSettings.IdxDrehzahl             = DataSettings.IdxOutput + DataSettings.OutputBytes;
  DataSettings.IdxAnalog               = DataSettings.IdxDrehzahl + DataSettings.SpeedBytes;
  DataSettings.IdxHmRegister           = DataSettings.IdxAnalog + DataSettings.AnalogBytes;
  DataSettings.IdxVolume               = DataSettings.IdxHmRegister + 1;
  DataSettings.IdxHeatMeter1           = DataSettings.IdxVolume + DataSettings.VolumeBytes;
  DataSettings.IdxkWh1                 = DataSettings.IdxHeatMeter1 + DataSettings.CurrentHmBytes;
  DataSettings.IdxMWh1                 = DataSettings.IdxkWh1 + 2;
  DataSettings.IdxHeatMeter2           = DataSettings.IdxMWh1 + DataSettings.MWhBytes;
  DataSettings.IdxkWh2                 = DataSettings.IdxHeatMeter2 + DataSettings.CurrentHmBytes;
  DataSettings.IdxMWh2                 = DataSettings.IdxkWh2 + 2;
  DataSettings.IdxHeatMeter3           = DataSettings.IdxMWh2 + DataSettings.MWhBytes;
  DataSettings.IdxkWh3                 = DataSettings.IdxHeatMeter3 + DataSettings.CurrentHmBytes;
  DataSettings.IdxMWh3                 = DataSettings.IdxkWh3 + 2;
}
/*********************************************************************************************\
  DLBus subs to get values from the P092_receiving bitstream
\*********************************************************************************************/
// one data frame has <DataSettings.DataBytes> data bytes + SYNC, e.g. 64 * (8+1+1) + 16 = 656
// 656 * 2 = 1312 (twice as much as a data frame is saved
// so there's one complete data frame

#define MaxDataBytes 64
#define AdditionalRecBytes 2
#define MaxDataBits (((MaxDataBytes + AdditionalRecBytes) * (8 + 1 + 1) + 16) * 4)
volatile byte BitStream[MaxDataBits];           // received bit stream
volatile int pulse_count;                       // number of received pulses
volatile int pulse_number;                      // max naumber of the received pulses
volatile unsigned long last_bit_change = 0;     // remember the last transition
volatile word MinPulseWidth, MaxPulseWidth, MinDoublePulseWidth, MaxDoublePulseWidth;
volatile unsigned long TimeInMicros;            // current time in µs
volatile unsigned long time_diff;               // time difference to previous pulse in µs

byte ByteStream[MaxDataBits / 8 + 1];   // every bit gets sorted into a bitmap
int bit_number;                         // bit size of the data frame

byte WrongTimeCnt;
word WrongTimings[5][6];

int start_bit;                          // first bit of data frame

// sensor types
#define UNUSED      0b000
#define DIGITAL     0b001
#define TEMP        0b010
#define VOLUME_FLOW 0b011
#define RAYS        0b110
#define ROOM        0b111

// room sensor modes
#define AUTO        0b00
#define NORMAL      0b01
#define LOWER       0b10
#define STANDBY     0b11

// Flags for pulse width
#define flgSingleWidth	0x02
#define flgDoubleWidth	0x04

// Types for guessing
#define GuessTrueAndDouble	0x01
#define GuessFalseAndDouble	0x02
#define GuessTrueAndSingle	0x03
#define GuessFalseAndSingle	0x04

// heat meter
int power_index, kwh_index, mwh_index;

/****************\
  DLBus P092_receiving
\****************/

boolean Plugin_092_Receiving(void) {
  byte rawval, val;
  int i;
  noInterrupts ();                              // make sure we don't get interrupted before we are ready
  pulse_count = 0;
  pulse_number = (((DataSettings.DataBytes + AdditionalRecBytes) * (8 + 1 + 1) + 16) * 4);
  MinPulseWidth = DataSettings.MinPulseWidth;
  MaxPulseWidth = DataSettings.MaxPulseWidth;
  MinDoublePulseWidth = DataSettings.MinDoublePulseWidth;
  MaxDoublePulseWidth = DataSettings.MaxDoublePulseWidth;
  addLog(LOG_LEVEL_INFO, F("P092_receiving ..."));
  P092_receiving = true;
  interrupts ();                                // interrupts allowed now, next instruction WILL be executed
  unsigned long start=millis();
  while ((millis()-start)<100) {
    // wait 100 ms
    yield();
  }
  if (pulse_count == 0) {
    // nothing received
    P092_receiving = false;
    addLog(LOG_LEVEL_ERROR, F("Error: Nothing received! No DL bus connected!"));
    return false;
  }
  while (P092_receiving) {
    // wait for end of P092_receiving
    yield();
  }
  addLog(LOG_LEVEL_INFO, F("Receive stopped."));

  WrongTimeCnt = 0;
  pulse_count = 0;
  for (i = 0; i <= pulse_number; i++) {
    // store BitStream into ByteStream
    rawval = BitStream[i];
    if ((rawval & 0xF0) != 0) {
      // wrong time_diff
      if ((pulse_count > 0) && (WrongTimeCnt < 5)) {
        WrongTimings[WrongTimeCnt][0] = i;
        WrongTimings[WrongTimeCnt][1] = pulse_count;
        WrongTimings[WrongTimeCnt][2] = bit_number;
        WrongTimings[WrongTimeCnt][3] = rawval;
        if ((rawval==0x80) && (BitStream[i-1]==0x05)) {
          // Add two additional short pulses
          P092_process_bit(0);
          P092_process_bit(1);
          WrongTimings[WrongTimeCnt][4] = flgSingleWidth;
          WrongTimings[WrongTimeCnt][5] = flgSingleWidth+1;
        }
        else {
          WrongTimings[WrongTimeCnt][4] = 0xff;
          WrongTimings[WrongTimeCnt][5] = 0xff;
        }
        WrongTimeCnt++;
      }
    }
    else {
      val = rawval & 0x01;
      if ((rawval & flgDoubleWidth) == flgDoubleWidth) {
        // double pulse width
        P092_process_bit(!val);
        P092_process_bit(val);
      }
      else {
        // single pulse width
        P092_process_bit(val);
      }
    }
  }
  addLog(LOG_LEVEL_INFO, F("BitStream copied."));

#ifdef PLUGIN_092_DEBUG
  if (WrongTimeCnt > 0) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	    String log = F("Wrong Timings: ");
	    addLog(LOG_LEVEL_INFO, log);
	    for (i = 0; i < WrongTimeCnt; i++) {
	      log = i + 1;
	      log += F(": PulseCount:");
	      log += WrongTimings[i][1];
	      log += F(": BitCount:");
	      log += WrongTimings[i][2];
	      log += F(" Value:0x");
	      log += String(WrongTimings[i][3], HEX);
	      log += F(" ValueBefore:0x");
	      log += String(BitStream[WrongTimings[i][0] - 1], HEX);
	      log += F(" ValueAfter:0x");
	      log += String(BitStream[WrongTimings[i][0] + 1], HEX);
	      if (WrongTimings[i][4]!=0xff) {
	        log += F(" Added:0x");
	        log += String(WrongTimings[i][4], HEX);
	      }
	      if (WrongTimings[i][5]!=0xff) {
	        log += F(" Added:0x");
	        log += String(WrongTimings[i][5], HEX);
	      }
	      addLog(LOG_LEVEL_INFO, log);
	    }
		}
  }
#endif // PLUGIN_092_DEBUG
  return true;
}

/*****************\
  DLBus interrupt
\*****************/

void P092_Pin_changed(void) {
  TimeInMicros = micros();
  time_diff = TimeInMicros - last_bit_change;
  last_bit_change = TimeInMicros;
  if (P092_receiving) {
    byte val = digitalRead(P092_DLB_Pin); // read state
    // check pulse width
    if (time_diff >= 2*MinDoublePulseWidth)     val |= 0x80; // longer then 2x double pulse width
    else if (time_diff > MaxDoublePulseWidth)   val |= 0x40; // longer then double pulse width
    else if (time_diff >= MinDoublePulseWidth)  val |= flgDoubleWidth; // double pulse width
    else if (time_diff > MaxPulseWidth)         val |= 0x20; // between double and single pulse width
    else if (time_diff < MinPulseWidth)         val |= 0x10; // shorter then single pulse width
    else                                        val |= flgSingleWidth; // single pulse width
    BitStream[pulse_count] = val;                            // set bit
    pulse_count++;
    P092_receiving = (pulse_count < pulse_number);     // stop P092_receiving when data frame is complete
  }
}

/*****************\
  DLBus processing
\*****************/

void P092_process_bit(byte b) {
  // ignore first pulse
  pulse_count++;
  if (pulse_count % 2)
    return;
  bit_number = (pulse_count / 2);
  if (b)
    ByteStream[bit_number / 8] |= (1 << (bit_number % 8)); // set bit
  else
    ByteStream[bit_number / 8] &= ~(1 << (bit_number % 8)); // clear bit
}

boolean P092_Processing(void) {
boolean inverted = false;
String log;
  addLog(LOG_LEVEL_INFO, F("Processing..."));
  start_bit = P092_Analyze(); // find the data frame's beginning
  // inverted signal?
  while (start_bit == -1) {
    if (inverted) {
      return false;
    }
    addLog(LOG_LEVEL_INFO, F("Invert bit stream..."));
    P092_Invert(); // invert again
    inverted = true;
    start_bit = P092_Analyze();
    if (start_bit == -1) {
      addLog(LOG_LEVEL_ERROR, F("Error: No data frame available!"));
      return false;
    }
    if ((bit_number-start_bit)<(pulse_number/4)) {
      // no complete data frame available
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	      addLog(LOG_LEVEL_INFO, F("Start bit too close to end of stream!"));
	      log = F("# Required bits: ");
	      log += pulse_number/4;
	      log += F(" StartBit: ");
	      log += start_bit;
	      log += F(" / EndBit: ");
	      log += bit_number;
	      addLog(LOG_LEVEL_INFO, log);
			}
      start_bit = -1;
    }
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	  log = F("StartBit: ");
	  log += start_bit;
	  log += F(" / EndBit: ");
	  log += bit_number;
	  addLog(LOG_LEVEL_INFO, log);
	}
  P092_Trim(); // remove start and stop bits
  if (P092_CheckDevice()) // check connected device
    return true;
  else {
    addLog(LOG_LEVEL_ERROR, F("Error: Device not found!"));
    return false;
  }
}

int P092_Analyze() {
  byte sync=0;
  // find SYNC (16 * sequential 1)
  for (int i = 0; i < bit_number; i++) {
    if (P092_ReadBit(i))
      sync++;
    else
      sync = 0;
    if (sync == 16) {
      // finde erste 0 // find first 0
      while (P092_ReadBit(i) == 1)
        i++;
      return i; // beginning of data frame
    }
  }
  // no data frame available. check signal?
  return -1;
}
void P092_Invert() {
  for (int i = 0; i < bit_number; i++)
    P092_WriteBit(i, P092_ReadBit(i) ? 0 : 1); // invert every bit
}
byte P092_ReadBit(int pos) {
  int row = pos / 8; // detect position in bitmap
  int col = pos % 8;
  return (((ByteStream[row]) >> (col)) & 0x01); // return bit
}
void P092_WriteBit(int pos, byte set) {
  int row = pos / 8; // detect position in bitmap
  int col = pos % 8;
  if (set)
    ByteStream[row] |= 1 << col; // set bit
  else
    ByteStream[row] &= ~(1 << col); // clear bit
}
void P092_Trim() {
  for (int i = start_bit, bit = 0; i < bit_number; i++) {
    int offset = i - start_bit;
    // ignore start and stop bits:
    // start bits: 0 10 20 30, also  x    % 10 == 0
    // stop bits:  9 19 29 39, also (x+1) % 10 == 0
    if (offset % 10 && (offset + 1) % 10) {
      P092_WriteBit(bit, P092_ReadBit(i));
      bit++;
    }
  }
}

/****************\
  DLBus get data
\****************/

boolean P092_CheckDevice() {
  // Data frame of a device?
  if (ByteStream[0] == DataSettings.DeviceByte0) {
    if ((DataSettings.DeviceBytes == 1) || (ByteStream[1] == DataSettings.DeviceByte1))
      return true;
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	  String log = F("# Received DeviceByte(s): 0x");
	  log += String(ByteStream[0], HEX);
	  if (DataSettings.DeviceBytes > 1)
	    log += String(ByteStream[1], HEX);
	  log += F(" Requested: 0x");
	  log += String(DataSettings.DeviceByte0, HEX);
	  if (DataSettings.DeviceBytes > 1)
	    log += String(DataSettings.DeviceByte1, HEX);
	  addLog(LOG_LEVEL_INFO, log);
	}
  return false;
}

boolean P092_CheckCRC() {
  // CRC check sum
  if (DataSettings.IdxCRC == 0)
    return true;
  addLog(LOG_LEVEL_INFO, F("Check CRC..."));
  word dataSum = 0;
  for (int i = 0; i < DataSettings.IdxCRC; i++)
    dataSum = dataSum + ByteStream[i];
  dataSum = dataSum & 0xff;
  if (dataSum == ByteStream[DataSettings.IdxCRC])
    return true;
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	  String log = F("# Calculated CRC: 0x");
	  log += String(dataSum, HEX);
	  log += F(" Received: 0x");
	  log += String(ByteStream[DataSettings.IdxCRC], HEX);
	  addLog(LOG_LEVEL_INFO, log);
	}
  return false;
}

boolean P092_GetData(int OptionIdx, int CurIdx) {
  String log;
  boolean result = false;
  switch (OptionIdx) {
    case 1:    //F("Sensor")
      log = F("Get Sensor");
      log += CurIdx;
      if (CurIdx > DataSettings.MaxSensors) {
        result=false;
        break;
      }
      ReadData.Idx = DataSettings.IdxSensor;
      result = P092_fetch_sensor(CurIdx);
      break;
    case 2:    //F("Sensor")
      log = F("Get ExtSensor");
      log += CurIdx;
      if (CurIdx > DataSettings.MaxExtSensors) {
        result=false;
        break;
      }
      ReadData.Idx = DataSettings.IdxExtSensor;
      result = P092_fetch_sensor(CurIdx);
      break;
    case 3:  //F("Digital output")
      log = F("Get DigitalOutput");
      log += CurIdx;
      if (CurIdx > (8 * DataSettings.OutputBytes)) {
        result=false;
        break;
      }
      result = P092_fetch_output(CurIdx);
      break;
    case 4:  //F("Speed step")
      log = F("Get SpeedStep");
      log += CurIdx;
      if (CurIdx > DataSettings.SpeedBytes) {
        result=false;
        break;
      }
      result = P092_fetch_speed(CurIdx);
      break;
    case 5:  //F("Analog output")
      log = F("Get AnalogOutput");
      log += CurIdx;
      if (CurIdx > DataSettings.AnalogBytes) {
        result=false;
        break;
      }
      result = P092_fetch_analog(CurIdx);
      break;
    case 6:  //F("Heat power (kW)")
      log = F("Get HeatPower");
      log += CurIdx;
      if (CurIdx > DataSettings.MaxHeatMeters) {
        result=false;
        break;
      }
      result = P092_fetch_heatpower(CurIdx);
      break;
    case 7:  //F("Heat meter (MWh)"
      log = F("Get HeatMeter");
      log += CurIdx;
      if (CurIdx > DataSettings.MaxHeatMeters) {
        result=false;
        break;
      }
      result = P092_fetch_heatmeter(CurIdx);
      break;
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	  log += F(": ");
	  if (result) {
	    log += String(ReadData.value, 1);
	  }
	  else {
	    log += F("nan");
	  }
	  addLog(LOG_LEVEL_INFO, log);
	}
  return result;
}

boolean P092_fetch_sensor(int number) {
  float value;
  ReadData.mode = -1;
  number = ReadData.Idx + (number - 1) * 2;
  int sensorvalue = (ByteStream[number + 1] << 8) | ByteStream[number];
  if (sensorvalue == 0) {
    return false;
  }
  byte sensortype = (sensorvalue & 0x7000) >> 12;
  if (!(sensorvalue & 0x8000)) { // sign positive
    sensorvalue &= 0xfff;
    // calculations for different sensor types
    switch (sensortype) {
      case DIGITAL:
        value = false;
        break;
      case TEMP:
        value = sensorvalue * 0.1;
        break;
      case RAYS:
        value = sensorvalue;
        break;
      case VOLUME_FLOW:
        value = sensorvalue * 4;
        break;
      case ROOM:
        ReadData.mode = (sensorvalue & 0x600) >> 9;
        value = (sensorvalue & 0x1ff) * 0.1;
        break;
      default:
        return false;
    }
  }
  else { // sign negative
    sensorvalue |= 0xf000;
    // calculations for different sensor types
    switch (sensortype) {
      case DIGITAL:
        value = true;
        break;
      case TEMP:
        value = (sensorvalue - 65536) * 0.1;
        break;
      case RAYS:
        value = sensorvalue - 65536;
        break;
      case VOLUME_FLOW:
        value = (sensorvalue - 65536) * 4;
        break;
      case ROOM:
        ReadData.mode = (sensorvalue & 0x600) >> 9;
        value = ((sensorvalue & 0x1ff) - 65536) * 0.1;
        break;
      default:
        return false;
    }
  }
  ReadData.value = value;
  return true;
}

boolean P092_fetch_output(int number) {
  int outputs;

  if (DataSettings.OutputBytes > 1)
    outputs = (ByteStream[DataSettings.IdxOutput + 1] << 8) | ByteStream[DataSettings.IdxOutput];
  else
    outputs = ByteStream[DataSettings.IdxOutput];
  ReadData.value = (!!(outputs & (1 << (number - 1))));
  return true;
}

boolean P092_fetch_speed(int number) {
  byte speedbyte;

  speedbyte = ByteStream[DataSettings.IdxDrehzahl + (number - 1)];
  if ((speedbyte & 0x80) == 0x80)
    return false;
  ReadData.value = (speedbyte & 0x1f);
  return true;
}

boolean P092_fetch_analog(int number) {
  byte analogbyte;

  analogbyte = ByteStream[DataSettings.IdxAnalog + (number - 1)];
  if ((analogbyte & 0x80) == 0x80)
    return false;
  ReadData.value = (analogbyte * 0.1);
  return true;
}

boolean P092_CheckHmRegister(int number) {
  switch (number) {
    case 1:
      if ((ByteStream[DataSettings.IdxHmRegister] & 0x1) != 0x01)
        return false;
      power_index = DataSettings.IdxHeatMeter1;
      kwh_index = DataSettings.IdxkWh1;
      mwh_index = DataSettings.IdxMWh1;
      break;
    case 2:
      if ((ByteStream[DataSettings.IdxHmRegister] & 0x2) != 0x02)
        return false;
      power_index = DataSettings.IdxHeatMeter2;
      kwh_index = DataSettings.IdxkWh2;
      mwh_index = DataSettings.IdxMWh2;
      break;
    case 3:
      if ((ByteStream[DataSettings.IdxHmRegister] & 0x4) != 0x04)
        return false;
      power_index = DataSettings.IdxHeatMeter3;
      kwh_index = DataSettings.IdxkWh3;
      mwh_index = DataSettings.IdxMWh3;
      break;
    default:
      return false;
  }
  return true;
}

boolean P092_fetch_heatpower(int number) {
  // current power
  int high;

  if (P092_CheckHmRegister(number) == false)
    return false;
  byte b1 = ByteStream[power_index];
  byte b2 = ByteStream[power_index + 1];
  if (DataSettings.CurrentHmBytes > 2) {
    byte b3 = ByteStream[power_index + 2];
    byte b4 = ByteStream[power_index + 3];
    high = 65536 * b4 + 256 * b3 + b2;
    int low = (b1 * 10) / 256;
    if (!(b4 & 0x80)) // sign positive
      ReadData.value = (10 * high + low) / 100;
    else // sign negative
      ReadData.value = (10 * (high - 65536) - low) / 100;
  }
  else {
    high = (b2 << 8) | b1;
    if ((b2 & 0x80) == 0) // sign positive
      ReadData.value = high / 10;
    else // sign negative
      ReadData.value = (high - 65536) / 10;
  }
  return true;
}

boolean P092_fetch_heatmeter(int number) {
  // heat meter
  int heat_meter;
  float heat_meter_mwh;

  if (P092_CheckHmRegister(number) == false)
    return false;
  heat_meter = (ByteStream[kwh_index + 1] << 8) | ByteStream[kwh_index];
  heat_meter_mwh = (heat_meter * 0.1) / 1000; // in MWh
  if (heat_meter_mwh > 1.0) {
    // in kWh
    heat_meter = heat_meter_mwh;
    heat_meter_mwh -= heat_meter;
  }
  // MWh
  heat_meter = (ByteStream[mwh_index + 1] << 8) | ByteStream[mwh_index];
  ReadData.value = heat_meter_mwh + heat_meter;
  return true;
}

#endif // USES_P092
