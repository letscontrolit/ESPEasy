#ifdef PLUGIN_BUILD_DEV

//#######################################################################################################
//#################################### Plugin-113: SI1145 - UV index / IR / visible  ####################
//#######################################################################################################
//#include <Adafruit_SI1145.h>

#define PLUGIN_113
#define PLUGIN_ID_113         113
#define PLUGIN_NAME_113       "SI1145 UV index (Adafruit-QA) [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_113 "VISIBLE"
#define PLUGIN_VALUENAME2_113 "INFRA"
#define PLUGIN_VALUENAME3_113 "UV"

// ADAFRUIT SI1145 LIBRARY - Adafruit_SI1145.h
// =============================================
#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include <Wire.h>

/* COMMANDS */
#define SI1145_PARAM_QUERY 0x80
#define SI1145_PARAM_SET 0xA0
#define SI1145_NOP 0x0
#define SI1145_RESET    0x01
#define SI1145_BUSADDR    0x02
#define SI1145_PS_FORCE    0x05
#define SI1145_ALS_FORCE    0x06
#define SI1145_PSALS_FORCE    0x07
#define SI1145_PS_PAUSE    0x09
#define SI1145_ALS_PAUSE    0x0A
#define SI1145_PSALS_PAUSE    0xB
#define SI1145_PS_AUTO    0x0D
#define SI1145_ALS_AUTO   0x0E
#define SI1145_PSALS_AUTO 0x0F
#define SI1145_GET_CAL    0x12
/* Parameters */
#define SI1145_PARAM_I2CADDR 0x00
#define SI1145_PARAM_CHLIST   0x01
#define SI1145_PARAM_CHLIST_ENUV 0x80
#define SI1145_PARAM_CHLIST_ENAUX 0x40
#define SI1145_PARAM_CHLIST_ENALSIR 0x20
#define SI1145_PARAM_CHLIST_ENALSVIS 0x10
#define SI1145_PARAM_CHLIST_ENPS1 0x01
#define SI1145_PARAM_CHLIST_ENPS2 0x02
#define SI1145_PARAM_CHLIST_ENPS3 0x04
#define SI1145_PARAM_PSLED12SEL   0x02
#define SI1145_PARAM_PSLED12SEL_PS2NONE 0x00
#define SI1145_PARAM_PSLED12SEL_PS2LED1 0x10
#define SI1145_PARAM_PSLED12SEL_PS2LED2 0x20
#define SI1145_PARAM_PSLED12SEL_PS2LED3 0x40
#define SI1145_PARAM_PSLED12SEL_PS1NONE 0x00
#define SI1145_PARAM_PSLED12SEL_PS1LED1 0x01
#define SI1145_PARAM_PSLED12SEL_PS1LED2 0x02
#define SI1145_PARAM_PSLED12SEL_PS1LED3 0x04
#define SI1145_PARAM_PSLED3SEL   0x03
#define SI1145_PARAM_PSENCODE   0x05
#define SI1145_PARAM_ALSENCODE  0x06
#define SI1145_PARAM_PS1ADCMUX   0x07
#define SI1145_PARAM_PS2ADCMUX   0x08
#define SI1145_PARAM_PS3ADCMUX   0x09
#define SI1145_PARAM_PSADCOUNTER   0x0A
#define SI1145_PARAM_PSADCGAIN 0x0B
#define SI1145_PARAM_PSADCMISC 0x0C
#define SI1145_PARAM_PSADCMISC_RANGE 0x20
#define SI1145_PARAM_PSADCMISC_PSMODE 0x04
#define SI1145_PARAM_ALSIRADCMUX   0x0E
#define SI1145_PARAM_AUXADCMUX   0x0F
#define SI1145_PARAM_ALSVISADCOUNTER   0x10
#define SI1145_PARAM_ALSVISADCGAIN 0x11
#define SI1145_PARAM_ALSVISADCMISC 0x12
#define SI1145_PARAM_ALSVISADCMISC_VISRANGE 0x20
#define SI1145_PARAM_ALSIRADCOUNTER   0x1D
#define SI1145_PARAM_ALSIRADCGAIN 0x1E
#define SI1145_PARAM_ALSIRADCMISC 0x1F
#define SI1145_PARAM_ALSIRADCMISC_RANGE 0x20
#define SI1145_PARAM_ADCCOUNTER_511CLK 0x70
#define SI1145_PARAM_ADCMUX_SMALLIR  0x00
#define SI1145_PARAM_ADCMUX_LARGEIR  0x03
/* REGISTERS */
#define SI1145_REG_PARTID  0x00
#define SI1145_REG_REVID  0x01
#define SI1145_REG_SEQID  0x02
#define SI1145_REG_INTCFG  0x03
#define SI1145_REG_INTCFG_INTOE 0x01
#define SI1145_REG_INTCFG_INTMODE 0x02
#define SI1145_REG_IRQEN  0x04
#define SI1145_REG_IRQEN_ALSEVERYSAMPLE 0x01
#define SI1145_REG_IRQEN_PS1EVERYSAMPLE 0x04
#define SI1145_REG_IRQEN_PS2EVERYSAMPLE 0x08
#define SI1145_REG_IRQEN_PS3EVERYSAMPLE 0x10
#define SI1145_REG_IRQMODE1 0x05
#define SI1145_REG_IRQMODE2 0x06
#define SI1145_REG_HWKEY  0x07
#define SI1145_REG_MEASRATE0 0x08
#define SI1145_REG_MEASRATE1  0x09
#define SI1145_REG_PSRATE  0x0A
#define SI1145_REG_PSLED21  0x0F
#define SI1145_REG_PSLED3  0x10
#define SI1145_REG_UCOEFF0  0x13
#define SI1145_REG_UCOEFF1  0x14
#define SI1145_REG_UCOEFF2  0x15
#define SI1145_REG_UCOEFF3  0x16
#define SI1145_REG_PARAMWR  0x17
#define SI1145_REG_COMMAND  0x18
#define SI1145_REG_RESPONSE  0x20
#define SI1145_REG_IRQSTAT  0x21
#define SI1145_REG_IRQSTAT_ALS  0x01
#define SI1145_REG_ALSVISDATA0 0x22
#define SI1145_REG_ALSVISDATA1 0x23
#define SI1145_REG_ALSIRDATA0 0x24
#define SI1145_REG_ALSIRDATA1 0x25
#define SI1145_REG_PS1DATA0 0x26
#define SI1145_REG_PS1DATA1 0x27
#define SI1145_REG_PS2DATA0 0x28
#define SI1145_REG_PS2DATA1 0x29
#define SI1145_REG_PS3DATA0 0x2A
#define SI1145_REG_PS3DATA1 0x2B
#define SI1145_REG_UVINDEX0 0x2C
#define SI1145_REG_UVINDEX1 0x2D
#define SI1145_REG_PARAMRD 0x2E
#define SI1145_REG_CHIPSTAT 0x30
#define SI1145_ADDR 0x60

class Adafruit_SI1145  {
 public:
  Adafruit_SI1145(void);
  boolean begin();
  void reset();
  uint16_t readUV();
  uint16_t readIR();
  uint16_t readVisible();
  uint16_t readProx();
 private:
  uint16_t read16(uint8_t addr);
  uint8_t read8(uint8_t addr);
  void write8(uint8_t reg, uint8_t val);
  uint8_t readParam(uint8_t p);
  uint8_t writeParam(uint8_t p, uint8_t v);
  uint8_t _addr;
};

// ADAFRUIT SI1145 LIBRARY - Adafruit_SI1145.cpp
// =============================================
Adafruit_SI1145::Adafruit_SI1145() {
  _addr = SI1145_ADDR;
}

boolean Adafruit_SI1145::begin(void) {
  Wire.begin();
  uint8_t id = read8(SI1145_REG_PARTID);
  if (id != 0x45) return false; // look for SI1145
  reset();
  /***********************************/
  // enable UVindex measurement coefficients!
  write8(SI1145_REG_UCOEFF0, 0x29);
  write8(SI1145_REG_UCOEFF1, 0x89);
  write8(SI1145_REG_UCOEFF2, 0x02);
  write8(SI1145_REG_UCOEFF3, 0x00);
  // enable UV sensor
  writeParam(SI1145_PARAM_CHLIST, SI1145_PARAM_CHLIST_ENUV |
  SI1145_PARAM_CHLIST_ENALSIR | SI1145_PARAM_CHLIST_ENALSVIS |
  SI1145_PARAM_CHLIST_ENPS1);
  // enable interrupt on every sample
  write8(SI1145_REG_INTCFG, SI1145_REG_INTCFG_INTOE);
  write8(SI1145_REG_IRQEN, SI1145_REG_IRQEN_ALSEVERYSAMPLE);
  /****************************** Prox Sense 1 */
  // program LED current
  write8(SI1145_REG_PSLED21, 0x03); // 20mA for LED 1 only
  writeParam(SI1145_PARAM_PS1ADCMUX, SI1145_PARAM_ADCMUX_LARGEIR);
  // prox sensor #1 uses LED #1
  writeParam(SI1145_PARAM_PSLED12SEL, SI1145_PARAM_PSLED12SEL_PS1LED1);
  // fastest clocks, clock div 1
  writeParam(SI1145_PARAM_PSADCGAIN, 0);
  // take 511 clocks to measure
  writeParam(SI1145_PARAM_PSADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);
  // in prox mode, high range
  writeParam(SI1145_PARAM_PSADCMISC, SI1145_PARAM_PSADCMISC_RANGE|
    SI1145_PARAM_PSADCMISC_PSMODE);
  writeParam(SI1145_PARAM_ALSIRADCMUX, SI1145_PARAM_ADCMUX_SMALLIR);
  // fastest clocks, clock div 1
  writeParam(SI1145_PARAM_ALSIRADCGAIN, 0);
  // take 511 clocks to measure
  writeParam(SI1145_PARAM_ALSIRADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);
  // in high range mode
  writeParam(SI1145_PARAM_ALSIRADCMISC, SI1145_PARAM_ALSIRADCMISC_RANGE);
  // fastest clocks, clock div 1
  writeParam(SI1145_PARAM_ALSVISADCGAIN, 0);
  // take 511 clocks to measure
  writeParam(SI1145_PARAM_ALSVISADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);
  // in high range mode (not normal signal)
  writeParam(SI1145_PARAM_ALSVISADCMISC, SI1145_PARAM_ALSVISADCMISC_VISRANGE);
  /************************/
  // measurement rate for auto
  write8(SI1145_REG_MEASRATE0, 0xFF); // 255 * 31.25uS = 8ms
  // auto run
  write8(SI1145_REG_COMMAND, SI1145_PSALS_AUTO);
  return true;
}

void Adafruit_SI1145::reset() {
  write8(SI1145_REG_MEASRATE0, 0);
  write8(SI1145_REG_MEASRATE1, 0);
  write8(SI1145_REG_IRQEN, 0);
  write8(SI1145_REG_IRQMODE1, 0);
  write8(SI1145_REG_IRQMODE2, 0);
  write8(SI1145_REG_INTCFG, 0);
  write8(SI1145_REG_IRQSTAT, 0xFF);
  write8(SI1145_REG_COMMAND, SI1145_RESET);
  delay(10);
  write8(SI1145_REG_HWKEY, 0x17);
  delay(10);
}

//////////////////////////////////////////////////////
// returns the UV index * 100 (divide by 100 to get the index)
uint16_t Adafruit_SI1145::readUV(void) { return read16(0x2C); }
// returns visible+IR light levels
uint16_t Adafruit_SI1145::readVisible(void) { return read16(0x22); }
// returns IR light levels
uint16_t Adafruit_SI1145::readIR(void) { return read16(0x24); }

// returns "Proximity" - assumes an IR LED is attached to LED
uint16_t Adafruit_SI1145::readProx(void) {
 return read16(0x26);
}

/*********************************************************************/
uint8_t Adafruit_SI1145::writeParam(uint8_t p, uint8_t v) {
  //Serial.print("Param 0x"); Serial.print(p, HEX);
  //Serial.print(" = 0x"); Serial.println(v, HEX);
  write8(SI1145_REG_PARAMWR, v);
  write8(SI1145_REG_COMMAND, p | SI1145_PARAM_SET);
  return read8(SI1145_REG_PARAMRD);
}

uint8_t Adafruit_SI1145::readParam(uint8_t p) {
  write8(SI1145_REG_COMMAND, p | SI1145_PARAM_QUERY);
  return read8(SI1145_REG_PARAMRD);
}

/*********************************************************************/
uint8_t  Adafruit_SI1145::read8(uint8_t reg) {
  uint16_t val;
    Wire.beginTransmission(_addr);
    Wire.write((uint8_t)reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)_addr, (uint8_t)1);
    return Wire.read();
}

uint16_t Adafruit_SI1145::read16(uint8_t a) {
  uint16_t ret;
  Wire.beginTransmission(_addr); // start transmission to device
  Wire.write(a); // sends register address to read from
  Wire.endTransmission(); // end transmission
  Wire.requestFrom(_addr, (uint8_t)2);// send data n-bytes read
  ret = Wire.read(); // receive DATA
  ret |= (uint16_t)Wire.read() << 8; // receive DATA
  return ret;
}

void Adafruit_SI1145::write8(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(_addr); // start transmission to device
  Wire.write(reg); // sends register address to write
  Wire.write(val); // sends value
  Wire.endTransmission(); // end transmission
}

Adafruit_SI1145 uv = Adafruit_SI1145();

boolean Plugin_113_init = false;

boolean Plugin_113(byte function, struct EventStruct *event, String& string)
  {
  boolean success=false;

  switch(function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_113;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_113);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_113));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_113));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_113));
        break;
      }

  case PLUGIN_READ:
    {
      if (!Plugin_113_init)
        {
          // ziadnu EXTRA inicializaciu netreba ...
          if (uv.begin()) { Plugin_113_init = true; }
        }
      float si1145_vis = 0.0;
      float si1145_ir = 0.0;
      float si1145_uv = 0.0;
      si1145_vis = uv.readVisible();
      si1145_ir = uv.readIR();
      si1145_uv = uv.readUV();
      si1145_uv /= 100.0;

      UserVar[event->BaseVarIndex] = si1145_vis;
      UserVar[event->BaseVarIndex+1] = si1145_ir;
      UserVar[event->BaseVarIndex+2] = si1145_uv;

      String log = F("SI1145: Visible: ");
      log += UserVar[event->BaseVarIndex];
      addLog(LOG_LEVEL_INFO,log);
      log = F("SI1145: Infrared: ");
      log += UserVar[event->BaseVarIndex+1];
      addLog(LOG_LEVEL_INFO,log);
      log = F("SI1145: UV index: ");
      log += UserVar[event->BaseVarIndex+2];
      addLog(LOG_LEVEL_INFO,log);
      success=true;
      break;

    }
  }
  return success;
}



#endif
