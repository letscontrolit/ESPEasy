#ifdef USES_P128
//#######################################################################################################
//################################# Plugin 128: AS3935 Lightning Sensor #################################
//#######################################################################################################
// _P128_AS3935.ino - AS3935 Franklin Lightning Sensor™ IC by AMS https://ams.com/as3935
// Derived from Edward Gutting/Raivis Rengelis library https://github.com/THP-JOE/AS3935_Wire 
// Copyright (c) 2018 Matej Supik (supik.matej@seznam.cz)

#include <Wire.h>

#define PLUGIN_128
#define PLUGIN_ID_128         128
#define PLUGIN_NAME_128       "AS3935 Lightning Sensor"
#define PLUGIN_VALUENAME1_128 "Distance" // Lightning distance [km]
#define PLUGIN_VALUENAME2_128 "Energy" // Lightning energy
#define PLUGIN_VALUENAME3_128 "Source" // Interrupt source [1=Noise level too high, 4=Disturber detected, 8=Lightning interrupt]
#define PLUGIN_128_DEBUG      false

// register access macros - register address, bitmask
#define AS3935_AFE_GB       0x00, 0x3E
#define AS3935_PWD          0x00, 0x01
#define AS3935_NF_LEV       0x01, 0x70
#define AS3935_WDTH         0x01, 0x0F
#define AS3935_CL_STAT      0x02, 0x40
#define AS3935_MIN_NUM_LIGH 0x02, 0x30
#define AS3935_SREJ         0x02, 0x0F
#define AS3935_LCO_FDIV     0x03, 0xC0
#define AS3935_MASK_DIST    0x03, 0x20
#define AS3935_INT          0x03, 0x0F
#define AS3935_DISTANCE     0x07, 0x1F
#define AS3935_DISP_LCO     0x08, 0x80
#define AS3935_DISP_SRCO    0x08, 0x40
#define AS3935_DISP_TRCO    0x08, 0x20
#define AS3935_TUN_CAP      0x08, 0x0F
#define AS3935_ENERGY_1     0x04, 0xFF
#define AS3935_ENERGY_2     0x05, 0xFF
#define AS3935_ENERGY_3     0x06, 0x1F

// other constants
#define AS3935_AFE_INDOOR   0x12
#define AS3935_AFE_OUTDOOR  0x0E

static boolean AS3935IrqTriggered[4];

void P128_reset(uint8_t addr);
bool P128_calibrate(uint8_t addr, uint8_t irq_pin);
void P128_powerDown(uint8_t addr);
void P128_powerUp(uint8_t addr);
uint16_t P128_interruptSource(uint8_t addr);
void P128_disableDisturbers(uint8_t addr);
void P128_enableDisturbers(uint8_t addr);
uint16_t P128_getMinimumLightnings(uint8_t addr);
uint16_t P128_setMinimumLightnings(uint8_t addr, uint16_t minlightning);
uint16_t P128_lightningDistanceKm(uint8_t addr);
long P128_lightningEnergy(uint8_t addr);
void P128_setIndoors(uint8_t addr);
void P128_setOutdoors(uint8_t addr);
uint16_t P128_getNoiseFloor(uint8_t addr);
uint16_t P128_setNoiseFloor(uint8_t addr, uint16_t noisefloor);
uint16_t P128_getSpikeRejection(uint8_t addr);
uint16_t P128_setSpikeRejection(uint8_t addr, uint16_t srej);
uint16_t P128_getWatchdogThreshold(uint8_t addr);
uint16_t P128_setWatchdogThreshold(uint8_t addr, uint16_t wdth);
void P128_clearStats(uint8_t addr);

boolean Plugin_128(byte function, struct EventStruct *event, String &string)
{
  boolean success = false;

  switch (function)
  {
  case PLUGIN_DEVICE_ADD:
  {
    //This case defines the device characteristics, edit appropriately

    Device[++deviceCount].Number = PLUGIN_ID_128;
    Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
    Device[deviceCount].VType = SENSOR_TYPE_TRIPLE;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].FormulaOption = false;
    Device[deviceCount].ValueCount = 3;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = false;
    Device[deviceCount].TimerOptional = false;
    Device[deviceCount].GlobalSyncOption = true;
    Device[deviceCount].DecimalsOnly = true;
    break;
  }

  case PLUGIN_GET_DEVICENAME:
  {
    //return the device name
    string = F(PLUGIN_NAME_128);
    break;
  }

  case PLUGIN_GET_DEVICEVALUENAMES:
  {
    //called when the user opens the module configuration page
    //it allows to add a new row for each output variable of the plugin
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_128));
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_128));
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_128));
    break;
  }

  case PLUGIN_WEBFORM_LOAD:
  {
    byte i2c_addr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
    int i2c_addr_options[3] = {0x01, 0x02, 0x03};
    addFormSelectorI2C(F("plugin_128_i2c_addr"), 3, i2c_addr_options, i2c_addr);

    bool mode = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
    addFormCheckBox(F("Indoor mode"), F("plugin_128_indoor_mode"), mode);

    byte noise_floor = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
    String noise_floor_options[8] = {F("390/28 µVrms"), F("630/45 µVrms"), F("860/62 µVrms"), F("1100/78 µVrms"), F("1140/95 µVrms"), F("1570/112 µVrms"), F("1800/130 µVrms"), F("2000/146 µVrms")};
    int noise_floor_option_values[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    addFormSelector(F("Noise floor"), F("plugin_128_noise_floor"), 8, noise_floor_options, noise_floor_option_values, noise_floor);

    byte spike_rejection = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
    addFormNumericBox(F("Spike rejection"), F("plugin_128_spike_rejection"), spike_rejection, 0, 15);

    byte watchdog_threshold = Settings.TaskDevicePluginConfig[event->TaskIndex][4];
    addFormNumericBox(F("Watchdog threshold"), F("plugin_128_watchdog_threshold"), watchdog_threshold, 0, 15);

    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE:
  {
    Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_128_i2c_addr"));

    Settings.TaskDevicePluginConfig[event->TaskIndex][1] = isFormItemChecked(F("plugin_128_indoor_mode"));

    Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_128_noise_floor"));
    Settings.TaskDevicePluginConfig[event->TaskIndex][3] = getFormItemInt(F("plugin_128_spike_rejection"));
    Settings.TaskDevicePluginConfig[event->TaskIndex][4] = getFormItemInt(F("plugin_128_watchdog_threshold"));

    success = true;
    break;
  }
  case PLUGIN_INIT:
  {
    //this case defines code to be executed when the plugin is initialised
    int irq_pin = Settings.TaskDevicePin1[event->TaskIndex];
    byte i2c_addr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

    pinMode(irq_pin, INPUT);

    addToLog(LOG_LEVEL_ERROR, String(P128_registerRead(i2c_addr, AS3935_AFE_GB)));

    // reset all internal register values to defaults
    P128_reset(i2c_addr);

    // first let's turn on disturber indication and print some register values from AS3935
    // tell AS3935 we are indoors, for outdoors use setOutdoors() function
    if (Settings.TaskDevicePluginConfig[event->TaskIndex][1])
      P128_setIndoors(i2c_addr);
    else
      P128_setOutdoors(i2c_addr);

    // and run calibration
    // if lightning detector can not tune tank circuit to required tolerance,
    // calibration function will return false
    if (!P128_calibrate(i2c_addr, irq_pin))
      addToLog(LOG_LEVEL_ERROR, F("AS3935   : Tuning out of range, check your sensor !"));
    else
      addToLog(LOG_LEVEL_INFO, F("AS3935   : Tuning ok"));

    byte noise_floor = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
    P128_setNoiseFloor(i2c_addr, noise_floor);
    addToLog(LOG_LEVEL_INFO, "AS3935   : Noise floor: " + String(noise_floor));

    byte spike_rejection = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
    P128_setSpikeRejection(i2c_addr, spike_rejection);
    addToLog(LOG_LEVEL_INFO, "AS3935   : Spike rejection: " + String(spike_rejection));

    byte watchdog_threshold = Settings.TaskDevicePluginConfig[event->TaskIndex][4];
    P128_setWatchdogThreshold(i2c_addr, watchdog_threshold);
    addToLog(LOG_LEVEL_INFO, "AS3935   : Watchdog threshold: " + String(watchdog_threshold));

    switch (i2c_addr)
    {
    case 0x01:
      attachInterrupt(irq_pin, p128_irq_0x01, RISING);
      break;
    case 0x02:
      attachInterrupt(irq_pin, p128_irq_0x02, RISING);
      break;
    case 0x03:
      attachInterrupt(irq_pin, p128_irq_0x03, RISING);
      break;
    }

    //after the plugin has been initialised successfuly, set success and break
    success = true;
    break;
  }

  case PLUGIN_READ:
  {
    //code to be executed to read data
    //It is executed according to the delay configured on the device configuration page, only once

    //after the plugin has read data successfuly, set success and break
    success = true;
    break;
  }

  case PLUGIN_WRITE:
  {
    //this case defines code to be executed when the plugin executes an action (command).
    //Commands can be accessed via rules or via http.
    //As an example, http://192.168.1.12//control?cmd=dothis
    //implies that there exists the comamnd "dothis"

    // if (plugin_not_initialised)
    //  break;

    //parse string to extract the command
    String tmpString = string;
    int argIndex = tmpString.indexOf(',');
    if (argIndex)
      tmpString = tmpString.substring(0, argIndex);

    String tmpStr = string;
    //int comma1 = tmpStr.indexOf(',');
    if (tmpString.equalsIgnoreCase(F("dothis")))
    {
      //do something
      success = true; //set to true only if plugin has executed a command successfully
    }

    break;
  }

  case PLUGIN_EXIT:
  {
    int irq_pin = Settings.TaskDevicePin1[event->TaskIndex];

    detachInterrupt(irq_pin);

    break;
  }

  case PLUGIN_ONCE_A_SECOND:
  {
    //code to be executed once a second. Tasks which do not require fast response can be added here
    // here we go into loop checking if interrupt has been triggered, which kind of defeats
    // the whole purpose of interrupts, but in real life you could put your chip to sleep
    // and lower power consumption or do other nifty things

    success = true;
  }

  case PLUGIN_TEN_PER_SECOND:
  {
    //code to be executed 10 times per second. Tasks which require fast response can be added here
    //be careful on what is added here. Heavy processing will result in slowing the module down!
    // Serial.println("jes");
    byte i2c_addr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

    if (AS3935IrqTriggered[i2c_addr])
    {
      // reset the flag
      AS3935IrqTriggered[i2c_addr] = false;
      // first step is to find out what caused interrupt
      // as soon as we read interrupt cause register, irq pin goes low
      int irqSource = P128_interruptSource(i2c_addr);
      UserVar[event->BaseVarIndex + 2] = irqSource;
      // returned value is bitmap field, bit 0 - noise level too high, bit 2 - disturber detected, and finally bit 3 - lightning!
      if (irqSource & 0b0001)
      {
        addLog(LOG_LEVEL_INFO, F("AS3935   : Noise level too high, try adjusting noise floor"));
      }
      else if (irqSource & 0b0100)
      {
        addLog(LOG_LEVEL_INFO, F("AS3935   : Disturber detected"));
      }
      else if (irqSource & 0b1000)
      {
        // need to find how far that lightning stroke, function returns approximate distance in kilometers,
        // where value 1 represents storm in detector's near victinity, and 63 - very distant, out of range stroke
        // everything in between is just distance in kilometers
        int strokeDistance = P128_lightningDistanceKm(i2c_addr);
        UserVar[event->BaseVarIndex] = strokeDistance;
        long energy = P128_lightningEnergy(i2c_addr);
        UserVar[event->BaseVarIndex + 1] = energy;
        if (strokeDistance == 1)
          addLog(LOG_LEVEL_INFO, F("AS3935   : Storm overhead, watch out!"));
        if (strokeDistance == 63)
          addLog(LOG_LEVEL_INFO, F("AS3935   : Out of range lightning detected"));
        if (strokeDistance < 63 && strokeDistance > 1)
          addLog(LOG_LEVEL_INFO, "AS3935   : Lightning detected " + String(strokeDistance) + " kilometers away");
      }
      else
        addLog(LOG_LEVEL_INFO, F("AS3935   : Undefined irq source"));

      sendData(event);
    }
    success = true;
  }
  } // switch
  return success;

} //function

//implement plugin specific procedures and functions here
void p128_irq_0x01()
{
  AS3935IrqTriggered[0x01] = true;
}

void p128_irq_0x02()
{
  AS3935IrqTriggered[0x02] = true;
}

void p128_irq_0x03()
{
  AS3935IrqTriggered[0x03] = true;
}

uint8_t P128_ffsz(uint8_t mask)
{
  uint8_t i = 0;
  if (mask)
    for (i = 1; ~mask & 1; i++)
      mask >>= 1;
  return i;
}

void P128_registerWrite(uint8_t addr, uint8_t reg, uint8_t mask, uint8_t data)
{
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission(false);
  //read 1 uint8_t
  Wire.requestFrom(addr, 1);
  //put it to regval
  uint8_t regval = Wire.read();

  //do masking
  regval &= ~(mask);
  if (mask)
    regval |= (data << (P128_ffsz(mask) - 1));
  else
    regval |= data;

  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(regval);
  Wire.endTransmission(false);
}

uint8_t P128_registerRead(uint8_t addr, uint8_t reg, uint8_t mask)
{
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission(false);
  //read 1 uint8_t
  Wire.requestFrom(addr, 1);
  //put it to regval
  uint8_t regval = Wire.read();
  //mask
  regval = regval & mask;
  if (mask)
    regval >>= (P128_ffsz(mask) - 1);
  return regval;
}

void P128_reset(uint8_t addr)
{
  //write to 0x3c, value 0x96
  Wire.beginTransmission(addr);
  Wire.write(0x3c);
  Wire.write(0x96);
  Wire.endTransmission(false);

  delay(2);
}

bool P128_calibrate(uint8_t addr, uint8_t irq_pin)
{
  uint16_t target = 3125, currentcount = 0, bestdiff = INT_MAX, currdiff = 0;
  uint8_t bestTune = 0, currTune = 0;
  unsigned long setUpTime;
  uint16_t currIrq, prevIrq;
  // set lco_fdiv divider to 0, which translates to 16
  // so we are looking for 31250Hz on irq pin
  // and since we are counting for 100ms that translates to number 3125
  // each capacitor changes second least significant digit
  // using this timing so this is probably the best way to go
  P128_registerWrite(addr, AS3935_LCO_FDIV, 0);
  P128_registerWrite(addr, AS3935_DISP_LCO, 1);
  // tuning is not linear, can't do any shortcuts here
  // going over all built-in cap values and finding the best
  for (currTune = 0; currTune <= 0x0F; currTune++)
  {
    P128_registerWrite(addr, AS3935_TUN_CAP, currTune);
    // let it settle
    delay(2);
    currentcount = 0;
    prevIrq = digitalRead(irq_pin);
    setUpTime = millis() + 100;
    while ((long)(millis() - setUpTime) < 0)
    {
      currIrq = digitalRead(irq_pin);
      if (currIrq > prevIrq)
      {
        currentcount++;
      }
      prevIrq = currIrq;
    }
    currdiff = target - currentcount;
    // don't look at me, abs() misbehaves
    if (currdiff < 0)
      currdiff = -currdiff;
    if (bestdiff > currdiff)
    {
      bestdiff = currdiff;
      bestTune = currTune;
    }
  }
  P128_registerWrite(addr, AS3935_TUN_CAP, bestTune);
  delay(2);
  P128_registerWrite(addr, AS3935_DISP_LCO, 0);
  // and now do RCO calibration
  Wire.beginTransmission(addr);
  Wire.write(0x3D);
  Wire.write(0x96);
  Wire.endTransmission(false);

  delay(3);
  // if error is over 109, we are outside allowed tuning range of +/-3.5%
  Serial.print("Difference ");
  Serial.println(bestdiff);
  Serial.print("Best Tune: ");
  Serial.println(bestTune);
  return bestdiff > 109 ? false : true;
}

void P128_powerDown(uint8_t addr)
{
  P128_registerWrite(addr, AS3935_PWD, 1);
}

void P128_powerUp(uint8_t addr)
{
  P128_registerWrite(addr, AS3935_PWD, 0);
  Wire.beginTransmission(addr);
  Wire.write(0x3D);
  Wire.write(0x96);
  Wire.endTransmission(false);
  delay(3);
}

uint16_t P128_interruptSource(uint8_t addr)
{
  return P128_registerRead(addr, AS3935_INT);
}

void P128_disableDisturbers(uint8_t addr)
{
  P128_registerWrite(addr, AS3935_MASK_DIST, 1);
}

void P128_enableDisturbers(uint8_t addr)
{
  P128_registerWrite(addr, AS3935_MASK_DIST, 0);
}

uint16_t P128_getMinimumLightnings(uint8_t addr)
{
  return P128_registerRead(addr, AS3935_MIN_NUM_LIGH);
}

uint16_t P128_setMinimumLightnings(uint8_t addr, uint16_t minlightning)
{
  P128_registerWrite(addr, AS3935_MIN_NUM_LIGH, minlightning);
  return P128_getMinimumLightnings(addr);
}

uint16_t P128_lightningDistanceKm(uint8_t addr)
{
  return P128_registerRead(addr, AS3935_DISTANCE);
}

long P128_lightningEnergy(uint8_t addr)
{
	long energy = 0;
	char bits8[4];

	bits8[3] = 0;
	bits8[2] = P128_registerRead(addr, AS3935_ENERGY_3);
	bits8[1] = P128_registerRead(addr, AS3935_ENERGY_2);
	bits8[0] = P128_registerRead(addr, AS3935_ENERGY_1);

	energy = bits8[2]*65536 + bits8[1]*256 + bits8[0];

	return energy;
}

void P128_setIndoors(uint8_t addr)
{
  P128_registerWrite(addr, AS3935_AFE_GB, AS3935_AFE_INDOOR);
}

void P128_setOutdoors(uint8_t addr)
{
  P128_registerWrite(addr, AS3935_AFE_GB, AS3935_AFE_OUTDOOR);
}

uint16_t P128_getNoiseFloor(uint8_t addr)
{
  return P128_registerRead(addr, AS3935_NF_LEV);
}

uint16_t P128_setNoiseFloor(uint8_t addr, uint16_t noisefloor)
{
  P128_registerWrite(addr, AS3935_NF_LEV, noisefloor);
  return P128_getNoiseFloor(addr);
}

uint16_t P128_getSpikeRejection(uint8_t addr)
{
  return P128_registerRead(addr, AS3935_SREJ);
}

uint16_t P128_setSpikeRejection(uint8_t addr, uint16_t srej)
{
  P128_registerWrite(addr, AS3935_SREJ, srej);
  return P128_getSpikeRejection(addr);
}

uint16_t P128_getWatchdogThreshold(uint8_t addr)
{
  return P128_registerRead(addr, AS3935_WDTH);
}

uint16_t P128_setWatchdogThreshold(uint8_t addr, uint16_t wdth)
{
  P128_registerWrite(addr, AS3935_WDTH, wdth);
  return P128_getWatchdogThreshold(addr);
}

void P128_clearStats(uint8_t addr)
{
  P128_registerWrite(addr, AS3935_CL_STAT, 1);
  P128_registerWrite(addr, AS3935_CL_STAT, 0);
  P128_registerWrite(addr, AS3935_CL_STAT, 1);
}

#endif