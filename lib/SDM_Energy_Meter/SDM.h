/* Template library for reading SDM 120/220/630 Modbus Energy meters.
*  Reading via Software Serial library & rs232<->rs485 converter
*  2016 Reaper7 (tested on wemos d1 mini->ESP8266 with Arduino 1.6.9 & 2.3.0 esp8266 core)
*  crc calculation by Jaime García (https://github.com/peninquen/Modbus-Energy-Monitor-Arduino/)
*/

//#define USE_HARDWARESERIAL                                                    //option - use hardware serial

#ifndef SDM_h
#define SDM_h
//------------------------------------------------------------------------------
#include <Arduino.h>
#if !defined ( USE_HARDWARESERIAL )
#include <SoftwareSerial.h>
#endif
//------------------------------------------------------------------------------
#define SDM_BAUD                            4800                                //baudrate
#define MAX_MILLIS_TO_WAIT                  500                                 //max time to wait for response from SDM

#if !defined ( USE_HARDWARESERIAL )
#define SDMSER_RX_PIN                       13                                  //RX-D7(wemos)-13
#define SDMSER_TX_PIN                       15                                  //TX-D8(wemos)-15
#else
#define SWAPHWSERIAL                        0                                   //when hwserial used, then swap uart pins from 3/1 to 13/15
#endif

#define DERE_PIN                            NOT_A_PIN                           //digital pin for control MAX485 DE/RE lines (connect DE & /RE together to this pin)

#define FRAMESIZE                           9                                   //size of out/in array
//------------------------------------------------------------------------------
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
#if !defined ( USE_HARDWARESERIAL )
template <long _speed = SDM_BAUD, int _rx_pin = SDMSER_RX_PIN, int _tx_pin = SDMSER_TX_PIN, int _dere_pin = DERE_PIN>
#else
template <long _speed = SDM_BAUD, int _dere_pin = DERE_PIN, bool _swapuart = SWAPHWSERIAL>
#endif
struct SDM {

#if !defined ( USE_HARDWARESERIAL )
#if defined ( ESP8266 )
  SoftwareSerial sdmSer = SoftwareSerial(_rx_pin, _tx_pin, false, 32);          //for esp8266 SoftwareSerial (https://github.com/plerup/espsoftwareserial)
#else
  SoftwareSerial sdmSer = SoftwareSerial(_rx_pin, _tx_pin);                     //for standard avr SoftwareSerial library
#endif
#else
#if defined ( ESP8266 )
  HardwareSerial sdmSer = HardwareSerial(0);
#else
  #error Hardware Serial option available only for ESP8266.
#endif
#endif

  private:

    uint16_t readingerrcode = SDM_ERR_NO_ERROR;                                 //4 = timeout; 3 = not enough bytes; 2 = number of bytes OK but bytes b0,b1 or b2 wrong, 1 = crc error
    uint16_t readingerrcount = 0;                                               //total errors couter 

    uint16_t calculateCRC(uint8_t *array, uint8_t num) {
      uint16_t temp, flag;
      temp = 0xFFFF;
      for (uint8_t i = 0; i < num; i++) {
        temp = temp ^ array[i];
        for (uint8_t j = 8; j; j--) {
          flag = temp & 0x0001;
          temp >>= 1;
          if (flag)
            temp ^= 0xA001;
        }
      }
      return temp;
    };

  public:

    void clearErrCode() {                                                       //clear last errorcode 
      readingerrcode = SDM_ERR_NO_ERROR;
    };

    void clearErrCount() {                                                      //clear total errors count 
      readingerrcount = 0;
    };

    uint16_t getErrCode(bool _clear = false) {                                  //return last errorcode (optional clear this value, default flase)
      uint16_t _tmp = readingerrcode;
      if (_clear == true)
        clearErrCode();
      return (_tmp);
    };

    uint16_t getErrCount(bool _clear = false) {                                 //return total errors count (optional clear this value, default flase)
      uint16_t _tmp = readingerrcount;
      if (_clear == true)
        clearErrCount();
      return (_tmp);
    };

    void begin() {
      sdmSer.begin(_speed);
#if defined ( USE_HARDWARESERIAL )
      if (_swapuart)
        sdmSer.swap();
#endif
      if (_dere_pin != NOT_A_PIN)                                               //set output pin mode for DE/RE pin when used (for control MAX485)
        pinMode(_dere_pin, OUTPUT);
    };

    float readVal(uint16_t reg, uint8_t node = SDM_B_01) {                      //read value from register = reg and from deviceId = node
      uint16_t temp;
      unsigned long resptime;
      uint8_t sdmarr[FRAMESIZE] = {node, SDM_B_02, 0, 0, SDM_B_05, SDM_B_06, 0, 0, 0};
      float res = NAN;
      uint16_t readErr = SDM_ERR_NO_ERROR;

      sdmarr[2] = highByte(reg);
      sdmarr[3] = lowByte(reg);

      temp = calculateCRC(sdmarr, FRAMESIZE - 3);                               //calculate out crc only from first 6 bytes

      sdmarr[6] = lowByte(temp);
      sdmarr[7] = highByte(temp);

#if !defined ( USE_HARDWARESERIAL )
      sdmSer.listen();                                                          //enable softserial rx interrupt
#endif

      while (sdmSer.available() > 0)  {                                         //read serial if any old data is available
        sdmSer.read();
      }

      if (_dere_pin != NOT_A_PIN)                                               //transmit to SDM  -> DE Enable, /RE Disable (for control MAX485)
        digitalWrite(_dere_pin, HIGH);

      delay(2);                                                                 //fix for issue (nan reading) by sjfaustino: https://github.com/reaper7/SDM_Energy_Meter/issues/7#issuecomment-272111524

      sdmSer.write(sdmarr, FRAMESIZE - 1);                                      //send 8 bytes

      sdmSer.flush();                                                           //clear out tx buffer

      if (_dere_pin != NOT_A_PIN)                                               //receive from SDM -> DE Disable, /RE Enable (for control MAX485)
        digitalWrite(_dere_pin, LOW);

      resptime = millis() + MAX_MILLIS_TO_WAIT;

      while (sdmSer.available() < FRAMESIZE) {
        if (resptime < millis()) {
          readErr = SDM_ERR_TIMEOUT;                                            //err debug (4)
          break;
        }
        yield();
      }

      if (readErr == SDM_ERR_NO_ERROR) {                                        //if no timeout...

        if(sdmSer.available() == FRAMESIZE) {
          for(int n=0; n<FRAMESIZE; n++) {
            sdmarr[n] = sdmSer.read();
          }

          if (sdmarr[0] == node && sdmarr[1] == SDM_B_02 && sdmarr[2] == SDM_REPLY_BYTE_COUNT) {

            if ((calculateCRC(sdmarr, FRAMESIZE - 2)) == ((sdmarr[8] << 8) | sdmarr[7])) {  //calculate crc from first 7 bytes and compare with received crc (bytes 7 & 8)
              ((uint8_t*)&res)[3]= sdmarr[3];
              ((uint8_t*)&res)[2]= sdmarr[4];
              ((uint8_t*)&res)[1]= sdmarr[5];
              ((uint8_t*)&res)[0]= sdmarr[6];
            } else {
              readErr = SDM_ERR_CRC_ERROR;                                      //err debug (1)
            }
          } else {
            readErr = SDM_ERR_WRONG_BYTES;                                      //err debug (2)
          }
        } else {
          readErr = SDM_ERR_NOT_ENOUGHT_BYTES;                                  //err debug (3)
        }
      }

      if (readErr != SDM_ERR_NO_ERROR) {                                        //if error then copy temp error value to global val and increment global error counter
        readingerrcode = readErr;
        readingerrcount++; 
      }

#if !defined ( USE_HARDWARESERIAL )
      sdmSer.end();                                                             //disable softserial rx interrupt
#endif

      return (res);
    };

};
#endif
