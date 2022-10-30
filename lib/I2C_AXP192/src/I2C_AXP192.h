#ifndef __I2C_AXP192_H__
#define __I2C_AXP192_H__

#include <Wire.h>
#include <Arduino.h>

#define I2C_AXP192_DEFAULT_ADDRESS 0x34

typedef struct {
  bool    EXTEN;
  bool    BACKUP;
  int16_t DCDC1;
  int16_t DCDC2;
  int16_t DCDC3;
  int16_t LDO2;
  int16_t LDO3;
  int16_t LDOIO;
  int     GPIO0;
  int     GPIO1;
  int     GPIO2;
  int     GPIO3;
  int     GPIO4;
} I2C_AXP192_InitDef;

class I2C_AXP192 {
public:

  I2C_AXP192(uint8_t  deviceAddress = I2C_AXP192_DEFAULT_ADDRESS,
             TwoWire& i2cPort       = Wire);
  void    begin(I2C_AXP192_InitDef initDef);

  void    setDCDC1(int16_t voltage);
  void    setDCDC2(int16_t voltage);
  void    setDCDC3(int16_t voltage);

  void    setLDO2(int16_t voltage);
  void    setLDO3(int16_t voltage);

  void    setLDOIO(int16_t voltage);
  void    setGPIO0(int voltage);
  void    setGPIO1(int voltage);
  void    setGPIO2(int voltage);
  void    setGPIO3(int voltage);
  void    setGPIO4(int voltage);

  void    setEXTEN(bool enable);
  void    setBACKUP(bool enable);

  float   getBatteryVoltage();
  float   getBatteryDischargeCurrent();
  float   getBatteryChargeCurrent();
  float   getBatteryPower();
  float   getAcinVolatge();
  float   getAcinCurrent();
  float   getVbusVoltage();
  float   getVbusCurrent();
  float   getInternalTemperature();
  float   getApsVoltage();

  void    powerOff();
  uint8_t getPekPress();

private:

  uint8_t readByte(uint8_t address);
  void    writeByte(uint8_t address,
                    uint8_t data);
  void    bitOn(uint8_t address,
                uint8_t bit);
  void    bitOff(uint8_t address,
                 uint8_t bit);
  void    setDCVoltage(uint8_t  number,
                       uint16_t voltage);
  uint8_t calcVoltageData(uint16_t value,
                          uint16_t maxv,
                          uint16_t minv,
                          uint16_t step);
  void setLDOVoltage(uint8_t  number,
                     uint16_t voltage);

  TwoWire *_i2cPort;
  int _deviceAddress;
};

#endif // ifndef __I2C_AXP192_H__
