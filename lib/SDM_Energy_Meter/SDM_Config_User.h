/* Library for reading SDM 120/220/230/630 Modbus Energy meters.
*  Reading via Hardware or Software Serial library & rs232<->rs485 converter
*  2016-2018 Reaper7 (tested on wemos d1 mini->ESP8266 with Arduino 1.9.0-beta & 2.4.1 esp8266 core)
*  crc calculation by Jaime García (https://github.com/peninquen/Modbus-Energy-Monitor-Arduino/)
*/
//USER CONFIG:
//------------------------------------------------------------------------------
/*
#undef USE_HARDWARESERIAL                                                       //undefine USE_HARDWARESERIAL
*/
// or
/*
#define USE_HARDWARESERIAL                                                      //define USE_HARDWARESERIAL
*/
//------------------------------------------------------------------------------
/*
#define SDM_UART_BAUD                       9600                                //define user baudrate
*/
//------------------------------------------------------------------------------
/*
#define DERE_PIN                            NOT_A_PIN                           //define user DERE_PIN for control MAX485 DE/RE lines (connect DE & /RE together to this pin)
*/
//------------------------------------------------------------------------------
#ifdef USE_HARDWARESERIAL
/*
  #define SDM_UART_CONFIG                   SERIAL_8N1                          //define user SDM_UART_CONFIG
*/
/*
  #define SWAPHWSERIAL                      0                                   //define user SWAPHWSERIAL, if true(1) then swap uart pins from 3/1 to 13/15 (only ESP8266)
*/
#endif
//------------------------------------------------------------------------------
/*
#define MAX_MILLIS_TO_WAIT                  500                                 //define user MAX_MILLIS_TO_WAIT to wait for response from SDM
*/
//------------------------------------------------------------------------------