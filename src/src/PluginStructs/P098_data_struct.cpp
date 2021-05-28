#include "../PluginStructs/P098_data_struct.h"

#ifdef USES_P098

# include "../ESPEasyCore/ESPEasyGPIO.h"

# define GPIO_PLUGIN_ID  1


const __FlashStringHelper * P098_config_struct::toString(P098_config_struct::PWM_mode_type PWM_mode) {
  switch (PWM_mode) {
    case P098_config_struct::PWM_mode_type::NoPWM:  return F("No PWM");
    case P098_config_struct::PWM_mode_type::PWM:    return F("PWM");

    case P098_config_struct::PWM_mode_type::MAX_TYPE: break;
  }

  return F("");
}

P098_data_struct::P098_data_struct(const P098_config_struct& config) : _config(config) {}

P098_data_struct::~P098_data_struct() {
  if (initialized) {
    detachInterrupt(digitalPinToInterrupt(_config.limitA.gpio));
    detachInterrupt(digitalPinToInterrupt(_config.limitB.gpio));
    detachInterrupt(digitalPinToInterrupt(_config.encoder.gpio));
  }
}

bool P098_data_struct::begin()
{
  if (!initialized) {
    initialized = true;

    stop();
    state = P098_data_struct::State::Idle;

    int interruptPinMode = 0;

    if (setPinMode(_config.limitA, interruptPinMode)) {
      attachInterruptArg(
        digitalPinToInterrupt(_config.limitA.gpio),
        reinterpret_cast<void (*)(void *)>(ISRlimitA),
        this, interruptPinMode);
    }

    if (setPinMode(_config.limitB, interruptPinMode)) {
      attachInterruptArg(
        digitalPinToInterrupt(_config.limitB.gpio),
        reinterpret_cast<void (*)(void *)>(ISRlimitB),
        this, interruptPinMode);
    }

    if (setPinMode(_config.encoder, interruptPinMode)) {
      attachInterruptArg(
        digitalPinToInterrupt(_config.encoder.gpio),
        reinterpret_cast<void (*)(void *)>(ISRencoder),
        this, CHANGE); // Act on 'CHANGE', not on rising/falling
    }
  }

  return true;
}

bool P098_data_struct::loop()
{
  const State old_state(state);

  switch (state) {
    case P098_data_struct::State::Idle:
      return true;
    case P098_data_struct::State::RunFwd:
    {
      release_limit_switch(_config.limitA, limitA, position);
      checkLimit(limitB);
      break;
    }
    case P098_data_struct::State::RunRev:
    {
      release_limit_switch(_config.limitB, limitB, position);
      checkLimit(limitA);
      break;
    }
    case P098_data_struct::State::StopLimitSw:
    case P098_data_struct::State::StopPosReached:
      // Still in a state that needs inspection
      return false;
  }

  // Must check when state has changed from running to some other state
  return old_state == state;
}

bool P098_data_struct::homePosSet() const
{
  return limitA.positionSet;
}

bool P098_data_struct::canRun()
{
  if (!homePosSet()) { return false; }

  switch (state) {
    case P098_data_struct::State::Idle:
    case P098_data_struct::State::RunFwd:
    case P098_data_struct::State::RunRev:
      return true;
    case P098_data_struct::State::StopLimitSw:
    case P098_data_struct::State::StopPosReached:
      // Still in a state that needs inspection
      return false;
  }
  return false;
}

void P098_data_struct::findHome()
{
  pos_dest = INT_MIN;
  startMoving();
}

void P098_data_struct::moveForward(int steps)
{
  if (steps <= 0) {
    pos_dest = INT_MAX;
  } else {
    pos_dest = position + steps;
  }
  startMoving();
}

void P098_data_struct::moveReverse(int steps)
{
  if (steps > 0) {
    pos_dest = position - steps;
    startMoving();
  }
}

bool P098_data_struct::moveToPos(int pos)
{
  if (!canRun()) { return false; }
  const int offset = pos - getPosition();

  pos_dest = position + offset;
  startMoving();
  return true;
}

void P098_data_struct::stop()
{
  setPinState(_config.motorFwd, 0);
  setPinState(_config.motorRev, 0);
}

int P098_data_struct::getPosition() const
{
//  if (!homePosSet()) { return -1; }
  return position - limitA.triggerpos;
}

void P098_data_struct::getLimitSwitchStates(bool& limitA_triggered, bool& limitB_triggered) const
{
  limitA_triggered = _config.limitA.readState();
  limitB_triggered = _config.limitB.readState();
}

void P098_data_struct::startMoving()
{
  // Stop first, to make sure both outputs will not be set high
  stop();

  if (pos_dest > position) {
    state = P098_data_struct::State::RunFwd;
    setPinState(_config.motorFwd, 1);
  } else {
    state = P098_data_struct::State::RunRev;
    setPinState(_config.motorRev, 1);
  }
}

void P098_data_struct::checkLimit(volatile P098_limit_switch_state& switch_state)
{
  if (switch_state.triggered) {
    stop();
    state = P098_data_struct::State::StopLimitSw;
    return;
  }
  checkPosition();
}

void P098_data_struct::checkPosition()
{
  bool mustStop = false;

  switch (state) {
    case P098_data_struct::State::RunFwd:

      mustStop = (position >= pos_dest);
      break;
    case P098_data_struct::State::RunRev:

      mustStop = (position <= pos_dest);
      break;
    default:
      return;
  }

  if (mustStop) {
    stop();
    state = P098_data_struct::State::StopPosReached;
  }
}

void P098_data_struct::setPinState(const P098_GPIO_config& gpio_config, byte state)
{
  GPIO_Write(
    GPIO_PLUGIN_ID,
    gpio_config.gpio,
    state == 0 ? gpio_config.low() : gpio_config.high(),
    PIN_MODE_OUTPUT);
}

bool P098_data_struct::setPinMode(const P098_GPIO_config& gpio_config, int& interruptPinMode)
{
  if (checkValidPortRange(GPIO_PLUGIN_ID, gpio_config.gpio)) {
    pinMode(gpio_config.gpio, gpio_config.pullUp ? INPUT_PULLUP : INPUT);
    interruptPinMode = gpio_config.inverted ? FALLING : RISING;
    return true;
  }
  return false;
}

void P098_data_struct::release_limit_switch(
  const P098_GPIO_config          & gpio_config,
  volatile P098_limit_switch_state& switch_state,
  int                               position)
{
  if (switch_state.triggered) {
    if (!gpio_config.readState()) {
      if (std::abs(switch_state.triggerpos - position) > P098_LIMIT_SWITCH_TRIGGERPOS_MARGIN) {
        switch_state.triggered = false;
      }
    }
  }
}

void ICACHE_RAM_ATTR P098_data_struct::ISRlimitA(P098_data_struct *self)
{
  if (!self->limitA.positionSet) {
    if (std::abs(self->limitA.triggerpos - self->position) > P098_LIMIT_SWITCH_TRIGGERPOS_MARGIN) {
      self->limitA.triggered  = true;
      self->limitA.positionSet = true;
      self->limitA.triggerpos = self->position;
    }
  }
}

void ICACHE_RAM_ATTR P098_data_struct::ISRlimitB(P098_data_struct *self)
{
  if (!self->limitB.positionSet) {
    if (std::abs(self->limitB.triggerpos - self->position) > P098_LIMIT_SWITCH_TRIGGERPOS_MARGIN) {
      self->limitB.triggered   = true;
      self->limitB.positionSet = true;
      self->limitB.triggerpos  = self->position;
    }
  }
}

void ICACHE_RAM_ATTR P098_data_struct::ISRencoder(P098_data_struct *self)
{
  switch (self->state) {
    case P098_data_struct::State::RunFwd:
      ++(self->position);
      break;
    case P098_data_struct::State::RunRev:
      --(self->position);
      break;
    default:
      break;
  }
}

#endif // ifdef USES_P098
