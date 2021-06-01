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

    if (setPinMode(_config.limitA)) {
      attachInterruptArg(
        digitalPinToInterrupt(_config.limitA.gpio),
        reinterpret_cast<void (*)(void *)>(ISRlimitA),
        this, CHANGE);
    }

    if (setPinMode(_config.limitB)) {
      attachInterruptArg(
        digitalPinToInterrupt(_config.limitB.gpio),
        reinterpret_cast<void (*)(void *)>(ISRlimitB),
        this, CHANGE);
    }

    if (setPinMode(_config.encoder)) {
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

  check_limit_switch(_config.limitA, limitA);
  check_limit_switch(_config.limitB, limitB);

  switch (state) {
    case P098_data_struct::State::Idle:
      return true;
    case P098_data_struct::State::RunFwd:
    {
      checkLimit(limitB);
      break;
    }
    case P098_data_struct::State::RunRev:
    {
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
  return limitA.switchposSet;
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
  pos_dest            = INT_MIN;
  limitA.switchposSet = false;
  startMoving();
}

void P098_data_struct::moveForward(int steps)
{
  if (steps <= 0) {
    pos_dest            = INT_MAX;
    limitB.switchposSet = false;
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
  if (switch_state.state == P098_limit_switch_state::State::High) {
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

bool P098_data_struct::setPinMode(const P098_GPIO_config& gpio_config)
{
  if (checkValidPortRange(GPIO_PLUGIN_ID, gpio_config.gpio)) {
    pinMode(gpio_config.gpio, gpio_config.pullUp ? INPUT_PULLUP : INPUT);
    return true;
  }
  return false;
}

void P098_data_struct::check_limit_switch(
  const P098_GPIO_config          & gpio_config,
  volatile P098_limit_switch_state& switch_state)
{
  if (switch_state.state == P098_limit_switch_state::State::TriggerWaitBounce) {
    if ((switch_state.lastChanged != 0) && (timePassedSince(switch_state.lastChanged) > gpio_config.debounceTime)) {
      if (gpio_config.readState()) {
        switch_state.state = P098_limit_switch_state::State::High;

        if (!switch_state.switchposSet) {
          switch_state.switchpos    = switch_state.triggerpos;
          switch_state.switchposSet = true;
        }
      }
    }
  }
}

void ICACHE_RAM_ATTR P098_data_struct::process_limit_switch(
  const P098_GPIO_config          & gpio_config,
  volatile P098_limit_switch_state& switch_state,
  int                               position)
{
  const bool pinState                           = gpio_config.readState();
  const P098_limit_switch_state::State oldState = switch_state.state;

  switch (oldState) {
    case P098_limit_switch_state::State::Low:

      if (pinState) {
        switch_state.state = P098_limit_switch_state::State::TriggerWaitBounce;
      }
      break;
    case P098_limit_switch_state::State::TriggerWaitBounce:

      if ((switch_state.lastChanged != 0) && (timePassedSince(switch_state.lastChanged) < gpio_config.debounceTime)) {
        if (!pinState) {
          switch_state.state = P098_limit_switch_state::State::Low;
        } else {
          // Apparently we missed a (logic) falling edge, thus reset position and timestamp.
          switch_state.lastChanged = millis();
          switch_state.triggerpos  = position;
        }
      } else {
        if (pinState) {
          // We can accept the stored trigger
          switch_state.state = P098_limit_switch_state::State::High;
        }
      }
      break;
    case P098_limit_switch_state::State::High:

      if (!pinState) {
        switch_state.state = P098_limit_switch_state::State::Low;
      }
      break;
  }

  if (oldState != switch_state.state) {
    switch (switch_state.state) {
      case P098_limit_switch_state::State::Low:
        switch_state.triggerpos  = 0;
        switch_state.lastChanged = 0;
        break;
      case P098_limit_switch_state::State::TriggerWaitBounce:
        switch_state.lastChanged = millis();
        switch_state.triggerpos  = position;
        break;
      case P098_limit_switch_state::State::High:

        if (!switch_state.switchposSet) {
          switch_state.switchpos    = switch_state.triggerpos;
          switch_state.switchposSet = true;
        }
        break;
    }
  }
}

void ICACHE_RAM_ATTR P098_data_struct::ISRlimitA(P098_data_struct *self)
{
  process_limit_switch(self->_config.limitA, self->limitA, self->position);
}

void ICACHE_RAM_ATTR P098_data_struct::ISRlimitB(P098_data_struct *self)
{
  process_limit_switch(self->_config.limitB, self->limitB, self->position);
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
