#ifdef USES_P092

// #######################################################################################################
// ########################### Plugin 092: DL-bus from Technische Alternative ############################
// #######################################################################################################

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

// #define PLUGIN_092_DEBUG    // additional debug messages in the log
#define PLUGIN_NAME_092       "Heating - DL-Bus (Technische Alternative)  [TESTING]"
#define PLUGIN_VALUENAME1_092 "Value"


boolean Plugin_092(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;


  const String plugin_092_ValStr = F("p092_Value");
  const String plugin_092_IdxStr = F("p092_Idx");


  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_092;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = P092_DLbus_ValueCount;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].DecimalsOnly       = true;
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
        const String Devices[P092_DLbus_DeviceCount] = { F("ESR21"), F("UVR31"), F("UVR1611"), F("UVR 61-3 (bis V8.2)"), F(
                                                           "UVR 61-3 (ab V8.3)") };
        const int DevTypes[P092_DLbus_DeviceCount] = { 21, 31, 1611, 6132, 6133 };

        addFormSelector(F("DL-Bus Type"), F("p092_dlbtype"), P092_DLbus_DeviceCount, Devices, DevTypes, NULL, PCONFIG(0), true);
      }
      {
        int P092_ValueType, P092_ValueIdx;
        const String plugin_092_DefValueName                  = F(PLUGIN_VALUENAME1_092);
        const int    P092_OptionTypes[P092_DLbus_OptionCount] = {
          // Index der Variablen
          0, // F("None")
          1, // F("Sensor")
          2, // F("Ext. sensor")
          3, // F("Digital output")
          4, // F("Speed step")
          5, // F("Analog output")
          6, // F("Heat power (kW)")
          7  // F("Heat meter (MWh)")
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
        P092_MaxIdx[0] = 0;      // None
        P092_MaxIdx[1] = 3;      // Sensor
        P092_MaxIdx[2] = 0;      // Ext. sensor
        P092_MaxIdx[3] = 1;      // Digital output
        P092_MaxIdx[4] = 0;      // Speed step
        P092_MaxIdx[5] = 0;      // Analog output
        P092_MaxIdx[6] = 0;      // Heat power (kW)
        P092_MaxIdx[7] = 0;      // Heat meter (MWh)

        switch (PCONFIG(0)) {
          case 21:               // ESR21
            P092_MaxIdx[2] = 6;  // Ext. sensor
            P092_MaxIdx[4] = 1;  // Speed step
            P092_MaxIdx[5] = 1;  // Analog output
            P092_MaxIdx[6] = 1;  // Heat power (kW)
            P092_MaxIdx[7] = 1;  // Heat meter (MWh)
            break;
          case 1611:             // UVR1611
            P092_MaxIdx[1] = 16; // Sensor
            P092_MaxIdx[3] = 13; // Digital output
            P092_MaxIdx[4] = 4;  // Speed step
            P092_MaxIdx[6] = 2;  // Heat power (kW)
            P092_MaxIdx[7] = 2;  // Heat meter (MWh)
            break;
          case 6132:             // UVR 61-3 (bis V8.2)
            P092_MaxIdx[1] = 6;  // Sensor
            P092_MaxIdx[3] = 8;  // Digital output
            P092_MaxIdx[4] = 1;  // Speed step
            P092_MaxIdx[5] = 1;  // Analog output
            P092_MaxIdx[6] = 1;  // Heat power (kW)
            P092_MaxIdx[7] = 1;  // Heat meter (MWh)
            break;
          case 6133:             // UVR 61-3 (ab V8.3)
            P092_MaxIdx[1] = 6;  // Sensor
            P092_MaxIdx[2] = 9;  // Ext. sensor
            P092_MaxIdx[3] = 3;  // Digital output
            P092_MaxIdx[4] = 1;  // Speed step
            P092_MaxIdx[5] = 2;  // Analog output
            P092_MaxIdx[6] = 3;  // Heat power (kW)
            P092_MaxIdx[7] = 3;  // Heat meter (MWh)
            break;
        }

        addFormSubHeader(F("Inputs"));

        for (int i = 0; i < P092_DLbus_ValueCount; i++) {
          P092_ValueType = PCONFIG(i + 1) >> 8;
          P092_ValueIdx  = PCONFIG(i + 1) & 0x00FF;

          addFormSelector(plugin_092_DefValueName,
                          plugin_092_ValStr,
                          P092_DLbus_OptionCount,
                          Options,
                          P092_OptionTypes,
                          NULL,
                          P092_ValueType,
                          true);

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
        0, // F("None")
        1, // [0,1°C]     F("Sensor")
        1, // [0,1°C]     F("Ext. sensor")
        0, //            F("Digital output")
        0, //            F("Speed step")
        1, // [0,1V]      F("Analog output")
        1, // [0,1kW]     F("Heat power (kW)") Attention: UVR1611 in 0,01kW
        4  // [0,0001MWh] F("Heat meter (MWh)")
      };

      PCONFIG(0) = getFormItemInt(F("p092_dlbtype"));

      if (PCONFIG(0) == 1611) { // only UVR1611
        P092_OptionValueDecimals[6] = 2;
      }

      for (int i = 0; i < P092_DLbus_ValueCount; i++) {
        int OptionIdx = getFormItemInt(plugin_092_ValStr);
        int CurIdx    = getFormItemInt(plugin_092_IdxStr);

        if (CurIdx < 1) {
          CurIdx = 1;
        }
        PCONFIG(i + 1)                                                     = (OptionIdx << 8) + CurIdx;
        ExtraTaskSettings.TaskDeviceValueDecimals[event->BaseVarIndex + i] = P092_OptionValueDecimals[OptionIdx];
      }
      P092_data_struct *P092_data =
        static_cast<P092_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P092_data) {
        addLog(LOG_LEVEL_ERROR, F("## P092_save: Error DL-Bus: Class not initialized!"));
        return false;
      }

#ifdef PLUGIN_092_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("PLUGIN_WEBFORM_SAVE :");
        log += F(" DLB_Pin:");
        log += CONFIG_PIN1;
        log += F(" DLbus_MinPulseWidth:");
        log += P092_data->P092_DataSettings.DLbus_MinPulseWidth;
        log += F(" DLbus_MaxPulseWidth:");
        log += P092_data->P092_DataSettings.DLbus_MaxPulseWidth;
        log += F(" DLbus_MinDoublePulseWidth:");
        log += P092_data->P092_DataSettings.DLbus_MinDoublePulseWidth;
        log += F(" DLbus_MaxDoublePulseWidth:");
        log += P092_data->P092_DataSettings.DLbus_MaxDoublePulseWidth;
        log += F(" IdxSensor:");
        log += P092_data->P092_DataSettings.IdxSensor;
        log += F(" IdxExtSensor:");
        log += P092_data->P092_DataSettings.IdxExtSensor;
        log += F(" IdxOutput:");
        log += P092_data->P092_DataSettings.IdxOutput;

        if (P092_data->P092_DataSettings.SpeedBytes > 0) {
          log += F(" IdxDrehzahl:");
          log += P092_data->P092_DataSettings.IdxDrehzahl;
        }

        if (P092_data->P092_DataSettings.AnalogBytes > 0) {
          log += F(" IdxAnalog:");
          log += P092_data->P092_DataSettings.IdxAnalog;
        }

        if (P092_data->P092_DataSettings.MaxHeatMeters > 0) {
          log += F(" IdxHmRegister:");
          log += P092_data->P092_DataSettings.IdxHmRegister;
        }

        if (P092_data->P092_DataSettings.VolumeBytes > 0) {
          log += F(" IdxVolume:");
          log += P092_data->P092_DataSettings.IdxVolume;
        }

        if (P092_data->P092_DataSettings.MaxHeatMeters > 0) {
          log += F(" IdxHM1:");
          log += P092_data->P092_DataSettings.IdxHeatMeter1;
          log += F(" IdxkWh1:");
          log += P092_data->P092_DataSettings.IdxkWh1;
          log += F(" IdxMWh1:");
          log += P092_data->P092_DataSettings.IdxMWh1;
        }

        if (P092_data->P092_DataSettings.MaxHeatMeters > 1) {
          log += F(" IdxHM2:");
          log += P092_data->P092_DataSettings.IdxHeatMeter2;
          log += F(" IdxkWh2:");
          log += P092_data->P092_DataSettings.IdxkWh2;
          log += F(" IdxMWh2:");
          log += P092_data->P092_DataSettings.IdxMWh2;
        }

        if (P092_data->P092_DataSettings.MaxHeatMeters > 2) {
          log += F(" IdxHM3:");
          log += P092_data->P092_DataSettings.IdxHeatMeter3;
          log += F(" IdxkWh3:");
          log += P092_data->P092_DataSettings.IdxkWh3;
          log += F(" IdxMWh3:");
          log += P092_data->P092_DataSettings.IdxMWh3;
        }
        log += F(" IdxCRC:");
        log += P092_data->P092_DataSettings.IdxCRC;
        addLog(LOG_LEVEL_INFO, log);
      }
#endif // PLUGIN_092_DEBUG
      UserVar[event->BaseVarIndex] = NAN;
      success                      = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P092_data_struct());
      P092_data_struct *P092_data =
        static_cast<P092_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P092_data) {
        addLog(LOG_LEVEL_INFO, F("P092_init ..."));

        if (!P092_data->init(CONFIG_PIN1)) {
          addLog(LOG_LEVEL_ERROR, F("## P092_init: Error DL-Bus: Class not initialized!"));
          clearPluginTaskData(event->TaskIndex);
          return false;
        }
        success                      = true;
        UserVar[event->BaseVarIndex] = NAN;
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      if (!NetworkConnected()) {
        return false;
      }
      P092_data_struct *P092_data =
        static_cast<P092_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P092_data) {
        return false;
      }

      if (P092_data->DLbus_Data->ISR_Receiving) {
        return false;
      }
      P092_data->Plugin_092_SetIndices(PCONFIG(0));

      if (P092_data->DLbus_Data->ISR_AllBitsReceived) {
        P092_data->DLbus_Data->ISR_AllBitsReceived = false;
        success                                    = P092_data->DLbus_Data->CheckTimings();

        if (success) {
          success = P092_data->DLbus_Data->Processing();
        }

        if (success) {
          success = P092_data->DLbus_Data->CheckCRC(P092_data->P092_DataSettings.IdxCRC);
        }

        if (success) {
          addLog(LOG_LEVEL_INFO, F("Received data OK"));
          P092_data->P092_LastReceived = millis();
        }
        P092_data->P092_ReceivedOK = success;
      }
      else {
        success = P092_data->P092_ReceivedOK;
      }

      if ((P092_data->P092_ReceivedOK == false) ||
          (timePassedSince(P092_data->P092_LastReceived) > (static_cast<long>(Settings.TaskDeviceTimer[event->TaskIndex] * 1000 / 2)))) {
        P092_data->Plugin_092_StartReceiving();
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
      P092_data_struct *P092_data =
        static_cast<P092_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P092_data) {
        addLog(LOG_LEVEL_ERROR, F("## P092_read: Error DL-Bus: Not initialized!"));
        return false;
      }

      if (P092_data->DLbus_Data->ISR_DLB_Pin != CONFIG_PIN1) {
        String log = F("## P092_read: Error DL-Bus: Device Pin setting not correct!");
        log += F(" DLB_Pin:");
        log += P092_data->DLbus_Data->ISR_DLB_Pin;
        log += F(" Setting:");
        log += CONFIG_PIN1;
        addLog(LOG_LEVEL_ERROR, log);
        return false;
      }
      success = P092_data->P092_ReceivedOK;

      if (P092_data->P092_ReceivedOK == false) {
        addLog(LOG_LEVEL_INFO, F("P092_read: Still receiving DL-Bus bits!"));
      }
      else {
        P092_data_struct::sP092_ReadData P092_ReadData;

        for (int i = 0; i < P092_DLbus_ValueCount; i++) {
          int OptionIdx = PCONFIG(i + 1) >> 8;
          int CurIdx    = PCONFIG(i + 1) & 0x00FF;

          if (P092_data->P092_GetData(OptionIdx, CurIdx, &P092_ReadData)) {
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

#endif // USES_P092
