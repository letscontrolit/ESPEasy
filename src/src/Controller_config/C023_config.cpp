#include "../Controller_config/C023_config.h"

#ifdef USES_C023

# include "../Controller_struct/C023_data_struct.h"

# define C023_BAUDRATE_LABEL     "baudrate"

void C023_ConfigStruct::validate() {
  ZERO_TERMINATE(DeviceEUI);
  ZERO_TERMINATE(DeviceAddr);
  ZERO_TERMINATE(NetworkSessionKey);
  ZERO_TERMINATE(AppSessionKey);

  if ((baudrate < 2400) || (baudrate > 115200)) {
    reset();
  }

  if (LoRaWAN_Class > static_cast<uint8_t>(LoRa_Helper::LoRaWANclass_e::C)) {
    // Set to default class A
    LoRaWAN_Class = static_cast<uint8_t>(LoRa_Helper::LoRaWANclass_e::A);
  }

  /*

     switch (frequencyplan) {
      case RN2xx3_datatypes::Freq_plan::SINGLE_CHANNEL_EU:
      case RN2xx3_datatypes::Freq_plan::TTN_EU:
      case RN2xx3_datatypes::Freq_plan::DEFAULT_EU:

        if ((rx2_freq < 867000000) || (rx2_freq > 870000000)) {
          rx2_freq = 0;
        }
        break;
      case RN2xx3_datatypes::Freq_plan::TTN_US:
        // FIXME TD-er: Need to find the ranges for US (and other regions)
        break;
      default:
        rx2_freq = 0;
        break;
     }
   */
}

void C023_ConfigStruct::reset() {
  ZERO_FILL(DeviceEUI);
  ZERO_FILL(DeviceAddr);
  ZERO_FILL(NetworkSessionKey);
  ZERO_FILL(AppSessionKey);
  baudrate = 57600;
  rxpin    = -1;
  txpin    = -1;
  resetpin = -1;
  dr       = 5;

  //  frequencyplan = RN2xx3_datatypes::Freq_plan::TTN_EU;
  rx2_freq = 0;

  //  stackVersion  = RN2xx3_datatypes::TTN_stack_version::TTN_v3;
  joinmethod = C023_USE_OTAA;
}

void C023_ConfigStruct::webform_load(C023_data_struct *C023_data) {
  validate();
  ESPEasySerialPort port = static_cast<ESPEasySerialPort>(serialPort);

  {
    addFormTextBox(F("Device EUI"), F("deveui"), DeviceEUI, C023_DEVICE_EUI_LEN - 1);
    String deveui_note = F("Leave empty to use HW DevEUI: ");

    if (C023_data != nullptr) {
      deveui_note += C023_data->hweui();
    }
    addFormNote(deveui_note, F("deveui_note"));
  }

  addFormTextBox(F("Device Addr"),         F("devaddr"), DeviceAddr,        C023_DEVICE_ADDR_LEN - 1);
  addFormTextBox(F("Network Session Key"), F("nskey"),   NetworkSessionKey, C023_NETWORK_SESSION_KEY_LEN - 1);
  addFormTextBox(F("App Session Key"),     F("appskey"), AppSessionKey,     C023_APP_SESSION_KEY_LEN - 1);

  LoRa_Helper::addLoRaWAN_JoinMethod_FormSelector(F("Activation Method"), F("joinmethod"), getJoinMethod());

  addTableSeparator(F("Connection Configuration"), 2, 3);

  /*
     {
     const __FlashStringHelper *options[] = { F("SINGLE_CHANNEL_EU"), F("TTN_EU"), F("TTN_US"), F("DEFAULT_EU") };
     int values[]                         =
     {
      RN2xx3_datatypes::Freq_plan::SINGLE_CHANNEL_EU,
      RN2xx3_datatypes::Freq_plan::TTN_EU,
      RN2xx3_datatypes::Freq_plan::TTN_US,
      RN2xx3_datatypes::Freq_plan::DEFAULT_EU
     };
     const FormSelectorOptions selector( NR_ELEMENTS(options), options, values);
     selector.addFormSelector(F("Frequency Plan"), F("frequencyplan"), frequencyplan);
     addFormNumericBox(F("RX2 Frequency"), F("rx2freq"), rx2_freq, 0);
     addUnit(F("Hz"));
     addFormNote(F("0 = default, or else override default"));
     }
   */

  LoRa_Helper::addLoRaWAN_DR_FormSelector(F("Data Rate"), F("dr"), getDR());
  LoRa_Helper::addLoRaWANclass_FormSelector(
    F("LoRaWAN Class"), F("loraclass"), getClass());

  addTableSeparator(F("LoRaWAN module"), 2, 3);
  {
    const __FlashStringHelper *options[] = {
      C023_AT_commands::toString(C023_AT_commands::LoRaModule_e::Dragino_LA66),
      C023_AT_commands::toString(C023_AT_commands::LoRaModule_e::RAK_3172)
    };
    int values[] =
    {
      static_cast<int>(C023_AT_commands::LoRaModule_e::Dragino_LA66),
      static_cast<int>(C023_AT_commands::LoRaModule_e::RAK_3172)
    };
    const FormSelectorOptions selector(NR_ELEMENTS(options), options, values);
    selector.addFormSelector(F("Module"), F("module"), LoRa_module);
  }

  addTableSeparator(F("Serial Port Configuration"), 2, 3);

  serialHelper_webformLoad(port, rxpin, txpin, true);

  // Show serial port selection
  addFormPinSelect(PinSelectPurpose::Generic_input,  formatGpioName_serialRX(false), F("taskdevicepin1"), rxpin);
  addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_serialTX(false), F("taskdevicepin2"), txpin);

  html_add_script(F("document.getElementById('serPort').onchange();"), false);

  addFormNumericBox(F("Baudrate"), F(C023_BAUDRATE_LABEL), baudrate, 2400, 115200);
  addUnit(F("baud"));
  addFormNote(F("Module default baudrate: 9600 bps"));

  // Optional reset pin RN2xx3
  addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output_optional(F("Reset")), F("taskdevicepin3"), resetpin);

  addTableSeparator(F("Downlink Messages"), 2, 3);

  LoRa_Helper::addEventFormatStructure_FormSelector(
    F("Event Format"), F("eventformat"), getEventFormat());


  if (C023_data != nullptr) {
    addTableSeparator(F("Statistics"), 2, 3);

    // Some information on detected device
    addRowLabel(F("Voltage"));
    addHtmlFloat(static_cast<float>(C023_data->getVbat()) / 1000.0f, 3);

    addRowLabel(F("Device Addr"));
    addHtml(C023_data->getDevaddr());

    uint32_t dnctr, upctr;

    if (C023_data->getFrameCounters(dnctr, upctr)) {
      addRowLabel(F("Frame Counters (down/up)"));
      addHtml(strformat(F("%u / %u"), dnctr, upctr));
    }

    addRowLabel(F("Last Command Error"));
    addHtml(C023_data->getLastError());

    addRowLabel(F("Sample Set Counter"));
    addHtmlInt(static_cast<uint32_t>(C023_data->getSampleSetCount()));

    addRowLabel(F("Data Rate"));
    addHtml(C023_data->getDataRate_str());
  }
}

void C023_ConfigStruct::webform_save() {
  reset();
  String deveui  = webArg(F("deveui"));
  String devaddr = webArg(F("devaddr"));
  String nskey   = webArg(F("nskey"));
  String appskey = webArg(F("appskey"));

  strlcpy(DeviceEUI,         deveui.c_str(),  sizeof(DeviceEUI));
  strlcpy(DeviceAddr,        devaddr.c_str(), sizeof(DeviceAddr));
  strlcpy(NetworkSessionKey, nskey.c_str(),   sizeof(NetworkSessionKey));
  strlcpy(AppSessionKey,     appskey.c_str(), sizeof(AppSessionKey));
  baudrate      = getFormItemInt(F(C023_BAUDRATE_LABEL), baudrate);
  rxpin         = getFormItemInt(F("taskdevicepin1"), rxpin);
  txpin         = getFormItemInt(F("taskdevicepin2"), txpin);
  resetpin      = getFormItemInt(F("taskdevicepin3"), resetpin);
  dr            = getFormItemInt(F("dr"), dr);
  eventFormat   = getFormItemInt(F("eventformat"), eventFormat);
  rx2_freq      = getFormItemInt(F("rx2freq"), rx2_freq);
  joinmethod    = getFormItemInt(F("joinmethod"), joinmethod);
  LoRaWAN_Class = getFormItemInt(F("loraclass"), LoRaWAN_Class);
  LoRa_module   = getFormItemInt(F("module"), LoRa_module);
  serialHelper_webformSave(serialPort, rxpin, txpin);
}

#endif // ifdef USES_C023
