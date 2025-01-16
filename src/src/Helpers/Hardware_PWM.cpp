#include "../Helpers/Hardware_PWM.h"

#include "../ESPEasyCore/ESPEasyGPIO.h"

#include "../Helpers/PortStatus.h"
#include "../Helpers/StringConverter.h"

#include "../Globals/GlobalMapPortStatus.h"


#if defined(ESP32)
# if ESP_IDF_VERSION_MAJOR < 5
#  include <soc/soc_caps.h>

// https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-ledc.c
#  ifndef LEDC_CHANNELS
#   ifdef SOC_LEDC_SUPPORT_HS_MODE
#    define LEDC_CHANNELS           (SOC_LEDC_CHANNEL_NUM << 1)
#   else // ifdef SOC_LEDC_SUPPORT_HS_MODE
#    define LEDC_CHANNELS           (SOC_LEDC_CHANNEL_NUM)
#   endif // ifdef SOC_LEDC_SUPPORT_HS_MODE
#  endif // ifndef LEDC_CHANNELS
int8_t   ledChannelPin[LEDC_CHANNELS];
uint32_t ledChannelFreq[LEDC_CHANNELS] = { 0 };
# endif // if ESP_IDF_VERSION_MAJOR < 5
#endif  // if defined(ESP32)


// ********************************************************************************
// Manage PWM state of GPIO pins.
// ********************************************************************************
void initAnalogWrite()
{
  #if defined(ESP32)
  # if ESP_IDF_VERSION_MAJOR < 5
  constexpr unsigned nrLedChannelPins = NR_ELEMENTS(ledChannelPin);

  for (uint8_t x = 0; x < nrLedChannelPins; x++) {
    ledChannelPin[x]  = -1;
    ledChannelFreq[x] = ledcSetup(x, 1000, 10); // Clear the channel
  }
  # endif // if ESP_IDF_VERSION_MAJOR < 5
  #endif // if defined(ESP32)
  #ifdef ESP8266

  // See https://github.com/esp8266/Arduino/commit/a67986915512c5304bd7c161cf0d9c65f66e0892
  analogWriteRange(1023);
  #endif // ifdef ESP8266
}

#if defined(ESP32)

// Match frequency and resolution and optionally adjust duty accordingly
// @return adjusted duty cycle
uint32_t adapt_ledc_frequency_resolution_duty(uint32_t& frequency, uint8_t& resolution, uint32_t duty = 0)
{
  while ((40000000u >> resolution) < frequency && resolution > 1) {
    --resolution;
    duty >>= 1;
  }
  return std::min(duty, static_cast<uint32_t>((1 << resolution) - 1));
}

# if ESP_IDF_VERSION_MAJOR < 5

int8_t attachLedChannel(int pin, uint32_t frequency, uint8_t resolution)
{
  static bool initialized = false;

  constexpr unsigned nrLedChannelPins = NR_ELEMENTS(ledChannelPin);

  if (!initialized) {
    for (uint8_t x = 0; x < nrLedChannelPins; x++) {
      ledChannelPin[x]  = -1;
      ledChannelFreq[x] = 0;
    }
    initialized = true;
  }


  // find existing channel if this pin has been used before
  int8_t ledChannel = -1;
  bool   mustSetup  = false;

  for (uint8_t x = 0; x < nrLedChannelPins; x++) {
    if (ledChannelPin[x] == pin) {
      ledChannel = x;
    }
  }

  if (ledChannel == -1)                                                  // no channel set for this pin
  {
    for (uint8_t x = 0; x < nrLedChannelPins && ledChannel == -1; ++x) { // find free channel
      if (ledChannelPin[x] == -1)
      {
        if (static_cast<uint32_t>(ledcReadFreq(x)) == ledChannelFreq[x]) {
          // Channel is not used by some other piece of code.
          ledChannel = x;
          mustSetup  = true;
          break;
        }
      }
    }
  }

  if (ledChannel == -1) { return ledChannel; }

  if (frequency != 0) {
    if (ledChannelFreq[ledChannel] != frequency)
    {
      // Frequency is given and has changed
      mustSetup = true;
    }
    ledChannelFreq[ledChannel] = frequency;
  } else if (ledChannelFreq[ledChannel] == 0) {
    mustSetup = true;

    // Set some default frequency
    ledChannelFreq[ledChannel] = 1000;
  }

  if (mustSetup) {
    // setup channel to resolution nr of bits and set frequency.
    ledChannelFreq[ledChannel] = ledcSetup(ledChannel, ledChannelFreq[ledChannel], 10);
    ledChannelPin[ledChannel]  = pin; // store pin nr
    ledcAttachPin(pin, ledChannel);   // attach to this pin
    //    pinMode(pin, OUTPUT);
  }

  return ledChannel;
}

void detachLedChannel(int pin)
{
  int8_t ledChannel = -1;

  constexpr unsigned nrLedChannelPins = NR_ELEMENTS(ledChannelPin);

  for (uint8_t x = 0; x < nrLedChannelPins; x++) {
    if (ledChannelPin[x] == pin) {
      ledChannel = x;
    }
  }

  if (ledChannel != -1) {
    ledcWrite(ledChannel, 0);
    ledcDetachPin(pin);
    ledChannelPin[ledChannel]  = -1;
    ledChannelFreq[ledChannel] = 0;
  }
}

# else // if ESP_IDF_VERSION_MAJOR < 5

// ESP_IDF >= 5.x finally manages the channels in the SDK

int8_t attachLedChannel(int pin, uint32_t frequency, uint8_t resolution)
{
  if (frequency == 0) {
    frequency = ESPEASY_PWM_DEFAULT_FREQUENCY;
  }
  ledcDetach(pin);  // See: https://github.com/espressif/arduino-esp32/issues/9212
  return ledcAttach(pin, frequency, resolution) ? 0 : -1;
}

void detachLedChannel(int pin)
{
  ledcDetach(pin);
}

# endif // if ESP_IDF_VERSION_MAJOR < 5

uint32_t analogWriteESP32(int pin, int value, uint32_t frequency)
{
  if (value == 0) {
    detachLedChannel(pin);
    return 0;
  }

  // find existing channel if this pin has been used before
  uint8_t resolution = 10;

  value = adapt_ledc_frequency_resolution_duty(frequency, resolution, value);
  int8_t ledChannel = attachLedChannel(pin, frequency, resolution);

  if (ledChannel != -1) {
    # if ESP_IDF_VERSION_MAJOR < 5
    ledcWrite(ledChannel, value);
    return ledChannelFreq[ledChannel];
    # else // if ESP_IDF_VERSION_MAJOR < 5
    ledcWrite(pin,        value);
    return ledcReadFreq(pin);
    # endif // if ESP_IDF_VERSION_MAJOR < 5
  }
  return 0;
}

#endif // if defined(ESP32)

bool set_Gpio_PWM_pct(int gpio, float dutyCycle_f, uint32_t frequency) {
  uint32_t dutyCycle = dutyCycle_f * 10.23f;

  return set_Gpio_PWM(gpio, dutyCycle, frequency);
}

bool set_Gpio_PWM(int gpio, uint32_t dutyCycle, uint32_t frequency) {
  uint32_t key;

  return set_Gpio_PWM(gpio, dutyCycle, 0, frequency, key);
}

bool set_Gpio_PWM(int gpio, uint32_t dutyCycle, uint32_t fadeDuration_ms, uint32_t& frequency, uint32_t& key)
{
  // For now, we only support the internal GPIO pins.
  if (!checkValidPortRange(PLUGIN_GPIO, gpio)) {
    return false;
  }
  portStatusStruct tempStatus;
  if (frequency == 0) frequency = 1000;

  // FIXME TD-er: PWM values cannot be stored very well in the portStatusStruct.
  key = createKey(PLUGIN_GPIO, gpio);

  // WARNING: operator [] creates an entry in the map if key does not exist
  // So the next command should be part of each command:
  tempStatus = globalMapPortStatus[key];

#if defined(ESP8266)
  pinMode(gpio, OUTPUT);

  if ((frequency > 0) && (frequency <= 40000)) {
    analogWriteFreq(frequency);
  }
#endif // if defined(ESP8266)

#if ESP_IDF_VERSION_MAJOR >= 5

  if (fadeDuration_ms == 0)
  {
    analogWriteESP32(gpio, dutyCycle, frequency);
  } else {
    uint8_t  resolution  = 10;
    uint32_t start_duty  = 0;
    uint32_t target_duty = dutyCycle;

    if (/*(tempStatus.mode == PIN_MODE_PWM) && */ (ledcReadFreq(gpio) != 0)) {
      start_duty = ledcRead(gpio);
    } else {
      // No PWM active, pick proper estimated start_duty based on pin state
      if (digitalRead(gpio)) {
        start_duty = (1 << resolution);
      }
    }

    if (frequency > 0) {
      // Adjust the frequency and resolution to keep them in the range of the used timer frequency.
      while ((40000000u >> resolution) < frequency && resolution > 7) {
        --resolution;

        if (start_duty > target_duty) {
          ++start_duty;
        }
        else {
          ++target_duty;
        }
        start_duty  >>= 1;
        target_duty >>= 1;
      }
    }
    start_duty  = std::min(start_duty, static_cast<uint32_t>((1 << resolution) - 1));
    target_duty = std::min(target_duty, static_cast<uint32_t>((1 << resolution) - 1));

    //    ledcDetach(gpio);

    if (!ledcWrite(gpio, start_duty)) {
      // Pin not yet attached
      if (!ledcAttach(gpio, frequency, resolution)) {
        addLog(LOG_LEVEL_ERROR, strformat(
          F("PWM : ledcAttach failed  gpio:%d freq:%d res:%d"),
          gpio, frequency, resolution));
        return false;
      }
    }

    if (!ledcFade(gpio, start_duty, target_duty, fadeDuration_ms)) {
      addLog(LOG_LEVEL_ERROR, F("PWM : ledcFade failed"));
      return false;
    }
  }

#else // if ESP_IDF_VERSION_MAJOR < 5

  if (fadeDuration_ms == 0)
  {
# if defined(ESP8266)
    analogWrite(gpio, dutyCycle);
# endif // if defined(ESP8266)
# if defined(ESP32)
    frequency = analogWriteESP32(gpio, dutyCycle, frequency);
# endif // if defined(ESP32)
  } else {
    const int32_t resolution_factor = (1 << 12);
    const uint8_t prev_mode         = tempStatus.mode;
    int32_t prev_value              = tempStatus.getDutyCycle();

    // Check to see if pin was already set to PWM.
    // If not, then set previous duty cycle close to logic pin state.
    if (prev_mode != PIN_MODE_PWM) {
      prev_value = (tempStatus.getValue() == 0) ? 0 : 1023;
    }

    const int32_t step_value = ((static_cast<int32_t>(dutyCycle) - prev_value) * resolution_factor) / static_cast<int32_t>(fadeDuration_ms);
    int32_t curr_value       = prev_value * resolution_factor;

    int i = fadeDuration_ms;

    while (i--) {
      curr_value += step_value;
      const int16_t new_value = curr_value / resolution_factor;
            # if defined(ESP8266)
      analogWrite(gpio, new_value);
            # endif // if defined(ESP8266)
            # if defined(ESP32)
      frequency = analogWriteESP32(gpio, new_value, frequency);
            # endif // if defined(ESP32)
      delay(1);
    }
  }

#endif // if ESP_IDF_VERSION_MAJOR >= 5

  // setPinState(pluginID, gpio, PIN_MODE_PWM, dutyCycle);
  tempStatus.mode      = PIN_MODE_PWM;
  tempStatus.dutyCycle = dutyCycle;
  tempStatus.command   = 1; // set to 1 in order to display the status in the PinStatus page

  savePortStatus(key, tempStatus);
  return true;
}
