#include "_Plugin_Helper.h"
#ifdef USES_P148


// #######################################################################################################
// #################### Plugin 148: Sonoff POWR3xxD and THR3xxD display ##################################
// #######################################################################################################


# define PLUGIN_148
# define PLUGIN_ID_148         148
# define PLUGIN_NAME_148       "Display - POWR3xxD/THR3xxD"


# define P148_DEVICE_SELECTOR  PCONFIG(0)
# define P148_GPIO_TM1621_DAT  CONFIG_PIN1
# define P148_GPIO_TM1621_WR   CONFIG_PIN2
# define P148_GPIO_TM1621_RD   CONFIG_PIN3
# define P148_GPIO_TM1621_CS   CONFIG_PORT

# define P148_FIRST_PAGE_ROW_INDEX 2
# define P148_NR_PAGE_ROW_INDICES 6
# define P148_MAX_PAGE_ROW_INDEX (P148_FIRST_PAGE_ROW_INDEX + P148_NR_PAGE_ROW_INDICES)
# define P148_PAGE1_ROW1_TASK  PCONFIG(P148_FIRST_PAGE_ROW_INDEX)
# define P148_PAGE1_ROW2_TASK  PCONFIG(3)
# define P148_PAGE2_ROW1_TASK  PCONFIG(4)
# define P148_PAGE2_ROW2_TASK  PCONFIG(5)
# define P148_PAGE3_ROW1_TASK  PCONFIG(6)
# define P148_PAGE3_ROW2_TASK  PCONFIG(7)


# include "src/PluginStructs/P148_data_struct.h"

boolean Plugin_148(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_148;
      Device[deviceCount].Type               = DEVICE_TYPE_CUSTOM0;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_148);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      const __FlashStringHelper *labels[] = {
        F("DAT"), F("WR"), F("RD"), F("CS")
      };
      int values[] = {
        P148_GPIO_TM1621_DAT,
        P148_GPIO_TM1621_WR,
        P148_GPIO_TM1621_RD,
        P148_GPIO_TM1621_CS
      };
      constexpr size_t nrElements = sizeof(values) / sizeof(values[0]);

      for (size_t i = 0; i < nrElements; ++i) {
        if (i != 0) { addHtml(event->String1); }
        addHtml(labels[i]);
        addHtml(F(":&nbsp;"));
        addHtml(formatGpioLabel(values[i], true));
      }
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P148_DEVICE_SELECTOR = 0;
      P148_GPIO_TM1621_DAT = -1;
      P148_GPIO_TM1621_CS  = -1;
      P148_GPIO_TM1621_WR  = -1;
      P148_GPIO_TM1621_RD  = -1;
      P148_data_struct::MonitorTaskValue_t MonitorTaskValue;

      for (int i = P148_FIRST_PAGE_ROW_INDEX; i < P148_MAX_PAGE_ROW_INDEX; ++i) {
        PCONFIG(i) = MonitorTaskValue.getPconfigValue();
      }

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // We load/save the TaskDevicePin ourselves to allow to combine the pin specific configuration be shown along with the pin selection.
      addFormPinSelect(PinSelectPurpose::Generic_output,
                       formatGpioName_output(F("TM1621 DAT")),
                       F("taskdevicepin1"),
                       P148_GPIO_TM1621_DAT);
      addFormPinSelect(PinSelectPurpose::Generic_output,
                       formatGpioName_output(F("TM1621 WR")),
                       F("taskdevicepin2"),
                       P148_GPIO_TM1621_WR);
      addFormPinSelect(PinSelectPurpose::Generic_output,
                       formatGpioName_output(F("TM1621 RD")),
                       F("taskdevicepin3"),
                       P148_GPIO_TM1621_RD);
      addFormPinSelect(PinSelectPurpose::Generic_output,
                       formatGpioName_output(F("TM1621 CS")),
                       F("TDP"),
                       P148_GPIO_TM1621_CS);

      {
        const __FlashStringHelper *options[] = { F("Custom"), F("Sonoff POWR3xxD"), F("Sonoff THR3xxD") };
        int optionValues[] {
          static_cast<int>(P148_data_struct::Tm1621Device::USER),
          static_cast<int>(P148_data_struct::Tm1621Device::POWR3xxD),
          static_cast<int>(P148_data_struct::Tm1621Device::THR3xxD)
        };
        constexpr size_t nrElements = sizeof(optionValues) / sizeof(optionValues[0]);

        addFormSelector(F("Device Template"), F("devtmpl"), nrElements, options, optionValues, P148_DEVICE_SELECTOR);
        addFormNote(F("GPIO settings will be ignored when selecting other than 'Custom'"));
      }

      addFormSubHeader(F("Display Values"));

      for (int i = P148_FIRST_PAGE_ROW_INDEX; i < P148_MAX_PAGE_ROW_INDEX; ++i) {
        if ((i % 2 == 0) && (i != P148_FIRST_PAGE_ROW_INDEX)) {
          addFormSeparator(2);
        }

        P148_data_struct::MonitorTaskValue_t MonitorTaskValue(PCONFIG(i));
        MonitorTaskValue.webformLoad(i);
      }

      // Add hidden checkbox which will only be checked when a select dropdown list is changed and settings should not be saved.
      html_TR_TD();

      addHtml(F("<label class='nosave'>&nbsp;<input "));
      addHtmlAttribute(F("type"), F("checkbox"));
      addHtmlAttribute(F("id"),   F("nosave"));
      addHtmlAttribute(F("name"), F("nosave"));
      addHtmlAttribute(F("style"), F("display:none"));
      addHtml(F("><span class='checkmark'></span></label>"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P148_DEVICE_SELECTOR = getFormItemInt(F("devtmpl"));

      switch (static_cast<P148_data_struct::Tm1621Device>(P148_DEVICE_SELECTOR)) {
        case P148_data_struct::Tm1621Device::USER:
          // We load/save the TaskDevicePin ourselves to allow to combine the pin specific configuration be shown along with the pin
          // selection.
          P148_GPIO_TM1621_DAT = getFormItemInt(F("taskdevicepin1"));
          P148_GPIO_TM1621_WR  = getFormItemInt(F("taskdevicepin2"));
          P148_GPIO_TM1621_RD  = getFormItemInt(F("taskdevicepin3"));
          P148_GPIO_TM1621_CS  = getFormItemInt(F("TDP"));
          break;
        case P148_data_struct::Tm1621Device::POWR3xxD:
          P148_GPIO_TM1621_DAT = 14;
          P148_GPIO_TM1621_WR  = 27;
          P148_GPIO_TM1621_RD  = 26;
          P148_GPIO_TM1621_CS  = 25;
          break;
        case P148_data_struct::Tm1621Device::THR3xxD:
          P148_GPIO_TM1621_DAT = 5;
          P148_GPIO_TM1621_WR  = 18;
          P148_GPIO_TM1621_RD  = 23;
          P148_GPIO_TM1621_CS  = 17;
          break;
      }

      for (int i = P148_FIRST_PAGE_ROW_INDEX; i < P148_MAX_PAGE_ROW_INDEX; ++i) {
        P148_data_struct::MonitorTaskValue_t MonitorTaskValue(PCONFIG(i));
        PCONFIG(i) = MonitorTaskValue.webformSave(i);
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      P148_data_struct::Tm1621_t config;
      config.pin_da = P148_GPIO_TM1621_DAT;
      config.pin_wr = P148_GPIO_TM1621_WR;
      config.pin_rd = P148_GPIO_TM1621_RD;
      config.pin_cs = P148_GPIO_TM1621_CS;
      config.device = static_cast<P148_data_struct::Tm1621Device>(P148_DEVICE_SELECTOR);
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P148_data_struct(config));
      P148_data_struct *P148_data =
        static_cast<P148_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P148_data) {
        if (P148_data->init()) {
          for (int i = P148_FIRST_PAGE_ROW_INDEX; i < P148_MAX_PAGE_ROW_INDEX; ++i) {
            P148_data->MonitorTaskValues[i - P148_FIRST_PAGE_ROW_INDEX] = P148_data_struct::MonitorTaskValue_t(PCONFIG(i));
          }

          P148_data->writeStrings(F("ESP"), F("Easy"));
          success = true;
        }
      }
      break;
    }

    case PLUGIN_READ:
    {
      P148_data_struct *P148_data =
        static_cast<P148_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P148_data) {
        P148_data->showPage();
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P148_data_struct *P148_data =
        static_cast<P148_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P148_data) {
        const String command = parseString(string, 1);

        if (equals(command, F("tm1621"))) {
          const String subcommand = parseString(string, 2);

          if (equals(subcommand, F("raw"))) {
            // Write raw data to the display
            // Typical use case: testing fonts
            const String rawdata_str = parseString(string, 3);
            uint64_t     rawdata;

            if (validUInt64FromString(rawdata_str, rawdata)) {
              success = true;
              P148_data->writeRawData(rawdata);
            }
          } else if (equals(subcommand, F("writerow"))) {
            // tm1621write,<rownr>,<string>
            const bool firstrow = event->Par2 <= 1;
            P148_data->setUnit(P148_data_struct::Tm1621UnitOfMeasure::None, firstrow);
            P148_data->writeString(firstrow, parseString(string, 4));
            success = true;
          } else if (equals(subcommand, F("write"))) {
            // tm1621write,<string1>,<string2>
            P148_data->setUnit(P148_data_struct::Tm1621UnitOfMeasure::None);
            const String str1 = parseString(string, 3);
            const String str2 = parseString(string, 4);

            if (str2.isEmpty()) {
              P148_data->writeString(true, str1);
            } else {
              P148_data->writeStrings(str1, str2);
            }
            success = true;
          } else if (equals(subcommand, F("voltamp"))) {
            // tm1621voltamp,<volt>,<amp>
            P148_data->setUnit(P148_data_struct::Tm1621UnitOfMeasure::Volt_Amp);
            P148_data->writeStrings(parseString(string, 3), parseString(string, 4));
            success = true;
          } else if (equals(subcommand, F("energy"))) {
            // tm1621energy,<kWh>,<Watt>
            P148_data->setUnit(P148_data_struct::Tm1621UnitOfMeasure::kWh_Watt);
            P148_data->writeStrings(parseString(string, 3), parseString(string, 4));
            success = true;
          } else if (equals(subcommand, F("celcius"))) {
            // tm1621celcius,<temperture>
            P148_data->setUnit(P148_data_struct::Tm1621UnitOfMeasure::Celsius, true);
            P148_data->writeString(true, parseString(string, 3));
            success = true;
          } else if (equals(subcommand, F("fahrenheit"))) {
            // tm1621fahrenheit,<temperture>
            P148_data->setUnit(P148_data_struct::Tm1621UnitOfMeasure::Fahrenheit, true);
            P148_data->writeString(true, parseString(string, 3));
            success = true;
          } else if (equals(subcommand, F("humidity"))) {
            // tm1621humidity,<%humidity>
            P148_data->setUnit(P148_data_struct::Tm1621UnitOfMeasure::Humidity, false);
            P148_data->writeString(false, parseString(string, 3));
            success = true;
          }
        }
      }

      break;
    }
  }
  return success;
}

#endif // ifdef USES_P148
