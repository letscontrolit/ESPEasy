#ifndef _HELPERS_IBUSCMD_HANDLER_H
#define _HELPERS_IBUSCMD_HANDLER_H

/** Readme:
 * This interface is used by BusCmd_Helper to enable different Buses to be used for handling the commands to devices connected on that bus.
 * The BusCmd_Handler_XXX should handle any hardware specific differences.
 * Planned implementations:
 * - I2C (implemented May 2025)
 * - SPI
 * - Serial (Using ESPEasySerial)
 * - MODBUS
 * - CAN-BUS
 * - RF
 */

/** Changelog:
 * 2025-05-13 tonhuisman: Add String data-format support, guarded with FEATURE_BUSCMD_STRING
 * 2025-05-10 tonhuisman: Extract interface and I2C implementation from Plugin P180 I2C Generic implementation
 */
#if FEATURE_BUS_COMMAND
# include <vector>

# define BUS_CMD_BYTE_DATA  std::vector<uint8_t>
# define BUS_CMD_WIDE_DATA  std::vector<uint16_t>


class IBusCmd_Handler // Interface/Abstract Class
{
public:

  IBusCmd_Handler() {}

  virtual ~IBusCmd_Handler() {}

  virtual bool              init()    = 0;
  virtual uint8_t           read8u()  = 0;
  virtual uint16_t          read16u() = 0;
  virtual uint32_t          read24u() = 0;
  virtual uint32_t          read32u() = 0;
  virtual uint8_t           read8uREG(uint16_t reg,
                                      bool     wideReg = false) = 0;
  virtual uint16_t          read16uREG(uint16_t reg,
                                       bool     wideReg = false) = 0;
  virtual uint32_t          read24uREG(uint16_t reg,
                                       bool     wideReg = false) = 0;
  virtual uint32_t          read32uREG(uint16_t reg,
                                       bool     wideReg = false) = 0;
  virtual BUS_CMD_BYTE_DATA read8uB(uint32_t size)      = 0;
  virtual BUS_CMD_WIDE_DATA read16uW(uint32_t size)     = 0;
  virtual BUS_CMD_BYTE_DATA read8uBREG(uint16_t reg,
                                       uint32_t size,
                                       bool     wideReg = false) = 0;
  virtual BUS_CMD_WIDE_DATA read16uWREG(uint16_t reg,
                                        uint32_t size,
                                        bool     wideReg = false) = 0;
  # if FEATURE_BUSCMD_STRING
  virtual String readString(uint32_t len) = 0;
  virtual String readStringREG(uint16_t reg,
                               uint32_t len,
                               bool     wideReg = false) = 0;
  # endif // if FEATURE_BUSCMD_STRING

  virtual bool     write8u(uint8_t data)   = 0;
  virtual bool     write16u(uint16_t data) = 0;
  virtual bool     write24u(uint32_t data) = 0;
  virtual bool     write32u(uint32_t data) = 0;
  virtual bool     write8uREG(uint16_t reg,
                              uint8_t  data,
                              bool     wideReg = false) = 0;
  virtual bool     write16uREG(uint16_t reg,
                               uint16_t data,
                               bool     wideReg = false) = 0;
  virtual bool     write24uREG(uint16_t reg,
                               uint32_t data,
                               bool     wideReg = false) = 0;
  virtual bool     write32uREG(uint16_t reg,
                               uint32_t data,
                               bool     wideReg             = false) = 0;
  virtual uint32_t write8uB(const BUS_CMD_BYTE_DATA& data)  = 0;
  virtual uint32_t write16uW(const BUS_CMD_WIDE_DATA& data) = 0;
  virtual uint32_t write8uBREG(uint16_t                 reg,
                               const BUS_CMD_BYTE_DATA& data,
                               bool                     wideReg = false) = 0;
  virtual uint32_t write16uWREG(uint16_t                 reg,
                                const BUS_CMD_WIDE_DATA& data,
                                bool                     wideReg = false) = 0;
  # if FEATURE_BUSCMD_STRING
  virtual uint32_t writeString(const String& data) = 0;
  virtual uint32_t writeStringReg(uint16_t      reg,
                                  const String& data,
                                  bool          wideReg = false) = 0;
  # endif // if FEATURE_BUSCMD_STRING
}; // class IBusCmd_Handler

DEF_UP(IBusCmd_Handler);

#endif // if FEATURE_BUS_COMMAND
#endif // ifndef _HELPERS_IBUSCMD_HANDLER_H
