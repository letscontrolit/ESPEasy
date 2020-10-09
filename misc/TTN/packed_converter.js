// Converter for device payload encoder "PACKED"
// copy&paste to TTN Console -> Applications -> PayloadFormat -> Converter

function Converter(decoded, port) {

    var converted = decoded;
    var name = "";

    if (port === 1) {
        if ('header' in converted) {
            switch (converted.header.plugin_id) {
                case 1:
                    converted.name = "Switch";
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    break;

                case 2:
                    converted.name = "ADC";
                    converted.analog  = converted.val_1;
                    break;

                case 3:
                    converted.name = "Pulse";
                    converted.count  = converted.val_1;
                    converted.total  = converted.val_2;
                    converted.time  = converted.val_3;
                    break;

                case 4:
                    converted.name = "Dallas";
                    converted.temp  = converted.val_1;
                    break;

                case 5:
                    converted.name = "DHT";
                    converted.temp  = converted.val_1;
                    converted.hum  = converted.val_2;
                    break;

                case 6:
                    converted.name = "BMP085";
                    converted.temp  = converted.val_1;
                    converted.pressure  = converted.val_2;
                    break;

                case 7:
                    converted.name = "PCF8591";
                    converted.analog  = converted.val_1;
                    break;

                case 8:
                    converted.name = "RFID";
                    {
                        // Not sure if it is present, since the sensor only has a single output value in ESPeasy.
                        var ulongvalue = converted.val_2 * 65536 + converted.val_1;
                        converted.tag = ulongvalue.toFixed(0);
                    }
                    break;

                case 9:
                    converted.name = "MCP";
                    converted.state  = converted.val_1;
                    break;

                case 10:
                    converted.name = "BH1750";
                    converted.lux  = converted.val_1;
                    break;

                case 11:
                    converted.name = "PME";
                    converted.v1  = converted.val_1;
                    break;

                case 12:
                    converted.name = "LCD";
                    // Should not output anything, echo the values just to be sure.
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    break;

                case 13:
                    converted.name = "HCSR04";
                    converted.dist  = converted.val_1;
                    break;

                case 14:
                    converted.name = "SI7021";
                    converted.temp  = converted.val_1;
                    converted.hum   = converted.val_2;
                    break;

                case 15:
                    converted.name = "TSL2561";
                    // Not sure if the last value is present.
                    converted.lux  = converted.val_1;
                    converted.infrared  = converted.val_2;
                    converted.broadband  = converted.val_3;
                    converted.ratio  = converted.val_4;
                    break;

                case 16:
                    converted.name = "IR_rx";
                    converted.IR  = converted.val_1;
                    break;

                case 17:
                    converted.name = "PN532";
                    {
                        // Not sure if it is present, since the sensor only has a single output value in ESPeasy.
                        var ulongvalue = converted.val_2 * 65536 + converted.val_1;
                        converted.tag = ulongvalue.toFixed(0);
                    }
                    break;

                case 18:
                    converted.name = "GP2Y10";
                    converted.dust  = converted.val_1;
                    break;

                case 19:
                    converted.name = "PCF8574";
                    converted.state  = converted.val_1;
                    break;

                case 20:
                    converted.name = "Ser2Net";
                    // Not sure if this one does output a useful value
                    converted.ser2net  = converted.val_1;
                    break;

                case 21:
                    converted.name = "Level";
                    converted.output  = converted.val_1;
                    break;

                case 22:
                    converted.name = "PCA9685";
                    // Should not output any value
                    /*
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    */
                    break;

                case 23:
                    converted.name = "OLED";
                    // Should not output any value
                    /*
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    */
                   break;

                case 24:
                    converted.name = "MLX90614";
                    converted.temp  = converted.val_1;
                    break;

                case 25:
                    converted.name = "ADS1115";
                    converted.analog  = converted.val_1;
                    break;

                case 26:
                    converted.name = "Sysinfo";
                    /*
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    */
                    converted.ip = [converted.ip1, converted.ip2, converted.ip3, converted.ip4].join('.');
                    break;

                case 27:
                    converted.name = "INA219";
                    converted.volt  = converted.val_1;
                    converted.current  = converted.val_2;
                    converted.power  = converted.val_3;
                    break;

                case 28:
                    converted.name = "BME280";
                    converted.temp  = converted.val_1;
                    converted.hum = converted.val_2;
                    converted.pressure = converted.val_3;
                    break;

                case 29:
                    converted.name = "MQTThelper";
                    converted.output  = converted.val_1;
                    break;

                case 30:
                    converted.name = "BMP280";
                    converted.temp  = converted.val_1;
                    converted.pressure  = converted.val_2;
                    break;

                case 31:
                    converted.name = "SHT1X";
                    converted.temp  = converted.val_1;
                    converted.hum  = converted.val_2;
                    break;

                case 32:
                    converted.name = "MS5611";
                    converted.temp  = converted.val_1;
                    converted.pressure  = converted.val_2;
                    break;

                case 33:
                    converted.name = "Dummy";
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    break;

                case 34:
                    converted.name = "DHT12";
                    converted.temp  = converted.val_1;
                    converted.hum  = converted.val_2;
                    break;

                case 35:
                    converted.name = "IRTX";
                    // Should not output anything
                    /*
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    */
                    break;

                case 36:
                    converted.name = "FrameOLED";
                    // Should not output anything
                    /*
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    */
                   break;

                case 37:
                    converted.name = "MQTTImport";
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    break;

                case 38:
                    converted.name = "NeoPixel";
                    // Should not output anything
                    /*
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    */
                   break;

                case 39:
                    converted.name = "Thermocouple";
                    converted.temp  = converted.val_1;
                    break;

                case 40:
                    converted.name = "ID12";
                    {
                        // Not sure if it is present, since the sensor only has a single output value in ESPeasy.
                        var ulongvalue = converted.val_2 * 65536 + converted.val_1;
                        converted.tag = ulongvalue.toFixed(0);
                    }
                    break;

                case 41:
                    converted.name = "NeoClock";
                    // Should not output anything
                    /*
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    */
                   break;

                case 42:
                    converted.name = "Candle";
                    converted.color  = converted.val_1;
                    converted.brightness  = converted.val_2;
                    converted.type  = converted.val_3;
                    break;

                case 43:
                    converted.name = "ClkOutput";
                    converted.output  = converted.val_1;
                    break;

                case 44:
                    converted.name = "P1WifiGateway";
                    converted.v1  = converted.val_1;
                    break;

                case 45:
                    converted.name = "MPU6050";
                    // Not sure what to output here, since it uses "send data option"
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    break;

                case 46:
                    converted.name = "VentusW266";
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    break;

                case 47:
                    converted.name = "i2c-soil-moisture-sensor";
                    converted.temp  = converted.val_1;
                    converted.moisture  = converted.val_2;
                    converted.light  = converted.val_3;
                    break;

                case 48:
                    converted.name = "Motorshield_v2";
                    break;

                case 49:
                    converted.name = "MHZ19";
                    converted.ppm  = converted.val_1;
                    converted.temp  = converted.val_2;
                    converted.U  = converted.val_3;
                    break;

                case 50:
                    converted.name = "TCS34725";
                    converted.R  = converted.val_1;
                    converted.G  = converted.val_2;
                    converted.B  = converted.val_3;
                    converted.colortemp  = converted.val_4;
                    break;

                case 51:
                    converted.name = "AM2320";
                    converted.temp  = converted.val_1;
                    converted.hum  = converted.val_2;
                    break;

                case 52:
                    converted.name = "SenseAir";
                    converted.ppm  = converted.val_1;
                    converted.temp  = converted.val_2;
                    break;

                case 53:
                    converted.name = "PMSx003";
                    converted.pm1_0  = converted.val_1;
                    converted.pm2_5  = converted.val_2;
                    converted.pm10  = converted.val_3;
                    break;

                case 54:
                    converted.name = "DMX512";
                    break;

                case 55:
                    converted.name = "Chiming";
                    break;

                case 56:
                    converted.name = "SDS011-Dust";
                    converted.pm2_5  = converted.val_1;
                    converted.pm10  = converted.val_2;
                    break;

                case 57:
                    converted.name = "HT16K33_LED";
                    break;

                case 58:
                    converted.name = "HT16K33_KeyPad";
                    converted.scancode  = converted.val_1;
                    break;

                case 59:
                    converted.name = "Encoder";
                    converted.counter  = converted.val_1;
                    break;

                case 60:
                    converted.name = "MCP3221";
                    converted.analog  = converted.val_1;
                    break;

                case 61:
                    converted.name = "KeyPad";
                    converted.scancode  = converted.val_1;
                    break;

                case 62:
                    converted.name = "MPR121_KeyPad";
                    converted.scancode  = converted.val_1;
                    break;

                case 63:
                    converted.name = "TTP229_KeyPad";
                    converted.scancode  = converted.val_1;
                    break;

                case 64:
                    converted.name = "APDS9960";
                    converted.gesture  = converted.val_1;
                    converted.proximity  = converted.val_2;
                    converted.light  = converted.val_3;
                    break;

                case 65:
                    converted.name = "DRF0299_MP3";
                    break;

                case 66:
                    converted.name = "VEML6040";
                    converted.R  = converted.val_1;
                    converted.G  = converted.val_2;
                    converted.B  = converted.val_3;
                    converted.W  = converted.val_4;
                    break;

                case 67:
                    converted.name = "HX711_Load_Cell";
                    converted.weight_A  = converted.val_1;
                    converted.weight_B  = converted.val_2;
                    break;

                case 68:
                    converted.name = "SHT3x";
                    converted.temp  = converted.val_1;
                    converted.hum  = converted.val_2;
                    break;

                case 69:
                    converted.name = "LM75A";
                    converted.temp  = converted.val_1;
                    break;

                case 70:
                    converted.name = "NeoPixel_Clock";
                    converted.enabled  = converted.val_1;
                    converted.brightness  = converted.val_2;
                    converted.marks  = converted.val_3;
                    break;

                case 71:
                    converted.name = "Kamstrup401";
                    converted.heat  = converted.val_1;
                    converted.volume  = converted.val_2;
                    break;

                case 72:
                    converted.name = "HDC1080";
                    converted.temp  = converted.val_1;
                    converted.hum  = converted.val_2;
                    break;

                case 73:
                    converted.name = "7DGT";
                    break;

                case 74:
                    converted.name = "TSL2591";
                    converted.lux  = converted.val_1;
                    converted.full  = converted.val_2;
                    converted.visible  = converted.val_3;
                    converted.infrared  = converted.val_4;
                    break;

                case 75:
                    converted.name = "Nextion";
                    converted.idx  = converted.val_1;
                    converted.value = converted.val_2;
                    break;

                case 76:
                    converted.name = "HLW8012";
                    converted.volt  = converted.val_1;
                    converted.current  = converted.val_2;
                    converted.power  = converted.val_3;
                    converted.powerfactor  = converted.val_4;
                    break;

                case 77:
                    converted.name = "CSE7766";
                    converted.volt  = converted.val_1;
                    converted.power  = converted.val_2;
                    converted.current  = converted.val_3;
                    converted.pulses  = converted.val_4;
                    break;

                case 78:
                    converted.name = "Eastron";
                    // Selectable outputs, so do not name them here
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    break;

                case 79:
                    converted.name = "Wemos_Motorshield";
                    break;

                case 80:
                    converted.name = "DallasIButton";
                    {
                        // Not sure if it is present, since the sensor only has a single output value in ESPeasy.
                        var ulongvalue = converted.val_2 * 65536 + converted.val_1;
                        converted.ibutton = ulongvalue.toFixed(0);
                    }
                    break;

                case 81:
                    converted.name = "Cron";
                    converted.lastexec  = converted.val_1;
                    converted.nextexec  = converted.val_2;
                    break;

                case 82:
                    converted.name = "GPS";
                    // The GPS plugin must be set first to output like this.
                    // HDOP is needed by TTN mapper to weigh the quality of the data.
                    // When using TTN mapper, make sure to output these values.
//                    converted.longitude  = converted.val_1;
//                    converted.latitude  = converted.val_2;
//                    converted.altitude  = converted.val_3;
//                    converted.hdop  = converted.val_4;
                    break;

                case 83:
                    converted.name = "SGP30";
                    converted.tvoc  = converted.val_1;
                    converted.eco2  = converted.val_2;
                    break;

                case 84:
                    converted.name = "VEML6070";
                    converted.uv_raw  = converted.val_1;
                    converted.uv_risk  = converted.val_2;
                    converted.uv_power  = converted.val_3;
                    break;

                case 85:
                    converted.name = "AcuDC243";
                    // This plugin can output any value, just using the default settings here.
                    // TODO TD-er: Must add binary representation of all values
                    converted.volt  = converted.val_1;
                    converted.current  = converted.val_2;
                    converted.watt  = converted.val_3;
                    converted.wh_total  = converted.val_4;
                    break;

                case 86:
                    converted.name = "Homie";
                    // Can select a number of outputs
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    break;

                case 87:
                    converted.name = "SerialProxy";
                    // This plugin currently outputs strings, so not useful for LoRa
                    /*
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    */
                    break;

                case 89:
                    converted.name = "Ping";
                    converted.fails = converted.val_1;
                    break;
    
                case 90:
                    converted.name = "CCS811";
                    converted.TVOC  = converted.val_1;
                    converted.eCO2  = converted.val_2;
                    break;
    
                case 91:
                    converted.name = "SerSwitch";
                    // Can select a number of outputs
                    converted.relay1  = converted.val_1;
                    converted.relay2  = converted.val_2;
                    converted.relay3  = converted.val_3;
                    converted.relay4  = converted.val_4;
                    break;
    
                case 92:
                    converted.name = "DLbus";
                    // Can select a number of outputs
                    // TODO TD-er: Must add binary representation of all values
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    break;

                case 93:
                    converted.name = "MitsubishiHP";
                    // Outputs string
                    // TODO TD-er: Must add binary representation of all values
                    converted.v1  = converted.val_1;
                    break;

                case 94:
                    converted.name = "CULreader";
                    // Outputs string
                    // TODO TD-er: Must add binary representation of all values
                    converted.v1  = converted.val_1;
                    break;

                case 97:
                    converted.name = "ESP32Touch";
                    converted.touch  = converted.val_1;
                    break;

                case 100:
                    converted.name = "DS2423counter";
                    // TODO TD-er: This is probably the worst possible value to send over a LoRa network, as packets may get lost.
                    converted.countdelta  = converted.val_1;
                    break;
                        

                default:
                    converted.v1  = converted.val_1;
                    converted.v2  = converted.val_2;
                    converted.v3  = converted.val_3;
                    converted.v4  = converted.val_4;
                    break;
            } // End Switch
            delete converted["val_1"];
            delete converted["val_2"];
            delete converted["val_3"];
            delete converted["val_4"];
        }

    }

    return converted;
}