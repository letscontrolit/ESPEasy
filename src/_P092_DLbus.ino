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

<<<<<<< HEAD
   @uwekaditz 2020-03-01 Memory usage optimized
	 CHG: Moved arrays into the class DLBus
   CHG: Moved arrays to PLUGIN_092_DEBUG

=======
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
	 @uwekaditz 2019-12-15 Memory usage optimized
	 CHG: Moved the array for the received bit changes to stativ uint_8t, the ISR call uses only a volatile pointer to it
	 CHG: some more defines and name changes for better explanation

	 @uwekaditz 2019-12-14 Timing optimized
	 CHG: Removed the while (P092_receiving) loop.
	 CHG: Starting of the receiving and processing of the received bit stream are now done in the PLUGIN_ONCE_A_SECOND call
			  PLUGIN_READ call just uses the already processed data

	 @uwekaditz 2019-12-08 Inital commit to mega

\**************************************************/

<<<<<<< HEAD
#include <DLBus.h>

=======
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
#define PLUGIN_092
#define PLUGIN_ID_092         92
//#define PLUGIN_092_DEBUG    // additional debug messages in the log
#define PLUGIN_NAME_092       "Heating - DL-Bus (Technische Alternative)"
#define PLUGIN_VALUENAME1_092 "Value"

#define P092_DLbus_OptionCount 8
#define P092_DLbus_ValueCount 1
#define P092_DLbus_DeviceCount 5

<<<<<<< HEAD
=======
void P092_Pin_changed() ICACHE_RAM_ATTR;
volatile uint8_t P092_DLB_Pin = 0xFF;
volatile boolean P092_receiving = false;            // receiving flag
volatile boolean P092_AllBitsReceived = false;
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
uint8_t P092_Last_DLB_Pin;
boolean P092_init = false;
boolean P092_ReceivedOK = false;
uint32_t P092_LastReceived = 0;
<<<<<<< HEAD
=======
uint8_t P092_MaxIdx[P092_DLbus_OptionCount];
int P092_ValueType[P092_DLbus_ValueCount];
int P092_ValueIdx[P092_DLbus_ValueCount];
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
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
<<<<<<< HEAD

DLBus *DLbus_Data = nullptr;
=======
sP092_ReadData P092_ReadData;

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
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3

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

<<<<<<< HEAD
boolean P092_GetData(int OptionIdx, int CurIdx, sP092_ReadData* ReadData);

=======
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
boolean Plugin_092(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;
  int OptionIdx, CurIdx;
<<<<<<< HEAD
  sP092_ReadData P092_ReadData;

  uint8_t P092_MaxIdx[P092_DLbus_OptionCount];
  int P092_ValueType, P092_ValueIdx;
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


  const String plugin_092_ValStr = F("p092_Value");
  const String plugin_092_IdxStr = F("p092_Idx");
=======

  const String plugin_092_Values[P092_DLbus_ValueCount] = {
    F("p092_Value")
  };
  const String plugin_092_Indizes[P092_DLbus_ValueCount] = {
    F("p092_Idx")
  };
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3


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
        P092_Last_DLB_Pin = CONFIG_PIN1;

<<<<<<< HEAD
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
=======
        const String plugin_092_ValueNames[P092_DLbus_ValueCount] = {
          F(PLUGIN_VALUENAME1_092)
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
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
<<<<<<< HEAD
        const int DevTypes[P092_DLbus_DeviceCount] = { 21, 31, 1611, 6132, 6133 };
=======
        int DevTypes[P092_DLbus_DeviceCount] = { 21, 31, 1611, 6132, 6133 };
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3

        addFormSelector(F("DL-Bus Type"), F("p092_dlbtype"), P092_DLbus_DeviceCount, Devices, DevTypes, NULL, PCONFIG(0), true );

        // Calculation of the max indices for each sensor type
<<<<<<< HEAD
        // default indizes for UVR31
=======
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
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
            P092_MaxIdx[0] = 0; // None
            P092_MaxIdx[1] = 3; // Sensor
            P092_MaxIdx[2] = 0; // Ext. sensor
            P092_MaxIdx[3] = 1; // Digital output
            P092_MaxIdx[4] = 0; // Speed step
            P092_MaxIdx[5] = 0; // Analog output
            P092_MaxIdx[6] = 0; // Heat power (kW)
            P092_MaxIdx[7] = 0; // Heat meter (MWh)
<<<<<<< HEAD
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
=======
            break;
          case 1611:  //UVR1611
            P092_MaxIdx[0] = 0; // None
            P092_MaxIdx[1] = 16; // Sensor
            P092_MaxIdx[2] = 0; // Ext. sensor
            P092_MaxIdx[3] = 13; // Digital output
            P092_MaxIdx[4] = 4; // Speed step
            P092_MaxIdx[5] = 0; // Analog output
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
            P092_MaxIdx[6] = 2; // Heat power (kW)
            P092_MaxIdx[7] = 2; // Heat meter (MWh)
            break;
          case 6132:  //UVR 61-3 (bis V8.2)
<<<<<<< HEAD
            P092_MaxIdx[1] = 6; // Sensor
=======
            P092_MaxIdx[0] = 0; // None
            P092_MaxIdx[1] = 6; // Sensor
            P092_MaxIdx[2] = 0; // Ext. sensor
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
            P092_MaxIdx[3] = 8; // Digital output
            P092_MaxIdx[4] = 1; // Speed step
            P092_MaxIdx[5] = 1; // Analog output
            P092_MaxIdx[6] = 1; // Heat power (kW)
            P092_MaxIdx[7] = 1; // Heat meter (MWh)
            break;
          case 6133:  //UVR 61-3 (ab V8.3)
<<<<<<< HEAD
=======
            P092_MaxIdx[0] = 0; // None
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
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
<<<<<<< HEAD
          P092_ValueType = PCONFIG(i + 1) >> 8;
          P092_ValueIdx = PCONFIG(i + 1) & 0x00FF;

          addFormSelector(plugin_092_DefValueName, plugin_092_ValStr, P092_DLbus_OptionCount, Options, P092_OptionTypes, NULL, P092_ValueType, true);
          if (P092_MaxIdx[P092_ValueType] > 1) {
            CurIdx = P092_ValueIdx;
            if (CurIdx < 1) {
              CurIdx = 1;
            }
            if (CurIdx > P092_MaxIdx[P092_ValueType]) {
              CurIdx = P092_MaxIdx[P092_ValueType];
            }
	          addHtml(F(" Index: "));
            addNumericBox(plugin_092_IdxStr, CurIdx, 1, P092_MaxIdx[P092_ValueType]);
=======
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
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
          }
        }

        UserVar[event->BaseVarIndex] = NAN;

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("p092_dlbtype"));
<<<<<<< HEAD
        if (PCONFIG(0) == 1611)  // only UVR1611
          P092_OptionValueDecimals[6] = 2;

        for (int i = 0; i < P092_DLbus_ValueCount; i++) {
          OptionIdx = getFormItemInt(plugin_092_ValStr);
          CurIdx = getFormItemInt(plugin_092_IdxStr);
=======
        Plugin_092_SetIndices(PCONFIG(0));

        for (int i = 0; i < P092_DLbus_ValueCount; i++) {
          OptionIdx = getFormItemInt(plugin_092_Values[i]);
          CurIdx = getFormItemInt(plugin_092_Indizes[i]);
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
          if (CurIdx < 1) {
            CurIdx = 1;
          }
          PCONFIG(i + 1) = (OptionIdx << 8) + CurIdx;
          ExtraTaskSettings.TaskDeviceValueDecimals[event->BaseVarIndex + i] = P092_OptionValueDecimals[OptionIdx];
        }

<<<<<<< HEAD
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
=======
        if (P092_Last_DLB_Pin != CONFIG_PIN1) {
          // pin number is changed -> run a new init
          P092_init = false;
          if (P092_DLB_Pin != 0xFF) {
            // interrupt was already attached to P092_DLB_Pin
            detachInterrupt(digitalPinToInterrupt(P092_DLB_Pin));
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
            addLog(LOG_LEVEL_INFO, F("P092_save: detachInterrupt"));
          }
        }

#ifdef PLUGIN_092_DEBUG
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
<<<<<<< HEAD

	        String log = F("PLUGIN_WEBFORM_SAVE :");
	        log += F(" DLB_Pin:");
	        log += CONFIG_PIN1;
=======
	        P092_DLB_Pin = CONFIG_PIN1;

	        String log = F("PLUGIN_WEBFORM_SAVE :");
	        log += F(" DLB_Pin:");
	        log += P092_DLB_Pin;
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
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
<<<<<<< HEAD
          addLog(LOG_LEVEL_INFO, F("P092_init ..."));
          if (DLbus_Data == nullptr) {
            DLbus_Data = new DLBus;
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
=======
          P092_init = true;
          addLog(LOG_LEVEL_INFO, F("P092_init: attachInterrupt"));
          P092_DLB_Pin = CONFIG_PIN1;
          pinMode(P092_DLB_Pin, INPUT_PULLUP);
          P092_receiving = false;
          P092_ReceivedOK = false;
          // on a CHANGE on the data pin P092_Pin_changed is called
          attachInterrupt(digitalPinToInterrupt(P092_DLB_Pin), P092_Pin_changed, CHANGE);
          UserVar[event->BaseVarIndex] = NAN;
        }
        Plugin_092_SetIndices(PCONFIG(0));
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        if (WiFi.status() != WL_CONNECTED) {
          return false;
        }
        if (P092_init == false) {
          return false;
        }
<<<<<<< HEAD
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
=======
        if (P092_receiving) {
          return false;
        }
        if (P092_AllBitsReceived) {
          P092_AllBitsReceived = false;
          success = Plugin_092_CheckTimings();
          if (success)
            success = P092_Processing();
          if (success)
            success = P092_CheckCRC();
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
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
<<<<<<< HEAD
        if (WiFi.status() != WL_CONNECTED) {
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
=======
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
          // too busy for DLbus while wifi connect is running
          addLog(LOG_LEVEL_ERROR, F("## Error DL-Bus: WiFi not connected!"));
          return false;
        }
        if (P092_init == false) {
          addLog(LOG_LEVEL_ERROR, F("## Error DL-Bus: Not initialized!"));
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
          return false;
        }
        success = P092_ReceivedOK;
        if (P092_ReceivedOK == false) {
<<<<<<< HEAD
          addLog(LOG_LEVEL_INFO, F("P092_read: Still receiving DL-Bus bits!"));
=======
          addLog(LOG_LEVEL_ERROR, F("## Error DL-Bus: Still receiving bits!"));
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
        }
        else {
          for (int i = 0; i < P092_DLbus_ValueCount; i++) {
            OptionIdx = PCONFIG(i + 1) >> 8;
            CurIdx = PCONFIG(i + 1) & 0x00FF;
<<<<<<< HEAD
            if (P092_GetData(OptionIdx, CurIdx, &P092_ReadData)) {
              UserVar[event->BaseVarIndex + i] = P092_ReadData.value;
            }
            else {
              addLog(LOG_LEVEL_ERROR, F("## P092_read: Error: No readings!"));
=======
            if (P092_GetData(OptionIdx, CurIdx)) {
              UserVar[event->BaseVarIndex + i] = P092_ReadData.value;
            }
            else {
              addLog(LOG_LEVEL_ERROR, F("Error: No readings!"));
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
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
<<<<<<< HEAD
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
=======
  switch (DeviceIndex) {
    case 21:  //ESR21
      P092_DataSettings.DataBytes = 31;
      P092_DataSettings.DLbus_MinPulseWidth = P092_min_width_488;
      P092_DataSettings.DLbus_MaxPulseWidth = P092_max_width_488;
      P092_DataSettings.DLbus_MinDoublePulseWidth = P092_double_min_width_488;
      P092_DataSettings.DLbus_MaxDoublePulseWidth = P092_double_max_width_488;

      P092_DataSettings.DeviceByte0 = 0x70;
      P092_DataSettings.DeviceByte1 = 0x8F;
      P092_DataSettings.DeviceBytes = 2;
      P092_DataSettings.DontCareBytes = 0;
      P092_DataSettings.TimeStampBytes = 0;
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

      P092_OptionValueDecimals[6] = 1; //[0,1kW]     F("Heat power (kW)")
      break;
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
    case 31:  //UVR31
      P092_DataSettings.DataBytes = 8;
      P092_DataSettings.DLbus_MinPulseWidth = P092_min_width_50;
      P092_DataSettings.DLbus_MaxPulseWidth = P092_max_width_50;
      P092_DataSettings.DLbus_MinDoublePulseWidth = P092_double_min_width_50;
      P092_DataSettings.DLbus_MaxDoublePulseWidth = P092_double_max_width_50;

      P092_DataSettings.DeviceByte0 = 0x30;
      P092_DataSettings.DeviceByte1 = 0;
<<<<<<< HEAD
      iDeviceBytes = 1;
      P092_DataSettings.MaxExtSensors = 0;
      P092_DataSettings.SpeedBytes = 0;
      P092_DataSettings.AnalogBytes = 0;
=======
      P092_DataSettings.DeviceBytes = 1;
      P092_DataSettings.DontCareBytes = 0;
      P092_DataSettings.TimeStampBytes = 0;
      P092_DataSettings.MaxSensors = 3;
      P092_DataSettings.MaxExtSensors = 0;
      P092_DataSettings.OutputBytes = 1;
      P092_DataSettings.SpeedBytes = 0;
      P092_DataSettings.AnalogBytes = 0;
      P092_DataSettings.VolumeBytes = 0;
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
      P092_DataSettings.MaxHeatMeters = 0;
      P092_DataSettings.CurrentHmBytes = 0;
      P092_DataSettings.MWhBytes = 0;
      P092_DataSettings.IdxCRC = 0;
      break;
    case 1611:  //UVR1611
      P092_DataSettings.DataBytes = 64;
<<<<<<< HEAD

      P092_DataSettings.DeviceByte0 = 0x80;
      P092_DataSettings.DeviceByte1 = 0x7F;
      iDontCareBytes = 1;
      iTimeStampBytes = 5;
=======
      P092_DataSettings.DLbus_MinPulseWidth = P092_min_width_488;
      P092_DataSettings.DLbus_MaxPulseWidth = P092_max_width_488;
      P092_DataSettings.DLbus_MinDoublePulseWidth = P092_double_min_width_488;
      P092_DataSettings.DLbus_MaxDoublePulseWidth = P092_double_max_width_488;

      P092_DataSettings.DeviceByte0 = 0x80;
      P092_DataSettings.DeviceByte1 = 0x7F;
      P092_DataSettings.DeviceBytes = 2;
      P092_DataSettings.DontCareBytes = 1;
      P092_DataSettings.TimeStampBytes = 5;
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
      P092_DataSettings.MaxSensors = 16;
      P092_DataSettings.MaxExtSensors = 0;
      P092_DataSettings.OutputBytes = 2;
      P092_DataSettings.SpeedBytes = 4;
      P092_DataSettings.AnalogBytes = 0;
<<<<<<< HEAD
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
=======
      P092_DataSettings.VolumeBytes = 0;
      P092_DataSettings.MaxHeatMeters = 2;
      P092_DataSettings.CurrentHmBytes = 4;
      P092_DataSettings.MWhBytes = 2;
      P092_DataSettings.IdxCRC = 63;

      P092_OptionValueDecimals[6] = 2; //[0,01kW]     F("Heat power (kW)")
      break;
    case 6132:  //UVR 61-3 (up to V8.2)
      P092_DataSettings.DataBytes = 35;
      P092_DataSettings.DLbus_MinPulseWidth = P092_min_width_488;
      P092_DataSettings.DLbus_MaxPulseWidth = P092_max_width_488;
      P092_DataSettings.DLbus_MinDoublePulseWidth = P092_double_min_width_488;
      P092_DataSettings.DLbus_MaxDoublePulseWidth = P092_double_max_width_488;

      P092_DataSettings.DeviceByte0 = 0x90;
      P092_DataSettings.DeviceByte1 = 0x6F;
      P092_DataSettings.DeviceBytes = 2;
      P092_DataSettings.DontCareBytes = 1;
      P092_DataSettings.TimeStampBytes = 5;
      P092_DataSettings.MaxSensors = 6;
      P092_DataSettings.MaxExtSensors = 0;
      P092_DataSettings.OutputBytes = 1;
      P092_DataSettings.SpeedBytes = 1;
      P092_DataSettings.AnalogBytes = 1;
      P092_DataSettings.VolumeBytes = 2;
      P092_DataSettings.MaxHeatMeters = 1;
      P092_DataSettings.CurrentHmBytes = 2;
      P092_DataSettings.MWhBytes = 4;
      P092_DataSettings.IdxCRC = 34;

      P092_OptionValueDecimals[6] = 1; //[0,1kW]     F("Heat power (kW)")
      break;
    case 6133:  //UVR 61-3 (from V8.3)
      P092_DataSettings.DataBytes = 62;
      P092_DataSettings.DLbus_MinPulseWidth = P092_min_width_488;
      P092_DataSettings.DLbus_MaxPulseWidth = P092_max_width_488;
      P092_DataSettings.DLbus_MinDoublePulseWidth = P092_double_min_width_488;
      P092_DataSettings.DLbus_MaxDoublePulseWidth = P092_double_max_width_488;

      P092_DataSettings.DeviceByte0 = 0x90;
      P092_DataSettings.DeviceByte1 = 0x9F;
      P092_DataSettings.DeviceBytes = 2;
      P092_DataSettings.DontCareBytes = 1;
      P092_DataSettings.TimeStampBytes = 5;
      P092_DataSettings.MaxSensors = 6;
      P092_DataSettings.MaxExtSensors = 9;
      P092_DataSettings.OutputBytes = 1;
      P092_DataSettings.SpeedBytes = 1;
      P092_DataSettings.AnalogBytes = 2;
      P092_DataSettings.VolumeBytes = 0;
      P092_DataSettings.MaxHeatMeters = 3;
      P092_DataSettings.CurrentHmBytes = 2;
      P092_DataSettings.MWhBytes = 2;
      P092_DataSettings.IdxCRC = 61;

      P092_OptionValueDecimals[6] = 1; //[0,1kW]     F("Heat power (kW)")
      break;
  }
  P092_DataSettings.IdxSensor               = P092_DataSettings.DeviceBytes + P092_DataSettings.DontCareBytes + P092_DataSettings.TimeStampBytes;
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
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
<<<<<<< HEAD

  return;
=======
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
}
/*********************************************************************************************\
  DLBus subs to get values from the P092_receiving bitstream
\*********************************************************************************************/
<<<<<<< HEAD
=======
// one data frame has <P092_DataSettings.DataBytes> data bytes + SYNC, e.g. 64 * (8+1+1) + 16 = 656
// 656 * 2 = 1312 (twice as much as a data frame is saved
// so there's one complete data frame

#define DLbus_MaxDataBytes 64
#define DLbus_AdditionalRecBytes 2
#define DLbus_StopBits 1
#define DLbus_StartBits 1
#define DLBus_SyncBits 16
#define DLBus_ReserveBytes 20
#define DLBus_BitChangeFactor 2
#define DLbus_MaxDataBits (((DLbus_MaxDataBytes + DLbus_AdditionalRecBytes) * (DLbus_StartBits + 8 + DLbus_StopBits) + DLBus_SyncBits) * DLBus_BitChangeFactor) + DLBus_ReserveBytes
        // MaxDataBits is double of the maximum bit length because each bit change is stored
        // (((64+2) * (8+1+1) + 16) * 2) + 50 = 1402 bytes

// variables used in interrupt function
volatile static uint8_t* DLbus_PtrChangeBitStream;          // pointer to received bit change stream
volatile uint16_t DLbus_pulse_count;                        // number of received pulses
volatile uint16_t DLbus_pulse_number;                       // max naumber of the received pulses
volatile uint32_t DLbus_TimeLastBitChange = 0;     			    // remember time of last transition
volatile uint16_t DLbus_MinPulseWidth, DLbus_MaxPulseWidth, DLbus_MinDoublePulseWidth, DLbus_MaxDoublePulseWidth;

static uint8_t DLbus_ChangeBitStream[DLbus_MaxDataBits];    // received bit change stream (each bit change is extended to uint8_t, containing the timing flags)
uint8_t DLbus_ByteStream[DLbus_MaxDataBits / 8 + 1];  // every bit gets sorted into a bitmap
uint16_t DLbus_bit_number;                                  // bit number of the received DLbus_ChangeBitStream

uint8_t DLbus_WrongTimeCnt;
uint16_t DLbus_WrongTimings[5][6];

int16_t DLbus_start_bit;                                    // first bit of data frame (-1 not recognized)
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3

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

<<<<<<< HEAD
boolean P092_fetch_sensor(int number, sP092_ReadData* ReadData);
boolean P092_fetch_output(int number, sP092_ReadData* ReadData);        // digital output byte(s)
boolean P092_fetch_speed(int number, sP092_ReadData* ReadData);         // speed byte(s)
boolean P092_fetch_analog(int number, sP092_ReadData* ReadData);        // analog byte(s)
boolean P092_fetch_heatpower(int number, sP092_ReadData* ReadData);     // heat power(s)
boolean P092_fetch_heatmeter(int number, sP092_ReadData* ReadData);     // heat meters(s)
=======
// Flags for pulse width (bit 0 is the content!)
#define DLbus_FlagSingleWidth	                0x02
#define DLbus_FlagDoubleWidth	                0x04
#define DLbus_FlagShorterThanSingleWidth      0x10
#define DLbus_FlagBetweenDoubleSingleWidth    0x20
#define DLbus_FlagLongerThanDoubleWidth       0x40
#define DLbus_FlagLongerThanTwiceDoubleWidth  0x80
#define DLbus_FlagsWrongTiming                (DLbus_FlagLongerThanTwiceDoubleWidth | DLbus_FlagLongerThanDoubleWidth | DLbus_FlagBetweenDoubleSingleWidth | DLbus_FlagShorterThanSingleWidth)
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3

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
<<<<<<< HEAD
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
=======
  noInterrupts ();                              // make sure we don't get interrupted before we are ready
  DLbus_pulse_count = 0;
  DLbus_pulse_number = (((P092_DataSettings.DataBytes + DLbus_AdditionalRecBytes) * (DLbus_StartBits + 8 +  DLbus_StopBits) + DLBus_SyncBits) * DLBus_BitChangeFactor) + DLBus_ReserveBytes;
  DLbus_MinPulseWidth = P092_DataSettings.DLbus_MinPulseWidth;
  DLbus_MaxPulseWidth = P092_DataSettings.DLbus_MaxPulseWidth;
  DLbus_MinDoublePulseWidth = P092_DataSettings.DLbus_MinDoublePulseWidth;
  DLbus_MaxDoublePulseWidth = P092_DataSettings.DLbus_MaxDoublePulseWidth;
  DLbus_PtrChangeBitStream = DLbus_ChangeBitStream;
  P092_receiving = true;
  P092_AllBitsReceived = false;
  interrupts ();                                // interrupts allowed now, next instruction WILL be executed
  uint32_t start=millis();
  addLog(LOG_LEVEL_INFO, F("P092_receiving ..."));
  while ((timePassedSince(start)<100) && (DLbus_pulse_count == 0)) {
    // wait for first pulse received (timeout 100ms)
    yield();
  }
  if (DLbus_pulse_count == 0) {
    // nothing received
    P092_receiving = false;
    addLog(LOG_LEVEL_ERROR, F("Error: Nothing received! No DL bus connected!"));
  }
  // while (P092_receiving) {
  //   // wait for end of P092_receiving
  //   // loop time depends on the device (bytes in protocol and clock frequency)
  //   // ESR21:     (((31 + 2) * (8 + 1 + 1) + 16) * 2)+50 = (346*2+50)= 742 bits @ 488Hz = 1,52s
  //   // UVR31:     ((( 8 + 2) * (8 + 1 + 1) + 16) * 2)+50 = (116*2+50)= 282 bits @  50Hz = 5,64s
  //   // UVR1611:   (((64 + 2) * (8 + 1 + 1) + 16) * 2)+50 = (676*2+50)=1402 bits @ 488Hz = 2,87s
  //   // UVR 61-3:  (((62 + 2) * (8 + 1 + 1) + 16) * 2)+50 = (676*2+50)=1402 bits @ 488Hz = 2,87s
  //   yield();
  // }
}

boolean Plugin_092_CheckTimings(void) {
  uint8_t rawval, val;
  int i;
  addLog(LOG_LEVEL_INFO, F("Receive stopped."));

  DLbus_WrongTimeCnt = 0;
  DLbus_pulse_count = 0;
  for (i = 0; i <= DLbus_pulse_number; i++) {
    // store DLbus_ChangeBitStream into DLbus_ByteStream
    rawval = DLbus_ChangeBitStream[i];
    if (rawval & DLbus_FlagsWrongTiming) {
      // wrong DLbus_time_diff
      if ((DLbus_pulse_count > 0) && (DLbus_WrongTimeCnt < 5)) {
        DLbus_WrongTimings[DLbus_WrongTimeCnt][0] = i;
        DLbus_WrongTimings[DLbus_WrongTimeCnt][1] = DLbus_pulse_count;
        DLbus_WrongTimings[DLbus_WrongTimeCnt][2] = DLbus_bit_number;
        DLbus_WrongTimings[DLbus_WrongTimeCnt][3] = rawval;
        if ((rawval == DLbus_FlagLongerThanTwiceDoubleWidth) && (DLbus_ChangeBitStream[i-1] == (DLbus_FlagDoubleWidth | 0x01))) {
          // Add two additional short pulses (low and high), previous bit is High and contains DLbus_FlagDoubleWidth
          P092_process_bit(0);
          P092_process_bit(1);
          DLbus_WrongTimings[DLbus_WrongTimeCnt][4] = DLbus_FlagSingleWidth;
          DLbus_WrongTimings[DLbus_WrongTimeCnt][5] = DLbus_FlagSingleWidth+1;
        }
        else {
          DLbus_WrongTimings[DLbus_WrongTimeCnt][4] = 0xff;
          DLbus_WrongTimings[DLbus_WrongTimeCnt][5] = 0xff;
        }
        DLbus_WrongTimeCnt++;
      }
    }
    else {
      val = rawval & 0x01;
      if ((rawval & DLbus_FlagDoubleWidth) == DLbus_FlagDoubleWidth) {
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
  addLog(LOG_LEVEL_INFO, F("DLbus_ChangeBitStream copied."));

#ifdef PLUGIN_092_DEBUG
  if (DLbus_WrongTimeCnt > 0) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	    String log = F("Wrong Timings: ");
	    addLog(LOG_LEVEL_INFO, log);
	    for (i = 0; i < DLbus_WrongTimeCnt; i++) {
	      log = i + 1;
	      log += F(": PulseCount:");
	      log += DLbus_WrongTimings[i][1];
	      log += F(": BitCount:");
	      log += DLbus_WrongTimings[i][2];
	      log += F(" Value:0x");
	      log += String(DLbus_WrongTimings[i][3], HEX);
	      log += F(" ValueBefore:0x");
	      log += String(DLbus_ChangeBitStream[DLbus_WrongTimings[i][0] - 1], HEX);
	      log += F(" ValueAfter:0x");
	      log += String(DLbus_ChangeBitStream[DLbus_WrongTimings[i][0] + 1], HEX);
	      if (DLbus_WrongTimings[i][4]!=0xff) {
	        log += F(" Added:0x");
	        log += String(DLbus_WrongTimings[i][4], HEX);
	      }
	      if (DLbus_WrongTimings[i][5]!=0xff) {
	        log += F(" Added:0x");
	        log += String(DLbus_WrongTimings[i][5], HEX);
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
  long DLbus_time_diff = usecPassedSince(DLbus_TimeLastBitChange);  // time difference to previous pulse in µs
  DLbus_TimeLastBitChange = micros();                                   // save last pin change time
  if (P092_receiving) {
    uint8_t val = digitalRead(P092_DLB_Pin);  // read state
    // check pulse width
    if (DLbus_time_diff >= 2*DLbus_MinDoublePulseWidth)     val |= DLbus_FlagLongerThanTwiceDoubleWidth;  // longer then 2x double pulse width
    else if (DLbus_time_diff > DLbus_MaxDoublePulseWidth)   val |= DLbus_FlagLongerThanDoubleWidth;       // longer then double pulse width
    else if (DLbus_time_diff >= DLbus_MinDoublePulseWidth)  val |= DLbus_FlagDoubleWidth;                 // double pulse width
    else if (DLbus_time_diff > DLbus_MaxPulseWidth)         val |= DLbus_FlagBetweenDoubleSingleWidth;    // between double and single pulse width
    else if (DLbus_time_diff < DLbus_MinPulseWidth)         val |= DLbus_FlagShorterThanSingleWidth;      // shorter then single pulse width
    else val |= DLbus_FlagSingleWidth;                                   // single pulse width
    if (DLbus_pulse_count < 2) {
      // check if sync is received
      if (val & DLbus_FlagLongerThanTwiceDoubleWidth) {
        // sync received
        *DLbus_PtrChangeBitStream = !(val & 0x01);
        *(DLbus_PtrChangeBitStream+1) = val;
        DLbus_pulse_count = 2;
      }
      else
        DLbus_pulse_count = 1;                                        // flag that interrupt is receiving
    }
    else {
      *(DLbus_PtrChangeBitStream+DLbus_pulse_count) = val;                      // set bit
      DLbus_pulse_count++;
      P092_receiving = (DLbus_pulse_count < DLbus_pulse_number);     // stop P092_receiving when data frame is complete
      P092_AllBitsReceived = !P092_receiving;
    }
  }
}

/*****************\
  DLBus processing
\*****************/

void P092_process_bit(uint8_t b) {
  // ignore first pulse
  DLbus_pulse_count++;
  if (DLbus_pulse_count % 2)
    return;
  DLbus_bit_number = (DLbus_pulse_count / 2);
  if (b)
    DLbus_ByteStream[DLbus_bit_number / 8] |= (1 << (DLbus_bit_number % 8)); // set bit
  else
    DLbus_ByteStream[DLbus_bit_number / 8] &= ~(1 << (DLbus_bit_number % 8)); // clear bit
}

boolean P092_Processing(void) {
boolean inverted = false;
String log;
  addLog(LOG_LEVEL_INFO, F("Processing..."));
  DLbus_start_bit = P092_Analyze(); // find the data frame's beginning
  // inverted signal?
  while (DLbus_start_bit == -1) {
    if (inverted) {
      addLog(LOG_LEVEL_ERROR, F("Error: Already inverted!"));
      return false;
    }
    addLog(LOG_LEVEL_INFO, F("Invert bit stream..."));
    P092_Invert(); // invert again
    inverted = true;
    DLbus_start_bit = P092_Analyze();
    if (DLbus_start_bit == -1) {
      addLog(LOG_LEVEL_ERROR, F("Error: No data frame available!"));
      return false;
    }
    uint16_t RequiredBitStreamLength = (DLbus_pulse_number - DLBus_ReserveBytes)/DLBus_BitChangeFactor;
    if ((DLbus_bit_number-DLbus_start_bit) < RequiredBitStreamLength) {
      // no complete data frame available (difference between start_bit and received bits is < RequiredBitStreamLength)
      addLog(LOG_LEVEL_ERROR, F("Start bit too close to end of stream!"));
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	      log = F("# Required bits: ");
	      log += RequiredBitStreamLength;
	      log += F(" StartBit: ");
	      log += DLbus_start_bit;
	      log += F(" / EndBit: ");
	      log += DLbus_bit_number;
	      addLog(LOG_LEVEL_INFO, log);
			}
      return false;
    }
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	  log = F("StartBit: ");
	  log += DLbus_start_bit;
	  log += F(" / EndBit: ");
	  log += DLbus_bit_number;
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
  uint8_t sync=0;
  // find SYNC (16 * sequential 1)
  for (int i = 0; i < DLbus_bit_number; i++) {
    if (P092_ReadBit(i))
      sync++;
    else
      sync = 0;
    if (sync == DLBus_SyncBits) {
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
  for (int i = 0; i < DLbus_bit_number; i++)
    P092_WriteBit(i, P092_ReadBit(i) ? 0 : 1); // invert every bit
}
uint8_t P092_ReadBit(int pos) {
  int row = pos / 8; // detect position in bitmap
  int col = pos % 8;
  return (((DLbus_ByteStream[row]) >> (col)) & 0x01); // return bit
}
void P092_WriteBit(int pos, uint8_t set) {
  int row = pos / 8; // detect position in bitmap
  int col = pos % 8;
  if (set)
    DLbus_ByteStream[row] |= 1 << col; // set bit
  else
    DLbus_ByteStream[row] &= ~(1 << col); // clear bit
}
void P092_Trim() {
  for (int i = DLbus_start_bit, bit = 0; i < DLbus_bit_number; i++) {
    int offset = i - DLbus_start_bit;
    // ignore start and stop bits:
    // start bits: 0 10 20 30, also  x    % 10 == 0
    // stop bits:  9 19 29 39, also (x+1) % 10 == 0
    if (offset % 10 && (offset + 1) % 10) {
      P092_WriteBit(bit, P092_ReadBit(i));
      bit++;
    }
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
  }
}

/****************\
  DLBus get data
\****************/

<<<<<<< HEAD
boolean P092_GetData(int OptionIdx, int CurIdx, sP092_ReadData* ReadData) {
=======
boolean P092_CheckDevice() {
  // Data frame of a device?
  if (DLbus_ByteStream[0] == P092_DataSettings.DeviceByte0) {
    if ((P092_DataSettings.DeviceBytes == 1) || (DLbus_ByteStream[1] == P092_DataSettings.DeviceByte1))
      return true;
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	  String log = F("# Received DeviceByte(s): 0x");
	  log += String(DLbus_ByteStream[0], HEX);
	  if (P092_DataSettings.DeviceBytes > 1)
	    log += String(DLbus_ByteStream[1], HEX);
	  log += F(" Requested: 0x");
	  log += String(P092_DataSettings.DeviceByte0, HEX);
	  if (P092_DataSettings.DeviceBytes > 1)
	    log += String(P092_DataSettings.DeviceByte1, HEX);
	  addLog(LOG_LEVEL_INFO, log);
	}
  return false;
}

boolean P092_CheckCRC() {
  // CRC check sum
  if (P092_DataSettings.IdxCRC == 0)
    return true;
  addLog(LOG_LEVEL_INFO, F("Check CRC..."));
  uint16_t dataSum = 0;
  for (int i = 0; i < P092_DataSettings.IdxCRC; i++)
    dataSum = dataSum + DLbus_ByteStream[i];
  dataSum = dataSum & 0xff;
  if (dataSum == DLbus_ByteStream[P092_DataSettings.IdxCRC])
    return true;
  addLog(LOG_LEVEL_ERROR, F("Check CRC failed!"));
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	  String log = F("# Calculated CRC: 0x");
	  log += String(dataSum, HEX);
	  log += F(" Received: 0x");
	  log += String(DLbus_ByteStream[P092_DataSettings.IdxCRC], HEX);
	  addLog(LOG_LEVEL_INFO, log);
	}
  return false;
}

boolean P092_GetData(int OptionIdx, int CurIdx) {
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
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
<<<<<<< HEAD
      ReadData->Idx = P092_DataSettings.IdxSensor;
      result = P092_fetch_sensor(CurIdx, ReadData);
=======
      P092_ReadData.Idx = P092_DataSettings.IdxSensor;
      result = P092_fetch_sensor(CurIdx);
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
      break;
    case 2:    //F("Sensor")
      log = F("Get ExtSensor");
      log += CurIdx;
      if (CurIdx > P092_DataSettings.MaxExtSensors) {
        result=false;
        break;
      }
<<<<<<< HEAD
      ReadData->Idx = P092_DataSettings.IdxExtSensor;
      result = P092_fetch_sensor(CurIdx, ReadData);
=======
      P092_ReadData.Idx = P092_DataSettings.IdxExtSensor;
      result = P092_fetch_sensor(CurIdx);
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
      break;
    case 3:  //F("Digital output")
      log = F("Get DigitalOutput");
      log += CurIdx;
      if (CurIdx > (8 * P092_DataSettings.OutputBytes)) {
        result=false;
        break;
      }
<<<<<<< HEAD
      result = P092_fetch_output(CurIdx, ReadData);
=======
      result = P092_fetch_output(CurIdx);
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
      break;
    case 4:  //F("Speed step")
      log = F("Get SpeedStep");
      log += CurIdx;
      if (CurIdx > P092_DataSettings.SpeedBytes) {
        result=false;
        break;
      }
<<<<<<< HEAD
      result = P092_fetch_speed(CurIdx, ReadData);
=======
      result = P092_fetch_speed(CurIdx);
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
      break;
    case 5:  //F("Analog output")
      log = F("Get AnalogOutput");
      log += CurIdx;
      if (CurIdx > P092_DataSettings.AnalogBytes) {
        result=false;
        break;
      }
<<<<<<< HEAD
      result = P092_fetch_analog(CurIdx, ReadData);
=======
      result = P092_fetch_analog(CurIdx);
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
      break;
    case 6:  //F("Heat power (kW)")
      log = F("Get HeatPower");
      log += CurIdx;
      if (CurIdx > P092_DataSettings.MaxHeatMeters) {
        result=false;
        break;
      }
<<<<<<< HEAD
      result = P092_fetch_heatpower(CurIdx, ReadData);
=======
      result = P092_fetch_heatpower(CurIdx);
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
      break;
    case 7:  //F("Heat meter (MWh)"
      log = F("Get HeatMeter");
      log += CurIdx;
      if (CurIdx > P092_DataSettings.MaxHeatMeters) {
        result=false;
        break;
      }
<<<<<<< HEAD
      result = P092_fetch_heatmeter(CurIdx, ReadData);
=======
      result = P092_fetch_heatmeter(CurIdx);
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
      break;
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
	  log += F(": ");
	  if (result) {
<<<<<<< HEAD
	    log += String(ReadData->value, 1);
=======
	    log += String(P092_ReadData.value, 1);
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
	  }
	  else {
	    log += F("nan");
	  }
	  addLog(LOG_LEVEL_INFO, log);
	}
  return result;
}

<<<<<<< HEAD
boolean P092_fetch_sensor(int number, sP092_ReadData* ReadData) {
  float value;
  ReadData->mode = -1;
  number = ReadData->Idx + (number - 1) * 2;
  int32_t sensorvalue = (DLbus_Data->ByteStream[number + 1] << 8) | DLbus_Data->ByteStream[number];
=======
boolean P092_fetch_sensor(int number) {
  float value;
  P092_ReadData.mode = -1;
  number = P092_ReadData.Idx + (number - 1) * 2;
  int32_t sensorvalue = (DLbus_ByteStream[number + 1] << 8) | DLbus_ByteStream[number];
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
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
<<<<<<< HEAD
        ReadData->mode = (sensorvalue & 0x600) >> 9;
=======
        P092_ReadData.mode = (sensorvalue & 0x600) >> 9;
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
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
<<<<<<< HEAD
        ReadData->mode = (sensorvalue & 0x600) >> 9;
=======
        P092_ReadData.mode = (sensorvalue & 0x600) >> 9;
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
        value = ((sensorvalue & 0x1ff) - 0x10000) * 0.1;
        break;
      default:
        return false;
    }
  }
<<<<<<< HEAD
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
=======
  P092_ReadData.value = value;
  return true;
}

boolean P092_fetch_output(int number) {
  int32_t outputs;

  if (P092_DataSettings.OutputBytes > 1)
    outputs = (DLbus_ByteStream[P092_DataSettings.IdxOutput + 1] << 8) | DLbus_ByteStream[P092_DataSettings.IdxOutput];
  else
    outputs = DLbus_ByteStream[P092_DataSettings.IdxOutput];
	if (outputs & (1 << (number - 1)))
  	P092_ReadData.value = 1;
	else
  	P092_ReadData.value = 0;
  return true;
}

boolean P092_fetch_speed(int number) {
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
  uint8_t speedbyte;

  if ((P092_DataSettings.IdxDrehzahl + (number - 1)) >= P092_DataSettings.IdxAnalog) {
    // wrong index for speed, overlapping next index (IdxAnalog)
    return false;
  }
<<<<<<< HEAD
  speedbyte = DLbus_Data->ByteStream[P092_DataSettings.IdxDrehzahl + (number - 1)];
  if (speedbyte & 0x80)
    return false;
  ReadData->value = (speedbyte & 0x1f);
  return true;
}

boolean P092_fetch_analog(int number, sP092_ReadData* ReadData) {
=======
  speedbyte = DLbus_ByteStream[P092_DataSettings.IdxDrehzahl + (number - 1)];
  if (speedbyte & 0x80)
    return false;
  P092_ReadData.value = (speedbyte & 0x1f);
  return true;
}

boolean P092_fetch_analog(int number) {
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
  uint8_t analogbyte;

  if ((P092_DataSettings.IdxAnalog + (number - 1)) >= P092_DataSettings.IdxHmRegister) {
    // wrong index for analog, overlapping next index (IdxHmRegister)
    return false;
  }
<<<<<<< HEAD
  analogbyte = DLbus_Data->ByteStream[P092_DataSettings.IdxAnalog + (number - 1)];
  if (analogbyte & 0x80)
    return false;
  ReadData->value = (analogbyte * 0.1);
=======
  analogbyte = DLbus_ByteStream[P092_DataSettings.IdxAnalog + (number - 1)];
  if (analogbyte & 0x80)
    return false;
  P092_ReadData.value = (analogbyte * 0.1);
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
  return true;
}

sDLbus_HMindex P092_CheckHmRegister(int number) {
sDLbus_HMindex result;

  result.IndexIsValid = 0;
  switch (number) {
    case 1:
<<<<<<< HEAD
      if ((DLbus_Data->ByteStream[P092_DataSettings.IdxHmRegister] & 0x1) == 0)
=======
      if ((DLbus_ByteStream[P092_DataSettings.IdxHmRegister] & 0x1) == 0)
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
        return result;
      result.power_index = P092_DataSettings.IdxHeatMeter1;
      result.kwh_index = P092_DataSettings.IdxkWh1;
      result.mwh_index = P092_DataSettings.IdxMWh1;
      break;
    case 2:
<<<<<<< HEAD
      if ((DLbus_Data->ByteStream[P092_DataSettings.IdxHmRegister] & 0x2) == 0)
=======
      if ((DLbus_ByteStream[P092_DataSettings.IdxHmRegister] & 0x2) == 0)
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
        return result;
      result.power_index = P092_DataSettings.IdxHeatMeter2;
      result.kwh_index = P092_DataSettings.IdxkWh2;
      result.mwh_index = P092_DataSettings.IdxMWh2;
      break;
    case 3:
<<<<<<< HEAD
      if ((DLbus_Data->ByteStream[P092_DataSettings.IdxHmRegister] & 0x4) == 0)
=======
      if ((DLbus_ByteStream[P092_DataSettings.IdxHmRegister] & 0x4) == 0)
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
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

<<<<<<< HEAD
boolean P092_fetch_heatpower(int number, sP092_ReadData* ReadData) {
=======
boolean P092_fetch_heatpower(int number) {
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
  // current power
  int32_t high;
  sDLbus_HMindex HMindex = P092_CheckHmRegister(number);
  if (HMindex.IndexIsValid == 0)
    return false;
<<<<<<< HEAD
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
=======
  uint8_t b1 = DLbus_ByteStream[HMindex.power_index];
  uint8_t b2 = DLbus_ByteStream[HMindex.power_index + 1];
  if (P092_DataSettings.CurrentHmBytes > 2) {
    uint8_t b3 = DLbus_ByteStream[HMindex.power_index + 2];
    uint8_t b4 = DLbus_ByteStream[HMindex.power_index + 3];
    high = 0x10000 * b4 + 0x100 * b3 + b2;
    int low = (b1 * 10) / 0x100;
    if (!(b4 & 0x80)) // sign positive
      P092_ReadData.value = (10 * high + low) / 100;
    else // sign negative
      P092_ReadData.value = (10 * (high - 0x10000) - low) / 100;
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
  }
  else {
    high = (b2 << 8) | b1;
    if ((b2 & 0x80) == 0) // sign positive
<<<<<<< HEAD
      ReadData->value = high / 10;
    else // sign negative
      ReadData->value = (high - 0x10000) / 10;
=======
      P092_ReadData.value = high / 10;
    else // sign negative
      P092_ReadData.value = (high - 0x10000) / 10;
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
  }
  return true;
}

<<<<<<< HEAD
boolean P092_fetch_heatmeter(int number, sP092_ReadData* ReadData) {
=======
boolean P092_fetch_heatmeter(int number) {
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
  // heat meter
  int32_t heat_meter;
  float heat_meter_mwh;

  sDLbus_HMindex HMindex = P092_CheckHmRegister(number);
  if (HMindex.IndexIsValid == 0)
    return false;
<<<<<<< HEAD
  heat_meter = (DLbus_Data->ByteStream[HMindex.kwh_index + 1] << 8) | DLbus_Data->ByteStream[HMindex.kwh_index];
=======
  heat_meter = (DLbus_ByteStream[HMindex.kwh_index + 1] << 8) | DLbus_ByteStream[HMindex.kwh_index];
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
  heat_meter_mwh = (heat_meter * 0.1) / 1000; // in MWh
  if (heat_meter_mwh > 1.0) {
    // in kWh
    heat_meter = heat_meter_mwh;
    heat_meter_mwh -= heat_meter;
  }
  // MWh
<<<<<<< HEAD
  heat_meter = (DLbus_Data->ByteStream[HMindex.mwh_index + 1] << 8) | DLbus_Data->ByteStream[HMindex.mwh_index];
  ReadData->value = heat_meter_mwh + heat_meter;
=======
  heat_meter = (DLbus_ByteStream[HMindex.mwh_index + 1] << 8) | DLbus_ByteStream[HMindex.mwh_index];
  P092_ReadData.value = heat_meter_mwh + heat_meter;
>>>>>>> 3f4af8f9c084cc2d974477b7cc12c5e26fe412f3
  return true;
}

#endif // USES_P092
