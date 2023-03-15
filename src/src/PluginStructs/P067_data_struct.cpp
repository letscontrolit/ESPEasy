#include "../PluginStructs/P067_data_struct.h"

#ifdef USES_P067
# include <GPIO_Direct_Access.h>

/****************************************************
* Convert a float to 2 ints
****************************************************/
void P067_float2int(float valFloat, int16_t *valInt0, int16_t *valInt1) {
  // FIXME TD-er: Casting from float* to integer* is not portable due to different binary data representations on different platforms.
  int16_t *fti = (int16_t *)&valFloat;

  *valInt0 = *fti++;
  *valInt1 = *fti;
}

/****************************************************
* Convert 2 ints to a float
****************************************************/
void P067_int2float(int16_t valInt0, int16_t valInt1, float *valFloat) {
  // FIXME TD-er: Casting from float* to integer* is not portable due to different binary data representations on different platforms.
  float offset = 0.0f; // Set to some value to prevent compiler warnings
  int16_t *itf = (int16_t *)&offset;

  *itf++    = valInt0;
  *itf      = valInt1;
  *valFloat = offset;
}

/**************************************************************************
* Constructor
**************************************************************************/
P067_data_struct::P067_data_struct(struct EventStruct *event,
                                   int8_t              pinSCL,
                                   int8_t              pinDOUT)
  : _pinSCL(pinSCL), _pinDOUT(pinDOUT)
{
  _modeChanA = P067_GET_CHANNEL_A_MODE_e;
  _modeChanB = P067_GET_CHANNEL_B_MODE_e;
  P067_int2float(P067_OFFSET_CHANNEL_A_1, P067_OFFSET_CHANNEL_A_2, &_offsetChanA);
  P067_int2float(P067_OFFSET_CHANNEL_B_1, P067_OFFSET_CHANNEL_B_2, &_offsetChanB);
}

/*****************************************************
* Destructor
*****************************************************/
P067_data_struct::~P067_data_struct() {}

/****************************************************
* Initialization
****************************************************/
bool P067_data_struct::init(struct EventStruct *event) {
  // Log anyway
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = concat(F("HX711: GPIO: SCL="), static_cast<int>(_pinSCL));
    log += concat(F(" DOUT="), static_cast<int>(_pinDOUT));
    addLogMove(LOG_LEVEL_INFO, log);
  }
  UserVar[event->BaseVarIndex]     = 0.0f; // Reset output
  UserVar[event->BaseVarIndex + 1] = 0.0f;

  if (isInitialized()) {
    pinMode(_pinSCL, OUTPUT); // Keep regular pinMode functions for initialization
    digitalWrite(_pinSCL, LOW);

    pinMode(_pinDOUT, INPUT); // Checked, doesn't seem applicable: https://github.com/bogde/HX711/issues/222

    return true;
  }
  return false;
}

/*****************************************************
* plugin_read
*****************************************************/
bool P067_data_struct::plugin_read(struct EventStruct *event)           {
  bool success = false;

  if (isInitialized()) {
    if ((_modeChanA == P067_ChannelA_State_e::modeAoff) && (_modeChanB == P067_ChannelB_State_e::modeBoff)) {
      addLog(LOG_LEVEL_INFO, F("HX711: No channel selected"));
    }

    // Channel A activated?
    if (_modeChanA != P067_ChannelA_State_e::modeAoff) {
      String log = concat(F("HX711: ("), (int)event->TaskIndex + 1);
      log += F(") ChanA: ");

      if (OversamplingCountChanA > 0) {
        UserVar[event->BaseVarIndex + 2] = static_cast<float>(OversamplingValueChanA) /
                                           OversamplingCountChanA;

        OversamplingValueChanA = 0;
        OversamplingCountChanA = 0;

        UserVar[event->BaseVarIndex] = UserVar[event->BaseVarIndex + 2] + _offsetChanA; // Offset

        log += formatUserVarNoCheck(event->TaskIndex, 0);

        if (P067_GET_CHANNEL_A_CALIB) { // Calibration channel A?
          int   adc1 = P067_CONFIG_CHANNEL_A_ADC1;
          int   adc2 = P067_CONFIG_CHANNEL_A_ADC2;
          float out1 = P067_CONFIG_CHANNEL_A_OUT1;
          float out2 = P067_CONFIG_CHANNEL_A_OUT2;

          if (adc1 != adc2) {
            const float normalized = static_cast<float>(UserVar[event->BaseVarIndex] - adc1) / static_cast<float>(adc2 - adc1);
            UserVar[event->BaseVarIndex] = normalized * (out2 - out1) + out1;

            log += F(" = ");
            log += formatUserVarNoCheck(event->TaskIndex, 0);
          }
        }
      } else {
        log += F("NO NEW VALUE");
      }
      addLogMove(LOG_LEVEL_INFO, log);
      success = !firstRead;
    }

    // Channel B activated?
    if (_modeChanB != P067_ChannelB_State_e::modeBoff) {
      String log = concat(F("HX711: ("), (int)event->TaskIndex + 1);
      log += F(") ChanB: ");

      if (OversamplingCountChanB > 0) {
        UserVar[event->BaseVarIndex + 3] = static_cast<float>(OversamplingValueChanB) /
                                           OversamplingCountChanB;

        OversamplingValueChanB = 0;
        OversamplingCountChanB = 0;

        UserVar[event->BaseVarIndex + 1] = UserVar[event->BaseVarIndex + 3] + _offsetChanB; // Offset

        log += formatUserVarNoCheck(event->TaskIndex, 1);

        if (P067_GET_CHANNEL_B_CALIB) { // Calibration channel B?
          int   adc1 = P067_CONFIG_CHANNEL_B_ADC1;
          int   adc2 = P067_CONFIG_CHANNEL_B_ADC2;
          float out1 = P067_CONFIG_CHANNEL_B_OUT1;
          float out2 = P067_CONFIG_CHANNEL_B_OUT2;

          if (adc1 != adc2) {
            const float normalized = (UserVar[event->BaseVarIndex + 1] - adc1) / static_cast<float>(adc2 - adc1);
            UserVar[event->BaseVarIndex + 1] = normalized * (out2 - out1) + out1;

            log += F(" = ");
            log += formatUserVarNoCheck(event->TaskIndex, 1);
          }
        }
      } else {
        log += F("NO NEW VALUE");
      }
      addLogMove(LOG_LEVEL_INFO, log);
      success = !firstRead;
    }
    firstRead = false;
  }
  return success;
}

/*****************************************************
* plugin_fifty_per_second
*****************************************************/
bool P067_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  bool success = false;

  if (isInitialized() && isDataReady()) {
    int32_t value = readHX711();
    success = true;

    switch (_channelRead) {
      case P067_Channel_e::chanA64:  //
      case P067_Channel_e::chanA128:
      {
        if (P067_GET_CHANNEL_A_OS) { // Oversampling on channel A?
          if (OversamplingCountChanA < 250) {
            OversamplingValueChanA += value;
            OversamplingCountChanA++;
          }
        } else {
          OversamplingValueChanA = value;
          OversamplingCountChanA = 1;
        }
        break;
      }
      case P067_Channel_e::chanB32:
      {
        if (P067_GET_CHANNEL_B_OS) { // Oversampling on channel B?
          if (OversamplingCountChanB < 250) {
            OversamplingValueChanB += value;
            OversamplingCountChanB++;
          }
        } else {
          OversamplingValueChanB = value;
          OversamplingCountChanB = 1;
        }
        break;
      }
    }
  }

  return success;
}

/*****************************************************
* plugin_write
*****************************************************/
bool P067_data_struct::plugin_write(struct EventStruct *event,
                                    String            & string) {
  bool success = false;

  String command = parseString(string, 1);

  if (equals(command, F("tarechana"))) {
    P067_float2int(-UserVar[event->BaseVarIndex + 2], &P067_OFFSET_CHANNEL_A_1, &P067_OFFSET_CHANNEL_A_2);
    P067_int2float(P067_OFFSET_CHANNEL_A_1, P067_OFFSET_CHANNEL_A_2, &_offsetChanA);
    OversamplingValueChanA = 0;
    OversamplingCountChanA = 0;

    addLog(LOG_LEVEL_INFO, F("HX711: tare channel A"));
    success = true;
  } else if (equals(command, F("tarechanb"))) {
    P067_float2int(-UserVar[event->BaseVarIndex + 3], &P067_OFFSET_CHANNEL_B_1, &P067_OFFSET_CHANNEL_B_2);
    P067_int2float(P067_OFFSET_CHANNEL_B_1, P067_OFFSET_CHANNEL_B_2, &_offsetChanB);
    OversamplingValueChanB = 0;
    OversamplingCountChanB = 0;

    addLog(LOG_LEVEL_INFO, F("HX711: tare channel B"));
    success = true;
  }

  return success;
}

/****************************************************************************************
* Private stuff
****************************************************************************************/
/****************************************************
* Minimal viable settings: GPIO pins valid?
****************************************************/
bool P067_data_struct::isInitialized() {
  return validGpio(_pinSCL) && validGpio(_pinDOUT);
}

/****************************************************
* New data available?
****************************************************/
bool P067_data_struct::isDataReady() {
  if (isInitialized()) {
    return !DIRECT_pinRead(_pinDOUT);
  }
  return false;
}

/****************************************************
* Read data from the load sensor
****************************************************/
int32_t P067_data_struct::readHX711() {
  int32_t  value = 0;
  uint32_t mask  = 0x00800000;

  _channelRead = _nextChannel;

  // Both channels off
  if ((_modeChanA == P067_ChannelA_State_e::modeAoff) && (_modeChanB == P067_ChannelB_State_e::modeBoff)) {
    DIRECT_pinWrite(_pinSCL, HIGH);
    return 0;
  }

  // Both channels on
  if ((_modeChanA != P067_ChannelA_State_e::modeAoff) && (_modeChanB != P067_ChannelB_State_e::modeBoff)) {
    // Both channels are activated -> do interleaved measurement
    _channelToggle = !_channelToggle;

    // FIXME tonhuisman: Toggling doesn't work as intended
    if (_channelToggle) {
      if (_modeChanA == P067_ChannelA_State_e::modeA64) {
        _nextChannel = P067_Channel_e::chanA64;
      } else {
        _nextChannel = P067_Channel_e::chanA128;
      }
    } else {
      _nextChannel = P067_Channel_e::chanB32;
    }
  } else {
    // Only one channel is activated
    if (_modeChanA == P067_ChannelA_State_e::modeA64) {
      _nextChannel = P067_Channel_e::chanA64;
    } else if (_modeChanA == P067_ChannelA_State_e::modeA128) {
      _nextChannel = P067_Channel_e::chanA128;
    }

    if (_modeChanB == P067_ChannelB_State_e::modeB32) {
      _nextChannel = P067_Channel_e::chanB32;
    }
  }

  for (uint8_t i = 0; i < 24; i++) {
    DIRECT_pinWrite(_pinSCL, HIGH);
    delayMicroseconds(1);
    DIRECT_pinWrite(_pinSCL, LOW);

    if (DIRECT_pinRead(_pinDOUT)) {
      value |= mask;
    }
    delayMicroseconds(1);
    mask >>= 1;
  }

  for (uint8_t i = 0; i < (static_cast < uint8_t > (_nextChannel) + 1); i++) {
    DIRECT_pinWrite(_pinSCL, HIGH);
    delayMicroseconds(1);
    DIRECT_pinWrite(_pinSCL, LOW);
    delayMicroseconds(1);
  }

  if (value & 0x00800000) { // negative?
    value |= 0xFF000000;    // expand sign bit to 32 bit
  }
  return value;
}

#endif // ifdef USES_P067
