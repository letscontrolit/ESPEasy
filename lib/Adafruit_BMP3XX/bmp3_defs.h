/**
* Copyright (c) 2020 Bosch Sensortec GmbH. All rights reserved.
*
* BSD-3-Clause
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its
*    contributors may be used to endorse or promote products derived from
*    this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* @file       bmp3_defs.h
* @date       2020-07-20
* @version    v2.0.1
*
*/

/*! @file bmp3_defs.h
 * @brief Sensor driver for BMP3 sensor */

#ifndef BMP3_DEFS_H_
#define BMP3_DEFS_H_

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif

/********************************************************/
/* header includes */
#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/kernel.h>
#else
#include <stdint.h>
#include <stddef.h>
#endif

/*************************** Common macros   *****************************/

#if !defined(UINT8_C) && !defined(INT8_C)
#define INT8_C(x)    S8_C(x)
#define UINT8_C(x)   U8_C(x)
#endif

#if !defined(UINT16_C) && !defined(INT16_C)
#define INT16_C(x)   S16_C(x)
#define UINT16_C(x)  U16_C(x)
#endif

#if !defined(INT32_C) && !defined(UINT32_C)
#define INT32_C(x)   S32_C(x)
#define UINT32_C(x)  U32_C(x)
#endif

#if !defined(INT64_C) && !defined(UINT64_C)
#define INT64_C(x)   S64_C(x)
#define UINT64_C(x)  U64_C(x)
#endif

/**@}*/
/**\name C standard macros */
#ifndef NULL
#ifdef __cplusplus
#define NULL         0
#else
#define NULL         ((void *) 0)
#endif
#endif

#ifndef TRUE
#define TRUE         UINT8_C(1)
#endif

#ifndef FALSE
#define FALSE        UINT8_C(0)
#endif

/********************************************************/
/**\name Compiler switch macros */
/**\name Uncomment the below line to use floating-point compensation */
#ifndef BMP3_DOUBLE_PRECISION_COMPENSATION
#define BMP3_DOUBLE_PRECISION_COMPENSATION
#endif

/********************************************************/
/**\name Macro definitions */

/**
 * BMP3_INTF_RET_TYPE is the read/write interface return type which can be overwritten by the build system.
 */
#ifndef BMP3_INTF_RET_TYPE
#define BMP3_INTF_RET_TYPE                      int8_t
#endif

/**
 * The last error code from read/write interface is stored in the device structure as intf_rslt.
 */
#ifndef BMP3_INTF_RET_SUCCESS
#define BMP3_INTF_RET_SUCCESS                   INT8_C(0)
#endif

/**\name I2C addresses */
#define BMP3_ADDR_I2C_PRIM                      UINT8_C(0x76)
#define BMP3_ADDR_I2C_SEC                       UINT8_C(0x77)

/**\name BMP3 chip identifier */
#define BMP3_CHIP_ID                            UINT8_C(0x50)
#define BMP390_CHIP_ID                          UINT8_C(0x60)

/**\name BMP3 pressure settling time (micro secs)*/
#define BMP3_SETTLE_TIME_PRESS                  UINT16_C(392)

/**\name BMP3 temperature settling time (micro secs) */
#define BMP3_SETTLE_TIME_TEMP                   UINT16_C(313)

/**\name BMP3 adc conversion time (micro secs) */
#define BMP3_ADC_CONV_TIME                      UINT16_C(2000)

/**\name Register Address */
#define BMP3_REG_CHIP_ID                        UINT8_C(0x00)
#define BMP3_REG_ERR                            UINT8_C(0x02)
#define BMP3_REG_SENS_STATUS                    UINT8_C(0x03)
#define BMP3_REG_DATA                           UINT8_C(0x04)
#define BMP3_REG_EVENT                          UINT8_C(0x10)
#define BMP3_REG_INT_STATUS                     UINT8_C(0x11)
#define BMP3_REG_FIFO_LENGTH                    UINT8_C(0x12)
#define BMP3_REG_FIFO_DATA                      UINT8_C(0x14)
#define BMP3_REG_FIFO_WM                        UINT8_C(0x15)
#define BMP3_REG_FIFO_CONFIG_1                  UINT8_C(0x17)
#define BMP3_REG_FIFO_CONFIG_2                  UINT8_C(0x18)
#define BMP3_REG_INT_CTRL                       UINT8_C(0x19)
#define BMP3_REG_IF_CONF                        UINT8_C(0x1A)
#define BMP3_REG_PWR_CTRL                       UINT8_C(0x1B)
#define BMP3_REG_OSR                            UINT8_C(0X1C)
#define BMP3_REG_ODR                            UINT8_C(0x1D)
#define BMP3_REG_CONFIG                         UINT8_C(0x1F)
#define BMP3_REG_CALIB_DATA                     UINT8_C(0x31)
#define BMP3_REG_CMD                            UINT8_C(0x7E)

/**\name Error status macros */
#define BMP3_ERR_FATAL                          UINT8_C(0x01)
#define BMP3_ERR_CMD                            UINT8_C(0x02)
#define BMP3_ERR_CONF                           UINT8_C(0x04)

/**\name Status macros */
#define BMP3_CMD_RDY                            UINT8_C(0x10)
#define BMP3_DRDY_PRESS                         UINT8_C(0x20)
#define BMP3_DRDY_TEMP                          UINT8_C(0x40)

/**\name Power mode macros */
#define BMP3_MODE_SLEEP                         UINT8_C(0x00)
#define BMP3_MODE_FORCED                        UINT8_C(0x01)
#define BMP3_MODE_NORMAL                        UINT8_C(0x03)

/**\name FIFO related macros */
/**\name FIFO enable  */
#define BMP3_ENABLE                             UINT8_C(0x01)
#define BMP3_DISABLE                            UINT8_C(0x00)

/**\name Interrupt pin configuration macros */
/**\name Open drain */
#define BMP3_INT_PIN_OPEN_DRAIN                 UINT8_C(0x01)
#define BMP3_INT_PIN_PUSH_PULL                  UINT8_C(0x00)

/**\name Level */
#define BMP3_INT_PIN_ACTIVE_HIGH                UINT8_C(0x01)
#define BMP3_INT_PIN_ACTIVE_LOW                 UINT8_C(0x00)

/**\name Latch */
#define BMP3_INT_PIN_LATCH                      UINT8_C(0x01)
#define BMP3_INT_PIN_NON_LATCH                  UINT8_C(0x00)

/**\name Advance settings  */
/**\name I2c watch dog timer period selection */
#define BMP3_I2C_WDT_SHORT_1_25_MS              UINT8_C(0x00)
#define BMP3_I2C_WDT_LONG_40_MS                 UINT8_C(0x01)

/**\name FIFO Sub-sampling macros */
#define BMP3_FIFO_NO_SUBSAMPLING                UINT8_C(0x00)
#define BMP3_FIFO_SUBSAMPLING_2X                UINT8_C(0x01)
#define BMP3_FIFO_SUBSAMPLING_4X                UINT8_C(0x02)
#define BMP3_FIFO_SUBSAMPLING_8X                UINT8_C(0x03)
#define BMP3_FIFO_SUBSAMPLING_16X               UINT8_C(0x04)
#define BMP3_FIFO_SUBSAMPLING_32X               UINT8_C(0x05)
#define BMP3_FIFO_SUBSAMPLING_64X               UINT8_C(0x06)
#define BMP3_FIFO_SUBSAMPLING_128X              UINT8_C(0x07)

/**\name Over sampling macros */
#define BMP3_NO_OVERSAMPLING                    UINT8_C(0x00)
#define BMP3_OVERSAMPLING_2X                    UINT8_C(0x01)
#define BMP3_OVERSAMPLING_4X                    UINT8_C(0x02)
#define BMP3_OVERSAMPLING_8X                    UINT8_C(0x03)
#define BMP3_OVERSAMPLING_16X                   UINT8_C(0x04)
#define BMP3_OVERSAMPLING_32X                   UINT8_C(0x05)

/**\name Filter setting macros */
#define BMP3_IIR_FILTER_DISABLE                 UINT8_C(0x00)
#define BMP3_IIR_FILTER_COEFF_1                 UINT8_C(0x01)
#define BMP3_IIR_FILTER_COEFF_3                 UINT8_C(0x02)
#define BMP3_IIR_FILTER_COEFF_7                 UINT8_C(0x03)
#define BMP3_IIR_FILTER_COEFF_15                UINT8_C(0x04)
#define BMP3_IIR_FILTER_COEFF_31                UINT8_C(0x05)
#define BMP3_IIR_FILTER_COEFF_63                UINT8_C(0x06)
#define BMP3_IIR_FILTER_COEFF_127               UINT8_C(0x07)

/**\name Odr setting macros */
#define BMP3_ODR_200_HZ                         UINT8_C(0x00)
#define BMP3_ODR_100_HZ                         UINT8_C(0x01)
#define BMP3_ODR_50_HZ                          UINT8_C(0x02)
#define BMP3_ODR_25_HZ                          UINT8_C(0x03)
#define BMP3_ODR_12_5_HZ                        UINT8_C(0x04)
#define BMP3_ODR_6_25_HZ                        UINT8_C(0x05)
#define BMP3_ODR_3_1_HZ                         UINT8_C(0x06)
#define BMP3_ODR_1_5_HZ                         UINT8_C(0x07)
#define BMP3_ODR_0_78_HZ                        UINT8_C(0x08)
#define BMP3_ODR_0_39_HZ                        UINT8_C(0x09)
#define BMP3_ODR_0_2_HZ                         UINT8_C(0x0A)
#define BMP3_ODR_0_1_HZ                         UINT8_C(0x0B)
#define BMP3_ODR_0_05_HZ                        UINT8_C(0x0C)
#define BMP3_ODR_0_02_HZ                        UINT8_C(0x0D)
#define BMP3_ODR_0_01_HZ                        UINT8_C(0x0E)
#define BMP3_ODR_0_006_HZ                       UINT8_C(0x0F)
#define BMP3_ODR_0_003_HZ                       UINT8_C(0x10)
#define BMP3_ODR_0_001_HZ                       UINT8_C(0x11)

/**\name Soft reset command */
#define BMP3_SOFT_RESET                         UINT8_C(0xB6)

/**\name FIFO flush command */
#define BMP3_FIFO_FLUSH                         UINT8_C(0xB0)

/**\name API success code */
#define BMP3_OK                                 INT8_C(0)

/**\name API error codes */
#define BMP3_E_NULL_PTR                         INT8_C(-1)
#define BMP3_E_DEV_NOT_FOUND                    INT8_C(-2)
#define BMP3_E_INVALID_ODR_OSR_SETTINGS         INT8_C(-3)
#define BMP3_E_CMD_EXEC_FAILED                  INT8_C(-4)
#define BMP3_E_CONFIGURATION_ERR                INT8_C(-5)
#define BMP3_E_INVALID_LEN                      INT8_C(-6)
#define BMP3_E_COMM_FAIL                        INT8_C(-7)
#define BMP3_E_FIFO_WATERMARK_NOT_REACHED       INT8_C(-8)

/**\name API warning codes */
#define BMP3_W_SENSOR_NOT_ENABLED               UINT8_C(1)
#define BMP3_W_INVALID_FIFO_REQ_FRAME_CNT       UINT8_C(2)

/**\name Macros to select the which sensor settings are to be set by the user.
 * These values are internal for API implementation. Don't relate this to
 * data sheet. */
#define BMP3_SEL_PRESS_EN                       UINT16_C(1 << 1)
#define BMP3_SEL_TEMP_EN                        UINT16_C(1 << 2)
#define BMP3_SEL_DRDY_EN                        UINT16_C(1 << 3)
#define BMP3_SEL_PRESS_OS                       UINT16_C(1 << 4)
#define BMP3_SEL_TEMP_OS                        UINT16_C(1 << 5)
#define BMP3_SEL_IIR_FILTER                     UINT16_C(1 << 6)
#define BMP3_SEL_ODR                            UINT16_C(1 << 7)
#define BMP3_SEL_OUTPUT_MODE                    UINT16_C(1 << 8)
#define BMP3_SEL_LEVEL                          UINT16_C(1 << 9)
#define BMP3_SEL_LATCH                          UINT16_C(1 << 10)
#define BMP3_SEL_I2C_WDT_EN                     UINT16_C(1 << 11)
#define BMP3_SEL_I2C_WDT                        UINT16_C(1 << 12)
#define BMP3_SEL_ALL                            UINT16_C(0x7FF)

/**\name Macros to select the which FIFO settings are to be set by the user
 * These values are internal for API implementation. Don't relate this to
 * data sheet.*/
#define BMP3_SEL_FIFO_MODE                      UINT16_C(1 << 1)
#define BMP3_SEL_FIFO_STOP_ON_FULL_EN           UINT16_C(1 << 2)
#define BMP3_SEL_FIFO_TIME_EN                   UINT16_C(1 << 3)
#define BMP3_SEL_FIFO_PRESS_EN                  UINT16_C(1 << 4)
#define BMP3_SEL_FIFO_TEMP_EN                   UINT16_C(1 << 5)
#define BMP3_SEL_FIFO_DOWN_SAMPLING             UINT16_C(1 << 6)
#define BMP3_SEL_FIFO_FILTER_EN                 UINT16_C(1 << 7)
#define BMP3_SEL_FIFO_FWTM_EN                   UINT16_C(1 << 8)
#define BMP3_SEL_FIFO_FULL_EN                   UINT16_C(1 << 9)

/**\name Sensor component selection macros
 * These values are internal for API implementation. Don't relate this to
 * data sheet.*/
#define BMP3_PRESS                              UINT8_C(1)
#define BMP3_TEMP                               UINT8_C(1 << 1)
#define BMP3_ALL                                UINT8_C(0x03)

/**\name Macros for bit masking */
#define BMP3_ERR_FATAL_MSK                      UINT8_C(0x01)

#define BMP3_ERR_CMD_MSK                        UINT8_C(0x02)
#define BMP3_ERR_CMD_POS                        UINT8_C(0x01)

#define BMP3_ERR_CONF_MSK                       UINT8_C(0x04)
#define BMP3_ERR_CONF_POS                       UINT8_C(0x02)

#define BMP3_STATUS_CMD_RDY_MSK                 UINT8_C(0x10)
#define BMP3_STATUS_CMD_RDY_POS                 UINT8_C(0x04)

#define BMP3_STATUS_DRDY_PRESS_MSK              UINT8_C(0x20)
#define BMP3_STATUS_DRDY_PRESS_POS              UINT8_C(0x05)

#define BMP3_STATUS_DRDY_TEMP_MSK               UINT8_C(0x40)
#define BMP3_STATUS_DRDY_TEMP_POS               UINT8_C(0x06)

#define BMP3_OP_MODE_MSK                        UINT8_C(0x30)
#define BMP3_OP_MODE_POS                        UINT8_C(0x04)

#define BMP3_PRESS_EN_MSK                       UINT8_C(0x01)

#define BMP3_TEMP_EN_MSK                        UINT8_C(0x02)
#define BMP3_TEMP_EN_POS                        UINT8_C(0x01)

#define BMP3_IIR_FILTER_MSK                     UINT8_C(0x0E)
#define BMP3_IIR_FILTER_POS                     UINT8_C(0x01)

#define BMP3_ODR_MSK                            UINT8_C(0x1F)

#define BMP3_PRESS_OS_MSK                       UINT8_C(0x07)

#define BMP3_TEMP_OS_MSK                        UINT8_C(0x38)
#define BMP3_TEMP_OS_POS                        UINT8_C(0x03)

#define BMP3_FIFO_MODE_MSK                      UINT8_C(0x01)

#define BMP3_FIFO_STOP_ON_FULL_MSK              UINT8_C(0x02)
#define BMP3_FIFO_STOP_ON_FULL_POS              UINT8_C(0x01)

#define BMP3_FIFO_TIME_EN_MSK                   UINT8_C(0x04)
#define BMP3_FIFO_TIME_EN_POS                   UINT8_C(0x02)

#define BMP3_FIFO_PRESS_EN_MSK                  UINT8_C(0x08)
#define BMP3_FIFO_PRESS_EN_POS                  UINT8_C(0x03)

#define BMP3_FIFO_TEMP_EN_MSK                   UINT8_C(0x10)
#define BMP3_FIFO_TEMP_EN_POS                   UINT8_C(0x04)

#define BMP3_FIFO_FILTER_EN_MSK                 UINT8_C(0x18)
#define BMP3_FIFO_FILTER_EN_POS                 UINT8_C(0x03)

#define BMP3_FIFO_DOWN_SAMPLING_MSK             UINT8_C(0x07)

#define BMP3_FIFO_FWTM_EN_MSK                   UINT8_C(0x08)
#define BMP3_FIFO_FWTM_EN_POS                   UINT8_C(0x03)

#define BMP3_FIFO_FULL_EN_MSK                   UINT8_C(0x10)
#define BMP3_FIFO_FULL_EN_POS                   UINT8_C(0x04)

#define BMP3_INT_OUTPUT_MODE_MSK                UINT8_C(0x01)

#define BMP3_INT_LEVEL_MSK                      UINT8_C(0x02)
#define BMP3_INT_LEVEL_POS                      UINT8_C(0x01)

#define BMP3_INT_LATCH_MSK                      UINT8_C(0x04)
#define BMP3_INT_LATCH_POS                      UINT8_C(0x02)

#define BMP3_INT_DRDY_EN_MSK                    UINT8_C(0x40)
#define BMP3_INT_DRDY_EN_POS                    UINT8_C(0x06)

#define BMP3_I2C_WDT_EN_MSK                     UINT8_C(0x02)
#define BMP3_I2C_WDT_EN_POS                     UINT8_C(0x01)

#define BMP3_I2C_WDT_SEL_MSK                    UINT8_C(0x04)
#define BMP3_I2C_WDT_SEL_POS                    UINT8_C(0x02)

#define BMP3_INT_STATUS_FWTM_MSK                UINT8_C(0x01)

#define BMP3_INT_STATUS_FFULL_MSK               UINT8_C(0x02)
#define BMP3_INT_STATUS_FFULL_POS               UINT8_C(0x01)

#define BMP3_INT_STATUS_DRDY_MSK                UINT8_C(0x08)
#define BMP3_INT_STATUS_DRDY_POS                UINT8_C(0x03)

/**\name    UTILITY MACROS  */
#define BMP3_SET_LOW_BYTE                       UINT16_C(0x00FF)
#define BMP3_SET_HIGH_BYTE                      UINT16_C(0xFF00)

/**\name Macro to combine two 8 bit data's to form a 16 bit data */
#define BMP3_CONCAT_BYTES(msb, lsb)             (((uint16_t)msb << 8) | (uint16_t)lsb)

#define BMP3_SET_BITS(reg_data, bitname, data) \
    ((reg_data & ~(bitname##_MSK)) | \
     ((data << bitname##_POS) & bitname##_MSK))

/* Macro variant to handle the bitname position if it is zero */
#define BMP3_SET_BITS_POS_0(reg_data, bitname, data) \
    ((reg_data & ~(bitname##_MSK)) | \
     (data & bitname##_MSK))

#define BMP3_GET_BITS(reg_data, bitname)        ((reg_data & (bitname##_MSK)) >> \
                                                 (bitname##_POS))

/* Macro variant to handle the bitname position if it is zero */
#define BMP3_GET_BITS_POS_0(reg_data, bitname)  (reg_data & (bitname##_MSK))

#define BMP3_GET_LSB(var)                       (uint8_t)(var & BMP3_SET_LOW_BYTE)
#define BMP3_GET_MSB(var)                       (uint8_t)((var & BMP3_SET_HIGH_BYTE) >> 8)

/**\name Macros related to size */
#define BMP3_LEN_CALIB_DATA                     UINT8_C(21)
#define BMP3_LEN_P_AND_T_HEADER_DATA            UINT8_C(7)
#define BMP3_LEN_P_OR_T_HEADER_DATA             UINT8_C(4)
#define BMP3_LEN_P_T_DATA                       UINT8_C(6)
#define BMP3_LEN_GEN_SETT                       UINT8_C(7)
#define BMP3_LEN_P_DATA                         UINT8_C(3)
#define BMP3_LEN_T_DATA                         UINT8_C(3)
#define BMP3_LEN_SENSOR_TIME                    UINT8_C(3)
#define BMP3_FIFO_MAX_FRAMES                    UINT8_C(73)

/*! Power control settings */
#define BMP3_POWER_CNTL                         UINT16_C(0x0006)

/*! Odr and filter settings */
#define BMP3_ODR_FILTER                         UINT16_C(0x00F0)

/*! Interrupt control settings */
#define BMP3_INT_CTRL                           UINT16_C(0x0708)

/*! Advance settings */
#define BMP3_ADV_SETT                           UINT16_C(0x1800)

/*! FIFO settings */

/*! Mask for fifo_mode, fifo_stop_on_full, fifo_time_en, fifo_press_en and
 * fifo_temp_en settings */
#define BMP3_FIFO_CONFIG_1                      UINT16_C(0x003E)

/*! Mask for fifo_sub_sampling and data_select settings */
#define BMP3_FIFO_CONFIG_2                      UINT16_C(0x00C0)

/*! Mask for fwtm_en and ffull_en settings */
#define BMP3_FIFO_INT_CTRL                      UINT16_C(0x0300)

/*! FIFO Header */
/*! FIFO temperature pressure header frame */
#define BMP3_FIFO_TEMP_PRESS_FRAME              UINT8_C(0x94)

/*! FIFO temperature header frame */
#define BMP3_FIFO_TEMP_FRAME                    UINT8_C(0x90)

/*! FIFO pressure header frame */
#define BMP3_FIFO_PRESS_FRAME                   UINT8_C(0x84)

/*! FIFO time header frame */
#define BMP3_FIFO_TIME_FRAME                    UINT8_C(0xA0)

/*! FIFO error header frame */
#define BMP3_FIFO_ERROR_FRAME                   UINT8_C(0x44)

/*! FIFO configuration change header frame */
#define BMP3_FIFO_CONFIG_CHANGE                 UINT8_C(0x48)

/********************************************************/

/*!
 * @brief Interface selection Enums
 */
enum bmp3_intf {
    /*! SPI interface */
    BMP3_SPI_INTF,
    /*! I2C interface */
    BMP3_I2C_INTF
};

/********************************************************/

/*!
 * @brief Type definitions
 */

/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific read functions of the user
 *
 * @param[in]     reg_addr : 8bit register address of the sensor
 * @param[out]    reg_data : Data from the specified address
 * @param[in]     length   : Length of the reg_data array
 * @param[in,out] intf_ptr : Void pointer that can enable the linking of descriptors
 *                           for interface related callbacks
 * @retval 0 for Success
 * @retval Non-zero for Failure
 */
typedef BMP3_INTF_RET_TYPE (*bmp3_read_fptr_t)(uint8_t reg_addr, uint8_t *read_data, uint32_t len, void *intf_ptr);

/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific write functions of the user
 *
 * @param[in]     reg_addr : 8bit register address of the sensor
 * @param[out]    reg_data : Data to the specified address
 * @param[in]     length   : Length of the reg_data array
 * @param[in,out] intf_ptr : Void pointer that can enable the linking of descriptors
 *                           for interface related callbacks
 * @retval 0 for Success
 * @retval Non-zero for Failure
 *
 */
typedef BMP3_INTF_RET_TYPE (*bmp3_write_fptr_t)(uint8_t reg_addr, const uint8_t *read_data, uint32_t len,
                                                void *intf_ptr);

/*!
 * @brief Delay function pointer which should be mapped to
 * delay function of the user
 *
 * @param[in] period              : Delay in microseconds.
 * @param[in, out] intf_ptr       : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs
 *
 */
typedef void (*bmp3_delay_us_fptr_t)(uint32_t period, void *intf_ptr);

/********************************************************/

/*!
 * @brief Register Trim Variables
 */
struct bmp3_reg_calib_data
{
    /**
     * @ Trim Variables
     */

    /**@{*/
    uint16_t par_t1;
    uint16_t par_t2;
    int8_t par_t3;
    int16_t par_p1;
    int16_t par_p2;
    int8_t par_p3;
    int8_t par_p4;
    uint16_t par_p5;
    uint16_t par_p6;
    int8_t par_p7;
    int8_t par_p8;
    int16_t par_p9;
    int8_t par_p10;
    int8_t par_p11;
    int64_t t_lin;

    /**@}*/
};

/*!
 * @brief bmp3 advance settings
 */
struct bmp3_adv_settings
{
    /*! i2c watch dog enable */
    uint8_t i2c_wdt_en;

    /*! i2c watch dog select */
    uint8_t i2c_wdt_sel;
};

/*!
 * @brief bmp3 odr and filter settings
 */
struct bmp3_odr_filter_settings
{
    /*! Pressure oversampling */
    uint8_t press_os;

    /*! Temperature oversampling */
    uint8_t temp_os;

    /*! IIR filter */
    uint8_t iir_filter;

    /*! Output data rate */
    uint8_t odr;
};

/*!
 * @brief bmp3 sensor status flags
 */
struct bmp3_sens_status
{
    /*! Command ready status */
    uint8_t cmd_rdy;

    /*! Data ready for pressure */
    uint8_t drdy_press;

    /*! Data ready for temperature */
    uint8_t drdy_temp;
};

/*!
 * @brief bmp3 interrupt status flags
 */
struct bmp3_int_status
{
    /*! fifo watermark interrupt */
    uint8_t fifo_wm;

    /*! fifo full interrupt */
    uint8_t fifo_full;

    /*! data ready interrupt */
    uint8_t drdy;
};

/*!
 * @brief bmp3 error status flags
 */
struct bmp3_err_status
{
    /*! fatal error */
    uint8_t fatal;

    /*! command error */
    uint8_t cmd;

    /*! configuration error */
    uint8_t conf;
};

/*!
 * @brief bmp3 status flags
 */
struct bmp3_status
{
    /*! Interrupt status */
    struct bmp3_int_status intr;

    /*! Sensor status */
    struct bmp3_sens_status sensor;

    /*! Error status */
    struct bmp3_err_status err;

    /*! power on reset status */
    uint8_t pwr_on_rst;
};

/*!
 * @brief bmp3 interrupt pin settings
 */
struct bmp3_int_ctrl_settings
{
    /*! Output mode */
    uint8_t output_mode;

    /*! Active high/low */
    uint8_t level;

    /*! Latched or Non-latched */
    uint8_t latch;

    /*! Data ready interrupt */
    uint8_t drdy_en;
};

/*!
 * @brief bmp3 device settings
 */
struct bmp3_settings
{
    /*! Power mode which user wants to set */
    uint8_t op_mode;

    /*! Enable/Disable pressure sensor */
    uint8_t press_en;

    /*! Enable/Disable temperature sensor */
    uint8_t temp_en;

    /*! ODR and filter configuration */
    struct bmp3_odr_filter_settings odr_filter;

    /*! Interrupt configuration */
    struct bmp3_int_ctrl_settings int_settings;

    /*! Advance settings */
    struct bmp3_adv_settings adv_settings;
};

/*!
 * @brief bmp3 fifo frame
 */
struct bmp3_fifo_data
{
    /*! Data buffer of user defined length is to be mapped here
     * 512 + 4 */
    uint8_t *buffer;

    /*! Number of bytes of data read from the fifo */
    uint16_t byte_count;

    /*! Number of frames to be read as specified by the user */
    uint8_t req_frames;

    /*! Will be equal to length when no more frames are there to parse */
    uint16_t start_idx;

    /*! Will contain the no of parsed data frames from fifo */
    uint8_t parsed_frames;

    /*! Configuration error */
    uint8_t config_err;

    /*! Sensor time */
    uint32_t sensor_time;

    /*! FIFO input configuration change */
    uint8_t config_change;

    /*! All available frames are parsed */
    uint8_t frame_not_available;
};

/*!
 * @brief bmp3 fifo configuration
 */
struct bmp3_fifo_settings
{
    /*! enable/disable */
    uint8_t mode;

    /*! stop on full enable/disable */
    uint8_t stop_on_full_en;

    /*! time enable/disable */
    uint8_t time_en;

    /*! pressure enable/disable */
    uint8_t press_en;

    /*! temperature enable/disable */
    uint8_t temp_en;

    /*! down sampling rate */
    uint8_t down_sampling;

    /*! filter enable/disable */
    uint8_t filter_en;

    /*! FIFO watermark enable/disable */
    uint8_t fwtm_en;

    /*! FIFO full enable/disable */
    uint8_t ffull_en;
};

/*!
 * @brief bmp3 bmp3 FIFO
 */
struct bmp3_fifo
{
    /*! FIFO frame structure */
    struct bmp3_fifo_data data;

    /*! FIFO config structure */
    struct bmp3_fifo_settings settings;
};

#ifdef BMP3_DOUBLE_PRECISION_COMPENSATION

/*!
 * @brief Quantized Trim Variables
 */
struct bmp3_quantized_calib_data
{
    /**
     * @ Quantized Trim Variables
     */

    /**@{*/
    double par_t1;
    double par_t2;
    double par_t3;
    double par_p1;
    double par_p2;
    double par_p3;
    double par_p4;
    double par_p5;
    double par_p6;
    double par_p7;
    double par_p8;
    double par_p9;
    double par_p10;
    double par_p11;
    double t_lin;

    /**@}*/
};

/*!
 * @brief Calibration data
 */
struct bmp3_calib_data
{
    /*! Quantized data */
    struct bmp3_quantized_calib_data quantized_calib_data;

    /*! Register data */
    struct bmp3_reg_calib_data reg_calib_data;
};

/*!
 * @brief bmp3 sensor structure which comprises of temperature and pressure
 * data.
 */
struct bmp3_data
{
    /*! Compensated temperature */
    double temperature;

    /*! Compensated pressure */
    double pressure;
};

#else

/*!
 * @brief bmp3 sensor structure which comprises of temperature and pressure
 * data.
 */
struct bmp3_data
{
    /*! Compensated temperature */
    int64_t temperature;

    /*! Compensated pressure */
    uint64_t pressure;
};

/*!
 * @brief Calibration data
 */
struct bmp3_calib_data
{
    /*! Register data */
    struct bmp3_reg_calib_data reg_calib_data;
};

#endif /* BMP3_DOUBLE_PRECISION_COMPENSATION */

/*!
 * @brief bmp3 sensor structure which comprises of un-compensated temperature
 * and pressure data.
 */
struct bmp3_uncomp_data
{
    /*! un-compensated pressure */
    uint32_t pressure;

    /*! un-compensated temperature */
    uint32_t temperature;
};

/*!
 * @brief bmp3 device structure
 */
struct bmp3_dev
{
    /*! Chip Id */
    uint8_t chip_id;

    /*!
     * The interface pointer is used to enable the user
     * to link their interface descriptors for reference during the
     * implementation of the read and write interfaces to the
     * hardware.
     */
    void *intf_ptr;

    /*! Interface Selection
     * For SPI, interface = BMP3_SPI_INTF
     * For I2C, interface = BMP3_I2C_INTF
     **/
    enum bmp3_intf intf;

    /*! To store interface pointer error */
    BMP3_INTF_RET_TYPE intf_rslt;

    /*! Decide SPI or I2C read mechanism */
    uint8_t dummy_byte;

    /*! Read function pointer */
    bmp3_read_fptr_t read;

    /*! Write function pointer */
    bmp3_write_fptr_t write;

    /*! Delay function pointer */
    bmp3_delay_us_fptr_t delay_us;

    /*! Trim data */
    struct bmp3_calib_data calib_data;

    /*! Sensor Settings */
    struct bmp3_settings settings;

    /*! Sensor and interrupt status flags */
    struct bmp3_status status;

    /*! FIFO data and settings structure */
    struct bmp3_fifo *fifo;
};

#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif /* BMP3_DEFS_H_ */
