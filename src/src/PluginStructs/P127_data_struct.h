#ifndef PLUGINSTRUCTS_P127_DATA_STRUCT_H
#define PLUGINSTRUCTS_P127_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P127

// #######################################################################################################
// ################################## Plugin 127 I2C CDM7160 CO2 sensor ##################################
// #######################################################################################################
//
//

# define P127_CONFIG_I2C_ADDRESS PCONFIG(0)
# define P127_CONFIG_ALTITUDE    PCONFIG(1)

# define CDM7160_ADDR   0x69 // default address
# define CDM7160_ADDR_0 0x68 // CAD0 tied to GND

# define  CDM7160_REG_RESET   0x00
# define  CDM7160_REG_CTL     0x01
# define  CDM7160_REG_STATUS  0x02
# define  CDM7160_REG_DATA    0x03
# define  CDM7160_REG_HIT     0x0A
# define  CDM7160_REG_FUNC    0x0F

# define  CDM7160_FLAG_REST  0x01
# define  CDM7160_FLAG_DOWN  0x00
# define  CDM7160_FLAG_CONT  0x06
# define  CDM7160_FLAG_BUSY  0x80
# define  CDM7160_FLAG_HPAE  0x04
# define  CDM7160_FLAG_PWME  0x01


struct P127_data_struct : public PluginTaskData_base {
  P127_data_struct(int8_t i2caddr);
  bool     init(uint16_t alt);
  bool     checkData();
  uint16_t readData();
  bool     setReset(void);
  uint8_t  getAltitude();
  uint8_t  getCompensation();

private:

  uint8_t  I2C_read8_ST_reg(uint8_t i2caddr,
                            byte    reg);
  uint16_t I2C_read16_LE_ST_reg(uint8_t i2caddr,
                                byte    reg);
  uint16_t getCO2();
  bool     setPowerDown(void);
  bool     setContinuous(void);
  bool     setAltitude(uint8_t alt);
  bool     clearAltitude(void);
  uint8_t  getStatus();

  byte     _i2cAddress;
  uint16_t _co2;
};

#endif // ifdef USES_P127
#endif // ifndef PLUGINSTRUCTS_P127_DATA_STRUCT_H
