#ifndef PLUGINSTRUCTS_P035_DATA_STRUCT_H
#define PLUGINSTRUCTS_P035_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P035

# include "../../ESPEasy-Globals.h"

# include <ArduinoJson.h>
# include <IRremoteESP8266.h>
# include <IRac.h>
# include <IRutils.h>
# include <IRsend.h>

extern void enableIR_RX(boolean enable); // To be found in _P016_IR.ino

# define STATE_SIZE_MAX 53U
# define PRONTO_MIN_LENGTH 6U

# define from_32hex(c) ((((c) | ('A' ^ 'a')) - '0') % 39)

# define P35_Ntimings 250u // Defines the ammount of timings that can be stored. Used in RAW and RAW2 encodings

# define P035_DEBUG_LOG    // Enable for some (extra) logging

struct P035_data_struct : public PluginTaskData_base {
public:

  P035_data_struct(int8_t gpioPin);

  P035_data_struct() = delete;
  virtual ~P035_data_struct();

  bool plugin_init(struct EventStruct *event);
  bool plugin_exit(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);

private:

  IRsend *Plugin_035_irSender = nullptr;

  # ifdef P016_P035_Extended_AC

  IRac          *Plugin_035_commonAc = nullptr;
  stdAc::state_t st{}, prev{};

  bool   handle_AC_IRremote(const String& irData);
  String listACProtocols();
  # endif // ifdef P016_P035_Extended_AC

  bool   handleIRremote(const String& cmd);
  bool   handleRawRaw2Encoding(const String& cmd);
  void   printToLog(const String& protocol,
                    const String& data,
                    int           bits,
                    int           repeats);
  String listProtocols();
  bool   addErrorTrue();
  bool   sendIRCode(int const      irtype,
                    uint64_t const code,
                    char const    *code_str,
                    uint16_t       bits,
                    uint16_t       repeat);
  bool parseStringAndSendAirCon(const int    irtype,
                                const String str);

  int8_t _gpioPin;
};
#endif // ifdef USES_P035
#endif // ifndef PLUGINSTRUCTS_P035_DATA_STRUCT_H
