#ifndef _HELPERS_BUSCMD_HANDLER_I2C_H
#define _HELPERS_BUSCMD_HANDLER_I2C_H

/** Changelog:
 * 2025-05-13 tonhuisman: Add support for String commands, guarded by FEATURE_BUSCMD_STRING
 * 2025-05-10 tonhuisman: Extracted from Plugin P180 I2C Generic
 */

#include "../../_Plugin_Helper.h"
#include "../Helpers/IBusCmd_Handler.h"
#include <Wire.h>

#include <vector>

// TODO 2025-05-11 tonhuisman: wideReg feature not implemented yet!

class BusCmd_Handler_I2C : public IBusCmd_Handler {
public:

  BusCmd_Handler_I2C() {}

  BusCmd_Handler_I2C(uint8_t           i2cAddress,
                     TwoWire          *wire =& Wire);

  virtual ~BusCmd_Handler_I2C() {}

  virtual bool                 init();
  virtual uint8_t              read8u();
  virtual uint16_t             read16u();
  virtual uint32_t             read24u();
  virtual uint32_t             read32u();
  virtual uint8_t              read8uREG(uint16_t reg,
                                         bool     wideReg = false);
  virtual uint16_t             read16uREG(uint16_t reg,
                                          bool     wideReg = false);
  virtual uint32_t             read24uREG(uint16_t reg,
                                          bool     wideReg = false);
  virtual uint32_t             read32uREG(uint16_t reg,
                                          bool     wideReg = false);
  virtual std::vector<uint8_t> read8uB(uint32_t size);
  virtual std::vector<uint16_t>read16uW(uint32_t size);
  virtual std::vector<uint8_t> read8uBREG(uint16_t reg,
                                          uint32_t size,
                                          bool     wideReg = false);
  virtual std::vector<uint16_t>read16uWREG(uint16_t reg,
                                           uint32_t size,
                                           bool     wideReg = false);
  #if FEATURE_BUSCMD_STRING
  virtual String readString(uint32_t len);
  virtual String readStringREG(uint16_t reg,
                               uint32_t len,
                               bool     wideReg = false);
  #endif // if FEATURE_BUSCMD_STRING

  virtual bool     write8u(uint8_t data);
  virtual bool     write16u(uint16_t data);
  virtual bool     write24u(uint32_t data);
  virtual bool     write32u(uint32_t data);
  virtual bool     write8uREG(uint16_t reg,
                              uint8_t  data,
                              bool     wideReg = false);
  virtual bool     write16uREG(uint16_t reg,
                               uint16_t data,
                               bool     wideReg = false);
  virtual bool     write24uREG(uint16_t reg,
                               uint32_t data,
                               bool     wideReg = false);
  virtual bool     write32uREG(uint16_t reg,
                               uint32_t data,
                               bool     wideReg = false);
  virtual uint32_t write8uB(std::vector<uint8_t>data);
  virtual uint32_t write16uW(std::vector<uint16_t>data);
  virtual uint32_t write8uBREG(uint16_t            reg,
                               std::vector<uint8_t>data,
                               bool                wideReg = false);
  virtual uint32_t write16uWREG(uint16_t             reg,
                                std::vector<uint16_t>data,
                                bool                 wideReg = false);
  #if FEATURE_BUSCMD_STRING
  virtual uint32_t writeString(const String& data);
  virtual uint32_t writeStringReg(uint16_t      reg,
                                  const String& data,
                                  bool          wideReg = false);
  #endif // if FEATURE_BUSCMD_STRING

private:

  uint8_t _i2cAddress;
  TwoWire *_wire;
  bool _ok;
};
#endif // ifndef _HELPERS_BUSCMD_HANDLER_I2C_H
