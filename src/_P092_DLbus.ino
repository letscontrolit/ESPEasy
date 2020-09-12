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

   The selected input needs a voltage divider as follows because the DL-Bus runs on 12 volts for
	 following devices: UVR31, UVR42, UVR64, HZR65, EEG30 and TFM66
   DLbus@12V - 8k6 - input@3.3V - 3k3 - ground

   For following devices just a pull up resistor is needed if the device is used stand alone:
	 UVR1611, UVR61-3 and ESR21

   @uwekaditz 2020-03-01 Memory usage optimized
	 CHG: Moved arrays into the class DLBus
   CHG: Moved arrays to PLUGIN_092_DEBUG

	 @uwekaditz 2019-12-15 Memory usage optimized
	 CHG: Moved the array for the received bit changes to stativ uint_8t, the ISR call uses only a volatile pointer to it
	 CHG: some more defines and name changes for better explanation

	 @uwekaditz 2019-12-14 Timing optimized
	 CHG: Removed the while (P092_receiving) loop.
	 CHG: Starting of the receiving and processing of the received bit stream are now done in the PLUGIN_ONCE_A_SECOND call
			  PLUGIN_READ call just uses the already processed data

	 @uwekaditz 2019-12-08 Inital commit to mega

\**************************************************/

#include "src/PluginStructs/P092_data_struct.h"

#define PLUGIN_092
#define PLUGIN_ID_092         92
//#define PLUGIN_092_DEBUG    // additional debug messages in the log
#define PLUGIN_NAME_092       "Heating - DL-Bus (Technische Alternative)  [TESTING]"
#define PLUGIN_VALUENAME1_092 "Value"

#define P092_DLbus_OptionCount 8
#define P092_DLbus_ValueCount 1
#define P092_DLbus_DeviceCount 5

uint8_t P092_Last_DLB_Pin;
boolean P092_init = false;
boolean P092_ReceivedOK = false;
uint32_t P092_LastReceived = 0;
struct _P092_DataStruct
{
  uint8_t				DataBytes;
  uint8_t       DeviceByte0;
  uint8_t       DeviceByte1;
  uint8_t       DeviceBytes;
  uint8_t       DontCareBytes;
  uint8_t       TimeStampBytes;
  uint8_t       MaxSensors;
  uint8_t       MaxExtSensors;
  uint8_t       OutputBytes;
  uint8_t       SpeedBytes;
  uint8_t       AnalogBytes;
  uint8_t       VolumeBytes;
  uint8_t       MaxHeatMeters;
  uint8_t       CurrentHmBytes;
  uint8_t       MWhBytes;

  uint16_t      DLbus_MinPulseWidth;
  uint16_t      DLbus_MaxPulseWidth;
  uint16_t      DLbus_MinDoublePulseWidth;
  uint16_t      DLbus_MaxDoublePulseWidth;
  uint8_t       IdxSensor;
  uint8_t       IdxExtSensor;
  uint8_t       IdxOutput;
  uint8_t       IdxDrehzahl;
  uint8_t       IdxAnalog;
  uint8_t       IdxHmRegister;
  uint8_t       IdxVolume;
  uint8_t       IdxHeatMeter1;
  uint8_t       IdxkWh1;
  uint8_t       IdxMWh1;
  uint8_t       IdxHeatMeter2;
  uint8_t       IdxkWh2;
  uint8_t       IdxMWh2;
  uint8_t       IdxHeatMeter3;
  uint8_t       IdxkWh3;
  uint8_t       IdxMWh3;
  uint8_t       IdxCRC;
} P092_DataSettings;

typedef struct {
  uint8_t Idx;
  uint8_t mode;
  float   value;
} sP092_ReadData;

DLBus *DLbus_Data = nullptr;

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

boolean P092_GetData(int OptionIdx, int CurIdx, sP092_ReadData* ReadData);

boolean Plugin_092(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;


  const String plugin_092_ValStr = F("p092_Value");
  const String plugin_092_IdxStr = F("p092_Idx");


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
        {
          const String Devices[P092_DLbus_DeviceCount] = { F("ESR21"), F("UVR31"), F("UVR1611"), F("UVR 61-3 (bis V8.2)"), F("UVR 61-3 (ab V8.3)") };
          const int DevTypes[P092_DLbus_DeviceCount] = { 21, 31, 1611, 6132, 6133 };

          addFormSelector(F("DL-Bus Type"), F("p092_dlbtype"), P092_DLbus_DeviceCount, Devices, DevTypes, NULL, PCONFIG(0), true );
        }
        {
          int P092_ValueType, P092_ValueIdx;
          P092_Last_DLB_Pin = CONFIG_PIN1;
          const String plugin_092_DefValueName = F(PLUGIN_VALUENAME1_092);
          const int P092_OptionTypes[P092_DLbus_OptionCount] = {
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

          uint8_t P092_MaxIdx[P092_DLbus_OptionCount];
          // Calculation of the max indices for each sensor type
          // default indizes for UVR31
              P092_MaxIdx[0] = 0; // None
              P092_MaxIdx[1] = 3; // Sensor
              P092_MaxIdx[2] = 0; // Ext. sensor
              P092_MaxIdx[3] = 1; // Digital output
              P092_MaxIdx[4] = 0; // Speed step
              P092_MaxIdx[5] = 0; // Analog output
              P092_MaxIdx[6] = 0; // Heat power (kW)
              P092_MaxIdx[7] = 0; // Heat meter (MWh)
          switch (PCONFIG(0)) {
            case 21:  //ESR21
              P092_MaxIdx[2] = 6; // Ext. sensor
              P092_MaxIdx[4] = 1; // Speed step
              P092_MaxIdx[5] = 1; // Analog output
              P092_MaxIdx[6] = 1; // Heat power (kW)
              P092_MaxIdx[7] = 1; // Heat meter (MWh)
              break;
            case 1611:  //UVR1611
              P092_MaxIdx[1] = 16; // Sensor
              P092_MaxIdx[3] = 13; // Digital output
              P092_MaxIdx[4] = 4; // Speed step
              P092_MaxIdx[6] = 2; // Heat power (kW)
              P092_MaxIdx[7] = 2; // Heat meter (MWh)
              break;
            case 6132:  //UVR 61-3 (bis V8.2)
              P092_MaxIdx[1] = 6; // Sensor
              P092_MaxIdx[3] = 8; // Digital output
              P092_MaxIdx[4] = 1; // Speed step
              P092_MaxIdx[5] = 1; // Analog output
              P092_MaxIdx[6] = 1; // Heat power (kW)
              P092_MaxIdx[7] = 1; // Heat meter (MWh)
              break;
            case 6133:  //UVR 61-3 (ab V8.3)
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
            P092_ValueType = PCONFIG(i + 1) >> 8;
            P092_ValueIdx = PCONFIG(i + 1) & 0x00FF;

            addFormSelector(plugin_092_DefValueName, plugin_092_ValStr, P092_DLbus_OptionCount, Options, P092_OptionTypes, NULL, P092_ValueType, true);
            if (P092_MaxIdx[P092_ValueType] > 1) {
              int CurIdx = P092_ValueIdx;
              if (CurIdx < 1) {
                CurIdx = 1;
              }
              if (CurIdx > P092_MaxIdx[P092_ValueType]) {
                CurIdx = P092_MaxIdx[P092_ValueType];
              }
              addHtml(F(" Index: "));
              addNumericBox(plugin_092_IdxStr, CurIdx, 1, P092_MaxIdx[P092_ValueType]);
            }
          }
        }

        UserVar[event->BaseVarIndex] = NAN;

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
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

        PCONFIG(0) = getFormItemInt(F("p092_dlbtype"));
        if (PCONFIG(0) == 1611)  // only UVR1611
          P092_OptionValueDecimals[6] = 2;

        for (int i = 0; i < P092_DLbus_ValueCount; i++) {
          int OptionIdx = getFormItemInt(plugin_092_ValStr);
          int CurIdx = getFormItemInt(plugin_092_IdxStr);
          if (CurIdx < 1) {
            CurIdx = 1;
          }
          PCONFIG(i + 1) = (OptionIdx << 8) + CurIdx;
          ExtraTaskSettings.TaskDeviceValueDecimals[event->BaseVarIndex + i] = P092_OptionValueDecimals[OptionIdx];
        }

        if (DLbus_Data == nullptr) {
          addLog(LOG_LEVEL_ERROR, F("## P092_save: Error DL-Bus: Class not initialized!"));
          return false;
        }

        if (P092_Last_DLB_Pin != CONFIG_PIN1) {
          // pin number is changed -> run a new init
          P092_init = false;
          if (DLbus_Data->ISR_DLB_Pin != 0xFF) {
            // interrupt was already attached to P092_DLB_Pin
            detachInterrupt(digitalPinToInterrupt(DLbus_Data->ISR_DLB_Pin));
            addLog(LOG_LEVEL_INFO, F("P092_save: detachInterrupt"));
          }
        }

#ifdef PLUGIN_092_DEBUG
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {

	        String log = F("PLUGIN_WEBFORM_SAVE :");
	        log += F(" DLB_Pin:");
	        log += CONFIG_PIN1;
	        log += F(" DLbus_MinPulseWidth:");
	        log += P092_DataSettings.DLbus_MinPulseWidth;
	        log += F(" DLbus_MaxPulseWidth:");
	        log += P092_DataSettings.DLbus_MaxPulseWidth;
	        log += F(" DLbus_MinDoublePulseWidth:");
	        log += P092_DataSettings.DLbus_MinDoublePulseWidth;
	        log += F(" DLbus_MaxDoublePulseWidth:");
	        log += P092_DataSettings.DLbus_MaxDoublePulseWidth;
	        log += F(" IdxSensor:");
	        log += P092_DataSettings.IdxSensor;
	        log += F(" IdxExtSensor:");
	        log += P092_DataSettings.IdxExtSensor;
	        log += F(" IdxOutput:");
	        log += P092_DataSettings.IdxOutput;
	        if (P092_DataSettings.SpeedBytes > 0) {
	          log += F(" IdxDrehzahl:");
	          log += P092_DataSettings.IdxDrehzahl;
	        }
	        if (P092_DataSettings.AnalogBytes > 0) {
	          log += F(" IdxAnalog:");
	          log += P092_DataSettings.IdxAnalog;
	        }
	        if (P092_DataSettings.MaxHeatMeters > 0) {
	          log += F(" IdxHmRegister:");
	          log += P092_DataSettings.IdxHmRegister;
	        }
	        if (P092_DataSettings.VolumeBytes > 0) {
	          log += F(" IdxVolume:");
	          log += P092_DataSettings.IdxVolume;
	        }
	        if (P092_DataSettings.MaxHeatMeters > 0) {
	          log += F(" IdxHM1:");
	          log += P092_DataSettings.IdxHeatMeter1;
	          log += F(" IdxkWh1:");
	          log += P092_DataSettings.IdxkWh1;
	          log += F(" IdxMWh1:");
	          log += P092_DataSettings.IdxMWh1;
	        }
	        if (P092_DataSettings.MaxHeatMeters > 1) {
	          log += F(" IdxHM2:");
	          log += P092_DataSettings.IdxHeatMeter2;
	          log += F(" IdxkWh2:");
	          log += P092_DataSettings.IdxkWh2;
	          log += F(" IdxMWh2:");
	          log += P092_DataSettings.IdxMWh2;
	        }
	        if (P092_DataSettings.MaxHeatMeters > 2) {
	          log += F(" IdxHM3:");
	          log += P092_DataSettings.IdxHeatMeter3;
	          log += F(" IdxkWh3:");
	          log += P092_DataSettings.IdxkWh3;
	          log += F(" IdxMWh3:");
	          log += P092_DataSettings.IdxMWh3;
	        }
	        log += F(" IdxCRC:");
	        log += P092_DataSettings.IdxCRC;
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
        }
        else {
          addLog(LOG_LEVEL_INFO, F("P092_init ..."));
          if (DLbus_Data == nullptr) {
            DLbus_Data = new (std::nothrow) DLBus;
            if (DLbus_Data == nullptr) {
              addLog(LOG_LEVEL_ERROR, F("## P092_init: Error DL-Bus: Class not initialized!"));
              return false;
            }
            DLbus_Data->LogLevelInfo = LOG_LEVEL_INFO;
            DLbus_Data->LogLevelError = LOG_LEVEL_ERROR;
            DLbus_Data->IsLogLevelInfo = loglevelActiveFor(LOG_LEVEL_INFO);
          }
          P092_init = true;
          P092_ReceivedOK = false;
          addLog(LOG_LEVEL_INFO, F("P092_init: attachInterrupt"));
          DLbus_Data->ISR_DLB_Pin = CONFIG_PIN1;
          pinMode(CONFIG_PIN1, INPUT_PULLUP);
          // on a CHANGE on the data pin P092_Pin_changed is called
          DLbus_Data->attachDLBusInterrupt();
          UserVar[event->BaseVarIndex] = NAN;
        }
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        if (!NetworkConnected()) {
          return false;
        }
        if (P092_init == false) {
          return false;
        }
        if (DLbus_Data == nullptr)
          return false;
        if (DLbus_Data->ISR_Receiving) {
          return false;
        }
        Plugin_092_SetIndices(PCONFIG(0));
        if (DLbus_Data->ISR_AllBitsReceived) {
          DLbus_Data->ISR_AllBitsReceived = false;
          success = DLbus_Data->CheckTimings();
          if (success)
            success = DLbus_Data->Processing();
          if (success)
            success = DLbus_Data->CheckCRC(P092_DataSettings.IdxCRC);
          if (success) {
            addLog(LOG_LEVEL_INFO, F("Received data OK"));
            P092_LastReceived = millis();
          }
          P092_ReceivedOK = success;
        }
        else
          success = P092_ReceivedOK;

        if ((P092_ReceivedOK == false) || (timePassedSince(P092_LastReceived)>(static_cast<long>(Settings.TaskDeviceTimer[event->TaskIndex] * 1000/2)))) {
          Plugin_092_StartReceiving();
          success = true;
        }
        break;
      }

    case PLUGIN_READ:
      {
        addLog(LOG_LEVEL_ERROR, F("PLUGIN_092_READ"));
        if (!NetworkConnected()) {
          // too busy for DLbus while wifi connect is running
          addLog(LOG_LEVEL_ERROR, F("## P092_read: Error DL-Bus: WiFi not connected!"));
          return false;
        }
        if (P092_init == false) {
          addLog(LOG_LEVEL_ERROR, F("## P092_read: Error DL-Bus: Not initialized!"));
          return false;
        }
        if (DLbus_Data == nullptr) {
          addLog(LOG_LEVEL_ERROR, F("## P092_read: Error DL-Bus: Class not initialized!"));
          return false;
        }
        if (DLbus_Data->ISR_DLB_Pin != CONFIG_PIN1) {
          String log = F("## P092_read: Error DL-Bus: Device Pin setting not correct!");
          log += F(" DLB_Pin:");
          log += DLbus_Data->ISR_DLB_Pin;
          log += F(" Setting:");
          log += CONFIG_PIN1;
          addLog(LOG_LEVEL_ERROR, log);
          return false;
        }
        success = P092_ReceivedOK;
        if (P092_ReceivedOK == false) {
          addLog(LOG_LEVEL_INFO, F("P092_read: Still receiving DL-Bus bits!"));
        }
        else {
          sP092_ReadData P092_ReadData;
          for (int i = 0; i < P092_DLbus_ValueCount; i++) {
            int OptionIdx = PCONFIG(i + 1) >> 8;
            int CurIdx = PCONFIG(i + 1) & 0x00FF;
            if (P092_GetData(OptionIdx, CurIdx, &P092_ReadData)) {
              UserVar[event->BaseVarIndex + i] = P092_ReadData.value;
            }
            else {
              addLog(LOG_LEVEL_ERROR, F("## P092_read: Error: No readings!"));
            }
          }
        }
        break;
      }
  }
  return success;
}

void Plugin_092_SetIndices(int DeviceIndex) {
  // Set the indices for the DL bus packet
int iDeviceBytes, iDontCareBytes, iTimeStampBytes;

  //default settings for ESR21
    P092_DataSettings.DataBytes = 31;
    P092_DataSettings.DLbus_MinPulseWidth = P092_min_width_488;
    P092_DataSettings.DLbus_MaxPulseWidth = P092_max_width_488;
    P092_DataSettings.DLbus_MinDoublePulseWidth = P092_double_min_width_488;
    P092_DataSettings.DLbus_MaxDoublePulseWidth = P092_double_max_width_488;

    P092_DataSettings.DeviceByte0 = 0x70;
    P092_DataSettings.DeviceByte1 = 0x8F;
    iDeviceBytes = 2;
    iDontCareBytes = 0;
    iTimeStampBytes = 0;
    P092_DataSettings.MaxSensors = 3;
    P092_DataSettings.MaxExtSensors = 6;
    P092_DataSettings.OutputBytes = 1;
    P092_DataSettings.SpeedBytes = 1;
    P092_DataSettings.AnalogBytes = 1;
    P092_DataSettings.VolumeBytes = 0;
    P092_DataSettings.MaxHeatMeters = 1;
    P092_DataSettings.CurrentHmBytes = 2;
    P092_DataSettings.MWhBytes = 2;
    P092_DataSettings.IdxCRC = 30;

  switch (DeviceIndex) {
    case 31:  //UVR31
      P092_DataSettings.DataBytes = 8;
      P092_DataSettings.DLbus_MinPulseWidth = P092_min_width_50;
      P092_DataSettings.DLbus_MaxPulseWidth = P092_max_width_50;
      P092_DataSettings.DLbus_MinDoublePulseWidth = P092_double_min_width_50;
      P092_DataSettings.DLbus_MaxDoublePulseWidth = P092_double_max_width_50;

      P092_DataSettings.DeviceByte0 = 0x30;
      P092_DataSettings.DeviceByte1 = 0;
      iDeviceBytes = 1;
      P092_DataSettings.MaxExtSensors = 0;
      P092_DataSettings.SpeedBytes = 0;
      P092_DataSettings.AnalogBytes = 0;
      P092_DataSettings.MaxHeatMeters = 0;
      P092_DataSettings.CurrentHmBytes = 0;
      P092_DataSettings.MWhBytes = 0;
      P092_DataSettings.IdxCRC = 0;
      break;
    case 1611:  //UVR1611
      P092_DataSettings.DataBytes = 64;

      P092_DataSettings.DeviceByte0 = 0x80;
      P092_DataSettings.DeviceByte1 = 0x7F;
      iDontCareBytes = 1;
      iTimeStampBytes = 5;
      P092_DataSettings.MaxSensors = 16;
      P092_DataSettings.MaxExtSensors = 0;
      P092_DataSettings.OutputBytes = 2;
      P092_DataSettings.SpeedBytes = 4;
      P092_DataSettings.AnalogBytes = 0;
      P092_DataSettings.MaxHeatMeters = 2;
      P092_DataSettings.CurrentHmBytes = 4;
      P092_DataSettings.IdxCRC = P092_DataSettings.DataBytes-1;

      break;
    case 6132:  //UVR 61-3 (up to V8.2)
      P092_DataSettings.DataBytes = 35;

      P092_DataSettings.DeviceByte0 = 0x90;
      P092_DataSettings.DeviceByte1 = 0x6F;
      iDontCareBytes = 1;
      iTimeStampBytes = 5;
      P092_DataSettings.MaxSensors = 6;
      P092_DataSettings.MaxExtSensors = 0;
      P092_DataSettings.VolumeBytes = 2;
      P092_DataSettings.MWhBytes = 4;
      P092_DataSettings.IdxCRC = P092_DataSettings.DataBytes-1;

      break;
    case 6133:  //UVR 61-3 (from V8.3)
      P092_DataSettings.DataBytes = 62;

      P092_DataSettings.DeviceByte0 = 0x90;
      P092_DataSettings.DeviceByte1 = 0x9F;
      iDontCareBytes = 1;
      iTimeStampBytes = 5;
      P092_DataSettings.MaxSensors = 6;
      P092_DataSettings.MaxExtSensors = 9;
      P092_DataSettings.MaxHeatMeters = 3;
      P092_DataSettings.IdxCRC = P092_DataSettings.DataBytes-1;

      break;
  }
  P092_DataSettings.IdxSensor               = iDeviceBytes + iDontCareBytes + iTimeStampBytes;
  P092_DataSettings.IdxExtSensor            = P092_DataSettings.IdxSensor + 2 * P092_DataSettings.MaxSensors;
  P092_DataSettings.IdxOutput               = P092_DataSettings.IdxExtSensor + 2 * P092_DataSettings.MaxExtSensors;
  P092_DataSettings.IdxDrehzahl             = P092_DataSettings.IdxOutput + P092_DataSettings.OutputBytes;
  P092_DataSettings.IdxAnalog               = P092_DataSettings.IdxDrehzahl + P092_DataSettings.SpeedBytes;
  P092_DataSettings.IdxHmRegister           = P092_DataSettings.IdxAnalog + P092_DataSettings.AnalogBytes;
  P092_DataSettings.IdxVolume               = P092_DataSettings.IdxHmRegister + 1;
  P092_DataSettings.IdxHeatMeter1           = P092_DataSettings.IdxVolume + P092_DataSettings.VolumeBytes;
  P092_DataSettings.IdxkWh1                 = P092_DataSettings.IdxHeatMeter1 + P092_DataSettings.CurrentHmBytes;
  P092_DataSettings.IdxMWh1                 = P092_DataSettings.IdxkWh1 + 2;
  P092_DataSettings.IdxHeatMeter2           = P092_DataSettings.IdxMWh1 + P092_DataSettings.MWhBytes;
  P092_DataSettings.IdxkWh2                 = P092_DataSettings.IdxHeatMeter2 + P092_DataSettings.CurrentHmBytes;
  P092_DataSettings.IdxMWh2                 = P092_DataSettings.IdxkWh2 + 2;
  P092_DataSettings.IdxHeatMeter3           = P092_DataSettings.IdxMWh2 + P092_DataSettings.MWhBytes;
  P092_DataSettings.IdxkWh3                 = P092_DataSettings.IdxHeatMeter3 + P092_DataSettings.CurrentHmBytes;
  P092_DataSettings.IdxMWh3                 = P092_DataSettings.IdxkWh3 + 2;

  return;
}
/*********************************************************************************************\
  DLBus subs to get values from the P092_receiving bitstream
\*********************************************************************************************/

// sensor types
#define DLbus_UNUSED              0b000
#define DLbus_Sensor_DIGITAL      0b001
#define DLbus_Sensor_TEMP         0b010
#define DLbus_Sensor_VOLUME_FLOW  0b011
#define DLbus_Sensor_RAYS         0b110
#define DLbus_Sensor_ROOM         0b111

// room sensor modes
#define DLbus_RSM_AUTO            0b00
#define DLbus_RSM_NORMAL          0b01
#define DLbus_RSM_LOWER           0b10
#define DLbus_RSM_STANDBY         0b11

boolean P092_fetch_sensor(int number, sP092_ReadData* ReadData);
boolean P092_fetch_output(int number, sP092_ReadData* ReadData);        // digital output byte(s)
boolean P092_fetch_speed(int number, sP092_ReadData* ReadData);         // speed byte(s)
boolean P092_fetch_analog(int number, sP092_ReadData* ReadData);        // analog byte(s)
boolean P092_fetch_heatpower(int number, sP092_ReadData* ReadData);     // heat power(s)
boolean P092_fetch_heatmeter(int number, sP092_ReadData* ReadData);     // heat meters(s)

// heat meter
typedef struct {
  uint8_t IndexIsValid;
  int32_t power_index;
  int32_t kwh_index;
  int32_t	mwh_index;
} sDLbus_HMindex;
sDLbus_HMindex P092_CheckHmRegister(int number);

/****************\
  DLBus P092_receiving
\****************/

void Plugin_092_StartReceiving(void) {
  DLbus_Data->ISR_Receiving = false;
  DLbus_Data->DeviceBytes[0] = P092_DataSettings.DeviceByte0;
  DLbus_Data->DeviceBytes[1] = P092_DataSettings.DeviceByte1;
  DLbus_Data->ISR_PulseNumber = (((P092_DataSettings.DataBytes + DLbus_AdditionalRecBytes) * (DLbus_StartBits + 8 +  DLbus_StopBits) + DLBus_SyncBits) * DLBus_BitChangeFactor) + DLBus_ReserveBytes;
  DLbus_Data->ISR_MinPulseWidth = P092_DataSettings.DLbus_MinPulseWidth;
  DLbus_Data->ISR_MaxPulseWidth = P092_DataSettings.DLbus_MaxPulseWidth;
  DLbus_Data->ISR_MinDoublePulseWidth = P092_DataSettings.DLbus_MinDoublePulseWidth;
  DLbus_Data->ISR_MaxDoublePulseWidth = P092_DataSettings.DLbus_MaxDoublePulseWidth;
  DLbus_Data->StartReceiving();
  uint32_t start=millis();
  addLog(LOG_LEVEL_INFO, F("P092_receiving ..."));
  while ((timePassedSince(start)<100) && (DLbus_Data->ISR_PulseCount == 0)) {
    // wait for first pulse received (timeout 100ms)
    yield();
  }
  if (DLbus_Data->ISR_PulseCount == 0) {
    // nothing received
    DLbus_Data->ISR_Receiving = false;
    addLog(LOG_LEVEL_ERROR, F("## StartReceiving: Error: Nothing received! No DL bus connected!"));
  }
}

/****************\
  DLBus get data
\****************/

boolean P092_GetData(int OptionIdx, int CurIdx, sP092_ReadData* ReadData) {
  String log;
  boolean result = false;
  switch (OptionIdx) {
    case 1:    //F("Sensor")
      log = F("Get Sensor");
      log += CurIdx;
      if (CurIdx > P092_DataSettings.MaxSensors) {
        result=false;
        break;
      }
      ReadData->Idx = P092_DataSettings.IdxSensor;
      result = P092_fetch_sensor(CurIdx, ReadData);
      break;
    case 2:    //F("Sensor")
      log = F("Get ExtSensor");
      log += CurIdx;
      if (CurIdx > P092_DataSettings.MaxExtSensors) {
        result=false;
        break;
      }
      ReadData->Idx = P092_DataSettings.IdxExtSensor;
      result = P092_fetch_sensor(CurIdx, ReadData);
      break;
    case 3:  //F("Digital output")
      log = F("Get DigitalOutput");
      log += CurIdx;
      if (CurIdx > (8 * P092_DataSettings.OutputBytes)) {
        result=false;
        break;
      }
      result = P092_fetch_output(CurIdx, ReadData);
      break;
    case 4:  //F("Speed step")
      log = F("Get SpeedStep");
      log += CurIdx;
      if (CurIdx > P092_DataSettings.SpeedBytes) {
        result=false;
        break;
      }
      result = P092_fetch_speed(CurIdx, ReadData);
      break;
    case 5:  //F("Analog output")
      log = F("Get AnalogOutput");
      log += CurIdx;
      if (CurIdx > P092_DataSettings.AnalogBytes) {
        result=false;
        break;
      }
      result = P092_fetch_analog(CurIdx, ReadData);
      break;
    case 6:  //F("Heat power (kW)")
      log = F("Get HeatPower");
      log += CurIdx;
      if (CurIdx > P092_DataSettings.MaxHeatMeters) {
        result=false;
        break;
      }
      result = P092_fetch_heatpower(CurIdx, ReadData);
      break;
    case 7:  //F("Heat meter (MWh)"
      log = F("Get HeatMeter");
      log += CurIdx;
      if (CurIdx > P092_DataSettings.MaxHeatMeters) {
        result=false;
        break;
      }
      result = P092_fetch_heatmeter(CurIdx, ReadData);
      break;
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	  log += F(": ");
	  if (result) {
	    log += String(ReadData->value, 1);
	  }
	  else {
	    log += F("nan");
	  }
	  addLog(LOG_LEVEL_INFO, log);
	}
  return result;
}

boolean P092_fetch_sensor(int number, sP092_ReadData* ReadData) {
  float value;
  ReadData->mode = -1;
  number = ReadData->Idx + (number - 1) * 2;
  int32_t sensorvalue = (DLbus_Data->ByteStream[number + 1] << 8) | DLbus_Data->ByteStream[number];
  if (sensorvalue == 0) {
    return false;
  }
  uint8_t sensortype = (sensorvalue & 0x7000) >> 12;
  if (!(sensorvalue & 0x8000)) { // sign positive
    sensorvalue &= 0xfff;
    // calculations for different sensor types
    switch (sensortype) {
      case DLbus_Sensor_DIGITAL:
        value = false;
        break;
      case DLbus_Sensor_TEMP:
        value = sensorvalue * 0.1;
        break;
      case DLbus_Sensor_RAYS:
        value = sensorvalue;
        break;
      case DLbus_Sensor_VOLUME_FLOW:
        value = sensorvalue * 4;
        break;
      case DLbus_Sensor_ROOM:
        ReadData->mode = (sensorvalue & 0x600) >> 9;
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
      case DLbus_Sensor_DIGITAL:
        value = true;
        break;
      case DLbus_Sensor_TEMP:
        value = (sensorvalue - 0x10000) * 0.1;
        break;
      case DLbus_Sensor_RAYS:
        value = sensorvalue - 0x10000;
        break;
      case DLbus_Sensor_VOLUME_FLOW:
        value = (sensorvalue - 0x10000) * 4;
        break;
      case DLbus_Sensor_ROOM:
        ReadData->mode = (sensorvalue & 0x600) >> 9;
        value = ((sensorvalue & 0x1ff) - 0x10000) * 0.1;
        break;
      default:
        return false;
    }
  }
  ReadData->value = value;
  return true;
}

boolean P092_fetch_output(int number, sP092_ReadData* ReadData) {
  int32_t outputs;

  if (P092_DataSettings.OutputBytes > 1)
    outputs = (DLbus_Data->ByteStream[P092_DataSettings.IdxOutput + 1] << 8) | DLbus_Data->ByteStream[P092_DataSettings.IdxOutput];
  else
    outputs = DLbus_Data->ByteStream[P092_DataSettings.IdxOutput];
	if (outputs & (1 << (number - 1)))
  	ReadData->value = 1;
	else
  	ReadData->value = 0;
  return true;
}

boolean P092_fetch_speed(int number, sP092_ReadData* ReadData) {
  uint8_t speedbyte;

  if ((P092_DataSettings.IdxDrehzahl + (number - 1)) >= P092_DataSettings.IdxAnalog) {
    // wrong index for speed, overlapping next index (IdxAnalog)
    return false;
  }
  speedbyte = DLbus_Data->ByteStream[P092_DataSettings.IdxDrehzahl + (number - 1)];
  if (speedbyte & 0x80)
    return false;
  ReadData->value = (speedbyte & 0x1f);
  return true;
}

boolean P092_fetch_analog(int number, sP092_ReadData* ReadData) {
  uint8_t analogbyte;

  if ((P092_DataSettings.IdxAnalog + (number - 1)) >= P092_DataSettings.IdxHmRegister) {
    // wrong index for analog, overlapping next index (IdxHmRegister)
    return false;
  }
  analogbyte = DLbus_Data->ByteStream[P092_DataSettings.IdxAnalog + (number - 1)];
  if (analogbyte & 0x80)
    return false;
  ReadData->value = (analogbyte * 0.1);
  return true;
}

sDLbus_HMindex P092_CheckHmRegister(int number) {
sDLbus_HMindex result;

  result.IndexIsValid = 0;
  switch (number) {
    case 1:
      if ((DLbus_Data->ByteStream[P092_DataSettings.IdxHmRegister] & 0x1) == 0)
        return result;
      result.power_index = P092_DataSettings.IdxHeatMeter1;
      result.kwh_index = P092_DataSettings.IdxkWh1;
      result.mwh_index = P092_DataSettings.IdxMWh1;
      break;
    case 2:
      if ((DLbus_Data->ByteStream[P092_DataSettings.IdxHmRegister] & 0x2) == 0)
        return result;
      result.power_index = P092_DataSettings.IdxHeatMeter2;
      result.kwh_index = P092_DataSettings.IdxkWh2;
      result.mwh_index = P092_DataSettings.IdxMWh2;
      break;
    case 3:
      if ((DLbus_Data->ByteStream[P092_DataSettings.IdxHmRegister] & 0x4) == 0)
        return result;
      result.power_index = P092_DataSettings.IdxHeatMeter3;
      result.kwh_index = P092_DataSettings.IdxkWh3;
      result.mwh_index = P092_DataSettings.IdxMWh3;
      break;
    default:
      return result;
  }
  result.IndexIsValid = 1;
  return result;
}

boolean P092_fetch_heatpower(int number, sP092_ReadData* ReadData) {
  // current power
  int32_t high;
  sDLbus_HMindex HMindex = P092_CheckHmRegister(number);
  if (HMindex.IndexIsValid == 0)
    return false;
  uint8_t b1 = DLbus_Data->ByteStream[HMindex.power_index];
  uint8_t b2 = DLbus_Data->ByteStream[HMindex.power_index + 1];
  if (P092_DataSettings.CurrentHmBytes > 2) {
    uint8_t b3 = DLbus_Data->ByteStream[HMindex.power_index + 2];
    uint8_t b4 = DLbus_Data->ByteStream[HMindex.power_index + 3];
    high = 0x10000 * b4 + 0x100 * b3 + b2;
    int low = (b1 * 10) / 0x100;
    if (!(b4 & 0x80)) // sign positive
      ReadData->value = (10 * high + low) / 100;
    else // sign negative
      ReadData->value = (10 * (high - 0x10000) - low) / 100;
  }
  else {
    high = (b2 << 8) | b1;
    if ((b2 & 0x80) == 0) // sign positive
      ReadData->value = high / 10;
    else // sign negative
      ReadData->value = (high - 0x10000) / 10;
  }
  return true;
}

boolean P092_fetch_heatmeter(int number, sP092_ReadData* ReadData) {
  // heat meter
  int32_t heat_meter;
  float heat_meter_mwh;

  sDLbus_HMindex HMindex = P092_CheckHmRegister(number);
  if (HMindex.IndexIsValid == 0)
    return false;
  heat_meter = (DLbus_Data->ByteStream[HMindex.kwh_index + 1] << 8) | DLbus_Data->ByteStream[HMindex.kwh_index];
  heat_meter_mwh = (heat_meter * 0.1) / 1000; // in MWh
  if (heat_meter_mwh > 1.0) {
    // in kWh
    heat_meter = heat_meter_mwh;
    heat_meter_mwh -= heat_meter;
  }
  // MWh
  heat_meter = (DLbus_Data->ByteStream[HMindex.mwh_index + 1] << 8) | DLbus_Data->ByteStream[HMindex.mwh_index];
  ReadData->value = heat_meter_mwh + heat_meter;
  return true;
}

#endif // USES_P092
