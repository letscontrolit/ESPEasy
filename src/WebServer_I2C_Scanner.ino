#ifdef WEBSERVER_I2C_SCANNER

#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface I2C scanner
// ********************************************************************************
void handle_i2cscanner_json() {
  checkRAM(F("handle_i2cscanner"));

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream();
  json_init();
  json_open(true);

  byte error, address;
  int  nDevices = 0;

  for (address = 1; address <= 127; address++)
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if ((error == 0) || (error == 4))
    {
      json_open();
      json_prop(F("addr"), String(formatToHex(address)));
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

            if (newpos != -1)
            {
              ++pos;
            }
          }
          json_close(true);
        }
        nDevices++;
      }
      json_close();
    }
  }
  json_close(true);
  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI


// FIXME TD-er: Query all included plugins for their supported addresses (return name of plugin)
String getKnownI2Cdevice(byte address) {
  String result;

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
      result =  F("TSL2561");
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
      result =  F("Adafruit Motorshield v2 (Catchall),HT16K33");
      break;
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
      result =  F("HT16K33");
      break;
    case 0x76:
      result =  F("BME280,BMP280,MS5607,MS5611,HT16K33");
      break;
    case 0x77:
      result =  F("BMP085,BMP180,BME280,BMP280,MS5607,MS5611,HT16K33");
      break;
    case 0x7f:
      result =  F("Arduino PME");
      break;
  }
  return result;
}

void handle_i2cscanner() {
  checkRAM(F("handle_i2cscanner"));

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  html_table_class_multirow();
  html_table_header(F("I2C Addresses in use"));
  html_table_header(F("Supported devices"));

  byte error, address;
  int  nDevices = 0;

  for (address = 1; address <= 127; address++)
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      html_TR_TD();
      addHtml(formatToHex(address));
      html_TD();
      String description = getKnownI2Cdevice(address);

      if (description.length() > 0) {
        description.replace(",", "<BR>");
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
  }

  if (nDevices == 0) {
    addHtml(F("<TR>No I2C devices found"));
  }

  html_end_table();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // WEBSERVER_I2C_SCANNER
