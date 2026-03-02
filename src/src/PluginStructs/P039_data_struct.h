#ifndef PLUGINSTRUCTS_P039_DATA_STRUCT_H
#define PLUGINSTRUCTS_P039_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P039

# include <SPI.h>

# include "../Globals/SPIe.h"

# define MAX31865_RD_ADDRESS(n) (MAX31865_READ_ADDR_BASE + (n))
# define MAX31865_WR_ADDRESS(n) (MAX31865_WRITE_ADDR_BASE + (n))

# define P039_SET                true
# define P039_RESET              false

// typically 500ns of wating on positive/negative edge of CS should be enough ( -> datasheet); to make sure we cover a lot of devices we
// spend 1ms
// FIX 2021-05-05: review of all covered device datasheets showed 2µs is more than enough; review with every newly added device
# define P039_CS_Delay() delayMicroseconds(2u)

# define P039_MAX_TYPE               PCONFIG(0)
# define P039_TC_TYPE                PCONFIG(1)
# define P039_FAM_TYPE               PCONFIG(2)
# define P039_RTD_TYPE               PCONFIG(3)
# define P039_CONFIG_4               PCONFIG(4)
# define P039_RTD_FILT_TYPE          PCONFIG(5)
# define P039_RTD_LM_TYPE            PCONFIG(6)
# define P039_RTD_LM_SHTDWN          PCONFIG(7)
# define P039_RTD_RES                PCONFIG_LONG(0)
# define P039_FLAGS                  PCONFIG_ULONG(3)
# define P039_TEMP_THRESHOLD_FLAG    0
# define P039_RTD_OFFSET             PCONFIG_FLOAT(0)
# define P039_TEMP_THRESHOLD         PCONFIG_FLOAT(1)

# define P039_TEMP_THRESHOLD_DEFAULT (-273.15f) // Default and minimum value
# define P039_TEMP_THRESHOLD_MIN     P039_TEMP_THRESHOLD_DEFAULT
# define P039_TEMP_THRESHOLD_MAX     (1000.0f)  // Max value
# define P039_TC                     0u
# define P039_RTD                    1u

# define P039_MAX6675               1
# define P039_MAX31855              2
# define P039_MAX31856              3
# define P039_MAX31865              4
# define P039_LM7x                  5

// MAX 6675 related defines

// bit masks to identify failures for MAX 6675
# define MAX6675_TC_DEVID            0x0002u
# define MAX6675_TC_OC               0x0004u

// MAX 31855 related defines

// bit masks to identify failures for MAX 31855
# define MAX31855_TC_OC              0x00000001u
# define MAX31855_TC_SC              0x00000002u
# define MAX31855_TC_SCVCC           0x00000004u
# define MAX31855_TC_GENFLT          0x00010000u


// MAX 31856 related defines

// base address for read/write acces to MAX 31856
# define MAX31856_READ_ADDR_BASE       0x00u
# define MAX31856_WRITE_ADDR_BASE      0x80u

// register offset values for MAX 31856
# define MAX31856_CR0                0u
# define MAX31856_CR1                1u
# define MAX31856_MASK               2u
# define MAX31856_CJHF               3u
# define MAX31856_CJLF               4u
# define MAX31856_LTHFTH             5u
# define MAX31856_LTHFTL             6u
# define MAX31856_LTLFTH             7u
# define MAX31856_LTLFTL             8u
# define MAX31856_CJTO               9u
# define MAX31856_CJTH               10u
# define MAX31856_CJTL               11u
# define MAX31856_LTCBH              12u
# define MAX31856_LTCBM              13u
# define MAX31856_LTCBL              14u
# define MAX31856_SR                 15u

# define MAX31856_NO_REG             16u

// bit masks to identify failures for MAX 31856
# define MAX31856_TC_OC              0x01u
# define MAX31856_TC_OVUV            0x02u
# define MAX31856_TC_TCLOW           0x04u
# define MAX31856_TC_TCLHIGH         0x08u
# define MAX31856_TC_CJLOW           0x10u
# define MAX31856_TC_CJHIGH          0x20u
# define MAX31856_TC_TCRANGE         0x40u
# define MAX31856_TC_CJRANGE         0x80u

// bit masks for access of configuration bits
# define MAX31856_SET_50HZ           0x01u
# define MAX31856_CLEAR_FAULTS       0x02u
# define MAX31856_FLT_ISR_MODE       0x04u
# define MAX31856_CJ_SENS_DISABLE    0x08u
# define MAX31856_FAULT_CTRL_MASK    0x30u
# define MAX31856_SET_ONE_SHOT       0x40u
# define MAX31856_SET_CONV_AUTO      0x80u


// RTD related defines

// MAX 31865 related defines

// waiting time until "in sequence" conversion is ready (-> used in case device is set to shutdown in between call cycles)
// typically 70ms should be fine, according to datasheet maximum -> 66ms - give a little adder to "be sure" conversion is done
// alternatively ONE SHOT bit could be polled (system/SPI bus load !)
# define MAX31865_CONVERSION_TIME    70ul
# define MAX31865_BIAS_WAIT_TIME     10ul

// MAX 31865 Main States
# define MAX31865_INIT_STATE         0u
# define MAX31865_BIAS_ON_STATE      1u
# define MAX31865_RD_STATE           2u
# define MAX31865_RDY_STATE          3u

// sensor type
# define MAX31865_PT100              0u
# define MAX31865_PT1000             1u

// base address for read/write acces to MAX 31865
# define MAX31865_READ_ADDR_BASE         0x00u
# define MAX31865_WRITE_ADDR_BASE        0x80u

// register offset values for MAX 31865
# define MAX31865_CONFIG                 0u
# define MAX31865_RTD_MSB                1u
# define MAX31865_RTD_LSB                2u
# define MAX31865_HFT_MSB                3u
# define MAX31865_HFT_LSB                4u
# define MAX31865_LFT_MSB                5u
# define MAX31865_LFT_LSB                6u
# define MAX31865_FAULT                  7u

// total number of registers in MAX 31865
# define MAX31865_NO_REG                 8u

// bit masks to identify failures for MAX 31865
# define MAX31865_FAULT_HIGHTHRESH   0x80u
# define MAX31865_FAULT_LOWTHRESH    0x40u
# define MAX31865_FAULT_REFINLOW     0x20u
# define MAX31865_FAULT_REFINHIGH    0x10u
# define MAX31865_FAULT_RTDINLOW     0x08u
# define MAX31865_FAULT_OVUV         0x04u

// bit masks for access of configuration bits
# define MAX31865_SET_50HZ           0x01u
# define MAX31865_CLEAR_FAULTS       0x02u
# define MAX31865_FAULT_CTRL_MASK    0x0Cu
# define MAX31865_SET_3WIRE          0x10u
# define MAX31865_SET_ONE_SHOT       0x20u
# define MAX31865_SET_CONV_AUTO      0x40u
# define MAX31865_SET_VBIAS_ON       0x80u

// LM7x related defines

// LM7x subtype defines
# define LM7x_SD70                   0x00u
# define LM7x_SD71                   0x01u
# define LM7x_SD74                   0x04u
# define LM7x_SD121                  0x05u
# define LM7x_SD122                  0x06u
# define LM7x_SD123                  0x07u
# define LM7x_SD124                  0x08u
# define LM7x_SD125                  0x09u

// bit masks for access of configuration bits
# define LM7x_CONV_RDY               0x02u


struct P039_data_struct : public PluginTaskData_base {
public:

  P039_data_struct() = delete;
  P039_data_struct(struct EventStruct *event);
  virtual ~P039_data_struct() = default;

  bool        begin(struct EventStruct*event);

  // Perform read and return true when an alert has been high
  bool        read(struct EventStruct *event);

  bool        plugin_tasktimer_in(struct EventStruct*event);

  static void AddMainsFrequencyFilterSelection(struct EventStruct *event);

private:

  float    readMax6675(struct EventStruct *event);
  float    readMax31855(struct EventStruct *event);
  float    readMax31856(struct EventStruct *event);
  float    readMax31865(struct EventStruct *event);
  void     MAX31865_clearFaults(int8_t l_CS_pin_no);
  void     MAX31865_setConType(int8_t  l_CS_pin_no,
                               uint8_t l_conType);
  float    convert_to_temperature(uint32_t l_rawvalue,
                                  float    RTDnominal,
                                  float    refResistor);
  uint16_t getNomResistor(uint8_t l_RType);
  int      convert_two_complement(uint32_t value,
                                  int      nr_bits);
  float    readLM7x(struct EventStruct *event);
  float    convertLM7xTemp(uint16_t l_rawValue,
                           uint16_t l_LM7xsubtype);
  uint16_t readLM7xRegisters(int8_t    l_CS_pin_no,
                             uint8_t   l_LM7xsubType,
                             uint8_t   l_runMode,
                             uint16_t *l_device_id);
  int      get_SPI_CS_Pin(struct EventStruct *event);
  void     init_SPI_CS_Pin(int8_t l_CS_pin_no);
  void     handle_SPI_CS_Pin(int8_t l_CS_pin_no,
                             bool   l_state);
  void     write8BitRegister(int8_t  l_CS_pin_no,
                             uint8_t l_address,
                             uint8_t value);
  void     write16BitRegister(int8_t   l_CS_pin_no,
                              uint8_t  l_address,
                              uint16_t value);
  uint8_t  read8BitRegister(int8_t  l_CS_pin_no,
                            uint8_t l_address);
  uint16_t read16BitRegister(int8_t  l_CS_pin_no,
                             uint8_t l_address);
  void     transfer_n_ByteSPI(int8_t   l_CS_pin_no,
                              uint8_t  l_noBytesToSend,
                              uint8_t *l_inoutMessageBuffer);
  void     change16BitRegister(int8_t   l_CS_pin_no,
                               uint8_t  l_readaddress,
                               uint8_t  l_writeaddress,
                               uint16_t l_flagmask,
                               bool     l_set_reset);
  void change8BitRegister(int8_t  l_CS_pin_no,
                          uint8_t l_readaddress,
                          uint8_t l_writeaddress,
                          uint8_t l_flagmask,
                          bool    l_set_reset);

  uint16_t      conversionResult = 0x0000u;
  uint8_t       deviceFaults     = 0x00u;
  unsigned long timer            = 0;
  bool          sensorFault      = false;
  bool          convReady        = false;

  SPIClass& _spi = SPI;
};


#endif // ifdef USES_P039
#endif // ifndef PLUGINSTRUCTS_P039_DATA_STRUCT_H
