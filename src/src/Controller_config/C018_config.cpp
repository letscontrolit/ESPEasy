#include "../Controller_config/C018_config.h"

#ifdef USES_C018

# include "../Controller_struct/C018_data_struct.h"

# define C018_BAUDRATE_LABEL     "baudrate"

void C018_ConfigStruct::validate() {
  ZERO_TERMINATE(DeviceEUI);
  ZERO_TERMINATE(DeviceAddr);
  ZERO_TERMINATE(NetworkSessionKey);
  ZERO_TERMINATE(AppSessionKey);

  if ((baudrate < 2400) || (baudrate > 115200)) {
    reset();
  }

  if (stackVersion >= RN2xx3_datatypes::TTN_stack_version::TTN_NOT_SET) {
    stackVersion = RN2xx3_datatypes::TTN_stack_version::TTN_v3;
  }

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
}

void C018_ConfigStruct::reset() {
  ZERO_FILL(DeviceEUI);
  ZERO_FILL(DeviceAddr);
  ZERO_FILL(NetworkSessionKey);
  ZERO_FILL(AppSessionKey);
  baudrate      = 57600;
  rxpin         = -1;
  txpin         = -1;
  resetpin      = -1;
  sf            = 7;
  frequencyplan = RN2xx3_datatypes::Freq_plan::TTN_EU;
  rx2_freq      = 0;
  stackVersion  = RN2xx3_datatypes::TTN_stack_version::TTN_v3;
  joinmethod    = C018_USE_OTAA;
}

void C018_ConfigStruct::webform_load(C018_data_struct *C018_data) const {
  const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(serialPort);

  {
    addFormTextBox(F("Device EUI"), F("deveui"), DeviceEUI, C018_DEVICE_EUI_LEN - 1);
    String deveui_note = F("Leave empty to use HW DevEUI: ");

    if (C018_data != nullptr) {
      deveui_note += C018_data->hweui();
    }
    addFormNote(deveui_note, F("deveui_note"));
  }

  addFormTextBox(F("Device Addr"),         F("devaddr"), DeviceAddr,        C018_DEVICE_ADDR_LEN - 1);
  addFormTextBox(F("Network Session Key"), F("nskey"),   NetworkSessionKey, C018_NETWORK_SESSION_KEY_LEN - 1);
  addFormTextBox(F("App Session Key"),     F("appskey"), AppSessionKey,     C018_APP_SESSION_KEY_LEN - 1);

  {
    const __FlashStringHelper *options[2] = { F("OTAA"),  F("ABP") };
    const int values[2]                   = { C018_USE_OTAA, C018_USE_ABP };
    addFormSelector_script(F("Activation Method"), F("joinmethod"), 2,
                           options, values, nullptr, joinmethod,
                           F("joinChanged(this)")); // Script to toggle OTAA/ABP fields visibility when changing selection.
  }
  html_add_script(F("document.getElementById('joinmethod').onchange();"), false);

  addTableSeparator(F("Connection Configuration"), 2, 3);
  {
    const __FlashStringHelper *options[4] = { F("SINGLE_CHANNEL_EU"), F("TTN_EU"), F("TTN_US"), F("DEFAULT_EU") };
    int values[4]                         =
    {
      RN2xx3_datatypes::Freq_plan::SINGLE_CHANNEL_EU,
      RN2xx3_datatypes::Freq_plan::TTN_EU,
      RN2xx3_datatypes::Freq_plan::TTN_US,
      RN2xx3_datatypes::Freq_plan::DEFAULT_EU
    };

    addFormSelector(F("Frequency Plan"), F("frequencyplan"), 4, options, values, nullptr, frequencyplan, false);
    addFormNumericBox(F("RX2 Frequency"), F("rx2freq"), rx2_freq, 0);
    addUnit(F("Hz"));
    addFormNote(F("0 = default, or else override default"));
  }
  {
    const __FlashStringHelper *options[2] = { F("TTN v2"), F("TTN v3") };
    int values[2]                         = {
      RN2xx3_datatypes::TTN_stack_version::TTN_v2,
      RN2xx3_datatypes::TTN_stack_version::TTN_v3
    };

    addFormSelector(F("TTN Stack"), F("ttnstack"), 2, options, values, nullptr, stackVersion, false);
  }

  addFormNumericBox(F("Spread Factor"), F("sf"), sf, 7, 12);
  addFormCheckBox(F("Adaptive Data Rate (ADR)"), F("adr"), adr);


  addTableSeparator(F("Serial Port Configuration"), 2, 3);

  serialHelper_webformLoad(port, rxpin, txpin, true);

  // Show serial port selection
  addFormPinSelect(PinSelectPurpose::Generic_input,  formatGpioName_RX(false), F("taskdevicepin1"), rxpin);
  addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_TX(false), F("taskdevicepin2"), txpin);

  html_add_script(F("document.getElementById('serPort').onchange();"), false);

  addFormNumericBox(F("Baudrate"), F(C018_BAUDRATE_LABEL), baudrate, 2400, 115200);
  addUnit(F("baud"));
  addFormNote(F("Module default baudrate: 57600 bps"));

  // Optional reset pin RN2xx3
  addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output_optional(F("Reset")), F("taskdevicepin3"), resetpin);

  addTableSeparator(F("Device Status"), 2, 3);

  if (C018_data != nullptr) {
    // Some information on detected device
    addRowLabel(F("Hardware DevEUI"));
    addHtml(C018_data->hweui());
    addRowLabel(F("Version Number"));
    addHtml(C018_data->sysver());

    addRowLabel(F("Voltage"));
    addHtmlFloat(static_cast<float>(C018_data->getVbat()) / 1000.0f, 3);

    addRowLabel(F("Device Addr"));
    addHtml(C018_data->getDevaddr());

    uint32_t dnctr, upctr;

    if (C018_data->getFrameCounters(dnctr, upctr)) {
      addRowLabel(F("Frame Counters (down/up)"));
      String values = String(dnctr);
      values += '/';
      values += upctr;
      addHtml(values);
    }

    addRowLabel(F("Last Command Error"));
    addHtml(C018_data->getLastError());

    addRowLabel(F("Sample Set Counter"));
    addHtmlInt(C018_data->getSampleSetCount());

    addRowLabel(F("Data Rate"));
    addHtml(C018_data->getDataRate());

    {
      RN2xx3_status status = C018_data->getStatus();

      addRowLabel(F("Status RAW value"));
      addHtmlInt(status.getRawStatus());

      addRowLabel(F("Activation Status"));
      addEnabled(status.Joined);

      addRowLabel(F("Silent Immediately"));
      addHtmlInt(status.SilentImmediately ? 1 : 0);
    }
  }
}

void C018_ConfigStruct::webform_save() {
  reset();
  const String deveui  = webArg(F("deveui"));
  const String devaddr = webArg(F("devaddr"));
  const String nskey   = webArg(F("nskey"));
  const String appskey = webArg(F("appskey"));

  strlcpy(DeviceEUI,         deveui.c_str(),  sizeof(DeviceEUI));
  strlcpy(DeviceAddr,        devaddr.c_str(), sizeof(DeviceAddr));
  strlcpy(NetworkSessionKey, nskey.c_str(),   sizeof(NetworkSessionKey));
  strlcpy(AppSessionKey,     appskey.c_str(), sizeof(AppSessionKey));
  baudrate      = getFormItemInt(F(C018_BAUDRATE_LABEL), baudrate);
  rxpin         = getFormItemInt(F("taskdevicepin1"), rxpin);
  txpin         = getFormItemInt(F("taskdevicepin2"), txpin);
  resetpin      = getFormItemInt(F("taskdevicepin3"), resetpin);
  sf            = getFormItemInt(F("sf"), sf);
  frequencyplan = getFormItemInt(F("frequencyplan"), frequencyplan);
  rx2_freq      = getFormItemInt(F("rx2freq"), rx2_freq);
  joinmethod    = getFormItemInt(F("joinmethod"), joinmethod);
  stackVersion  = getFormItemInt(F("ttnstack"), stackVersion);
  adr           = isFormItemChecked(F("adr"));
  serialHelper_webformSave(serialPort, rxpin, txpin);
}

#endif // ifdef USES_C018
