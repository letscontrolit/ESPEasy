#ifndef HELPERS_MODBUS_RTU_H
#define HELPERS_MODBUS_RTU_H


#include <Arduino.h>
#include <ESPeasySerial.h>


#define MODBUS_RECEIVE_BUFFER 256
#define MODBUS_BROADCAST_ADDRESS 0xFE

#define MODBUS_READ_HOLDING_REGISTERS 0x03
#define MODBUS_READ_INPUT_REGISTERS   0x04
#define MODBUS_WRITE_SINGLE_REGISTER  0x06
#define MODBUS_WRITE_MULTIPLE_REGISTERS  0x10

#define MODBUS_CMD_READ_RAM      0x44
#define MODBUS_CMD_READ_EEPROM   0x46
#define MODBUS_CMD_WRITE_RAM     0x41
#define MODBUS_CMD_WRITE_EEPROM  0x43


#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION        1
#define MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS    2
#define MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE      3
#define MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE 4
#define MODBUS_EXCEPTION_ACKNOWLEDGE             5
#define MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY    6
#define MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE    7
#define MODBUS_EXCEPTION_MEMORY_PARITY           8
#define MODBUS_EXCEPTION_NOT_DEFINED             9
#define MODBUS_EXCEPTION_GATEWAY_PATH            10
#define MODBUS_EXCEPTION_GATEWAY_TARGET          11

/* Additional error codes for the processCommand return values */
#define MODBUS_BADCRC   (MODBUS_EXCEPTION_GATEWAY_TARGET + 1)
#define MODBUS_BADDATA  (MODBUS_EXCEPTION_GATEWAY_TARGET + 2)
#define MODBUS_BADEXC   (MODBUS_EXCEPTION_GATEWAY_TARGET + 3)
#define MODBUS_UNKEXC   (MODBUS_EXCEPTION_GATEWAY_TARGET + 4)
#define MODBUS_MDATA    (MODBUS_EXCEPTION_GATEWAY_TARGET + 5)
#define MODBUS_BADSLAVE (MODBUS_EXCEPTION_GATEWAY_TARGET + 6)
#define MODBUS_TIMEOUT  (MODBUS_EXCEPTION_GATEWAY_TARGET + 7)
#define MODBUS_NODATA   (MODBUS_EXCEPTION_GATEWAY_TARGET + 8)


struct ModbusRTU_struct  {
  ModbusRTU_struct() = default;

  ~ModbusRTU_struct();

  void reset();

  bool init(const ESPEasySerialPort port,
            const int16_t serial_rx,
            const int16_t serial_tx,
            int16_t       baudrate,
            uint8_t          address);

  bool init(const ESPEasySerialPort port,
            const int16_t serial_rx,
            const int16_t serial_tx,
            int16_t       baudrate,
            uint8_t          address,
            int8_t        dere_pin);

  bool isInitialized() const;

  void getStatistics(uint32_t& pass,
                     uint32_t& fail,
                     uint32_t& nodata) const;

  void     setModbusTimeout(uint16_t timeout);

  uint16_t getModbusTimeout() const;

  String   getDevice_description(uint8_t slaveAddress);

  // Read from RAM or EEPROM
  void     buildRead_RAM_EEPROM(uint8_t  slaveAddress,
                                uint8_t  functionCode,
                                short startAddress,
                                uint8_t  number_bytes);

  // Write to the Special Control Register (SCR)
  void buildWriteCommandRegister(uint8_t slaveAddress,
                                 uint8_t value);

  void buildWriteMult16bRegister(uint8_t     slaveAddress,
                                 uint16_t startAddress,
                                 uint16_t value);

  void buildFrame(uint8_t  slaveAddress,
                  uint8_t  functionCode,
                  short startAddress,
                  short parameter);

  void build_modbus_MEI_frame(uint8_t slaveAddress,
                              uint8_t device_id,
                              uint8_t object_id);

  String MEI_objectid_to_name(uint8_t object_id);

  String parse_modbus_MEI_response(unsigned int& object_value_int,
                                   uint8_t        & next_object_id,
                                   bool        & more_follows,
                                   uint8_t        & conformity_level);

  void     logModbusException(uint8_t value);

  /*
     String log_buffer(uint8_t *buffer, int length) {
      String log;
      log.reserve(3 * length + 5);
      for (int i = 0; i < length; ++i) {
        String hexvalue(buffer[i], HEX);
        hexvalue.toUpperCase();
        log += hexvalue;
        log += ' ';
      }
      log += '(';
      log += length;
      log += ')';
      return log;
     }
   */
  uint8_t     processCommand();

  uint32_t read_32b_InputRegister(short address);

  uint32_t read_32b_HoldingRegister(short address);

  float    read_float_HoldingRegister(short address);

  int      readInputRegister(short address,
                             uint8_t& errorcode);

  int      readHoldingRegister(short address,
                               uint8_t& errorcode);

  // Write to holding register.
  int writeSingleRegister(short address,
                          short value);

  int writeSingleRegister(short address,
                          short value,
                          uint8_t& errorcode);

  // Function 16 (0x10) "Write Multiple Registers" to write to a single holding register
  int  writeMultipleRegisters(short address,
                              short value);

  uint8_t modbus_get_MEI(uint8_t          slaveAddress,
                      uint8_t          object_id,
                      String      & result,
                      unsigned int& object_value_int,
                      uint8_t        & next_object_id,
                      bool        & more_follows,
                      uint8_t        & conformity_level);

  void modbus_log_MEI(uint8_t slaveAddress);

  int  process_16b_register(uint8_t  slaveAddress,
                            uint8_t  functionCode,
                            short startAddress,
                            short parameter,
                            uint8_t& errorcode);

  // Still writing single register, but calling it using "Preset Multiple Registers" function (FC=16)
  int preset_mult16b_register(uint8_t     slaveAddress,
                              uint16_t startAddress,
                              uint16_t value);

  bool process_32b_register(uint8_t      slaveAddress,
                            uint8_t      functionCode,
                            short     startAddress,
                            uint32_t& result);

  int          writeSpecialCommandRegister(uint8_t command);

  unsigned int read_RAM_EEPROM(uint8_t  command,
                               uint8_t  startAddress,
                               uint8_t  nrBytes,
                               uint8_t& errorcode);

  // Compute the MODBUS RTU CRC
  static unsigned int ModRTU_CRC(uint8_t *buf,
                                 int   len);

  uint32_t            readTypeId();

  uint32_t            readSensorId();

  uint8_t             getLastError() const;

  uint32_t            getFailedReadsSinceLastValid() const;

  String detected_device_description;

private:

  void startWrite();

  void startRead();

  uint8_t     _sendframe[12]                   = { 0 };
  uint8_t     _sendframe_used                  = 0;
  uint8_t     _recv_buf[MODBUS_RECEIVE_BUFFER] = { 0 };
  uint8_t     _recv_buf_used                   = 0;
  uint8_t     _modbus_address                  = MODBUS_BROADCAST_ADDRESS;
  int8_t   _dere_pin                        = -1;
  uint32_t _reads_pass                      = 0;
  uint32_t _reads_crc_failed                = 0;
  uint32_t _reads_nodata                    = 0; // This will be reset as soon as a valid packet has been received.
  uint16_t _modbus_timeout                  = 180;
  uint8_t  _last_error                      = 0;

  ESPeasySerial *easySerial = nullptr;
};


#endif // HELPERS_MODBUS_RTU_H
