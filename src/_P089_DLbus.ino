//#######################################################################################################
//########################### Plugin 089: DL-bus from Technische Alternative ############################
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

#define PLUGIN_089
#define PLUGIN_ID_089         89
//#define PLUGIN_089_DEBUG
#define PLUGIN_NAME_089       "Heating - DL-Bus (Technische Alternative)"
#define PLUGIN_VALUENAME1_089 "Value"

#define DLbus_OptionCount 8
#define DLbus_ValueCount 1
#define DLbus_DeviceCount 5

void Plugin_089_Pin_changed() ICACHE_RAM_ATTR;
volatile byte Plugin_089_DLB_Pin;
volatile boolean receiving = false;                    // Übertragungs-Flag // currently receiving?
boolean Plugin_089_init = false;
boolean Plugin_089_ReceivedOK = false;
unsigned long Plugin_089_LastReceived = 0;
byte Plugin_089_MaxIdx[DLbus_OptionCount];
int Plugin_089_ValueType[DLbus_ValueCount];
int Plugin_089_ValueIdx[DLbus_ValueCount];
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

int OptionTypes[DLbus_OptionCount] = {
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

int OptionValueDecimals[DLbus_OptionCount] = {
  // Dezimalstellen der Variablen
  0,  //F("None")
  1,  //[0,1°C]     F("Sensor")
  1,  //[0,1°C]     F("Ext. sensor")
  0,  //            F("Digital output")
  0,  //            F("Speed step")
  1,  //[0,1V]      F("Analog output")
  1,  //[0,1kW]     F("Heat power (kW)") Achtung: UVR1611 in 0,01kW
  4   //[0,0001MWh] F("Heat meter (MWh)")
};

// Dekodierung des Manchester-Codes // decoding the manchester code
// Pulsweite bei 488hz: 1000ms/488 = 2,048ms = 2048µs
// 2048µs / 2 = 1024µs (2 Pulse für ein Bit)
// pulse width at 488hz: 1000ms/488 = 2,048ms = 2048µs
// 2048µs / 2 = 1024µs (2 pulses for one bit)
// Pulsweite bei 50hz: 1000ms/50 = 20ms = 20000µs
// 20000µs / 2 = 10000µs (2 Pulse für ein Bit)
// pulse width at 50hz: 1000ms/50 = 20ms = 20000µs
// 20000µs / 2 = 10000µs (2 pulses for one bit)
#define pulse_width_488          1024   // µs
#define pulse_width_50           10000  // µs

// % Toleranz für Abweichungen bei der Pulsweite
#define percentage_variance   10 // % tolerance for variances at the pulse width
// 1001 oder 0110 sind zwei aufeinanderfolgende Pulse ohne Übergang
// 1001 or 0110 are two sequential pulses without transition
#define double_pulse_width_488  (pulse_width_488 * 2)
#define double_pulse_width_50   (pulse_width_50 * 2)

// Berechnung der Toleranzgrenzen für Abweichungen
// calculating the tolerance limits for variances
#define min_width_488           (pulse_width_488 - (pulse_width_488 *  percentage_variance / 100))
#define max_width_488           (pulse_width_488 + (pulse_width_488 * percentage_variance / 100))
#define double_min_width_488    (double_pulse_width_488 - (pulse_width_488 * percentage_variance / 100))
#define double_max_width_488    (double_pulse_width_488 + (pulse_width_488 * percentage_variance / 100))
#define min_width_50            (pulse_width_50 - (pulse_width_50 *  percentage_variance / 100))
#define max_width_50            (pulse_width_50 + (pulse_width_50 * percentage_variance / 100))
#define double_min_width_50     (double_pulse_width_50 - (pulse_width_50 * percentage_variance / 100))
#define double_max_width_50     (double_pulse_width_50 + (pulse_width_50 * percentage_variance / 100))

boolean Plugin_089(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  int OptionIdx, CurIdx;
  const String plugin_089_Values[DLbus_ValueCount] = {
    F("p089_Value")
  };
  const String plugin_089_Indizes[DLbus_ValueCount] = {
    F("p089_Idx")
  };


  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_089;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = DLbus_ValueCount;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        Device[deviceCount].DecimalsOnly = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_089);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_089));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        const String plugin_089_ValueNames[DLbus_ValueCount] = {
          F(PLUGIN_VALUENAME1_089)
        };
        const String Options[DLbus_OptionCount] = {
          F("None"),
          F("Sensor"),
          F("Ext. sensor"),
          F("Digital output"),
          F("Speed step"),
          F("Analog output"),
          F("Heat power (kW)"),
          F("Heat meter (MWh)")
        };
        const String Devices[DLbus_DeviceCount] = { F("ESR21"), F("UVR31"), F("UVR1611"), F("UVR 61-3 (bis V8.2)"), F("UVR 61-3 (ab V8.3)") };
        int DevTypes[DLbus_DeviceCount] = { 21, 31, 1611, 6132, 6133 };

        addFormSelector(F("DL-Bus Type"), F("p089_dlbtype"), DLbus_DeviceCount, Devices, DevTypes, NULL, PCONFIG(0), true );

        // Calculation of the max indices for each sensor type
        switch (PCONFIG(0)) {
          case 21:  //ESR21
            Plugin_089_MaxIdx[0] = 0; // None
            Plugin_089_MaxIdx[1] = 3; // Sensor
            Plugin_089_MaxIdx[2] = 6; // Ext. sensor
            Plugin_089_MaxIdx[3] = 1; // Digital output
            Plugin_089_MaxIdx[4] = 1; // Speed step
            Plugin_089_MaxIdx[5] = 1; // Analog output
            Plugin_089_MaxIdx[6] = 1; // Heat power (kW)
            Plugin_089_MaxIdx[7] = 1; // Heat meter (MWh)
            break;
          case 31:  //UVR31
            Plugin_089_MaxIdx[0] = 0; // None
            Plugin_089_MaxIdx[1] = 3; // Sensor
            Plugin_089_MaxIdx[2] = 0; // Ext. sensor
            Plugin_089_MaxIdx[3] = 1; // Digital output
            Plugin_089_MaxIdx[4] = 0; // Speed step
            Plugin_089_MaxIdx[5] = 0; // Analog output
            Plugin_089_MaxIdx[6] = 0; // Heat power (kW)
            Plugin_089_MaxIdx[7] = 0; // Heat meter (MWh)
            break;
          case 1611:  //UVR1611
            Plugin_089_MaxIdx[0] = 0; // None
            Plugin_089_MaxIdx[1] = 16; // Sensor
            Plugin_089_MaxIdx[2] = 0; // Ext. sensor
            Plugin_089_MaxIdx[3] = 13; // Digital output
            Plugin_089_MaxIdx[4] = 4; // Speed step
            Plugin_089_MaxIdx[5] = 0; // Analog output
            Plugin_089_MaxIdx[6] = 2; // Heat power (kW)
            Plugin_089_MaxIdx[7] = 2; // Heat meter (MWh)
            break;
          case 6132:  //UVR 61-3 (bis V8.2)
            Plugin_089_MaxIdx[0] = 0; // None
            Plugin_089_MaxIdx[1] = 6; // Sensor
            Plugin_089_MaxIdx[2] = 0; // Ext. sensor
            Plugin_089_MaxIdx[3] = 8; // Digital output
            Plugin_089_MaxIdx[4] = 1; // Speed step
            Plugin_089_MaxIdx[5] = 1; // Analog output
            Plugin_089_MaxIdx[6] = 1; // Heat power (kW)
            Plugin_089_MaxIdx[7] = 1; // Heat meter (MWh)
            break;
          case 6133:  //UVR 61-3 (ab V8.3)
            Plugin_089_MaxIdx[0] = 0; // None
            Plugin_089_MaxIdx[1] = 6; // Sensor
            Plugin_089_MaxIdx[2] = 9; // Ext. sensor
            Plugin_089_MaxIdx[3] = 3; // Digital output
            Plugin_089_MaxIdx[4] = 1; // Speed step
            Plugin_089_MaxIdx[5] = 2; // Analog output
            Plugin_089_MaxIdx[6] = 3; // Heat power (kW)
            Plugin_089_MaxIdx[7] = 3; // Heat meter (MWh)
            break;
        }

        addFormSubHeader(F("Inputs"));

        for (int i = 0; i < DLbus_ValueCount; i++) {
          Plugin_089_ValueType[i] = PCONFIG(i + 1) >> 8;
          Plugin_089_ValueIdx[i] = PCONFIG(i + 1) & 0x00FF;

          addFormSelector(plugin_089_ValueNames[i], plugin_089_Values[i], DLbus_OptionCount, Options, OptionTypes, NULL, Plugin_089_ValueType[i], true);
          if (Plugin_089_MaxIdx[Plugin_089_ValueType[i]] > 1) {
            CurIdx = Plugin_089_ValueIdx[i];
            if (CurIdx < 1) {
              CurIdx = 1;
            }
            if (CurIdx > Plugin_089_MaxIdx[Plugin_089_ValueType[i]]) {
              CurIdx = Plugin_089_MaxIdx[Plugin_089_ValueType[i]];
            }
	    addHtml(F(" Index: "));
            addNumericBox(plugin_089_Indizes[i], CurIdx, 1, Plugin_089_MaxIdx[Plugin_089_ValueType[i]]);
          }
        }

        UserVar[event->BaseVarIndex] = NAN;

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("p089_dlbtype"));
        Plugin_089_SetIndices(PCONFIG(0));

        for (int i = 0; i < DLbus_ValueCount; i++) {
          OptionIdx = getFormItemInt(plugin_089_Values[i]);
          CurIdx = getFormItemInt(plugin_089_Indizes[i]);
          if (CurIdx < 1) {
            CurIdx = 1;
          }
          PCONFIG(i + 1) = (OptionIdx << 8) + CurIdx;
          ExtraTaskSettings.TaskDeviceValueDecimals[event->BaseVarIndex + i] = OptionValueDecimals[OptionIdx];
        }

#ifdef PLUGIN_089_DEBUG
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	        Plugin_089_DLB_Pin = CONFIG_PIN1;

	        String log = F("PLUGIN_WEBFORM_SAVE :");
	        log += F(" DLB_Pin:");
	        log += Plugin_089_DLB_Pin;
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
#endif
        UserVar[event->BaseVarIndex] = NAN;
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (Plugin_089_init) {
          addLog(LOG_LEVEL_ERROR, F("PLUGIN_089_INIT -> Already done!"));
          return true;
        }
        Plugin_089_init = true;
        addLog(LOG_LEVEL_ERROR, F("PLUGIN_089_INIT: attachInterrupt"));
        Plugin_089_DLB_Pin = CONFIG_PIN1;
        pinMode(Plugin_089_DLB_Pin, INPUT_PULLUP);
        receiving = false;
        // bei einem CHANGE am Daten-Pin wird pin_changed aufgerufen
        // on a CHANGE on the data pin pin_changed is called
        attachInterrupt(digitalPinToInterrupt(Plugin_089_DLB_Pin), Plugin_089_Pin_changed, CHANGE);
        UserVar[event->BaseVarIndex] = NAN;
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        addLog(LOG_LEVEL_ERROR, F("PLUGIN_089_READ"));
        if (Plugin_089_DLB_Pin != CONFIG_PIN1) {
          String log = F("## Error DL-Bus: Device Pin setting not correct!");
          log += F(" DLB_Pin:");
          log += Plugin_089_DLB_Pin;
          log += F(" Setting:");
          log += CONFIG_PIN1;
          addLog(LOG_LEVEL_ERROR, log);
          return false;
        }
        if (WiFi.status() != WL_CONNECTED) {
          addLog(LOG_LEVEL_ERROR, F("## Error DL-Bus: WiFi not connected!"));
          return false;
        }
        if (receiving) {
          addLog(LOG_LEVEL_ERROR, F("## Error DL-Bus: Still receiving!"));
          return false;
        }
        if (Plugin_089_init == false) {
          addLog(LOG_LEVEL_ERROR, F("## Error DL-Bus: Not initialized!"));
          return false;
        }
        Plugin_089_SetIndices(PCONFIG(0));

        if ((Plugin_089_ReceivedOK == false) || ((millis()-Plugin_089_LastReceived)>(Settings.TaskDeviceTimer[event->TaskIndex] * 1000/2))) {
          success = Plugin_089_Receiving();
          if (success)
            success = Plugin_089_Processing();
          if (success)
            success = Plugin_089_CheckCRC();
          if (success)
            Plugin_089_LastReceived = millis();
          Plugin_089_ReceivedOK = success;
        }
        else
          success = true;
        if (success)
        {
          for (int i = 0; i < DLbus_ValueCount; i++) {
            OptionIdx = PCONFIG(i + 1) >> 8;
            CurIdx = PCONFIG(i + 1) & 0x00FF;
            if (Plugin_089_GetData(OptionIdx, CurIdx)) {
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

void Plugin_089_SetIndices(int DeviceIndex) {
  // Sert the indices for the DL bus packet
  switch (DeviceIndex) {
    case 21:  //ESR21
      DataSettings.DataBytes = 31;
      DataSettings.MinPulseWidth = min_width_488;
      DataSettings.MaxPulseWidth = max_width_488;
      DataSettings.MinDoublePulseWidth = double_min_width_488;
      DataSettings.MaxDoublePulseWidth = double_max_width_488;

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

      OptionValueDecimals[6] = 1; //[0,1kW]     F("Heat power (kW)")
      break;
    case 31:  //UVR31
      DataSettings.DataBytes = 8;
      DataSettings.MinPulseWidth = min_width_50;
      DataSettings.MaxPulseWidth = max_width_50;
      DataSettings.MinDoublePulseWidth = double_min_width_50;
      DataSettings.MaxDoublePulseWidth = double_max_width_50;

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
      DataSettings.MinPulseWidth = min_width_488;
      DataSettings.MaxPulseWidth = max_width_488;
      DataSettings.MinDoublePulseWidth = double_min_width_488;
      DataSettings.MaxDoublePulseWidth = double_max_width_488;

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

      OptionValueDecimals[6] = 2; //[0,01kW]     F("Heat power (kW)")
      break;
    case 6132:  //UVR 61-3 (bis V8.2)
      DataSettings.DataBytes = 35;
      DataSettings.MinPulseWidth = min_width_488;
      DataSettings.MaxPulseWidth = max_width_488;
      DataSettings.MinDoublePulseWidth = double_min_width_488;
      DataSettings.MaxDoublePulseWidth = double_max_width_488;

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

      OptionValueDecimals[6] = 1; //[0,1kW]     F("Heat power (kW)")
      break;
    case 6133:  //UVR 61-3 (ab V8.3)
      DataSettings.DataBytes = 62;
      DataSettings.MinPulseWidth = min_width_488;
      DataSettings.MaxPulseWidth = max_width_488;
      DataSettings.MinDoublePulseWidth = double_min_width_488;
      DataSettings.MaxDoublePulseWidth = double_max_width_488;

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

      OptionValueDecimals[6] = 1; //[0,1kW]     F("Heat power (kW)")
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
  DLBus subs to get values from the receiving bitstream
  \*********************************************************************************************/
// ein Datenrahmen hat <DataSettings.DataBytes> Datenbytes + SYNC, also z.B. 64 * (8+1+1) + 16 = 656
// 656 * 2 = 1312 (das Doppelte eines Datenrahmens wird gespeichert,
// so dass ein ganzer Datenrahmen da ist)
// one data frame has <DataSettings.DataBytes> data bytes + SYNC, e.g. 64 * (8+1+1) + 16 = 656
// 656 * 2 = 1312 (twice as much as a data frame is saved
// so there's one complete data frame

#define MaxDataBytes 64
#define AdditionalRecBytes 2
#define MaxDataBits (((MaxDataBytes + AdditionalRecBytes) * (8 + 1 + 1) + 16) * 4)
volatile byte BitStream[MaxDataBits];           // Empfangener Bit stream // received bit stream
volatile int pulse_count;                       // Anzahl der empfangenen Pulse // number of received pulses
volatile int pulse_number;                      // Maximale anzahl der empfangenen Pulse // max naumber of the received pulses
volatile unsigned long last_bit_change = 0;     // Merken des letzten Übergangs // remember the last transition
volatile word MinPulseWidth, MaxPulseWidth, MinDoublePulseWidth, MaxDoublePulseWidth;
volatile unsigned long TimeInMicros;            // aktuelle Zeit in µs // current time in µs
volatile unsigned long time_diff;               // Different zum verhergehenden Puls in µs    // time difference to previous pulse in µs

byte ByteStream[MaxDataBits / 8 + 1];   // jedes Bit wird in eine Bitmap einsortiert  // every bit gets sorted into a bitmap
int bit_number;                         // Bit-Größe des Datenrahmens // bit size of the data frame

byte WrongTimeCnt;
word WrongTimings[5][6];

int start_bit;                                 // erstes Bit des Datenrahmens      // first bit of data frame

// Sensortypen
// sensor types
#define UNUSED      0b000
#define DIGITAL     0b001
#define TEMP        0b010
#define VOLUME_FLOW 0b011
#define RAYS        0b110
#define ROOM        0b111

// Modi für Raumsensor
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

// Wärmemengenzähler
// heat meter
int power_index, kwh_index, mwh_index;

/****************\
  DLBus receiving
  \****************/

boolean Plugin_089_Receiving(void) {
  byte rawval, val;
  int i;
  noInterrupts ();                              // make sure we don't get interrupted before we are ready
  pulse_count = 0;
  pulse_number = (((DataSettings.DataBytes + AdditionalRecBytes) * (8 + 1 + 1) + 16) * 4);
  MinPulseWidth = DataSettings.MinPulseWidth;
  MaxPulseWidth = DataSettings.MaxPulseWidth;
  MinDoublePulseWidth = DataSettings.MinDoublePulseWidth;
  MaxDoublePulseWidth = DataSettings.MaxDoublePulseWidth;
  addLog(LOG_LEVEL_INFO, F("Receiving ..."));
  receiving = true;
  interrupts ();                                // interrupts allowed now, next instruction WILL be executed
  unsigned long start=millis();
  while ((millis()-start)<100) {
    // 100ms warten // wait 100 ms
    yield();
  }
  if (pulse_count == 0) {
    // nichts empfangen  // nothing received
    receiving = false;
    addLog(LOG_LEVEL_ERROR, F("Error: Nothing received! No DL bus connected!"));
    return false;
  }
  while (receiving) {
    // auf das Ende des Empfangs warten // wait for end of receiving
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
          Plugin_089_process_bit(0);
          Plugin_089_process_bit(1);
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
        // doppelte Pulsweite // double pulse width
        Plugin_089_process_bit(!val);
        Plugin_089_process_bit(val);
      }
      else {
        // einfache Pulsweite // single pulse width
        Plugin_089_process_bit(val);
      }
    }
  }
  addLog(LOG_LEVEL_INFO, F("BitStream copied."));

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
  return true;
}

/*****************\
  DLBus interrupt
  \*****************/

void Plugin_089_Pin_changed(void) {
  TimeInMicros = micros();
  time_diff = TimeInMicros - last_bit_change;
  last_bit_change = TimeInMicros;
  if (receiving) {
    byte val = digitalRead(Plugin_089_DLB_Pin); // Zustand einlesen // read state
    // Pulsweite testen // check pulse width
    if (time_diff >= 2*MinDoublePulseWidth)     val |= 0x80; // länger als 2x doppelte Pulsweite // longer then 2x double pulse width
    else if (time_diff > MaxDoublePulseWidth)   val |= 0x40; // länger als doppelte Pulsweite  // longer then double pulse width
    else if (time_diff >= MinDoublePulseWidth)  val |= flgDoubleWidth; // doppelte Pulsweite // double pulse width
    else if (time_diff > MaxPulseWidth)         val |= 0x20; // zwischen doppelter und einfacher Pulsweite // between double and single pulse width
    else if (time_diff < MinPulseWidth)         val |= 0x10; // kürzer als einfache Pulsweite // shorter then single pulse width
    else                                        val |= flgSingleWidth; // einfache Pulsweite // single pulse width
    BitStream[pulse_count] = val;                            // Bit setzen // set bit
    pulse_count++;
    receiving = (pulse_count < pulse_number);     // beende Übertragung, wenn Datenrahmen vollständig // stop receiving when data frame is complete
  }
}

/*****************\
  DLBus processing
  \*****************/

void Plugin_089_process_bit(byte b) {
  // den ersten Impuls ignorieren // ignore first pulse
  pulse_count++;
  if (pulse_count % 2)
    return;
  bit_number = (pulse_count / 2);
  if (b)
    ByteStream[bit_number / 8] |= (1 << (bit_number % 8)); // Bit setzen // set bit
  else
    ByteStream[bit_number / 8] &= ~(1 << (bit_number % 8)); // Bit löschen // clear bit
}

boolean Plugin_089_Processing(void) {
boolean inverted = false;
String log;
  addLog(LOG_LEVEL_INFO, F("Processing..."));
  start_bit = Plugin_089_Analyze(); // Anfang des Datenrahmens finden // find the data frame's beginning
  // invertiertes Signal? // inverted signal?
  while (start_bit == -1) {
    if (inverted) {
      return false;
    }
    addLog(LOG_LEVEL_INFO, F("Invert bit stream..."));
    Plugin_089_Invert(); // erneut invertieren // invert again
    inverted = true;
    start_bit = Plugin_089_Analyze();
    if (start_bit == -1) {
      addLog(LOG_LEVEL_ERROR, F("Error: No data frame available!"));
      return false;
    }
    if ((bit_number-start_bit)<(pulse_number/4)) {
      // kein kompletter Datenrahmen vorhanden  // no complete data frame available
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
  Plugin_089_Trim(); // Start- und Stopbits entfernen // remove start and stop bits
  if (Plugin_089_CheckDevice()) // verbundenes Gerät prüfen // check connected device
    return true;
  else {
    addLog(LOG_LEVEL_ERROR, F("Error: Device not found!"));
    return false;
  }
}

int Plugin_089_Analyze() {
  byte sync=0;
  // finde SYNC (16 * aufeinanderfolgend 1) // find SYNC (16 * sequential 1)
  for (int i = 0; i < bit_number; i++) {
    if (Plugin_089_ReadBit(i))
      sync++;
    else
      sync = 0;
    if (sync == 16) {
      // finde erste 0 // find first 0
      while (Plugin_089_ReadBit(i) == 1)
        i++;
      return i; // Anfang des Datenrahmens // beginning of data frame
    }
  }
  // kein Datenrahmen vorhanden. Signal überprüfen?
  return -1; // no data frame available. check signal?
}
void Plugin_089_Invert() {
  for (int i = 0; i < bit_number; i++)
    Plugin_089_WriteBit(i, Plugin_089_ReadBit(i) ? 0 : 1); // jedes Bit umkehren // invert every bit
}
byte Plugin_089_ReadBit(int pos) {
  int row = pos / 8; // Position in Bitmap ermitteln // detect position in bitmap
  int col = pos % 8;
  return (((ByteStream[row]) >> (col)) & 0x01); // Bit zurückgeben // return bit
}
void Plugin_089_WriteBit(int pos, byte set) {
  int row = pos / 8; // Position in Bitmap ermitteln // detect position in bitmap
  int col = pos % 8;
  if (set)
    ByteStream[row] |= 1 << col; // Bit setzen // set bit
  else
    ByteStream[row] &= ~(1 << col); // Bit löschen // clear bit
}
void Plugin_089_Trim() {
  for (int i = start_bit, bit = 0; i < bit_number; i++) {
    int offset = i - start_bit;
    // Start- und Stop-Bits ignorieren:
    // Startbits: 0 10 20 30, also  x    % 10 == 0
    // Stopbits:  9 19 29 39, also (x+1) % 10 == 0
    // ignore start and stop bits:
    // start bits: 0 10 20 30, also  x    % 10 == 0
    // stop bits:  9 19 29 39, also (x+1) % 10 == 0
    if (offset % 10 && (offset + 1) % 10) {
      Plugin_089_WriteBit(bit, Plugin_089_ReadBit(i));
      bit++;
    }
  }
}

/****************\
  DLBus get data
  \****************/

boolean Plugin_089_CheckDevice() {
  // Datenrahmen von eines Gerätes?
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

boolean Plugin_089_CheckCRC() {
  // CRC-Prüfsumme
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

boolean Plugin_089_GetData(int OptionIdx, int CurIdx) {
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
      result = fetch_sensor(CurIdx);
      break;
    case 2:    //F("Sensor")
      log = F("Get ExtSensor");
      log += CurIdx;
      if (CurIdx > DataSettings.MaxExtSensors) {
        result=false;
        break;
      }
      ReadData.Idx = DataSettings.IdxExtSensor;
      result = fetch_sensor(CurIdx);
      break;
    case 3:  //F("Digital output")
      log = F("Get DigitalOutput");
      log += CurIdx;
      if (CurIdx > (8 * DataSettings.OutputBytes)) {
        result=false;
        break;
      }
      result = fetch_output(CurIdx);
      break;
    case 4:  //F("Speed step")
      log = F("Get SpeedStep");
      log += CurIdx;
      if (CurIdx > DataSettings.SpeedBytes) {
        result=false;
        break;
      }
      result = fetch_speed(CurIdx);
      break;
    case 5:  //F("Analog output")
      log = F("Get AnalogOutput");
      log += CurIdx;
      if (CurIdx > DataSettings.AnalogBytes) {
        result=false;
        break;
      }
      result = fetch_analog(CurIdx);
      break;
    case 6:  //F("Heat power (kW)")
      log = F("Get HeatPower");
      log += CurIdx;
      if (CurIdx > DataSettings.MaxHeatMeters) {
        result=false;
        break;
      }
      result = fetch_heatpower(CurIdx);
      break;
    case 7:  //F("Heat meter (MWh)"
      log = F("Get HeatMeter");
      log += CurIdx;
      if (CurIdx > DataSettings.MaxHeatMeters) {
        result=false;
        break;
      }
      result = fetch_heatmeter(CurIdx);
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

boolean fetch_sensor(int number) {
  float value;
  ReadData.mode = -1;
  number = ReadData.Idx + (number - 1) * 2;
  int sensorvalue = (ByteStream[number + 1] << 8) | ByteStream[number];
  if (sensorvalue == 0) {
    return false;
  }
  byte sensortype = (sensorvalue & 0x7000) >> 12;
  if (!(sensorvalue & 0x8000)) { // Vorzeichen positiv // sign positive
    sensorvalue &= 0xfff;
    // Berechnungen für unterschiedliche Sensortypen
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
  else { // Vorzeichen negativ // sign negative
    sensorvalue |= 0xf000;
    // Berechnungen für unterschiedliche Sensortypen
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

boolean fetch_output(int number) {
  int outputs;

  if (DataSettings.OutputBytes > 1)
    outputs = (ByteStream[DataSettings.IdxOutput + 1] << 8) | ByteStream[DataSettings.IdxOutput];
  else
    outputs = ByteStream[DataSettings.IdxOutput];
  ReadData.value = (!!(outputs & (1 << (number - 1))));
  return true;
}

boolean fetch_speed(int number) {
  byte speedbyte;

  speedbyte = ByteStream[DataSettings.IdxDrehzahl + (number - 1)];
  if ((speedbyte & 0x80) == 0x80)
    return false;
  ReadData.value = (speedbyte & 0x1f);
  return true;
}

boolean fetch_analog(int number) {
  byte analogbyte;

  analogbyte = ByteStream[DataSettings.IdxAnalog + (number - 1)];
  if ((analogbyte & 0x80) == 0x80)
    return false;
  ReadData.value = (analogbyte * 0.1);
  return true;
}

boolean CheckHmRegister(int number) {
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

boolean fetch_heatpower(int number) {
  // Momentanleistung // current power
  int high;

  if (CheckHmRegister(number) == false)
    return false;
  byte b1 = ByteStream[power_index];
  byte b2 = ByteStream[power_index + 1];
  if (DataSettings.CurrentHmBytes > 2) {
    byte b3 = ByteStream[power_index + 2];
    byte b4 = ByteStream[power_index + 3];
    high = 65536 * b4 + 256 * b3 + b2;
    int low = (b1 * 10) / 256;
    if (!(b4 & 0x80)) // Vorzeichen positiv // sign positive
      ReadData.value = (10 * high + low) / 100;
    else // Vorzeichen negativ // sign negative
      ReadData.value = (10 * (high - 65536) - low) / 100;
  }
  else {
    high = (b2 << 8) | b1;
    if ((b2 & 0x80) == 0) // Vorzeichen positiv // sign positive
      ReadData.value = high / 10;
    else
      ReadData.value = (high - 65536) / 10;
  }
  return true;
}

boolean fetch_heatmeter(int number) {
  // Wärmemengenzähler // heat meter
  int heat_meter;
  float heat_meter_mwh;

  if (CheckHmRegister(number) == false)
    return false;
  // kWh
  heat_meter = (ByteStream[kwh_index + 1] << 8) | ByteStream[kwh_index];
  heat_meter_mwh = (heat_meter * 0.1) / 1000; // in MWh
  if (heat_meter_mwh > 1.0) {
    heat_meter = heat_meter_mwh;
    heat_meter_mwh -= heat_meter;
  }
  // MWh
  heat_meter = (ByteStream[mwh_index + 1] << 8) | ByteStream[mwh_index];
  ReadData.value = heat_meter_mwh + heat_meter;
  return true;
}
