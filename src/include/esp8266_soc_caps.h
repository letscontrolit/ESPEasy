#pragma once

#ifdef ESP8266

# include <cstdint>
# include <eagle_soc.h>

/******************************************************************************\
* Detect core versions *******************************************************
\******************************************************************************/

  # if defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)  || defined(ARDUINO_ESP8266_RELEASE_2_4_2)
    #  ifndef CORE_2_4_X
      #   define CORE_2_4_X
    #  endif
  # endif // if defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)  || defined(ARDUINO_ESP8266_RELEASE_2_4_2)

  # if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)
    #  ifndef CORE_PRE_2_4_2
      #   define CORE_PRE_2_4_2
    #  endif
  # endif // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)

  # if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(CORE_2_4_X)
    #  ifndef CORE_PRE_2_5_0
      #   define CORE_PRE_2_5_0
    #  endif
  # else // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(CORE_2_4_X)
    #  ifndef CORE_POST_2_5_0
      #   define CORE_POST_2_5_0
    #  endif
  # endif // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(CORE_2_4_X)


  # ifdef FORCE_PRE_2_5_0
    #  ifdef CORE_POST_2_5_0
      #   undef CORE_POST_2_5_0
    #  endif
  # endif // ifdef FORCE_PRE_2_5_0

/*
 #ifndef CORE_POST_2_5_0
 #define STR_HELPER(x) #x
 #define STR(x) STR_HELPER(x)
 #endif
 */


# define SOC_ADC_SUPPORTED           1
# define SOC_DAC_SUPPORTED           0
# define SOC_UART_SUPPORTED          1
# define SOC_MCPWM_SUPPORTED         0
# define SOC_GPTIMER_SUPPORTED       0
# define SOC_SDMMC_HOST_SUPPORTED    1
# define SOC_BT_SUPPORTED            0
# define SOC_PCNT_SUPPORTED          0
# define SOC_PHY_SUPPORTED           0
# define SOC_WIFI_SUPPORTED          1
# define SOC_SDIO_SLAVE_SUPPORTED    0
# define SOC_TWAI_SUPPORTED          1
# define SOC_EFUSE_SUPPORTED         0
# define SOC_EMAC_SUPPORTED          0
# define SOC_ULP_SUPPORTED           0
# define SOC_CCOMP_TIMER_SUPPORTED   0
# define SOC_RTC_FAST_MEM_SUPPORTED  0
# define SOC_RTC_SLOW_MEM_SUPPORTED  0
# define SOC_RTC_MEM_SUPPORTED       0
# define SOC_I2S_SUPPORTED           0
# define SOC_RMT_SUPPORTED           0
# define SOC_SDM_SUPPORTED           0
# define SOC_GPSPI_SUPPORTED         1
# define SOC_LEDC_SUPPORTED          0
# define SOC_I2C_SUPPORTED           1
# define SOC_SUPPORT_COEXISTENCE     0
# define SOC_AES_SUPPORTED           0
# define SOC_MPI_SUPPORTED           0
# define SOC_SHA_SUPPORTED           0
# define SOC_FLASH_ENC_SUPPORTED     0
# define SOC_SECURE_BOOT_SUPPORTED   0
# define SOC_TOUCH_SENSOR_SUPPORTED  0
# define SOC_BOD_SUPPORTED           0
# define SOC_ULP_FSM_SUPPORTED       0
# define SOC_CLK_TREE_SUPPORTED      0
# define SOC_MPU_SUPPORTED           0
# define SOC_WDT_SUPPORTED           0
# define SOC_SPI_FLASH_SUPPORTED     1
# define SOC_RNG_SUPPORTED           0
# define SOC_LIGHT_SLEEP_SUPPORTED   1
# define SOC_DEEP_SLEEP_SUPPORTED    1
# define SOC_PM_SUPPORTED            0

/*-------------------------- CPU CAPS ----------------------------------------*/
# define SOC_CPU_CORES_NUM               1


/*-------------------------- GPIO CAPS ---------------------------------------*/

// ESP8266 has 1 GPIO peripheral
# define SOC_GPIO_PORT                   (1U)
# define SOC_GPIO_PIN_COUNT              16


// 0~16 valid except flash pins
constexpr uint32_t SOC_GPIO_VALID_GPIO_MASK = (0x1FFFFUL & ~(0UL | BIT6 | BIT7 | BIT8 | BIT11));
# define SOC_GPIO_VALID_OUTPUT_GPIO_MASK (SOC_GPIO_VALID_GPIO_MASK)

# define SOC_GPIO_IN_RANGE_MAX           SOC_GPIO_PIN_COUNT
# define SOC_GPIO_OUT_RANGE_MAX          SOC_GPIO_PIN_COUNT

/*-------------------------- I2C CAPS ----------------------------------------*/

// ESP32 has 2 I2C
# define SOC_I2C_NUM                (1U)
# define SOC_HP_I2C_NUM             (1)

# define SOC_I2C_FIFO_LEN        (32) /*!< I2C hardware FIFO depth */
# define SOC_I2C_CMD_REG_NUM     (16) /*!< Number of I2C command registers */
# define SOC_I2C_SUPPORT_SLAVE   (0)

# define SOC_I2C_SUPPORT_APB     (0)
# define SOC_I2C_SUPPORT_10BIT_ADDR (0)

# define SOC_I2C_STOP_INDEPENDENT (0)

/*-------------------------- SPI CAPS ----------------------------------------*/
# define SOC_SPI_PERIPH_NUM     (1U)
# define SOC_SPI_SCLK           (14)
# define SOC_SPI_MISO           (12)
# define SOC_SPI_MOSI           (13)


/*-------------------------- UART CAPS ---------------------------------------*/
# define SOC_UART_NUM                (2)
# define SOC_TX0 1
# define SOC_RX0 3
# define SOC_UART_FIFO_LEN           (128)       /*!< The UART hardware FIFO length */




  # include <c_types.h>

  # ifndef CORE_POST_3_0_0
    #  ifndef IRAM_ATTR
      #   define IRAM_ATTR ICACHE_RAM_ATTR
    #  endif
  # endif // ifndef CORE_POST_3_0_0
  # ifndef SOC_WIFI_SUPPORTED
    #  define SOC_WIFI_SUPPORTED  1
  # endif
  # ifndef FEATURE_WIFI
    #  define FEATURE_WIFI        1
  # endif


/*-------------------------- GPIO CAPS ----------------------------------------*/
# ifndef GPIO_PIN_COUNT
  #  define GPIO_PIN_COUNT                      (SOC_GPIO_PIN_COUNT)
# endif

/// Check whether it is a valid GPIO number
# define GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num >= 0) && \
                                       (((1ULL << (gpio_num))&SOC_GPIO_VALID_GPIO_MASK) != 0))

/// Check whether it can be a valid GPIO number of output mode
# define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) ((gpio_num >= 0) && \
                                              (((1ULL << (gpio_num))&SOC_GPIO_VALID_OUTPUT_GPIO_MASK) != 0))

/// Check whether it can be a valid digital I/O pad
# define GPIO_IS_VALID_DIGITAL_IO_PAD(gpio_num) ((gpio_num >= 0) && \
                                                 (((1ULL << (gpio_num))&SOC_GPIO_VALID_DIGITAL_IO_PAD_MASK) != 0))


#endif // ifdef ESP8266
