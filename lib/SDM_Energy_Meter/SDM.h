/* Library for reading SDM 72/120/220/230/630 Modbus Energy meters.
*  Reading via Hardware or Software Serial library & rs232<->rs485 converter
*  2016-2023 Reaper7 (tested on wemos d1 mini->ESP8266 with Arduino 1.8.10 & 2.5.2 esp8266 core)
*  crc calculation by Jaime García (https://github.com/peninquen/Modbus-Energy-Monitor-Arduino/)
*/
//------------------------------------------------------------------------------
#ifndef SDM_h
#define SDM_h
//------------------------------------------------------------------------------
#include <Arduino.h>
#include <SDM_Config_User.h>
#if defined ( USE_HARDWARESERIAL )
  #include <HardwareSerial.h>
#else
  #include <ESPeasySerial.h>
#endif
//------------------------------------------------------------------------------
//DEFAULT CONFIG (DO NOT CHANGE ANYTHING!!! for changes use SDM_Config_User.h):
//------------------------------------------------------------------------------
#if !defined ( SDM_UART_BAUD )
  #define SDM_UART_BAUD                               4800                      //  default baudrate
#endif

#if !defined ( DERE_PIN )
  #define DERE_PIN                                    NOT_A_PIN                 //  default digital pin for control MAX485 DE/RE lines (connect DE & /RE together to this pin)
#endif

#if defined ( USE_HARDWARESERIAL )

  #if !defined ( SDM_UART_CONFIG )
    #define SDM_UART_CONFIG                           SERIAL_8N1                //  default hardware uart config
  #endif

  #if defined ( ESP8266 ) && !defined ( SWAPHWSERIAL )
    #define SWAPHWSERIAL                              0                         //  (only esp8266) when hwserial used, then swap uart pins from 3/1 to 13/15 (default not swap)
  #endif

  #if defined ( ESP32 )
    #if !defined ( SDM_RX_PIN )
      #define SDM_RX_PIN                              -1                        //  use default rx pin for selected port
    #endif
    #if !defined ( SDM_TX_PIN )
      #define SDM_TX_PIN                              -1                        //  use default tx pin for selected port
    #endif
  #endif

#else

  #if defined ( ESP8266 ) || defined ( ESP32 )
    #if !defined ( SDM_UART_CONFIG )
      #define SDM_UART_CONFIG                         SERIAL_8N1              //  default softwareware uart config for esp8266/esp32
    #endif
  #endif

//  #if !defined ( SDM_RX_PIN ) || !defined ( SDM_TX_PIN )
//    #error "SDM_RX_PIN and SDM_TX_PIN must be defined in SDM_Config_User.h for Software Serial option)"
//  #endif

  #if !defined ( SDM_RX_PIN )
    #define SDM_RX_PIN                                -1
  #endif
  #if !defined ( SDM_TX_PIN )
    #define SDM_TX_PIN                                -1
  #endif

#endif

#if !defined ( WAITING_TURNAROUND_DELAY )
  #define WAITING_TURNAROUND_DELAY                    500                       //  time in ms to wait for process current request
#endif

#if !defined ( RESPONSE_TIMEOUT )
  #define RESPONSE_TIMEOUT                            10                       //  time in ms to wait for return response from all devices before next request
#endif

#if !defined ( SDM_MIN_DELAY )
  #define SDM_MIN_DELAY                               1                        //  minimum value (in ms) for WAITING_TURNAROUND_DELAY and RESPONSE_TIMEOUT
#endif

#if !defined ( SDM_MAX_DELAY )
  #define SDM_MAX_DELAY                               20                      //  maximum value (in ms) for WAITING_TURNAROUND_DELAY and RESPONSE_TIMEOUT
#endif

//------------------------------------------------------------------------------

#define SDM_ERR_NO_ERROR                              0                         //  no error
#define SDM_ERR_ILLEGAL_FUNCTION                      1
#define SDM_ERR_ILLEGAL_DATA_ADDRESS                  2
#define SDM_ERR_ILLEGAL_DATA_VALUE                    3
#define SDM_ERR_SLAVE_DEVICE_FAILURE                  5

#define SDM_ERR_CRC_ERROR                             11                         //  crc error
#define SDM_ERR_WRONG_BYTES                           12                         //  bytes b0,b1 or b2 wrong
#define SDM_ERR_NOT_ENOUGHT_BYTES                     13                         //  not enough bytes from sdm
#define SDM_ERR_TIMEOUT                               14                         //  timeout
#define SDM_ERR_EXCEPTION                             15
#define SDM_ERR_STILL_WAITING                         16

//------------------------------------------------------------------------------

#define SDM_READ_HOLDING_REGISTER                     0x03
#define SDM_READ_INPUT_REGISTER                       0x04
#define SDM_WRITE_HOLDING_REGISTER                    0x10

#define FRAMESIZE                                     9                         //  size of out/in array
#define SDM_REPLY_BYTE_COUNT                          0x04                      //  number of bytes with data

#define SDM_B_01                                      0x01                      //  BYTE 1 -> slave address (default value 1 read from node 1)
#define SDM_B_02                                      SDM_READ_INPUT_REGISTER   //  BYTE 2 -> function code (default value 0x04 read from 3X input registers)
#define SDM_B_05                                      0x00                      //  BYTE 5
#define SDM_B_06                                      0x02                      //  BYTE 6
                                                                                //  BYTES 3 & 4 (BELOW)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
//      REGISTERS LIST FOR SDM DEVICES                                                                                                                                |
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
//      REGISTER NAME                                 REGISTER ADDRESS              UNIT        | SDM630  | SDM230  | SDM220  | SDM120CT| SDM120  | SDM72D  | SDM72 V2|
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define SDM_PHASE_1_VOLTAGE                           0x0000                    //  V           |    1    |    1    |    1    |    1    |    1    |         |    1    |         
#define SDM_PHASE_2_VOLTAGE                           0x0002                    //  V           |    1    |         |         |         |         |         |    1    |
#define SDM_PHASE_3_VOLTAGE                           0x0004                    //  V           |    1    |         |         |         |         |         |    1    |
#define SDM_PHASE_1_CURRENT                           0x0006                    //  A           |    1    |    1    |    1    |    1    |    1    |         |    1    |
#define SDM_PHASE_2_CURRENT                           0x0008                    //  A           |    1    |         |         |         |         |         |    1    |
#define SDM_PHASE_3_CURRENT                           0x000A                    //  A           |    1    |         |         |         |         |         |    1    |
#define SDM_PHASE_1_POWER                             0x000C                    //  W           |    1    |    1    |    1    |    1    |    1    |         |    1    |
#define SDM_PHASE_2_POWER                             0x000E                    //  W           |    1    |         |         |         |         |         |    1    |
#define SDM_PHASE_3_POWER                             0x0010                    //  W           |    1    |         |         |         |         |         |    1    |
#define SDM_PHASE_1_APPARENT_POWER                    0x0012                    //  VA          |    1    |    1    |    1    |    1    |    1    |         |    1    |
#define SDM_PHASE_2_APPARENT_POWER                    0x0014                    //  VA          |    1    |         |         |         |         |         |    1    |
#define SDM_PHASE_3_APPARENT_POWER                    0x0016                    //  VA          |    1    |         |         |         |         |         |    1    |
#define SDM_PHASE_1_REACTIVE_POWER                    0x0018                    //  VAr         |    1    |    1    |    1    |    1    |    1    |         |    1    |
#define SDM_PHASE_2_REACTIVE_POWER                    0x001A                    //  VAr         |    1    |         |         |         |         |         |    1    |
#define SDM_PHASE_3_REACTIVE_POWER                    0x001C                    //  VAr         |    1    |         |         |         |         |         |    1    |
#define SDM_PHASE_1_POWER_FACTOR                      0x001E                    //              |    1    |    1    |    1    |    1    |    1    |         |    1    |
#define SDM_PHASE_2_POWER_FACTOR                      0x0020                    //              |    1    |         |         |         |         |         |    1    |
#define SDM_PHASE_3_POWER_FACTOR                      0x0022                    //              |    1    |         |         |         |         |         |    1    |
#define SDM_PHASE_1_ANGLE                             0x0024                    //  Degrees     |    1    |    1    |    1    |    1    |         |         |         |
#define SDM_PHASE_2_ANGLE                             0x0026                    //  Degrees     |    1    |         |         |         |         |         |         |
#define SDM_PHASE_3_ANGLE                             0x0028                    //  Degrees     |    1    |         |         |         |         |         |         |
#define SDM_AVERAGE_L_TO_N_VOLTS                      0x002A                    //  V           |    1    |         |         |         |         |         |    1    |
#define SDM_AVERAGE_LINE_CURRENT                      0x002E                    //  A           |    1    |         |         |         |         |         |    1    |
#define SDM_SUM_LINE_CURRENT                          0x0030                    //  A           |    1    |         |         |         |         |         |    1    |
#define SDM_TOTAL_SYSTEM_POWER                        0x0034                    //  W           |    1    |         |         |         |         |    1    |    1    |
#define SDM_TOTAL_SYSTEM_APPARENT_POWER               0x0038                    //  VA          |    1    |         |         |         |         |         |    1    |
#define SDM_TOTAL_SYSTEM_REACTIVE_POWER               0x003C                    //  VAr         |    1    |         |         |         |         |         |    1    |
#define SDM_TOTAL_SYSTEM_POWER_FACTOR                 0x003E                    //              |    1    |         |         |         |         |         |    1    |
#define SDM_TOTAL_SYSTEM_PHASE_ANGLE                  0x0042                    //  Degrees     |    1    |         |         |         |         |         |         |
#define SDM_FREQUENCY                                 0x0046                    //  Hz          |    1    |    1    |    1    |    1    |    1    |         |    1    |
#define SDM_IMPORT_ACTIVE_ENERGY                      0x0048                    //  kWh/MWh     |    1    |    1    |    1    |    1    |    1    |    1    |    1    |
#define SDM_EXPORT_ACTIVE_ENERGY                      0x004A                    //  kWh/MWh     |    1    |    1    |    1    |    1    |    1    |    1    |    1    |
#define SDM_IMPORT_REACTIVE_ENERGY                    0x004C                    //  kVArh/MVArh |    1    |    1    |    1    |    1    |    1    |         |         |
#define SDM_EXPORT_REACTIVE_ENERGY                    0x004E                    //  kVArh/MVArh |    1    |    1    |    1    |    1    |    1    |         |         |
#define SDM_VAH_SINCE_LAST_RESET                      0x0050                    //  kVAh/MVAh   |    1    |         |         |         |         |         |         |
#define SDM_AH_SINCE_LAST_RESET                       0x0052                    //  Ah/kAh      |    1    |         |         |         |         |         |         |
#define SDM_TOTAL_SYSTEM_POWER_DEMAND                 0x0054                    //  W           |    1    |    1    |         |         |         |         |         |
#define SDM_MAXIMUM_TOTAL_SYSTEM_POWER_DEMAND         0x0056                    //  W           |    1    |    1    |         |         |         |         |         |
#define SDM_CURRENT_SYSTEM_POSITIVE_POWER_DEMAND      0x0058                    //  W           |         |    1    |         |         |         |         |         |
#define SDM_MAXIMUM_SYSTEM_POSITIVE_POWER_DEMAND      0x005A                    //  W           |         |    1    |         |         |         |         |         |
#define SDM_CURRENT_SYSTEM_REVERSE_POWER_DEMAND       0x005C                    //  W           |         |    1    |         |         |         |         |         |
#define SDM_MAXIMUM_SYSTEM_REVERSE_POWER_DEMAND       0x005E                    //  W           |         |    1    |         |         |         |         |         |
#define SDM_TOTAL_SYSTEM_VA_DEMAND                    0x0064                    //  VA          |    1    |         |         |         |         |         |         |
#define SDM_MAXIMUM_TOTAL_SYSTEM_VA_DEMAND            0x0066                    //  VA          |    1    |         |         |         |         |         |         |
#define SDM_NEUTRAL_CURRENT_DEMAND                    0x0068                    //  A           |    1    |         |         |         |         |         |         |
#define SDM_MAXIMUM_NEUTRAL_CURRENT                   0x006A                    //  A           |    1    |         |         |         |         |         |         |
#define SDM_REACTIVE_POWER_DEMAND                     0x006C                    //  VAr         |    1    |         |         |         |         |         |         |
#define SDM_MAXIMUM_REACTIVE_POWER_DEMAND             0x006E                    //  VAr         |    1    |         |         |         |         |         |         |
#define SDM_LINE_1_TO_LINE_2_VOLTS                    0x00C8                    //  V           |    1    |         |         |         |         |         |    1    |
#define SDM_LINE_2_TO_LINE_3_VOLTS                    0x00CA                    //  V           |    1    |         |         |         |         |         |    1    |
#define SDM_LINE_3_TO_LINE_1_VOLTS                    0x00CC                    //  V           |    1    |         |         |         |         |         |    1    |
#define SDM_AVERAGE_LINE_TO_LINE_VOLTS                0x00CE                    //  V           |    1    |         |         |         |         |         |    1    |
#define SDM_NEUTRAL_CURRENT                           0x00E0                    //  A           |    1    |         |         |         |         |         |    1    |
#define SDM_PHASE_1_LN_VOLTS_THD                      0x00EA                    //  %           |    1    |         |         |         |         |         |         |
#define SDM_PHASE_2_LN_VOLTS_THD                      0x00EC                    //  %           |    1    |         |         |         |         |         |         |
#define SDM_PHASE_3_LN_VOLTS_THD                      0x00EE                    //  %           |    1    |         |         |         |         |         |         |
#define SDM_PHASE_1_CURRENT_THD                       0x00F0                    //  %           |    1    |         |         |         |         |         |         |
#define SDM_PHASE_2_CURRENT_THD                       0x00F2                    //  %           |    1    |         |         |         |         |         |         |
#define SDM_PHASE_3_CURRENT_THD                       0x00F4                    //  %           |    1    |         |         |         |         |         |         |
#define SDM_AVERAGE_LINE_TO_NEUTRAL_VOLTS_THD         0x00F8                    //  %           |    1    |         |         |         |         |         |         |
#define SDM_AVERAGE_LINE_CURRENT_THD                  0x00FA                    //  %           |    1    |         |         |         |         |         |         |
#define SDM_TOTAL_SYSTEM_POWER_FACTOR_INV             0x00FE                    //              |    1    |         |         |         |         |         |         |
#define SDM_PHASE_1_CURRENT_DEMAND                    0x0102                    //  A           |    1    |    1    |         |         |         |         |         |
#define SDM_PHASE_2_CURRENT_DEMAND                    0x0104                    //  A           |    1    |         |         |         |         |         |         |
#define SDM_PHASE_3_CURRENT_DEMAND                    0x0106                    //  A           |    1    |         |         |         |         |         |         |
#define SDM_MAXIMUM_PHASE_1_CURRENT_DEMAND            0x0108                    //  A           |    1    |    1    |         |         |         |         |         |
#define SDM_MAXIMUM_PHASE_2_CURRENT_DEMAND            0x010A                    //  A           |    1    |         |         |         |         |         |         |
#define SDM_MAXIMUM_PHASE_3_CURRENT_DEMAND            0x010C                    //  A           |    1    |         |         |         |         |         |         |
#define SDM_LINE_1_TO_LINE_2_VOLTS_THD                0x014E                    //  %           |    1    |         |         |         |         |         |         |
#define SDM_LINE_2_TO_LINE_3_VOLTS_THD                0x0150                    //  %           |    1    |         |         |         |         |         |         |
#define SDM_LINE_3_TO_LINE_1_VOLTS_THD                0x0152                    //  %           |    1    |         |         |         |         |         |         |
#define SDM_AVERAGE_LINE_TO_LINE_VOLTS_THD            0x0154                    //  %           |    1    |         |         |         |         |         |         |
#define SDM_TOTAL_ACTIVE_ENERGY                       0x0156                    //  kWh         |    1    |    1    |    1    |    1    |    1    |    1    |    1    |
#define SDM_TOTAL_REACTIVE_ENERGY                     0x0158                    //  kVArh       |    1    |    1    |    1    |    1    |    1    |         |    1    |
#define SDM_L1_IMPORT_ACTIVE_ENERGY                   0x015A                    //  kWh         |    1    |         |         |         |         |         |         |
#define SDM_L2_IMPORT_ACTIVE_ENERGY                   0x015C                    //  kWh         |    1    |         |         |         |         |         |         |
#define SDM_L3_IMPORT_ACTIVE_ENERGY                   0x015E                    //  kWh         |    1    |         |         |         |         |         |         |
#define SDM_L1_EXPORT_ACTIVE_ENERGY                   0x0160                    //  kWh         |    1    |         |         |         |         |         |         |
#define SDM_L2_EXPORT_ACTIVE_ENERGY                   0x0162                    //  kWh         |    1    |         |         |         |         |         |         |
#define SDM_L3_EXPORT_ACTIVE_ENERGY                   0x0164                    //  kWh         |    1    |         |         |         |         |         |         |
#define SDM_L1_TOTAL_ACTIVE_ENERGY                    0x0166                    //  kWh         |    1    |         |         |         |         |         |         |
#define SDM_L2_TOTAL_ACTIVE_ENERGY                    0x0168                    //  kWh         |    1    |         |         |         |         |         |         |
#define SDM_L3_TOTAL_ACTIVE_ENERGY                    0x016a                    //  kWh         |    1    |         |         |         |         |         |         |
#define SDM_L1_IMPORT_REACTIVE_ENERGY                 0x016C                    //  kVArh       |    1    |         |         |         |         |         |         |
#define SDM_L2_IMPORT_REACTIVE_ENERGY                 0x016E                    //  kVArh       |    1    |         |         |         |         |         |         |
#define SDM_L3_IMPORT_REACTIVE_ENERGY                 0x0170                    //  kVArh       |    1    |         |         |         |         |         |         |
#define SDM_L1_EXPORT_REACTIVE_ENERGY                 0x0172                    //  kVArh       |    1    |         |         |         |         |         |         |
#define SDM_L2_EXPORT_REACTIVE_ENERGY                 0x0174                    //  kVArh       |    1    |         |         |         |         |         |         |
#define SDM_L3_EXPORT_REACTIVE_ENERGY                 0x0176                    //  kVArh       |    1    |         |         |         |         |         |         |
#define SDM_L1_TOTAL_REACTIVE_ENERGY                  0x0178                    //  kVArh       |    1    |         |         |         |         |         |         |
#define SDM_L2_TOTAL_REACTIVE_ENERGY                  0x017A                    //  kVArh       |    1    |         |         |         |         |         |         |
#define SDM_L3_TOTAL_REACTIVE_ENERGY                  0x017C                    //  kVArh       |    1    |         |         |         |         |         |         |
#define SDM_CURRENT_RESETTABLE_TOTAL_ACTIVE_ENERGY    0x0180                    //  kWh         |         |    1    |         |         |         |    1    |    1    |
#define SDM_CURRENT_RESETTABLE_TOTAL_REACTIVE_ENERGY  0x0182                    //  kVArh       |         |    1    |         |         |         |         |         |
#define SDM_CURRENT_RESETTABLE_IMPORT_ENERGY          0x0184                    //  kWh         |         |         |         |         |         |    1    |    1    |
#define SDM_CURRENT_RESETTABLE_EXPORT_ENERGY          0x0186                    //  kWh         |         |         |         |         |         |    1    |    1    |
#define SDM_CURRENT_RESETTABLE_IMPORT_REACTIVE_ENERGY 0x0188                    //  kVArh       |         |         |         |         |         |    1    |    1    |
#define SDM_CURRENT_RESETTABLE_EXPORT_REACTIVE_ENERGY 0x018A                    //  kVArh       |         |         |         |         |         |    1    |    1    |
#define SDM_NET_KWH                                   0x018C                    //  kWh         |         |         |         |         |         |         |    1    |
#define SDM_NET_KVARH                                 0x018E                    //  kVArh       |         |         |         |         |         |         |    1    |
#define SDM_IMPORT_POWER                              0x0500                    //  W           |         |         |         |         |         |    1    |    1    |
#define SDM_EXPORT_POWER                              0x0502                    //  W           |         |         |         |         |         |    1    |    1    |
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
//      REGISTERS LIST FOR DDM DEVICE                                                                     |
//---------------------------------------------------------------------------------------------------------
//      REGISTER NAME                                 REGISTER ADDRESS              UNIT        | DDM18SD |
//---------------------------------------------------------------------------------------------------------
#define DDM_PHASE_1_VOLTAGE                           0x0000                    //  V           |    1    |
#define DDM_PHASE_1_CURRENT                           0x0008                    //  A           |    1    |
#define DDM_PHASE_1_POWER                             0x0012                    //  W           |    1    |
#define DDM_PHASE_1_REACTIVE_POWER                    0x001A                    //  VAr         |    1    |
#define DDM_PHASE_1_POWER_FACTOR                      0x002A                    //              |    1    |
#define DDM_FREQUENCY                                 0x0036                    //  Hz          |    1    |
#define DDM_IMPORT_ACTIVE_ENERGY                      0x0100                    //  kWh         |    1    |
#define DDM_IMPORT_REACTIVE_ENERGY                    0x0400                    //  kVArh       |    1    |
//---------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------
//      REGISTERS LIST FOR TAC2100 DEVICE                                                                     |
//-------------------------------------------------------------------------------------------------------------
//      REGISTER NAME                                 REGISTER ADDRESS              UNIT            | TAC2100 |
//-------------------------------------------------------------------------------------------------------------
#define TAC2100_PHASE_1_VOLTAGE                           0x0000                    //  V           |    1    |  
#define TAC2100_PHASE_1_CURRENT                           0x0006                    //  A           |    1    |
#define TAC2100_PHASE_1_POWER                             0x000C                    //  W           |    1    |
#define TAC2100_PHASE_1_APPARENT_POWER                    0x0012                    //  VA          |    1    |
#define TAC2100_PHASE_1_REACTIVE_POWER                    0x0018                    //  VAr         |    1    |
#define TAC2100_PHASE_1_POWER_FACTOR                      0x001E                    //              |    1    |
#define TAC2100_PHASE_1_ANGLE                             0x0024                    //  Degrees     |    1    |
#define TAC2100_FREQUENCY                                 0x0030                    //  Hz          |    1    |
#define TAC2100_IMPORT_ACTIVE_ENERGY                      0x0500                    //  kWh/MWh     |    1    |
#define TAC2100_EXPORT_ACTIVE_ENERGY                      0x0502                    //  kWh/MWh     |    1    |
#define TAC2100_IMPORT_REACTIVE_ENERGY                    0x0508                    //  kVArh/MVArh |    1    |
#define TAC2100_EXPORT_REACTIVE_ENERGY                    0x050A                    //  kVArh/MVArh |    1    |
#define TAC2100_TOTAL_SYSTEM_POWER_DEMAND                 0x008C                    //  W           |    1    |
#define TAC2100_MAXIMUM_TOTAL_SYSTEM_POWER_DEMAND         0x00A2                    //  W           |    1    |
#define TAC2100_CURRENT_SYSTEM_POSITIVE_POWER_DEMAND      0x009A                    //  W           |    1    |
#define TAC2100_MAXIMUM_SYSTEM_POSITIVE_POWER_DEMAND      0x00B0                    //  W           |    1    |
#define TAC2100_CURRENT_SYSTEM_REVERSE_POWER_DEMAND       0x009C                    //  W           |    1    |
#define TAC2100_MAXIMUM_SYSTEM_REVERSE_POWER_DEMAND       0x00B2                    //  W           |    1    |
#define TAC2100_PHASE_1_CURRENT_DEMAND                    0x0092                    //  A           |    1    |
#define TAC2100_MAXIMUM_PHASE_1_CURRENT_DEMAND            0x00A8                    //  A           |    1    |
#define TAC2100_TOTAL_ACTIVE_ENERGY                       0x0504                    //  kWh         |    1    |
#define TAC2100_TOTAL_REACTIVE_ENERGY                     0x050C                    //  kVArh       |    1    |
#define TAC2100_REACTIVE_POWER_DEMAND                     0x008E                    //  VAr         |    1    |
#define TAC2100_APPARENT_POWER_DEMAND                     0x0090                    //  VA          |    1    |
#define TAC2100_MAX_REACTIVE_POWER_DEMAND                 0x00A4                    //  VAr         |    1    |
#define TAC2100_MAX_APPARENT_POWER_DEMAND                 0x00A6                    //  VA          |    1    |
#define TAC2100_NATURE_OF_LOAD                            0x004E                    //              |    1    |
//TAC2100_NATURE_OF_LOAD: (Resistive=1, inductive=2, capacitive=3, Non Load=4)                                |
//-------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
//      REGISTERS LIST FOR DEVICE SETTINGS                                                                |
//---------------------------------------------------------------------------------------------------------
//      REGISTER NAME                                 REGISTER ADDRESS              UNIT        | DEVNAME |
//---------------------------------------------------------------------------------------------------------

// Read minutes into first demand calculation. 
// When the Demand Time reaches the Demand Period
// then the demand values are valid.
#define SDM_HOLDING_DEMAND_TIME                       0x0000

// Write demand period: 0~60 minutes.
// Default 60.
// Range: 0~60, 0 means function disabled
#define SDM_HOLDING_DEMAND_PERIOD                     0x0002

// Write relay on period in milliseconds: 
// 60, 100 or 200 ms.
// default: 100 ms
#define SDM_HOLDING_RELAY_PULSE_WIDTH                 0x000C

// Parity / stop bit settings:
// 0 = One stop bit and no parity, default. 
// 1 = One stop bit and even parity. 
// 2 = One s top bit and odd parity.
// 3 = Two stop bits and no parity.
// Requires a restart to become effective.
#define SDM_HOLDING_NETWORK_PARITY_STOP               0x0012  

// Ranges from 1 to 247. Default ID is 1.
#define SDM_HOLDING_METER_ID                          0x0014

// Write the network port baud rate for MODBUS Protocol, where:
/*
SDM120 / SDM230:
  0 = 2400 baud (default)
  1 = 4800 baud
  2 = 9600 baud
  5 = 1200 baud

SDM320 / SDM530Y:
  0 = 2400 baud
  1 = 4800 baud
  2 = 9600 baud (default)
  5 = 1200 band

SDM630 / SDM72 / SDM72V2:
  0 = 2400 baud
  1 = 4800 baud
  2 = 9600 baud (default)
  3 = 19200 baud
  4 = 38400 baud
*/
#define SDM_HOLDING_BAUD_RATE                         0x001C

// Write MODBUS Protocol input parameter for pulse out 1:
// 1: Import active energy
// 2: Import + export (total) active energy 
// 4: Export active energy (default).
// 5: Import reactive energy
// 6: Import + export (total) reactive energy
// 8: Export reactive energy
#define SDM_HOLDING_PULSE_1_OUTPUT_MODE               0x0056



#define SDM_HOLDING_SERIAL_NUMBER                     0xFC00
#define SDM_HOLDING_SOFTWARE_VERSION                  0xFC03



//-----------------------------------------------------------------------------------------------------------------------------------------------------------

class SDM {
  public:
#if defined ( USE_HARDWARESERIAL )                                              //  hardware serial
  #if defined ( ESP8266 )                                                       //  on esp8266
    SDM(HardwareSerial& serial, long baud = SDM_UART_BAUD, int dere_pin = DERE_PIN, int config = SDM_UART_CONFIG, bool swapuart = SWAPHWSERIAL);
  #elif defined ( ESP32 )                                                       //  on esp32
    SDM(HardwareSerial& serial, long baud = SDM_UART_BAUD, int dere_pin = DERE_PIN, int config = SDM_UART_CONFIG, int8_t rx_pin = SDM_RX_PIN, int8_t tx_pin = SDM_TX_PIN);
  #else                                                                         //  on avr
    SDM(HardwareSerial& serial, long baud = SDM_UART_BAUD, int dere_pin = DERE_PIN, int config = SDM_UART_CONFIG);
  #endif
#else                                                                           //  software serial
    SDM(ESPeasySerial& serial, long baud = SDM_UART_BAUD, int dere_pin = DERE_PIN);
#endif
    virtual ~SDM();

    void begin(void);
    float readVal(uint16_t reg, uint8_t node = SDM_B_01);                       //  read value from register = reg and from deviceId = node
    void startReadVal(uint16_t reg, uint8_t node = SDM_B_01, uint8_t functionCode = SDM_B_02);                   //  Start sending out the request to read a register from a specific node (allows for async access)
    uint16_t readValReady(uint8_t node = SDM_B_01, uint8_t functionCode = SDM_B_02);                             //  Check to see if a reply is ready reading from a node (allow for async access)
    float decodeFloatValue() const;

    float readHoldingRegister(uint16_t reg, uint8_t node = SDM_B_01);
    bool writeHoldingRegister(float value, uint16_t reg, uint8_t node = SDM_B_01);

    uint32_t getSerialNumber(uint8_t node = SDM_B_01);


    uint16_t getErrCode(bool _clear = false);                                   //  return last errorcode (optional clear this value, default flase)
    uint32_t getErrCount(bool _clear = false);                                  //  return total errors count (optional clear this value, default flase)
    uint32_t getSuccCount(bool _clear = false);                                 //  return total success count (optional clear this value, default false)
    void clearErrCode();                                                        //  clear last errorcode
    void clearErrCount();                                                       //  clear total errors count
    void clearSuccCount();                                                      //  clear total success count
    void setMsTurnaround(uint16_t _msturnaround = WAITING_TURNAROUND_DELAY);    //  set new value for WAITING_TURNAROUND_DELAY (ms), min=SDM_MIN_DELAY, max=SDM_MAX_DELAY
    void setMsTimeout(uint16_t _mstimeout = RESPONSE_TIMEOUT);                  //  set new value for RESPONSE_TIMEOUT (ms), min=SDM_MIN_DELAY, max=SDM_MAX_DELAY
    uint16_t getMsTurnaround();                                                 //  get current value of WAITING_TURNAROUND_DELAY (ms)
    uint16_t getMsTimeout();                                                    //  get current value of RESPONSE_TIMEOUT (ms)

  private:

    bool validChecksum(const uint8_t* data, size_t messageLength) const;

    void modbusWrite(uint8_t* data, size_t messageLength);

#if defined ( USE_HARDWARESERIAL )
    HardwareSerial& sdmSer;
#else
    ESPeasySerial& sdmSer;
#endif

#if defined ( USE_HARDWARESERIAL )
    int _config = SDM_UART_CONFIG;
  #if defined ( ESP8266 )
    bool _swapuart = SWAPHWSERIAL;
  #elif defined ( ESP32 )
    int8_t _rx_pin = -1;
    int8_t _tx_pin = -1;
  #endif
#else
  #if defined ( ESP8266 ) || defined ( ESP32 )
    int _config = SDM_UART_CONFIG;
  #endif
    int8_t _rx_pin = -1;
    int8_t _tx_pin = -1; 
#endif
    long _baud = SDM_UART_BAUD;
    int _dere_pin = DERE_PIN;
    uint16_t readingerrcode = SDM_ERR_NO_ERROR;                                 //  4 = timeout; 3 = not enough bytes; 2 = number of bytes OK but bytes b0,b1 or b2 wrong, 1 = crc error
    uint16_t msturnaround = WAITING_TURNAROUND_DELAY;
    uint16_t mstimeout = RESPONSE_TIMEOUT;
    uint32_t readingerrcount = 0;                                               //  total errors counter
    uint32_t readingsuccesscount = 0;                                           //  total success counter
    unsigned long resptime = 0;
    uint8_t sdmarr[FRAMESIZE] = {};
    uint16_t calculateCRC(const uint8_t *array, uint8_t len) const;
    void flush(unsigned long _flushtime = 0);                                   //  read serial if any old data is available or for a given time in ms
    void dereSet(bool _state = LOW);                                            //  for control MAX485 DE/RE pins, LOW receive from SDM, HIGH transmit to SDM
};
#endif // SDM_h
