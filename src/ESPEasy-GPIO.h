#ifndef ESPEASY_GPIO_H
#define ESPEASY_GPIO_H

#define PLUGIN_ID_000    0

// portStatusStruct.mode (max. 8)
#define PIN_MODE_UNDEFINED                  0
#define PIN_MODE_INPUT                      1
#define PIN_MODE_OUTPUT                     2
#define PIN_MODE_PWM                        3
#define PIN_MODE_SERVO                      4
#define PIN_MODE_INPUT_PULLUP               5
#define PIN_MODE_OFFLINE                    6


#define SEARCH_PIN_STATE                 true
#define NO_SEARCH_PIN_STATE             false


// See description/discussion:
// https://github.com/letscontrolit/ESPEasy/pull/2057#issuecomment-445310454
struct portStatusStruct {
  portStatusStruct() : state(-1), output(-1), command(0), portstatus_init(0), mode(0), task(0), monitor(0), previousTask(-1)
    {}

  int8_t state   : 2;  // -1,0,1
  int8_t output  : 2;  // -1,0,1
  int8_t command : 2;  // 0,1  true when a command (GPIO,PCFGPIO,MCPGPIO,TOGGLEGPIO, etc.) has referenced that gpio/port.
  int8_t portstatus_init    : 2;  // true when the gpio/port has been set during boot in the hardware page

  uint8_t mode    : 3; // 7 current values (max. 8)
  uint8_t task    : 4; // 0-15 (max. 16)  If task = 0 it means that no task are referencing that pin.
  uint8_t monitor : 1; // 0,1  true means the gpio/port should send an event when it changes

  int8_t previousTask;

  bool readyToDelete() const {
    return ((task <= 0) && (monitor <= 0) && (command <= 0));
  }

  bool mustPollGpioState() const {
    return (monitor != 0) || (command != 0) || (portstatus_init != 0);
  }

  bool portStateError() const {
    return state != 0 && state != 1;
  }

  bool getPinState() const {
    return state == 1;
  }

  void updatePinState(int8_t newState) {
    state = newState;
  }
};

std::map < uint32_t, portStatusStruct > globalMapPortStatus;

// Forward declarations
uint32_t createInternalGpioKey(uint16_t portNumber);
bool     read_GPIO_state(struct EventStruct *event);
bool     read_GPIO_state(byte gpio_pin,
                         byte pinMode);
bool     checkValidGpioPin(byte gpio_pin);
bool     getGpioInfo(int   gpio_pin,
                     int & nodemcu_pinnr,
                     bool& input,
                     bool& output,
                     bool& warning);


/**********************************************************
*                                                         *
* Helper Functions for managing the status data structure *
*                                                         *
**********************************************************/
void savePortStatus(uint32_t key, struct portStatusStruct& tempStatus) {
  if (tempStatus.readyToDelete()) {
    globalMapPortStatus.erase(key);
  }
  else {
    globalMapPortStatus[key] = tempStatus;
  }
}

bool existPortStatus(uint32_t key) {
  return globalMapPortStatus.find(key) != globalMapPortStatus.end();
}

void removeTaskFromPort(uint32_t key) {
  if (existPortStatus(key)) {
    portStatusStruct& portstatus = globalMapPortStatus[key];

    if (portstatus.task > 0) {
      --portstatus.task;
    }

    if (portstatus.readyToDelete() && (portstatus.portstatus_init <= 0)) {
      globalMapPortStatus.erase(key);
    }
  }
}

void removeMonitorFromPort(uint32_t key) {
  if (existPortStatus(key)) {
    portStatusStruct& portstatus = globalMapPortStatus[key];
    portstatus.monitor = 0;

    if (portstatus.readyToDelete() && (portstatus.portstatus_init <= 0)) {
      globalMapPortStatus.erase(key);
    }
  }
}

void addMonitorToPort(uint32_t key) {
  globalMapPortStatus[key].monitor = 1;
}

uint32_t createKey(uint16_t pluginNumber, uint16_t portNumber) {
  return (uint32_t)pluginNumber << 16 | portNumber;
}

uint16_t getPluginFromKey(uint32_t key) {
  return (uint16_t)(key >> 16);
}

uint16_t getPortFromKey(uint32_t key) {
  return (uint16_t)(key);
}

uint32_t createInternalGpioKey(uint16_t portNumber) {
  return createKey(PLUGIN_ID_000, portNumber);
}

// return true when pin can be used.
bool checkValidGpioPin(byte gpio_pin) {
  int  nodemcu_pinnr = -1;
  bool input, output, warning;

  return getGpioInfo(gpio_pin, nodemcu_pinnr, input, output, warning);
}

bool read_GPIO_state(struct EventStruct *event) {
  byte gpio_pin      = Settings.TaskDevicePin1[event->TaskIndex];
  const uint32_t key = createInternalGpioKey(gpio_pin);

  if (existPortStatus(key)) {
    return read_GPIO_state(gpio_pin, globalMapPortStatus[key].mode);
  }
  return false;
}

bool read_GPIO_state(byte gpio_pin, byte pinMode) {
  bool canRead = false;

  switch (pinMode)
  {
  case PIN_MODE_UNDEFINED:
  case PIN_MODE_INPUT:
  case PIN_MODE_INPUT_PULLUP:
  case PIN_MODE_OUTPUT:
    canRead = true;
    break;
  case PIN_MODE_PWM:
    break;
  case PIN_MODE_SERVO:
    break;
  case PIN_MODE_OFFLINE:
    break;
  default:
    break;
  }

  if (!canRead) { return false; }

  // Do not read from the pin while mode is set to PWM or servo.
  // See https://github.com/letscontrolit/ESPEasy/issues/2117#issuecomment-443516794
  const auto pinstate = digitalRead(gpio_pin);
//  const uint32_t key  = createInternalGpioKey(gpio_pin);
//  globalMapPortStatus[key].state = pinstate;
  return pinstate == HIGH;
}

#endif // ESPEASY_GPIO_H
