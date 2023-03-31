#ifndef PLUGINSTRUCTS_P008_DATA_STRUCT_H
#define PLUGINSTRUCTS_P008_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P008

# define P008_DATA_BITS       PCONFIG(0)
# define P008_HEX_AS_DEC      PCONFIG(1)
# define P008_AUTO_REMOVE     PCONFIG(2)
# define P008_REMOVE_EVENT    PCONFIG(3)
# define P008_COMPATIBILITY   PCONFIG(4)
# define P008_REMOVE_VALUE    PCONFIG_LONG(0)
# define P008_REMOVE_TIMEOUT  PCONFIG_LONG(1)

# define P008_TIMEOUT_LIMIT   5 // Number of loops through plugin_one_per_second = 5 second time-out

struct P008_data_struct : public PluginTaskData_base {
public:

  P008_data_struct(struct EventStruct *event);

  P008_data_struct() = delete;
  virtual ~P008_data_struct();

  bool plugin_init(struct EventStruct *event);
  bool plugin_once_a_second(struct EventStruct *event);
  bool plugin_timer_in(struct EventStruct *event);
  bool plugin_get_config(struct EventStruct *event,
                         String            & string);

  volatile uint8_t  bitCount  = 0u;   // Count the number of bits received.
  volatile uint64_t keyBuffer = 0ull; // A 64-bit-long keyBuffer into which the number is stored.

private:

  uint64_t    castHexAsDec(uint64_t hexValue);

  static void Plugin_008_shift_bit_in_buffer(P008_data_struct *self,
                                             uint8_t           bit);
  static void Plugin_008_interrupt1(P008_data_struct *self);
  static void Plugin_008_interrupt2(P008_data_struct *self);

  uint8_t timeoutCount = 0u;
  bool    initialised  = false;
  bool    bufferValid  = false;
  uint8_t bufferBits   = 0u;

  int8_t _pin1 = -1;
  int8_t _pin2 = -1;
};

#endif // ifdef USES_P008
#endif // ifndef PLUGINSTRUCTS_P008_DATA_STRUCT_H
