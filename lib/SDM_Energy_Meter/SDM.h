/* Library for reading SDM 120/220/230/630 Modbus Energy meters.
*  Reading via Hardware or Software Serial library & rs232<->rs485 converter
*  2016-2018 Reaper7 (tested on wemos d1 mini->ESP8266 with Arduino 1.9.0-beta & 2.4.1 esp8266 core)
*  crc calculation by Jaime García (https://github.com/peninquen/Modbus-Energy-Monitor-Arduino/)
*/
//------------------------------------------------------------------------------
#ifndef SDM_h
#define SDM_h
//------------------------------------------------------------------------------
#include <Arduino.h>
#include <SDM_Config_User.h>
#ifdef USE_HARDWARESERIAL
  #include <HardwareSerial.h>
#else
  #include <ESPeasySoftwareSerial.h>
//  #include <SoftwareSerial.h>
#endif
//------------------------------------------------------------------------------
//DEFAULT CONFIG (DO NOT CHANGE ANYTHING!!! for changes use SDM_Config_User.h):
//------------------------------------------------------------------------------
#ifndef SDM_UART_BAUD
  #define SDM_UART_BAUD                     4800                                //default baudrate
#endif

#ifndef DERE_PIN
  #define DERE_PIN                          NOT_A_PIN                           //default digital pin for control MAX485 DE/RE lines (connect DE & /RE together to this pin)
#endif

#ifdef USE_HARDWARESERIAL

  #ifndef SDM_UART_CONFIG
    #define SDM_UART_CONFIG                 SERIAL_8N1                          //default hardware uart config
  #endif

  #ifndef SWAPHWSERIAL
    #define SWAPHWSERIAL                    0                                   //(only esp8266) when hwserial used, then swap uart pins from 3/1 to 13/15 (default not swap)
  #endif

#endif

#ifndef MAX_MILLIS_TO_WAIT
  #define MAX_MILLIS_TO_WAIT                500                                 //default max time to wait for response from SDM
#endif
//------------------------------------------------------------------------------
#define FRAMESIZE                           9                                   //size of out/in array
#define SDM_REPLY_BYTE_COUNT                0x04                                //number of bytes with data

#define SDM_B_01                            0x01                                //BYTE 1 -> slave address (default value 1 read from node 1)
#define SDM_B_02                            0x04                                //BYTE 2 -> function code (default value 0x04 read from 3X input registers)
                                                                                //BYTES 3 & 4 (BELOW)
//SDM 120 registers
#define SDM120C_VOLTAGE                     0x0000                              //V
#define SDM120C_CURRENT                     0x0006                              //A
#define SDM120C_POWER                       0x000C                              //W
#define SDM120C_ACTIVE_APPARENT_POWER       0x0012                              //VA
#define SDM120C_REACTIVE_APPARENT_POWER     0x0018                              //VAR
#define SDM120C_POWER_FACTOR                0x001E                              //
#define SDM120C_FREQUENCY                   0x0046                              //Hz
#define SDM120C_IMPORT_ACTIVE_ENERGY        0x0048                              //Wh
#define SDM120C_EXPORT_ACTIVE_ENERGY        0x004A                              //Wh
#define SDM120C_TOTAL_ACTIVE_ENERGY         0x0156                              //Wh
//SDM 220 registers
#define SDM220T_VOLTAGE                     0x0000                              //V
#define SDM220T_CURRENT                     0x0006                              //A
#define SDM220T_POWER                       0x000C                              //W
#define SDM220T_ACTIVE_APPARENT_POWER       0x0012                              //VA
#define SDM220T_REACTIVE_APPARENT_POWER     0x0018                              //VAR
#define SDM220T_POWER_FACTOR                0x001E                              //
#define SDM220T_PHASE_ANGLE                 0x0024                              //DEGREE
#define SDM220T_FREQUENCY                   0x0046                              //Hz
#define SDM220T_IMPORT_ACTIVE_ENERGY        0x0048                              //Wh
#define SDM220T_EXPORT_ACTIVE_ENERGY        0x004A                              //Wh
#define SDM220T_IMPORT_REACTIVE_ENERGY      0x004C                              //VARh
#define SDM220T_EXPORT_REACTIVE_ENERGY      0x004E                              //VARh
#define SDM220T_TOTAL_ACTIVE_ENERGY         0x0156                              //Wh
#define SDM220T_TOTAL_REACTIVE_ENERGY       0x0158                              //VARh
//SDM 230 registers
#define SDM230_VOLTAGE                      0x0000                              //V
#define SDM230_CURRENT                      0x0006                              //A
#define SDM230_POWER                        0x000C                              //W
#define SDM230_ACTIVE_APPARENT_POWER        0x0012                              //VA
#define SDM230_REACTIVE_APPARENT_POWER      0x0018                              //VAR
#define SDM230_POWER_FACTOR                 0x001E                              //
#define SDM230_PHASE_ANGLE                  0x0024                              //DEGREE
#define SDM230_FREQUENCY                    0x0046                              //Hz
#define SDM230_IMPORT_ACTIVE_ENERGY         0x0048                              //Wh
#define SDM230_EXPORT_ACTIVE_ENERGY         0x004A                              //Wh
#define SDM230_IMPORT_REACTIVE_ENERGY       0x004C                              //VARh
#define SDM230_EXPORT_REACTIVE_ENERGY       0x004E                              //VARh
#define SDM230_TOTAL_SYSTEM_POWER_DEMAND    0x0054                              //W
#define SDM230_MAXIMUM_SYSTEM_POWER_DEMAND  0x0056                              //W
#define SDM230_CURRENT_POSITIVE_POWER_DEMAND 0x0058                             //W
#define SDM230_MAXIMUM_POSITIVE_POWER_DEMAND 0x005A                             //W
#define SDM230_CURRENT_REVERSE_POWER_DEMAND 0x005C                              //W
#define SDM230_MAXIMUM_REVERSE_POWER_DEMAND 0x005E                              //W
#define SDM230_CURRENT_DEMAND               0x0102                              //Amps
#define SDM230_MAXIMUM_CURRENT_DEMAND       0x0108                              //Amps
#define SDM230_TOTAL_ACTIVE_ENERGY          0x0156                              //kwh
#define SDM230_TOTAL_REACTIVE_ENERGY        0x0158                              //kvarh
#define SDM230_CURRENT_RESETTABLE_TOTAL_ACTIVE_ENERGY   0x0180                  //Wh
#define SDM230_CURRENT_RESETTABLE_TOTAL_REACTIVE_ENERGY 0x0182                  //VARh
//SDM 630 registers
#define SDM630_VOLTAGE1                     0x0000                              //V
#define SDM630_VOLTAGE2                     0x0002                              //V
#define SDM630_VOLTAGE3                     0x0004                              //V
#define SDM630_CURRENT1                     0x0006                              //A
#define SDM630_CURRENT2                     0x0008                              //A
#define SDM630_CURRENT3                     0x000A                              //A
#define SDM630_CURRENTSUM                   0x0030                              //A
#define SDM630_POWER1                       0x000C                              //W
#define SDM630_POWER2                       0x000E                              //W
#define SDM630_POWER3                       0x0010                              //W
#define SDM630_POWERTOTAL                   0x0034                              //W
#define SDM630_VOLT_AMPS1                   0x0012                              //VA
#define SDM630_VOLT_AMPS2                   0x0014                              //VA
#define SDM630_VOLT_AMPS3                   0x0016                              //VA
#define SDM630_VOLT_AMPS_TOTAL              0x0038                              //VA
#define SDM630_VOLT_AMPS_REACTIVE1          0x0018                              //VAr
#define SDM630_VOLT_AMPS_REACTIVE2          0x001A                              //VAr
#define SDM630_VOLT_AMPS_REACTIVE3          0x001C                              //VAr
#define SDM630_VOLT_AMPS_REACTIVE_TOTAL     0x003C                              //VAr
#define SDM630_POWER_FACTOR1                0x001E
#define SDM630_POWER_FACTOR2                0x0020
#define SDM630_POWER_FACTOR3                0x0022
#define SDM630_POWER_FACTOR_TOTAL           0x003E
#define SDM630_PHASE_ANGLE1                 0x0024                              //Degrees
#define SDM630_PHASE_ANGLE2                 0x0026                              //Degrees
#define SDM630_PHASE_ANGLE3                 0x0028                              //Degrees
#define SDM630_PHASE_ANGLE_TOTAL            0x0042                              //Degrees
#define SDM630_VOLTAGE_AVERAGE              0x002A                              //V
#define SDM630_CURRENT_AVERAGE              0x002E                              //A
#define SDM630_FREQUENCY                    0x0046                              //HZ
#define SDM630_IMPORT_ACTIVE_ENERGY         0x0048                              //Wh
#define SDM630_EXPORT_ACTIVE_ENERGY         0x004A                              //Wh
#define SDM630_IMPORT_REACTIVE_ENERGY       0x004C                              //VARh
#define SDM630_EXPORT_REACTIVE_ENERGY       0x004E                              //VARh
#define SDM630_TOTAL_SYSTEM_POWER_DEMAND    0x0054                              //W
#define SDM630_MAXIMUM_TOTAL_SYSTEM_POWER   0x0056                              //W

#define SDM_B_05                            0x00                                //BYTE 5
#define SDM_B_06                            0x02                                //BYTE 6
//------------------------------------------------------------------------------
#define SDM_ERR_NO_ERROR                    0                                   //no error
#define SDM_ERR_CRC_ERROR                   1                                   //crc error
#define SDM_ERR_WRONG_BYTES                 2                                   //bytes b0,b1 or b2 wrong
#define SDM_ERR_NOT_ENOUGHT_BYTES           3                                   //not enough bytes from sdm
#define SDM_ERR_TIMEOUT                     4                                   //timeout
//------------------------------------------------------------------------------
class SDM {
  public:
#ifdef USE_HARDWARESERIAL
    SDM(HardwareSerial& serial, long baud = SDM_UART_BAUD, int dere_pin = DERE_PIN, int config = SDM_UART_CONFIG, bool swapuart = SWAPHWSERIAL);
#else
    SDM(ESPeasySoftwareSerial& serial, long baud = SDM_UART_BAUD, int dere_pin = DERE_PIN);
#endif
    virtual ~SDM();

    void begin(void);
    float readVal(uint16_t reg, uint8_t node = SDM_B_01);                       //read value from register = reg and from deviceId = node
    uint16_t getErrCode(bool _clear = false);                                   //return last errorcode (optional clear this value, default flase)
    uint16_t getErrCount(bool _clear = false);                                  //return total errors count (optional clear this value, default flase)
    void clearErrCode();                                                        //clear last errorcode
    void clearErrCount();                                                       //clear total errors count

  private:
#ifdef USE_HARDWARESERIAL
    HardwareSerial& sdmSer;
#else
    ESPeasySoftwareSerial& sdmSer;
#endif

#ifdef USE_HARDWARESERIAL
    int _config = SDM_UART_CONFIG;
    bool _swapuart = SWAPHWSERIAL;
#endif
    long _baud = SDM_UART_BAUD;
    int _dere_pin = DERE_PIN;
    uint16_t readingerrcode = SDM_ERR_NO_ERROR;                                 //4 = timeout; 3 = not enough bytes; 2 = number of bytes OK but bytes b0,b1 or b2 wrong, 1 = crc error
    uint16_t readingerrcount = 0;                                               //total errors couter
    uint16_t calculateCRC(uint8_t *array, uint8_t num);
};
#endif //SDM_h
