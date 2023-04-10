#include "../WebServer/I2C_Scanner.h"

#ifdef WEBSERVER_I2C_SCANNER

#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/HTML_wrappers.h"

#include "../Globals/Settings.h"

#include "../Helpers/Hardware.h"
#include "../Helpers/StringConverter.h"



#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface I2C scanner
// ********************************************************************************

int scanI2CbusForDevices_json( // Utility function for scanning the I2C bus for valid devices, with JSON output
        int8_t muxAddr
      , int8_t channel
      , int nDevices
      #if FEATURE_I2CMULTIPLEXER
      , i2c_addresses_t &excludeDevices
      #endif // if FEATURE_I2CMULTIPLEXER
) {
  uint8_t error, address;

  for (address = 1; address <= 127; address++)
  {
    #if FEATURE_I2CMULTIPLEXER
    bool skipCheck = false;
    if (channel != -1 && excludeDevices.size() > address) {
      skipCheck = excludeDevices[address];
    }
    if (!skipCheck) { // Ignore I2C multiplexer and addresses to exclude when scanning its channels
    #endif // if FEATURE_I2CMULTIPLEXER
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      delay(1);

      if ((error == 0) || (error == 4))
      {
        json_open();
        json_prop(F("addr"), formatToHex(address, 2));
        #if FEATURE_I2CMULTIPLEXER
        if (muxAddr != -1) {
          if (channel == -1){
            json_prop(F("I2Cbus"), F("Standard I2C bus"));
            excludeDevices[address] = true;
          } else {
            String i2cChannel = F("Multiplexer channel ");
            i2cChannel += String(channel);
            json_prop(F("I2Cbus"), i2cChannel);
          }
        }
        #endif // if FEATURE_I2CMULTIPLEXER
        json_number(F("status"), String(error));

        if (error == 4) {
          json_prop(F("error"), F("Unknown error at address "));
        } else {
          String description = getKnownI2Cdevice(address);

          if (description.length() > 0) {
            json_open(true, F("known devices"));
            int pos = 0;

            while (pos >= 0) {
              int newpos = description.indexOf(',', pos);

              if (pos != 0) {
                addHtml(',');
              }

              if (newpos == -1) {
                json_quote_val(description.substring(pos));
              } else {
                json_quote_val(description.substring(pos, newpos));
              }
              pos = newpos;

              if (newpos != -1) {
                ++pos;
              }
            }
            json_close(true);
          }
          nDevices++;
        }
        json_close();
        addHtml('\n');
      }
    #if FEATURE_I2CMULTIPLEXER
    }
    #endif // if FEATURE_I2CMULTIPLEXER
  }
  return nDevices;
}

void handle_i2cscanner_json() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_i2cscanner"));
  #endif

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream();
  json_init();
  json_open(true);

  int  nDevices = 0;

  I2CSelect_Max100kHz_ClockSpeed();    // Always scan in low speed to also find old/slow devices
  #if FEATURE_I2CMULTIPLEXER
  i2c_addresses_t mainBusDevices;
  mainBusDevices.resize(128);
  for (int i = 0; i < 128; i++) {
    mainBusDevices[i] = false;
  }
  nDevices = scanI2CbusForDevices_json(Settings.I2C_Multiplexer_Addr, -1, nDevices, mainBusDevices); // Channel -1 = standard I2C bus
  #else // if FEATURE_I2CMULTIPLEXER
  nDevices = scanI2CbusForDevices_json(-1, -1, nDevices); // Standard scan
  #endif // if FEATURE_I2CMULTIPLEXER

  #if FEATURE_I2CMULTIPLEXER
  if (isI2CMultiplexerEnabled()) {
    uint8_t mux_max = I2CMultiplexerMaxChannels();
    for (int8_t channel = 0; channel < mux_max; channel++) {
      I2CMultiplexerSelect(channel);
      nDevices += scanI2CbusForDevices_json(Settings.I2C_Multiplexer_Addr, channel, nDevices, mainBusDevices); // Specific channels
    }
    I2CMultiplexerOff();
  }
  #endif // if FEATURE_I2CMULTIPLEXER
  I2CSelectHighClockSpeed(); // Reset bus to standard speed
  
  json_close(true);
  TXBuffer.endStream();
}
#endif // WEBSERVER_NEW_UI


String getKnownI2Cdevice(uint8_t address) {
  String result;

  #if FEATURE_I2C_DEVICE_SCAN
  for (uint8_t x = 0; x <= deviceCount; x++) {
    const deviceIndex_t deviceIndex = DeviceIndex_sorted[x];

    if (validDeviceIndex(deviceIndex)) {
      const pluginID_t pluginID = DeviceIndex_to_Plugin_id[deviceIndex];

      if (validPluginID(pluginID) &&
          checkPluginI2CAddressFromDeviceIndex(deviceIndex, address)) {
        result += F("(Device) ");

        # if defined(PLUGIN_BUILD_DEV) || defined(PLUGIN_SET_MAX) // Use same name as in Add Device combobox
        result += 'P';

        if (pluginID < 10) { result += '0'; }

        if (pluginID < 100) { result += '0'; }
        result += pluginID;
        result += F(" - ");
        # endif // if defined(PLUGIN_BUILD_DEV) || defined(PLUGIN_SET_MAX)
        result += getPluginNameFromDeviceIndex(deviceIndex);
        result += ',';
      }
    }
  }
  #endif // if FEATURE_I2C_DEVICE_SCAN
  #ifndef LIMIT_BUILD_SIZE

  switch (address)
  {
    case 0x10:
      result += F("VEML6075");
      break;
    case 0x11:
      result += F("VEML6075,I2C_MultiRelay");
      break;
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
      result += F("I2C_MultiRelay");
      break;
    case 0x1D:
      result +=  F("ADXL345");
      break;
    case 0x1E:
      result +=  F("HMC5883L");
      break;
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x25:
    case 0x26:
    case 0x27:
      result +=  F("PCF8574,MCP23017,LCD,PCF8575");
      break;
    case 0x23:
      result +=  F("PCF8574,MCP23017,LCD,BH1750,PCF8575");
      break;
    case 0x24:
      result +=  F("PCF8574,MCP23017,LCD,PN532,PCF8575");
      break;
    case 0x29:
      result +=  F("TSL2561,TSL2591,TCS34725,VL53L0X,VL53L1X");
      break;
    case 0x30:
      result +=  F("VL53L0X,VL53L1X");
      break;
    case 0x34:
      result +=  F("AXP192");
      break;
    case 0x36:
      result +=  F("MAX1704x,Adafruit Rotary enc");
      break;
    case 0x37:
      result +=  F("Adafruit Rotary enc");
      break;
    case 0x38:
      result +=  F("LCD,PCF8574A,AHT10/20/21,VEML6070,Adafruit Rotary enc");
      break;
    case 0x39:
      result +=  F("LCD,PCF8574A,TSL2561,APDS9960,AHT10,Adafruit Rotary enc");
      break;
    case 0x3A:
    case 0x3B:
      result +=  F("LCD,PCF8574A,Adafruit Rotary enc");
      break;
    case 0x3C:
    case 0x3D:
      result +=  F("LCD,PCF8574A,OLED,Adafruit Rotary enc");
      break;
    case 0x3E:
    case 0x3F:
      result +=  F("LCD,PCF8574A");
      break;
    case 0x40:
      result +=  F("SI7021,HTU21D,INA219,PCA9685,HDC10xx,M5Stack Rotary enc");
      break;
    case 0x41:
    case 0x42:
    case 0x43:
      result +=  F("INA219");
      break;
    case 0x44:
    case 0x45:
      result +=  F("SHT30/31/35,INA219");
      break;
    case 0x48:
    case 0x4A:
    case 0x4B:
      result +=  F("PCF8591,ADS1115,LM75A,INA219");
      break;
    case 0x49:
      result +=  F("PCF8591,ADS1115,TSL2561,LM75A,INA219");
      break;
    case 0x4C:
    case 0x4E:
    case 0x4F:
      result +=  F("PCF8591,LM75A,INA219");
      break;
    case 0x4D:
      result +=  F("PCF8591,MCP3221,LM75A,INA219");
      break;
    case 0x51:
      result +=  F("PCF8563");
      break;
    case 0x53:
      result +=  F("ADXL345,LTR390");
      break;
    case 0x55:
      result +=  F("DFRobot Rotary enc,BeFlE Moisture");
      break;
    case 0x54:
    case 0x56:
    case 0x57:
      result +=  F("DFRobot Rotary enc");
      break;
    case 0x58:
      result +=  F("SGP30");
      break;
    case 0x5A:
      result +=  F("MLX90614,MPR121,CCS811");
      break;
    case 0x5B:
      result +=  F("MPR121,CCS811");
      break;
    case 0x5C:
      result +=  F("DHT12,AM2320,BH1750,MPR121");
      break;
    case 0x5D:
      result +=  F("MPR121");
      break;
    case 0x60:
      result +=  F("Adafruit Motorshield v2,SI1145");
      break;
    case 0x61:
      result += F("Atlas EZO DO,SCD30");
      break;
    case 0x62:
      result += F("Atlas EZO ORP,SCD4x");
      break;
    case 0x63:
      result += F("Atlas EZO pH");
      break;
    case 0x64:
      result += F("Atlas EZO EC");
      break;
    case 0x68:
      result +=  F("MPU6050,DS1307,DS3231,PCF8523,ITG3205,CDM7160");
      break;
    case 0x69:
      result +=  F("ITG3205,CDM7160");
      break;
    case 0x70:
      result +=  F("Adafruit Motorshield v2 (Catchall),HT16K33,TCA9543a/6a/8a I2C multiplexer,PCA9540 I2C multiplexer");
      break;
    case 0x71:
    case 0x72:
    case 0x73:
      result +=  F("HT16K33,TCA9543a/6a/8a I2C multiplexer");
      break;
    case 0x74:
      result +=  F("HT16K33,TCA9546a/8a I2C multiplexer");
      break;
    case 0x75:
      result +=  F("HT16K33,TCA9546a/8a I2C multiplexer,IP5306");
      break;
    case 0x76:
      result +=  F("BMP280,BME280,BME680,MS5607,MS5611,HT16K33,TCA9546a/8a I2C multiplexer");
      break;
    case 0x77:
      result +=  F("BMP085,BMP180,BMP280,BME280,BME680,MS5607,MS5611,HT16K33,TCA9546a/8a I2C multiplexer");
      break;
    case 0x7f:
      result +=  F("Arduino PME");
      break;
  }
  #endif // LIMIT_BUILD_SIZE
  return result;
}

int scanI2CbusForDevices( // Utility function for scanning the I2C bus for valid devices, with HTML table output
        int8_t muxAddr
      , int8_t channel
      , int nDevices
      #if FEATURE_I2CMULTIPLEXER
      , i2c_addresses_t &excludeDevices
      #endif // if FEATURE_I2CMULTIPLEXER
) {
  uint8_t error, address;

  for (address = 1; address <= 127; address++)
  {
    #if FEATURE_I2CMULTIPLEXER
    bool skipCheck = false;
    if (channel != -1 && excludeDevices.size() > address) {
      skipCheck = excludeDevices[address];
    }
    if (!skipCheck) { // Ignore I2C multiplexer and addresses to exclude when scanning its channels
    #endif // if FEATURE_I2CMULTIPLEXER
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      delay(1);

      switch (error) {
        case 0:
      {
        html_TR_TD();
        #if FEATURE_I2CMULTIPLEXER
        if (muxAddr != -1) {
          if (channel == -1){
            addHtml(F("Standard I2C bus"));
            excludeDevices[address] = true;
          } else {
            addHtml(F("Multiplexer channel "));
            addHtmlInt(channel);
          }
          html_TD();
        }
        #endif // if FEATURE_I2CMULTIPLEXER
        addHtml(formatToHex(address, 2));
        html_TD();
        String description = getKnownI2Cdevice(address);

        if (description.length() > 0) {
          description.replace(F(","), F("<BR>"));
          addHtml(description);
        }
        nDevices++;
        break;
      }
      case 2: // NACK on transmit address, thus not found 
        break;
      case 3: 
      {
        html_TR_TD();
        addHtml(F("NACK on transmit data to address "));
        addHtml(formatToHex(address, 2));
        break;
      }
      case 4:
      {
        html_TR_TD();
        addHtml(F("SDA low at address "));
        addHtml(formatToHex(address, 2));
        I2CForceResetBus_swap_pins(address);
        addHtml(F(" Reset bus attempted"));
        break;
      }
      }
    #if FEATURE_I2CMULTIPLEXER
    }
    #endif // if FEATURE_I2CMULTIPLEXER
  }
  return nDevices;
}

// FIXME TD-er: Query all included plugins for their supported addresses (return name of plugin)
void handle_i2cscanner() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_i2cscanner"));
  #endif

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  html_table_class_multirow();
  #if FEATURE_I2CMULTIPLEXER
  if (isI2CMultiplexerEnabled()) {
    html_table_header(F("I2C bus"));
  }
  #endif // if FEATURE_I2CMULTIPLEXER
  html_table_header(F("I2C Addresses in use"));
  html_table_header(F("Supported devices"));

  if (Settings.isI2CEnabled()) {
    int  nDevices = 0;
    I2CSelect_Max100kHz_ClockSpeed();  // Scan bus using low speed
    #if FEATURE_I2CMULTIPLEXER
    i2c_addresses_t mainBusDevices;
    mainBusDevices.resize(128);
    for (int i = 0; i < 128; i++) {
      mainBusDevices[i] = false;
    }
    nDevices = scanI2CbusForDevices(Settings.I2C_Multiplexer_Addr, -1, nDevices, mainBusDevices); // Channel -1 = standard I2C bus
    #else // if FEATURE_I2CMULTIPLEXER
    nDevices = scanI2CbusForDevices(-1, -1, nDevices); // Standard scan
    #endif // if FEATURE_I2CMULTIPLEXER

    #if FEATURE_I2CMULTIPLEXER
    if (isI2CMultiplexerEnabled()) {
      uint8_t mux_max = I2CMultiplexerMaxChannels();
      for (int8_t channel = 0; channel < mux_max; channel++) {
        I2CMultiplexerSelect(channel);
        nDevices += scanI2CbusForDevices(Settings.I2C_Multiplexer_Addr, channel, nDevices, mainBusDevices);
      }
      I2CMultiplexerOff();
    }
    #endif // if FEATURE_I2CMULTIPLEXER
    I2CSelectHighClockSpeed();   // By default the bus is in standard speed

    if (nDevices == 0) {
      addHtml(F("<TR>No I2C devices found"));
    }
  } else {
    addHtml(F("<TR>I2C pins not configured"));
  }

  html_end_table();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}
#endif // WEBSERVER_I2C_SCANNER
