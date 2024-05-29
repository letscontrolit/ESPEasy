#include "../Commands/OneWire.h"

#if FEATURE_DALLAS_HELPER && FEATURE_COMMAND_OWSCAN

# include "../Commands/Common.h"

# include "../Helpers/Dallas1WireHelper.h"
# include "../Helpers/Hardware_GPIO.h"
# include "../Helpers/StringConverter.h"

String Command_OneWire_Owscan(struct EventStruct *event,
                              const char         *Line) {
  int pinnr; bool input; bool output1; bool output; bool warning;

  if (getGpioInfo(event->Par1, pinnr, input, output1, warning) && input) {      // Input pin required
    if (!parseString(Line, 3).isEmpty()) {
      if (getGpioInfo(event->Par2, pinnr, input, output, warning) && !output) { // Output required if specified
        return return_command_failed();                                         // Argument error
      }
    } else {
      event->Par2 = event->Par1;                                                // RX and TX on same pin

      if (!output1) {                                                           // Single pin must be input & output capable
        return return_command_failed();                                         // Argument error
      }
    }
    pinMode(event->Par1, INPUT);

    if (event->Par2 != event->Par1) {
      pinMode(event->Par2, OUTPUT);
    }

    uint8_t addr[8]{};

    Dallas_reset(event->Par1, event->Par2);
    String res;
    res.reserve(80);

    while (Dallas_search(addr, event->Par1, event->Par2)) { // Scan the 1-wire
      res += Dallas_format_address(addr);
      res += '\n';
    }
    return res;
  }

  return return_command_failed();
}

#endif // if FEATURE_DALLAS_HELPER && FEATURE_COMMAND_OWSCAN
