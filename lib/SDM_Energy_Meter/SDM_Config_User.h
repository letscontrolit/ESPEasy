/* Library for reading SDM 72/120/220/230/630 Modbus Energy meters.
*  Reading via Hardware or Software Serial library & rs232<->rs485 converter
*  2016-2023 Reaper7 (tested on wemos d1 mini->ESP8266 with Arduino 1.8.10 & 2.5.2 esp8266 core)
*  crc calculation by Jaime García (https://github.com/peninquen/Modbus-Energy-Monitor-Arduino/)
*/

/*
*  USER CONFIG:
*/

//------------------------------------------------------------------------------

/*
*  define or undefine USE_HARDWARESERIAL (uncomment only one or none)
*/
//#undef USE_HARDWARESERIAL
//#define USE_HARDWARESERIAL

//------------------------------------------------------------------------------

/*
*  define user baudrate
*/
//#define SDM_UART_BAUD                       9600

//------------------------------------------------------------------------------

/*
*  define user SDM_RX_PIN and SDM_TX_PIN for esp/avr Software Serial option
*  or ESP32 with Hardware Serial if default core pins are not suitable
*/
#if defined ( USE_HARDWARESERIAL )
  #if defined ( ESP32 )
    #define SDM_RX_PIN                        13
    #define SDM_TX_PIN                        15
  #endif
#else
  #if defined ( ESP8266 ) || defined ( ESP32 )
    #define SDM_RX_PIN                        13
    #define SDM_TX_PIN                        15
  #else
    #define SDM_RX_PIN                        10
    #define SDM_TX_PIN                        11
  #endif
#endif

//------------------------------------------------------------------------------

/*
*  define user DERE_PIN for control MAX485 DE/RE lines (connect DE & /RE together to this pin)
*/
//#define DERE_PIN                            NOT_A_PIN

//------------------------------------------------------------------------------

#if defined ( USE_HARDWARESERIAL )

  /*
  *  define user SDM_UART_CONFIG for hardware serial
  */
  //#define SDM_UART_CONFIG                   SERIAL_8N1

  //----------------------------------------------------------------------------

  /*
  *  define user SWAPHWSERIAL, if true(1) then swap uart pins from 3/1 to 13/15 (only ESP8266)
  */
  //#define SWAPHWSERIAL                      0

#else

  /*
  *  define user SDM_UART_CONFIG for software serial
  */
  //#define SDM_UART_CONFIG                   SWSERIAL_8N1

#endif

//------------------------------------------------------------------------------

/*
*  define user WAITING_TURNAROUND_DELAY time in ms to wait for process current request
*/
//#define WAITING_TURNAROUND_DELAY            200

//------------------------------------------------------------------------------

/*
*  define user RESPONSE_TIMEOUT time in ms to wait for return response from all devices before next request
*/
//#define RESPONSE_TIMEOUT                    500

//------------------------------------------------------------------------------
