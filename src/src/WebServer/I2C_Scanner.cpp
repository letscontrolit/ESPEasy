#include "../WebServer/I2C_Scanner.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/HTML_wrappers.h"

#include "../Globals/Settings.h"

#include "../Helpers/Hardware.h"
#include "../Helpers/StringConverter.h"


#ifdef WEBSERVER_I2C_SCANNER

#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface I2C scanner
// ********************************************************************************

int scanI2CbusForDevices_json( // Utility function for scanning the I2C bus for valid devices, with JSON output
        int8_t muxAddr
      , int8_t channel
      , int nDevices
#ifdef FEATURE_I2CMULTIPLEXER
      , i2c_addresses_t &excludeDevices
#endif
) {
  byte error, address;

  for (address = 1; address <= 127; address++)
  {
#ifdef FEATURE_I2CMULTIPLEXER
    bool skipCheck = false;
    if (channel != -1 && excludeDevices.size() > address) {
      skipCheck = excludeDevices[address];
    }
    if (!skipCheck) { // Ignore I2C multiplexer and addresses to exclude when scanning its channels
#endif
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      delay(1);

      if ((error == 0) || (error == 4))
      {
        json_open();
        json_prop(F("addr"), String(formatToHex(address)));
#ifdef FEATURE_I2CMULTIPLEXER
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
#endif
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
                addHtml(",");
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
        addHtml("\n");
      }
#ifdef FEATURE_I2CMULTIPLEXER
    }
#endif
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

  I2CSelectClockSpeed(true);    // Always scan in low speed to also find old/slow devices
#ifdef FEATURE_I2CMULTIPLEXER
  i2c_addresses_t mainBusDevices;
  mainBusDevices.resize(128);
  for (int i = 0; i < 128; i++) {
    mainBusDevices[i] = false;
  }
  nDevices = scanI2CbusForDevices_json(Settings.I2C_Multiplexer_Addr, -1, nDevices, mainBusDevices); // Channel -1 = standard I2C bus
#else
  nDevices = scanI2CbusForDevices_json(-1, -1, nDevices); // Standard scan
#endif

#ifdef FEATURE_I2CMULTIPLEXER
  if (isI2CMultiplexerEnabled()) {
    uint8_t mux_max = I2CMultiplexerMaxChannels();
    for (int8_t channel = 0; channel < mux_max; channel++) {
      I2CMultiplexerSelect(channel);
      nDevices += scanI2CbusForDevices_json(Settings.I2C_Multiplexer_Addr, channel, nDevices, mainBusDevices); // Specific channels
    }
    I2CMultiplexerOff();
  }
#endif
  I2CSelectClockSpeed(false); // Reset bus to standard speed
  
  json_close(true);
  TXBuffer.endStream();
}
#endif // WEBSERVER_NEW_UI


String getKnownI2Cdevice(byte address) {
  String result;
  #ifndef LIMIT_BUILD_SIZE

  switch (address)
  {
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x25:
    case 0x26:
    case 0x27:
      result =  F("PCF8574,MCP23017,LCD");
      break;
    case 0x23:
      result =  F("PCF8574,MCP23017,LCD,BH1750");
      break;
    case 0x24:
      result =  F("PCF8574,MCP23017,LCD,PN532");
      break;
    case 0x29:
      result =  F("TSL2561,TCS34725");
      break;
    case 0x38:
    case 0x3A:
    case 0x3B:
    case 0x3E:
    case 0x3F:
      result =  F("PCF8574A");
      break;
    case 0x39:
      result =  F("PCF8574A,TSL2561,APDS9960");
      break;
    case 0x3C:
    case 0x3D:
      result =  F("PCF8574A,OLED");
      break;
    case 0x40:
      result =  F("SI7021,HTU21D,INA219,PCA9685,HDC1080");
      break;
    case 0x41:
    case 0x42:
    case 0x43:
      result =  F("INA219");
      break;
    case 0x44:
    case 0x45:
      result =  F("SHT30/31/35");
      break;
    case 0x48:
    case 0x4A:
    case 0x4B:
      result =  F("PCF8591,ADS1115,LM75A");
      break;
    case 0x49:
      result =  F("PCF8591,ADS1115,TSL2561,LM75A");
      break;
    case 0x4C:
    case 0x4E:
    case 0x4F:
      result =  F("PCF8591,LM75A");
      break;
    case 0x4D:
      result =  F("PCF8591,MCP3221,LM75A");
      break;
    case 0x58:
      result =  F("SGP30");
      break;
    case 0x5A:
      result =  F("MLX90614,MPR121,CCS811");
      break;
    case 0x5B:
      result =  F("MPR121,CCS811");
      break;
    case 0x5C:
      result =  F("DHT12,AM2320,BH1750,MPR121");
      break;
    case 0x5D:
      result =  F("MPR121");
      break;
    case 0x60:
      result =  F("Adafruit Motorshield v2,SI1145");
      break;
    case 0x70:
      result =  F("Adafruit Motorshield v2 (Catchall),HT16K33,TCA9543a/6a/8a I2C multiplexer,PCA9540 I2C multiplexer");
      break;
    case 0x71:
    case 0x72:
    case 0x73:
      result =  F("HT16K33,TCA9543a/6a/8a I2C multiplexer");
      break;
    case 0x74:
    case 0x75:
      result =  F("HT16K33,TCA9546a/8a I2C multiplexer");
      break;
    case 0x76:
      result =  F("BMP280,BME280,BME680,MS5607,MS5611,HT16K33,TCA9546a/8a I2C multiplexer");
      break;
    case 0x77:
      result =  F("BMP085,BMP180,BMP280,BME280,BME680,MS5607,MS5611,HT16K33,TCA9546a/8a I2C multiplexer");
      break;
    case 0x7f:
      result =  F("Arduino PME");
      break;
  }
  #endif // LIMIT_BUILD_SIZE
  return result;
}

int scanI2CbusForDevices( // Utility function for scanning the I2C bus for valid devices, with HTML table output
        int8_t muxAddr
      , int8_t channel
      , int nDevices
#ifdef FEATURE_I2CMULTIPLEXER
      , i2c_addresses_t &excludeDevices
#endif
) {
  byte error, address;

  for (address = 1; address <= 127; address++)
  {
#ifdef FEATURE_I2CMULTIPLEXER
    bool skipCheck = false;
    if (channel != -1 && excludeDevices.size() > address) {
      skipCheck = excludeDevices[address];
    }
    if (!skipCheck) { // Ignore I2C multiplexer and addresses to exclude when scanning its channels
#endif
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      delay(1);

      if (error == 0)
      {
        html_TR_TD();
#ifdef FEATURE_I2CMULTIPLEXER
        if (muxAddr != -1) {
          if (channel == -1){
            addHtml(F("Standard I2C bus"));
            excludeDevices[address] = true;
          } else {
            String i2cChannel = F("Multiplexer channel ");
            i2cChannel += String(channel);
            addHtml(i2cChannel);
          }
          html_TD();
        }
#endif
        addHtml(formatToHex(address));
        html_TD();
        String description = getKnownI2Cdevice(address);

        if (description.length() > 0) {
          description.replace(F(","), F("<BR>"));
          addHtml(description);
        }
        nDevices++;
      }
      else if (error == 4)
      {
        html_TR_TD();
        addHtml(F("Unknown error at address "));
        addHtml(formatToHex(address));
      }
#ifdef FEATURE_I2CMULTIPLEXER
    }
#endif
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
#ifdef FEATURE_I2CMULTIPLEXER
  if (isI2CMultiplexerEnabled()) {
    html_table_header(F("I2C bus"));
  }
#endif
  html_table_header(F("I2C Addresses in use"));
  html_table_header(F("Supported devices"));

  int  nDevices = 0;
  I2CSelectClockSpeed(true);  // Scan bus using low speed
#ifdef FEATURE_I2CMULTIPLEXER
  i2c_addresses_t mainBusDevices;
  mainBusDevices.resize(128);
  for (int i = 0; i < 128; i++) {
    mainBusDevices[i] = false;
  }
  nDevices = scanI2CbusForDevices(Settings.I2C_Multiplexer_Addr, -1, nDevices, mainBusDevices); // Channel -1 = standard I2C bus
#else
  nDevices = scanI2CbusForDevices(-1, -1, nDevices); // Standard scan
#endif

#ifdef FEATURE_I2CMULTIPLEXER
  if (isI2CMultiplexerEnabled()) {
    uint8_t mux_max = I2CMultiplexerMaxChannels();
    for (int8_t channel = 0; channel < mux_max; channel++) {
      I2CMultiplexerSelect(channel);
      nDevices += scanI2CbusForDevices(Settings.I2C_Multiplexer_Addr, channel, nDevices, mainBusDevices);
    }
    I2CMultiplexerOff();
  }
#endif
  I2CSelectClockSpeed(false);   // By default the bus is in standard speed

  if (nDevices == 0) {
    addHtml(F("<TR>No I2C devices found"));
  }

  html_end_table();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}
#endif // WEBSERVER_I2C_SCANNER
