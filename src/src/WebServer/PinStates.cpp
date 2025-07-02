#include "../WebServer/PinStates.h"

#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Forms.h"

#include "../DataStructs/PinMode.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Helpers/PortStatus.h"

#ifdef ESP32
# include <esp32-hal-periman.h>
# include <driver/gpio.h>
# include <esp_private/esp_gpio_reserve.h>

// # include <hal/gpio_types.h>
 # include <hal/gpio_hal.h>

// #include <hal/gpio_ll.h>

# include <esp_private/io_mux.h>
#endif // ifdef ESP32

#ifdef WEBSERVER_NEW_UI


// ********************************************************************************
// Web Interface pin state list
// ********************************************************************************
void handle_pinstates_json() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_pinstates"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream();

  bool first = true;
  addHtml('[');

  for (auto it = globalMapPortStatus.begin(); it != globalMapPortStatus.end(); ++it)
  {
    if (!first) {
      addHtml(',');
    } else {
      first = false;
    }
    addHtml('{');


    const uint16_t plugin = getPluginFromKey(it->first);
    const uint16_t port   = getPortFromKey(it->first);

    stream_next_json_object_value(F("plugin"),  plugin);
    stream_next_json_object_value(F("port"),    port);
    stream_next_json_object_value(F("state"),   it->second.state);
    stream_next_json_object_value(F("task"),    it->second.task);
    stream_next_json_object_value(F("monitor"), it->second.monitor);
    stream_next_json_object_value(F("command"), it->second.command);
    stream_last_json_object_value(F("init"), it->second.init);
  }

  addHtml(']');

  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_PINSTATES

void handle_pinstates() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_pinstates"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  # ifdef ESP32
  html_BR();
  addFormHeader(F("Pin states"));
  # endif // ifdef ESP32

  html_table_class_multirow();
  html_TR();
  html_table_header(F("Plugin"), F("RTDPlugin/_Plugin.html#list-of-official-plugins"), 0);
  html_table_header(F("GPIO"));
  html_table_header(F("Mode"));
  html_table_header(F("Value/State"));
  html_table_header(F("Task"));
  html_table_header(F("Monitor"));
  html_table_header(F("Command"));
  html_table_header(F("Init"));

  for (auto it = globalMapPortStatus.begin(); it != globalMapPortStatus.end(); ++it)
  {
    html_TR_TD();
    const pluginID_t plugin = getPluginFromKey(it->first);
    const uint16_t   port   = getPortFromKey(it->first);
    addHtml(plugin.toDisplayString());
    html_TD();
    addHtmlInt(port);
    html_TD();
    addHtml(getPinModeString(it->second.mode));
    html_TD();
    addHtmlInt(it->second.getValue());
    html_TD();
    addHtmlInt(it->second.task);
    html_TD();
    addHtmlInt(it->second.monitor);
    html_TD();
    addHtmlInt(it->second.command);
    html_TD();
    addHtmlInt(it->second.init);
  }

  html_end_table();

# ifdef ESP32

  // Collect which pin should be looked into more in-depth.
  uint64_t io_bit_mask{};

  html_BR();
  addFormHeader(F("Peripheral Bus Type per GPIO"));
  html_table_class_multirow();
  html_TR();
#  if defined(BOARD_HAS_PIN_REMAP)
  html_table_header(F("Dnnn|GPIO"));
#  else // if defined(BOARD_HAS_PIN_REMAP)
  html_table_header(F("GPIO"));
#  endif // if defined(BOARD_HAS_PIN_REMAP)
  html_table_header(F("Bus Type"));
  html_table_header(F("Bus Num"));
  html_table_header(F("Bus Channel"));

  for (int i = 0; i <= MAX_GPIO; ++i) {
    if (!perimanPinIsValid(i)) {
      continue; // invalid pin
    }
    peripheral_bus_type_t type = perimanGetPinBusType(i);

    if (type == ESP32_BUS_TYPE_INIT) {
      continue; // unused pin
    }


#  if defined(BOARD_HAS_PIN_REMAP)
    int dpin = gpioNumberToDigitalPin(i);

    if (dpin < 0) {
      continue; // pin is not exported
    } else {
      html_TR_TD();
      addHtml(strformat(F("D%-3d|%4u"), dpin, i));
    }
#  else // if defined(BOARD_HAS_PIN_REMAP)
    html_TR_TD();
    addHtml(strformat(F("%4u"), i));
#  endif // if defined(BOARD_HAS_PIN_REMAP)
    const char *extra_type = perimanGetPinBusExtraType(i);
    html_TD();

    if (extra_type) {
      addHtml(strformat(F("%s [%s]"), perimanGetTypeName(type), extra_type));
    } else {
      addHtml(strformat(F("%s"), perimanGetTypeName(type)));
    }
    int8_t bus_number = perimanGetPinBusNum(i);
    html_TD();

    if (bus_number != -1) {
      addHtmlInt(bus_number);
    }
    int8_t bus_channel = perimanGetPinBusChannel(i);
    html_TD();

    if (bus_channel != -1) {
      addHtmlInt(bus_channel);
    }
    io_bit_mask |= BIT64(i);
  }

  html_end_table();
  #  if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)

  if (io_bit_mask) {
    html_BR();

    addFormHeader(F("GPIO IO configuration"));
    html_table_class_multirow();
    html_TR();
    html_table_header(F("GPIO"));
    html_table_header(F("PullUp"));
    html_table_header(F("PullDown"));
    html_table_header(F("DriveCap"));
    html_table_header(F("InputEn"));
    html_table_header(F("OutputEn"));
    html_table_header(F("OpenDrain"));
    html_table_header(F("FuncSel"));
    html_table_header(F("MUX ID"),
                      F("https://github.com/espressif/esp-idf/blob/master/components/soc/"
                        ESP32XX
                        "/include/soc/gpio_sig_map.h"),
                      0);
    html_table_header(F("SleepSelEn"));


    bool reservedPinFound = false;
    bool simpleGPIO_found = false;

    for (int gpio_num = 0; gpio_num <= MAX_GPIO; ++gpio_num) {
      if ((io_bit_mask & BIT64(gpio_num)) == 0) {
        continue; // skip pin
      }

      html_TR_TD();
      addHtmlInt(gpio_num);

      if (esp_gpio_is_reserved(BIT64(gpio_num))) {
        reservedPinFound = true;
        addHtml(F(" (*)"));
      }

      gpio_io_config_t io_config = {};
      gpio_get_io_config((gpio_num_t)gpio_num, &io_config);

      // PullUp
      html_TD();
      addEnabled(io_config.pu);

      // PullDown
      html_TD();
      addEnabled(io_config.pd);

      // DriveCap
      html_TD();
      addHtmlInt((uint32_t)io_config.drv);

      // InputEn
      html_TD();
      addEnabled(io_config.ie);

      // OutputEn
      html_TD();
      // When the IO is used as a simple GPIO output, oe signal can only be controlled by the oe register
      // When the IO is not used as a simple GPIO output, oe signal could be controlled by the peripheral
      if ((io_config.sig_out != SIG_GPIO_OUT_IDX) && io_config.oe_ctrl_by_periph) {
        addHtml(F("[periph_sig_ctrl]"));
      } else {
        addEnabled(io_config.oe);
      }
      if ((io_config.fun_sel == PIN_FUNC_GPIO) && (io_config.oe_inv)) {
        addHtml(F(" (inversed)"));
      }

      // OpenDrain
      html_TD();
      addEnabled(io_config.od);

      // FuncSel
      html_TD();
      addHtmlInt(io_config.fun_sel);
      addHtml(strformat(F(" (%s)"), (io_config.fun_sel == PIN_FUNC_GPIO) ? "GPIO" : "IOMUX"));


      // MUX ID
      html_TD();

      if (io_config.fun_sel == PIN_FUNC_GPIO) {
        addHtml(F("Out: "));

        if (io_config.sig_out == SIG_GPIO_OUT_IDX) {
          addHtml(F("(**)")); // simple GPIO output
          simpleGPIO_found = true;
        } else {
          addHtmlInt(io_config.sig_out);
        }
        html_BR();
      }

      if (io_config.ie && (io_config.fun_sel == PIN_FUNC_GPIO)) {
        uint32_t cnt = 0;
        addHtml(F("In: "));

        for (int i = 0; i < SIG_GPIO_OUT_IDX; i++) {
          if (gpio_ll_get_in_signal_connected_io(GPIO_HAL_GET_HW(GPIO_PORT_0), i) == gpio_num) {
            cnt++;
            addHtml(' ');
            addHtmlInt(i);
          }
        }

        if (cnt == 0) {
          addHtml(F("(**)")); // simple GPIO input
          simpleGPIO_found = true;
        }
      }

      // SleepSelEn
      html_TD();
      addHtmlInt(io_config.slp_sel);
    }
    html_end_table();


    if (reservedPinFound) {
      html_BR();
      addHtml(F("(*) GPIO is reserved, or in use by some peripheral"));
    }

    if (simpleGPIO_found) {
      html_BR();
      addHtml(F("(**) Simple GPIO input/output"));
    }
  }

#  endif // if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)

# endif // ifdef ESP32
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // ifdef WEBSERVER_PINSTATES
