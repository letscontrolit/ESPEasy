// Decoder for device payload encoder "PACKED"
// copy & paste to TTN Console V3 -> Applications -> Payload formatters -> Uplink -> Javascript
function decodeUplink(input) {
  var data = {};

  var decoded = false;

  if (input.bytes.length === 0) {
    // Do nothing
  } else {
    if (input.fPort === 1) {
      switch (input.bytes[0]) {
        case 26:
          // SysInfo
          data = decode_plugin(input.fPort, input.bytes,
            [header, uint24, uint24, int8, vcc, pct_8, uint8, uint8, uint8, uint8, uint24, uint16],
            ['header', 'uptime', 'freeheap', 'rssi', 'vcc', 'load', 'ip1', 'ip2', 'ip3', 'ip4', 'web', 'freestack']);
          decoded = true;
          break;

        case 82:
          // GPS
          if (input.bytes.length === 18) {
            data = decode_plugin(input.fPort, input.bytes, [header, latLng, latLng, altitude, uint16_1e2, hdop, uint8, uint8],
              ['header', 'latitude', 'longitude', 'altitude', 'speed', 'hdop', 'max_snr', 'sat_tracked']);
            decoded = true;
          } else if (input.bytes.length === 21) {
            data = decode_plugin(input.fPort, input.bytes, [header, latLng, latLng, altitude, uint16_1e2, hdop, uint8, uint8, uint24_1e2],
              ['header', 'latitude', 'longitude', 'altitude', 'speed', 'hdop', 'max_snr', 'sat_tracked', 'distance_total_km']);
            decoded = true;
          } else {
            data = decode_plugin(input.fPort, input.bytes, [header, latLng, latLng, altitude, uint16_1e2, hdop, uint8, uint8, uint24_1e2, uint24_1e1],
              ['header', 'latitude', 'longitude', 'altitude', 'speed', 'hdop', 'max_snr', 'sat_tracked', 'distance_total_km', 'distance_ref']);
            decoded = true;
          }
          break;

        case 85:
          // AcuDC243
          // FIXME TD-er: Same code as in P108, Make new type for this with 4 selectable variables
          data = decode_plugin(input.fPort, input.bytes, [header, uint8, int32_1e4, uint8, int32_1e4, uint8, int32_1e4, uint8, int32_1e4],
            ['header', 'unit1', 'val_1', 'unit2', 'val_2', 'unit3', 'val_3', 'unit4', 'val_4']);
          decoded = true;
          break;

        case 102:
          // PZEM004T v30
          data = decode(bytes, [header, int16_1e1, int32_1e3, int32_1e1, int32_1e1, uint16_1e2, uint8_1e1],
            ['header', 'voltage', 'current', 'power', 'energy', 'powerfactor', 'frequency']);
          data.frequency += 40;
          decoded = true;
          break;

        case 108:
          // DDS238-x ZN
          // FIXME TD-er: Same code as in P085, Make new type for this with 4 selectable variables
          data = decode_plugin(input.fPort, input.bytes, [header, uint8, int32_1e4, uint8, int32_1e4, uint8, int32_1e4, uint8, int32_1e4],
            ['header', 'unit1', 'val_1', 'unit2', 'val_2', 'unit3', 'val_3', 'unit4', 'val_4']);
          decoded = true;
          break;

      }

      if (!decoded) {
        if (input.bytes.length === 9) {
          data = decode_plugin(input.fPort, input.bytes, [header, int32_1e4], ['header', 'val_1']);
          decoded = true;
        }
        // Dual value
        if (input.bytes.length === 13) {
          data = decode_plugin(input.fPort, input.bytes, [header, int32_1e4, int32_1e4], ['header', 'val_1', 'val_2']);
          decoded = true;
        }
        // Triple value
        if (input.bytes.length === 17) {
          data = decode_plugin(input.fPort, input.bytes, [header, int32_1e4, int32_1e4, int32_1e4], ['header', 'val_1', 'val_2', 'val_3']);
          decoded = true;
        }
        // Quad value
        if (input.bytes.length === 21) {
          data = decode_plugin(input.fPort, input.bytes, [header, int32_1e4, int32_1e4, int32_1e4, int32_1e4], ['header', 'val_1', 'val_2', 'val_3', 'val_4']);
          decoded = true;
        }
      }
    }

  }
  data.bytes = input.bytes; 
  data.port  = input.fPort;

  return {
    data: data,
    warnings: [],
    errors: []
  };
}


// ----- contents of /src/decoder.js --------------------------------------------
// https://github.com/thesolarnomad/lora-serialization/blob/master/src/decoder.js

var bytesToInt = function(bytes) {
  var i = 0;
  for (var x = 0; x < bytes.length; x++) {
    i |= (bytes[x] << (x * 8));
  }
  return i;
};

var version = function(bytes) {
  if (bytes.length !== version.BYTES) {
    throw new Error('version must have exactly 10 bytes');
  }
  return String.fromCharCode.apply(null, bytes).split('\u0000')[0];
};
version.BYTES = 10;

var uint8 = function(bytes) {
  if (bytes.length !== uint8.BYTES) {
    throw new Error('uint8 must have exactly 1 byte');
  }
  return bytesToInt(bytes);
};
uint8.BYTES = 1;

var uint16 = function(bytes) {
  if (bytes.length !== uint16.BYTES) {
    throw new Error('uint16 must have exactly 2 bytes');
  }
  return bytesToInt(bytes);
};
uint16.BYTES = 2;

var uint24 = function(bytes) {
  if (bytes.length !== uint24.BYTES) {
    throw new Error('uint24 must have exactly 3 bytes');
  }
  return bytesToInt(bytes);
};
uint24.BYTES = 3;

var uint32 = function(bytes) {
  if (bytes.length !== uint32.BYTES) {
    throw new Error('uint32 must have exactly 4 bytes');
  }
  return bytesToInt(bytes);
};
uint32.BYTES = 4;

var uint64 = function(bytes) {
  if (bytes.length !== uint64.BYTES) {
    throw new Error('uint64 must have exactly 8 bytes');
  }
  return bytesToInt(bytes);
};
uint64.BYTES = 8;

var int8 = function(bytes) {
  if (bytes.length !== int8.BYTES) {
    throw new Error('int8 must have exactly 1 byte');
  }
  var value = +(bytesToInt(bytes));
  if (value > 127) {
    value -= 256;
  }
  return value;
};
int8.BYTES = 1;

var int16 = function(bytes) {
  if (bytes.length !== int16.BYTES) {
    throw new Error('int16 must have exactly 2 bytes');
  }
  var value = +(bytesToInt(bytes));
  if (value > 32767) {
    value -= 65536;
  }
  return value;
};
int16.BYTES = 2;


var int24 = function(bytes) {
  if (bytes.length !== int24.BYTES) {
    throw new Error('int24 must have exactly 3 bytes');
  }
  var value = +(bytesToInt(bytes));
  if (value > 8388608) {
    value -= 16777216;
  }
  return value;
};
int24.BYTES = 3;

var int32 = function(bytes) {
  if (bytes.length !== int32.BYTES) {
    throw new Error('int32 must have exactly 4 bytes');
  }
  var value = +(bytesToInt(bytes));
  if (value > 2147483647) {
    value -= 4294967296;
  }
  return value;
};
int32.BYTES = 4;

// Basic types with a factor in them.
var uint8_1e3 = function(bytes) {
  return +(uint8(bytes) / 1e3).toFixed(3);
};
uint8_1e3.BYTES = uint8.BYTES;
var uint8_1e2 = function(bytes) {
  return +(uint8(bytes) / 1e2).toFixed(2);
};
uint8_1e2.BYTES = uint8.BYTES;
var uint8_1e1 = function(bytes) {
  return +(uint8(bytes) / 1e1).toFixed(1);
};
uint8_1e1.BYTES = uint8.BYTES;

var uint16_1e5 = function(bytes) {
  return +(uint16(bytes) / 1e5).toFixed(5);
};
uint16_1e5.BYTES = uint16.BYTES;
var uint16_1e4 = function(bytes) {
  return +(uint16(bytes) / 1e4).toFixed(4);
};
uint16_1e4.BYTES = uint16.BYTES;
var uint16_1e3 = function(bytes) {
  return +(uint16(bytes) / 1e3).toFixed(3);
};
uint16_1e3.BYTES = uint16.BYTES;
var uint16_1e2 = function(bytes) {
  return +(uint16(bytes) / 1e2).toFixed(2);
};
uint16_1e2.BYTES = uint16.BYTES;
var uint16_1e1 = function(bytes) {
  return +(uint16(bytes) / 1e1).toFixed(1);
};
uint16_1e1.BYTES = uint16.BYTES;

var uint24_1e6 = function(bytes) {
  return +(uint24(bytes) / 1e6).toFixed(6);
};
uint24_1e6.BYTES = uint24.BYTES;
var uint24_1e5 = function(bytes) {
  return +(uint24(bytes) / 1e5).toFixed(5);
};
uint24_1e5.BYTES = uint24.BYTES;
var uint24_1e4 = function(bytes) {
  return +(uint24(bytes) / 1e4).toFixed(4);
};
uint24_1e4.BYTES = uint24.BYTES;
var uint24_1e3 = function(bytes) {
  return +(uint24(bytes) / 1e3).toFixed(3);
};
uint24_1e3.BYTES = uint24.BYTES;
var uint24_1e2 = function(bytes) {
  return +(uint24(bytes) / 1e2).toFixed(2);
};
uint24_1e2.BYTES = uint24.BYTES;
var uint24_1e1 = function(bytes) {
  return +(uint24(bytes) / 1e1).toFixed(1);
};
uint24_1e1.BYTES = uint24.BYTES;

var uint32_1e6 = function(bytes) {
  return +(uint32(bytes) / 1e6).toFixed(6);
};
uint32_1e6.BYTES = uint32.BYTES;
var uint32_1e5 = function(bytes) {
  return +(uint32(bytes) / 1e5).toFixed(5);
};
uint32_1e5.BYTES = uint32.BYTES;
var uint32_1e4 = function(bytes) {
  return +(uint32(bytes) / 1e4).toFixed(4);
};
uint32_1e4.BYTES = uint32.BYTES;
var uint32_1e3 = function(bytes) {
  return +(uint32(bytes) / 1e3).toFixed(3);
};
uint32_1e3.BYTES = uint32.BYTES;
var uint32_1e2 = function(bytes) {
  return +(uint32(bytes) / 1e2).toFixed(2);
};
uint32_1e2.BYTES = uint32.BYTES;
var uint32_1e1 = function(bytes) {
  return +(uint32(bytes) / 1e1).toFixed(1);
};
uint32_1e1.BYTES = uint32.BYTES;

var int8_1e3 = function(bytes) {
  return +(int8(bytes) / 1e3).toFixed(3);
};
int8_1e3.BYTES = int8.BYTES;
var int8_1e2 = function(bytes) {
  return +(int8(bytes) / 1e2).toFixed(2);
};
int8_1e2.BYTES = int8.BYTES;
var int8_1e1 = function(bytes) {
  return +(int8(bytes) / 1e1).toFixed(1);
};
int8_1e1.BYTES = int8.BYTES;

var int16_1e5 = function(bytes) {
  return +(int16(bytes) / 1e5).toFixed(5);
};
int16_1e5.BYTES = int16.BYTES;
var int16_1e4 = function(bytes) {
  return +(int16(bytes) / 1e4).toFixed(4);
};
int16_1e4.BYTES = int16.BYTES;
var int16_1e3 = function(bytes) {
  return +(int16(bytes) / 1e3).toFixed(3);
};
int16_1e3.BYTES = int16.BYTES;
var int16_1e2 = function(bytes) {
  return +(int16(bytes) / 1e2).toFixed(2);
};
int16_1e2.BYTES = int16.BYTES;
var int16_1e1 = function(bytes) {
  return +(int16(bytes) / 1e1).toFixed(1);
};
int16_1e1.BYTES = int16.BYTES;

var int24_1e6 = function(bytes) {
  return +(int24(bytes) / 1e6).toFixed(6);
};
int24_1e6.BYTES = int24.BYTES;
var int24_1e5 = function(bytes) {
  return +(int24(bytes) / 1e5).toFixed(5);
};
int24_1e5.BYTES = int24.BYTES;
var int24_1e4 = function(bytes) {
  return +(int24(bytes) / 1e4).toFixed(4);
};
int24_1e4.BYTES = int24.BYTES;
var int24_1e3 = function(bytes) {
  return +(int24(bytes) / 1e3).toFixed(3);
};
int24_1e3.BYTES = int24.BYTES;
var int24_1e2 = function(bytes) {
  return +(int24(bytes) / 1e2).toFixed(2);
};
int24_1e2.BYTES = int24.BYTES;
var int24_1e1 = function(bytes) {
  return +(int24(bytes) / 1e1).toFixed(1);
};
int24_1e1.BYTES = int24.BYTES;

var int32_1e6 = function(bytes) {
  return +(int32(bytes) / 1e6).toFixed(6);
};
int32_1e6.BYTES = int32.BYTES;
var int32_1e5 = function(bytes) {
  return +(int32(bytes) / 1e5).toFixed(5);
};
int32_1e5.BYTES = int32.BYTES;
var int32_1e4 = function(bytes) {
  return +(int32(bytes) / 1e4).toFixed(4);
};
int32_1e4.BYTES = int32.BYTES;
var int32_1e3 = function(bytes) {
  return +(int32(bytes) / 1e3).toFixed(3);
};
int32_1e3.BYTES = int32.BYTES;
var int32_1e2 = function(bytes) {
  return +(int32(bytes) / 1e2).toFixed(2);
};
int32_1e2.BYTES = int32.BYTES;
var int32_1e1 = function(bytes) {
  return +(int32(bytes) / 1e1).toFixed(1);
};
int32_1e1.BYTES = int32.BYTES;


var pluginid = function(bytes) {
  return +(uint8(bytes));
};
pluginid.BYTES = uint8.BYTES;


var latLng = function(bytes) {
  // 2^23 / 180 = 46603...
  return +(int24(bytes) / 46600);
};
latLng.BYTES = int24.BYTES;

var hdop = function(bytes) {
  return +(uint8(bytes) / 10).toFixed(2);
};
hdop.BYTES = uint8.BYTES;

var altitude = function(bytes) {
  // Option to increase altitude resolution (also on encoder side)
  return +(int16(bytes) / 4 - 1000).toFixed(1);
};
altitude.BYTES = int16.BYTES;

// -1 .. 5.12V
var vcc = function(bytes) {
  return +(uint8(bytes) / 41.83 - 1.0).toFixed(2);
};
vcc.BYTES = uint8.BYTES;

// 0 .. 100%
var pct_8 = function(bytes) {
  return +(uint8(bytes) / 2.56).toFixed(2);
};
pct_8.BYTES = uint8.BYTES;


var bitmap1 = function(byte) {
  if (byte.length !== bitmap1.BYTES) {
    throw new Error('Bitmap must have exactly 1 byte');
  }
  var i = bytesToInt(byte);
  var bm = ('00000000' + Number(i).toString(2)).substr(-8).split('').map(Number).map(Boolean);
  return ['adr', 'screensaver', 'screen', 'countermode', 'blescan', 'antenna', 'filter', 'alarm']
    .reduce(function(obj, pos, index) {
      obj[pos] = +bm[index];
      return obj;
    }, {});
};
bitmap1.BYTES = 1;

var bitmap2 = function(byte) {
  if (byte.length !== bitmap2.BYTES) {
    throw new Error('Bitmap must have exactly 1 byte');
  }
  var i = bytesToInt(byte);
  var bm = ('00000000' + Number(i).toString(2)).substr(-8).split('').map(Number).map(Boolean);
  return ['gps', 'alarm', 'bme', 'counter', 'sensor1', 'sensor2', 'sensor3', 'battery']
    .reduce(function(obj, pos, index) {
      obj[pos] = +bm[index];
      return obj;
    }, {});
};
bitmap2.BYTES = 1;

var header = function(byte) {
  if (byte.length !== header.BYTES) {
    throw new Error('header must have exactly 5 bytes');
  }
  var values = [0, 0, 0, 0];
  values[0] = bytesToInt(byte.slice(0, 1));
  values[1] = bytesToInt(byte.slice(1, 3));
  values[2] = bytesToInt(byte.slice(3, 4));
  values[3] = bytesToInt(byte.slice(4, 5));

  return ['plugin_id', 'IDX', 'samplesetcount', 'valuecount']
    .reduce(function(obj, pos, index) {
      obj[pos] = +values[index];
      return obj;
    }, {});
};
header.BYTES = 5;


var decode = function(bytes, mask, names) {

  var maskLength = mask.reduce(function(prev, cur) {
    return prev + cur.BYTES;
  }, 0);
  if (bytes.length < maskLength) {
    throw new Error('Mask length is ' + maskLength + ' whereas input is ' + bytes.length);
  }

  names = names || [];
  var offset = 0;
  return mask
    .map(function(decodeFn) {
      var current = bytes.slice(offset, offset += decodeFn.BYTES);
      return decodeFn(current);
    })
    .reduce(function(prev, cur, idx) {
      prev[names[idx] || idx] = cur;
      return prev;
    }, {});
};


var decode_plugin = function(fPort, bytes, mask, names) {
  return Converter(decode(bytes, mask, names), fPort);
};


function Converter(decoded, fPort) {

  var converted = decoded;
  var name = "";

  if (fPort === 1) {
    if ('header' in converted) {
      switch (converted.header.plugin_id) {
        case 1:
          converted.name = "Switch";
          converted.v1 = converted.val_1;
          converted.v2 = converted.val_2;
          converted.v3 = converted.val_3;
          converted.v4 = converted.val_4;
          break;

        case 2:
          converted.name = "ADC";
          converted.analog = converted.val_1;
          break;

        case 3:
          converted.name = "Pulse";
          converted.count = converted.val_1;
          converted.total = converted.val_2;
          converted.time = converted.val_3;
          break;

        case 4:
          converted.name = "Dallas";
          // For compatibility reasons, also include temp.
          converted.temp = converted.val_1;
          converted.temp1 = converted.val_1;
          converted.temp2 = converted.val_2;
          converted.temp3 = converted.val_3;
          converted.temp4 = converted.val_4;
          break;

        case 5:
          converted.name = "DHT";
          converted.temp = converted.val_1;
          converted.hum = converted.val_2;
          break;

        case 6:
          converted.name = "BMP085";
          converted.temp = converted.val_1;
          converted.pressure = converted.val_2;
          break;

        case 7:
          converted.name = "PCF8591";
          converted.analog = converted.val_1;
          break;

        case 8:
          converted.name = "RFID"; {
            // Not sure if it is present, since the sensor only has a single output value in ESPeasy.
            var ulongvalue = converted.val_2 * 65536 + converted.val_1;
            converted.tag = ulongvalue.toFixed(0);
          }
          break;

        case 9:
          converted.name = "MCP";
          converted.state = converted.val_1;
          break;

        case 10:
          converted.name = "BH1750";
          converted.lux = converted.val_1;
          break;

        case 11:
          converted.name = "PME";
          converted.v1 = converted.val_1;
          break;

        case 12:
          converted.name = "LCD";
          // Should not output anything, echo the values just to be sure.
          converted.v1 = converted.val_1;
          converted.v2 = converted.val_2;
          converted.v3 = converted.val_3;
          converted.v4 = converted.val_4;
          break;

        case 13:
          converted.name = "HCSR04";
          converted.dist = converted.val_1;
          break;

        case 14:
          converted.name = "SI7021";
          converted.temp = converted.val_1;
          converted.hum = converted.val_2;
          break;

        case 15:
          converted.name = "TSL2561";
          // Not sure if the last value is present.
          converted.lux = converted.val_1;
          converted.infrared = converted.val_2;
          converted.broadband = converted.val_3;
          converted.ratio = converted.val_4;
          break;

        case 16:
          converted.name = "IR_rx";
          converted.IR = converted.val_1;
          break;

        case 17:
          converted.name = "PN532"; {
            // Not sure if it is present, since the sensor only has a single output value in ESPeasy.
            var ulongvalue = converted.val_2 * 65536 + converted.val_1;
            converted.tag = ulongvalue.toFixed(0);
          }
          break;

        case 18:
          converted.name = "GP2Y10";
          converted.dust = converted.val_1;
          break;

        case 19:
          converted.name = "PCF8574";
          converted.state = converted.val_1;
          break;

        case 20:
          converted.name = "Ser2Net";
          // Not sure if this one does output a useful value
          converted.ser2net = converted.val_1;
          break;

        case 21:
          converted.name = "Level";
          converted.output = converted.val_1;
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
          converted.temp = converted.val_1;
          break;

        case 25:
          converted.name = "ADS1115";
          converted.analog = converted.val_1;
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
          converted.volt = converted.val_1;
          converted.current = converted.val_2;
          converted.power = converted.val_3;
          break;

        case 28:
          converted.name = "BME280";
          converted.temp = converted.val_1;
          converted.hum = converted.val_2;
          converted.pressure = converted.val_3;
          break;

        case 29:
          converted.name = "MQTThelper";
          converted.output = converted.val_1;
          break;

        case 30:
          converted.name = "BMP280";
          converted.temp = converted.val_1;
          converted.pressure = converted.val_2;
          break;

        case 31:
          converted.name = "SHT1X";
          converted.temp = converted.val_1;
          converted.hum = converted.val_2;
          break;

        case 32:
          converted.name = "MS5611";
          converted.temp = converted.val_1;
          converted.pressure = converted.val_2;
          break;

        case 33:
          converted.name = "Dummy";
          converted.v1 = converted.val_1;
          converted.v2 = converted.val_2;
          converted.v3 = converted.val_3;
          converted.v4 = converted.val_4;
          break;

        case 34:
          converted.name = "DHT12";
          converted.temp = converted.val_1;
          converted.hum = converted.val_2;
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
          converted.v1 = converted.val_1;
          converted.v2 = converted.val_2;
          converted.v3 = converted.val_3;
          converted.v4 = converted.val_4;
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
          converted.temp = converted.val_1;
          break;

        case 40:
          converted.name = "ID12"; {
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
          converted.color = converted.val_1;
          converted.brightness = converted.val_2;
          converted.type = converted.val_3;
          break;

        case 43:
          converted.name = "ClkOutput";
          converted.output = converted.val_1;
          break;

        case 44:
          converted.name = "P1WifiGateway";
          converted.v1 = converted.val_1;
          break;

        case 45:
          converted.name = "MPU6050";
          // Not sure what to output here, since it uses "send data option"
          converted.v1 = converted.val_1;
          converted.v2 = converted.val_2;
          converted.v3 = converted.val_3;
          converted.v4 = converted.val_4;
          break;

        case 46:
          converted.name = "VentusW266";
          converted.v1 = converted.val_1;
          converted.v2 = converted.val_2;
          converted.v3 = converted.val_3;
          break;

        case 47:
          converted.name = "i2c-soil-moisture-sensor";
          converted.temp = converted.val_1;
          converted.moisture = converted.val_2;
          converted.light = converted.val_3;
          break;

        case 48:
          converted.name = "Motorshield_v2";
          break;

        case 49:
          converted.name = "MHZ19";
          converted.ppm = converted.val_1;
          converted.temp = converted.val_2;
          converted.U = converted.val_3;
          break;

        case 50:
          converted.name = "TCS34725";
          converted.R = converted.val_1;
          converted.G = converted.val_2;
          converted.B = converted.val_3;
          converted.colortemp = converted.val_4;
          break;

        case 51:
          converted.name = "AM2320";
          converted.temp = converted.val_1;
          converted.hum = converted.val_2;
          break;

        case 52:
          converted.name = "SenseAir";
          converted.ppm = converted.val_1;
          converted.temp = converted.val_2;
          break;

        case 53:
          converted.name = "PMSx003";
          converted.pm1_0 = converted.val_1;
          converted.pm2_5 = converted.val_2;
          converted.pm10 = converted.val_3;
          break;

        case 54:
          converted.name = "DMX512";
          break;

        case 55:
          converted.name = "Chiming";
          break;

        case 56:
          converted.name = "SDS011-Dust";
          converted.pm2_5 = converted.val_1;
          converted.pm10 = converted.val_2;
          break;

        case 57:
          converted.name = "HT16K33_LED";
          break;

        case 58:
          converted.name = "HT16K33_KeyPad";
          converted.scancode = converted.val_1;
          break;

        case 59:
          converted.name = "Encoder";
          converted.counter = converted.val_1;
          break;

        case 60:
          converted.name = "MCP3221";
          converted.analog = converted.val_1;
          break;

        case 61:
          converted.name = "KeyPad";
          converted.scancode = converted.val_1;
          break;

        case 62:
          converted.name = "MPR121_KeyPad";
          converted.scancode = converted.val_1;
          break;

        case 63:
          converted.name = "TTP229_KeyPad";
          converted.scancode = converted.val_1;
          break;

        case 64:
          converted.name = "APDS9960";
          converted.gesture = converted.val_1;
          converted.proximity = converted.val_2;
          converted.light = converted.val_3;
          break;

        case 65:
          converted.name = "DRF0299_MP3";
          break;

        case 66:
          converted.name = "VEML6040";
          converted.R = converted.val_1;
          converted.G = converted.val_2;
          converted.B = converted.val_3;
          converted.W = converted.val_4;
          break;

        case 67:
          converted.name = "HX711_Load_Cell";
          converted.weight_A = converted.val_1;
          converted.weight_B = converted.val_2;
          break;

        case 68:
          converted.name = "SHT3x";
          converted.temp = converted.val_1;
          converted.hum = converted.val_2;
          break;

        case 69:
          converted.name = "LM75A";
          converted.temp = converted.val_1;
          break;

        case 70:
          converted.name = "NeoPixel_Clock";
          converted.enabled = converted.val_1;
          converted.brightness = converted.val_2;
          converted.marks = converted.val_3;
          break;

        case 71:
          converted.name = "Kamstrup401";
          converted.heat = converted.val_1;
          converted.volume = converted.val_2;
          break;

        case 72:
          converted.name = "HDC1080";
          converted.temp = converted.val_1;
          converted.hum = converted.val_2;
          break;

        case 73:
          converted.name = "7DGT";
          break;

        case 74:
          converted.name = "TSL2591";
          converted.lux = converted.val_1;
          converted.full = converted.val_2;
          converted.visible = converted.val_3;
          converted.infrared = converted.val_4;
          break;

        case 75:
          converted.name = "Nextion";
          converted.idx = converted.val_1;
          converted.value = converted.val_2;
          break;

        case 76:
          converted.name = "HLW8012";
          converted.volt = converted.val_1;
          converted.current = converted.val_2;
          converted.power = converted.val_3;
          converted.powerfactor = converted.val_4;
          break;

        case 77:
          converted.name = "CSE7766";
          converted.volt = converted.val_1;
          converted.power = converted.val_2;
          converted.current = converted.val_3;
          converted.pulses = converted.val_4;
          break;

        case 78:
          converted.name = "Eastron";
          // Selectable outputs, so do not name them here
          converted.v1 = converted.val_1;
          converted.v2 = converted.val_2;
          converted.v3 = converted.val_3;
          converted.v4 = converted.val_4;
          break;

        case 79:
          converted.name = "Wemos_Motorshield";
          break;

        case 80:
          converted.name = "DallasIButton"; {
            // Not sure if it is present, since the sensor only has a single output value in ESPeasy.
            var ulongvalue = converted.val_2 * 65536 + converted.val_1;
            converted.ibutton = ulongvalue.toFixed(0);
          }
          break;

        case 81:
          converted.name = "Cron";
          converted.lastexec = converted.val_1;
          converted.nextexec = converted.val_2;
          break;

        case 82:
          converted.name = "GPS";
          // GPS data is already decoded in packed_decoder.js
          // HDOP is needed by TTN mapper to weigh the quality of the data.
          // When using TTN mapper, make sure to output these values.
          break;

        case 83:
          converted.name = "SGP30";
          converted.tvoc = converted.val_1;
          converted.eco2 = converted.val_2;
          break;

        case 84:
          converted.name = "VEML6070";
          converted.uv_raw = converted.val_1;
          converted.uv_risk = converted.val_2;
          converted.uv_power = converted.val_3;
          break;

        case 85:
          converted.name = "AcuDC243";
          // This plugin can output any value, so show string representation 
          // of the unit of measure
          converted.unit1 = getAcuDC243Unit(converted.unit1);
          converted.unit2 = getAcuDC243Unit(converted.unit2);
          converted.unit3 = getAcuDC243Unit(converted.unit3);
          converted.unit4 = getAcuDC243Unit(converted.unit4);
          converted.v1 = converted.val_1;
          converted.v2 = converted.val_2;
          converted.v3 = converted.val_3;
          converted.v4 = converted.val_4;
          break;

        case 86:
          converted.name = "Homie";
          // Can select a number of outputs
          converted.v1 = converted.val_1;
          converted.v2 = converted.val_2;
          converted.v3 = converted.val_3;
          converted.v4 = converted.val_4;
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
          converted.TVOC = converted.val_1;
          converted.eCO2 = converted.val_2;
          break;

        case 91:
          converted.name = "SerSwitch";
          // Can select a number of outputs
          converted.relay1 = converted.val_1;
          converted.relay2 = converted.val_2;
          converted.relay3 = converted.val_3;
          converted.relay4 = converted.val_4;
          break;

        case 92:
          converted.name = "DLbus";
          // Can select a number of outputs
          // TODO TD-er: Must add binary representation of all values
          converted.v1 = converted.val_1;
          converted.v2 = converted.val_2;
          converted.v3 = converted.val_3;
          converted.v4 = converted.val_4;
          break;

        case 93:
          converted.name = "MitsubishiHP";
          // Outputs string
          // TODO TD-er: Must add binary representation of all values
          converted.v1 = converted.val_1;
          break;

        case 94:
          converted.name = "CULreader";
          // Outputs string
          // TODO TD-er: Must add binary representation of all values
          converted.v1 = converted.val_1;
          break;

        case 97:
          converted.name = "ESP32Touch";
          converted.touch = converted.val_1;
          break;

        case 100:
          converted.name = "DS2423counter";
          // TODO TD-er: This is probably the worst possible value to send over a LoRa network, as packets may get lost.
          converted.countdelta = converted.val_1;
          break;

        case 102:
          converted.name = "PZEM004T v30";
          break;

        case 106:
          converted.name = "BME680";
          converted.temp = converted.val_1;
          converted.hum = converted.val_2;
          converted.pressure = converted.val_3;
          converted.gas = converted.val_4;
          break;

        case 107:
          converted.name = "SI1145";
          converted.visible = converted.val_1;
          converted.infra = converted.val_2;
          converted.uv = converted.val_3;
          break;

        case 108:
          converted.name = "DDS238-x ZN";
          // This plugin can output any value, so show string representation 
          // of the unit of measure
          converted.unit1 = getDDS238_xUnit(converted.unit1);
          converted.unit2 = getDDS238_xUnit(converted.unit2);
          converted.unit3 = getDDS238_xUnit(converted.unit3);
          converted.unit4 = getDDS238_xUnit(converted.unit4);
          converted.v1 = converted.val_1;
          converted.v2 = converted.val_2;
          converted.v3 = converted.val_3;
          converted.v4 = converted.val_4;
          break;

        case 110:
          converted.name = "VL53L0X";
          converted.distance = converted.val_1;
          break;

        case 111:
          converted.name = "RC522 RFID"; {
            // Not sure if it is present, since the sensor only has a single output value in ESPeasy.
            var ulongvalue = converted.val_2 * 65536 + converted.val_1;
            converted.tag = ulongvalue.toFixed(0);
          }
          break;

        case 113:
          converted.name = "VL53L1X";
          converted.distance = converted.val_1;
          converted.ambient = converted.val_2;
          break;

        default:
          converted.v1 = converted.val_1;
          converted.v2 = converted.val_2;
          converted.v3 = converted.val_3;
          converted.v4 = converted.val_4;
          break;
      } // End Switch
      delete converted["val_1"];
      delete converted["val_2"];
      delete converted["val_3"];
      delete converted["val_4"];
    }

  }

  return converted;
};

function getAcuDC243Unit(unit_id) {
  switch (unit_id) {
    case 0: // P085_QUERY_V       0
      return "V";
    case 1: // P085_QUERY_A       1
      return "A";
    case 2: // P085_QUERY_W       2
      return "W";
    case 3: // P085_QUERY_Wh_imp  3
      return "Wh imp";
    case 4: // P085_QUERY_Wh_exp  4
      return "Wh exp";
    case 5: // P085_QUERY_Wh_tot  5
      return "Wh total";
    case 6: // P085_QUERY_Wh_net  6
      return "Wh net";
    case 7: // P085_QUERY_h_tot   7
      return "hours tot";
    case 8: // P085_QUERY_h_load  8
      return "hours load";
  }
  return "unknown" + unit_id;
};


function getDDS238_xUnit(unit_id) {
  switch (unit_id) {
    case 0: // P108_QUERY_V       0
      return "V";
    case 1: // P108_QUERY_A       1
      return "A";
    case 2: // P108_QUERY_W       2
      return "W";
    case 3: // P108_QUERY_VA      3
      return "VA";
    case 4: // P108_QUERY_PF      4
      return "cosphi";
    case 5: // P108_QUERY_F       5
      return "Hz";
    case 6: // P108_QUERY_Wh_imp  6
      return "Wh imp";
    case 7: // P108_QUERY_Wh_exp  7
      return "Wh exp";
    case 8: // P108_QUERY_Wh_tot  8
      return "Wh total";
  }
  return "unknown" + unit_id;
};



if (typeof module === 'object' && typeof module.exports !== 'undefined') {
  module.exports = {
    uint8: uint8,
    uint16: uint16,
    uint24: uint24,
    uint32: uint32,
    int8: int8,
    int16: int16,
    int24: int24,
    int32: int32,
    uint8_1e3: uint8_1e3,
    uint8_1e2: uint8_1e2,
    uint8_1e1: uint8_1e1,
    uint16_1e5: uint16_1e5,
    uint16_1e4: uint16_1e4,
    uint16_1e3: uint16_1e3,
    uint16_1e2: uint16_1e2,
    uint16_1e1: uint16_1e1,
    uint24_1e6: uint24_1e6,
    uint24_1e5: uint24_1e5,
    uint24_1e4: uint24_1e4,
    uint24_1e3: uint24_1e3,
    uint24_1e2: uint24_1e2,
    uint24_1e1: uint24_1e1,
    uint32_1e6: uint32_1e6,
    uint32_1e5: uint32_1e5,
    uint32_1e4: uint32_1e4,
    uint32_1e3: uint32_1e3,
    uint32_1e2: uint32_1e2,
    uint32_1e1: uint32_1e1,
    int8_1e3: int8_1e3,
    int8_1e2: int8_1e2,
    int8_1e1: int8_1e1,
    int16_1e5: int16_1e5,
    int16_1e4: int16_1e4,
    int16_1e3: int16_1e3,
    int16_1e2: int16_1e2,
    int16_1e1: int16_1e1,
    int24_1e6: int24_1e6,
    int24_1e5: int24_1e5,
    int24_1e4: int24_1e4,
    int24_1e3: int24_1e3,
    int24_1e2: int24_1e2,
    int24_1e1: int24_1e1,
    int32_1e6: int32_1e6,
    int32_1e5: int32_1e5,
    int32_1e4: int32_1e4,
    int32_1e3: int32_1e3,
    int32_1e2: int32_1e2,
    int32_1e1: int32_1e1,
    pluginid: pluginid,
    latLng: latLng,
    hdop: hdop,
    altitude: altitude,
    vcc: vcc,
    pct_8: pct_8,
    bitmap1: bitmap1,
    bitmap2: bitmap2,
    header: header,
    version: version,
    decode: decode
  };
}