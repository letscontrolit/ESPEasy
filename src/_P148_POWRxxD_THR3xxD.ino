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
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].TimerOptional      = false;
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

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Display"));

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
        success = P148_data->init();
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P148_data_struct *P148_data =
        static_cast<P148_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P148_data) {
        const String command = parseString(string, 1);

        if (command.startsWith(F("tm1621"))) {
          if (command.equals(F("tm1621raw"))) {
            // Write raw data to the display
            // Typical use case: testing fonts
            const String rawdata_str = parseString(string, 2);
            uint64_t     rawdata;

            if (validUInt64FromString(rawdata_str, rawdata)) {
              success = true;
              P148_data->TM1621WritePixelBuffer(rawdata);
            }
          } else if (command.equals(F("tm1621write"))) {
            // tm1621write,<rownr>,<string>
            const String str = parseString(string, 3);
            P148_data->TM1621WriteString(event->Par1 <= 1, str);
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
