// Decoder for device payload encoder "PACKED"
// copy&paste to TTN Console -> Applications -> PayloadFormat -> Decoder

// Original from https://github.com/cyberman54/ESP32-Paxcounter

function Decoder(bytes, port) {

  var decoded = {};

  if (bytes.length === 0) {
    return {};
  }

  if (port === 1) {
    // Single value
    if (bytes.length === 8) {
      return decode(bytes, [pluginid, uint16, uint8, int32_1e4], ['plugin_id', 'IDX', 'valuecount', 'val_1']);
    }
    // Dual value
    if (bytes.length === 12) {
      return decode(bytes, [pluginid, uint16, uint8, int32_1e4, int32_1e4], ['plugin_id', 'IDX', 'valuecount', 'val_1', 'val_2']);
    }
    // Triple value
    if (bytes.length === 16) {
      return decode(bytes, [pluginid, uint16, uint8, int32_1e4, int32_1e4, int32_1e4], ['plugin_id', 'IDX', 'valuecount', 'val_1', 'val_2', 'val_3']);
    }
    // Quad value
    if (bytes.length === 20) {
      return decode(bytes, [pluginid, uint16, uint8, int32_1e4, int32_1e4, int32_1e4, int32_1e4], ['plugin_id', 'IDX', 'valuecount', 'val_1', 'val_2', 'val_3', 'val_4']);
    }
  }

}


// ----- contents of /src/decoder.js --------------------------------------------
// https://github.com/thesolarnomad/lora-serialization/blob/master/src/decoder.js

var bytesToInt = function (bytes) {
  var i = 0;
  for (var x = 0; x < bytes.length; x++) {
    i |= (bytes[x] << (x * 8));
  }
  return i;
};

var version = function (bytes) {
  if (bytes.length !== version.BYTES) {
    throw new Error('version must have exactly 10 bytes');
  }
  return String.fromCharCode.apply(null, bytes).split('\u0000')[0];
};
version.BYTES = 10;

var uint8 = function (bytes) {
  if (bytes.length !== uint8.BYTES) {
    throw new Error('uint8 must have exactly 1 byte');
  }
  return bytesToInt(bytes);
};
uint8.BYTES = 1;

var uint8_1e3 = function (bytes) {
  return +(uint8(bytes) / 1e3).toFixed(3);
};
uint8_1e3.BYTES = uint8.BYTES;

var uint8_1e2 = function (bytes) {
  return +(uint8(bytes) / 1e2).toFixed(2);
};
uint8_1e2.BYTES = uint8.BYTES;

var uint8_1e1 = function (bytes) {
  return +(uint8(bytes) / 1e1).toFixed(1);
};
uint8_1e1.BYTES = uint8.BYTES;

var uint16 = function (bytes) {
  if (bytes.length !== uint16.BYTES) {
    throw new Error('uint16 must have exactly 2 bytes');
  }
  return bytesToInt(bytes);
};
uint16.BYTES = 2;

var uint16_1e6 = function (bytes) {
  return +(uint16(bytes) / 1e6).toFixed(6);
};
uint16_1e6.BYTES = uint16.BYTES;

var uint16_1e4 = function (bytes) {
  return +(uint16(bytes) / 1e4).toFixed(4);
};
uint16_1e4.BYTES = uint16.BYTES;

var uint16_1e2 = function (bytes) {
  return +(uint16(bytes) / 1e2).toFixed(2);
};
uint16_1e2.BYTES = uint16.BYTES;

var uint24 = function (bytes) {
  if (bytes.length !== uint24.BYTES) {
    throw new Error('uint24 must have exactly 3 bytes');
  }
  return bytesToInt(bytes);
};
uint24.BYTES = 3;

var uint24_1e6 = function (bytes) {
  return +(uint24(bytes) / 1e6).toFixed(6);
};
uint24_1e6.BYTES = uint24.BYTES;

var uint24_1e4 = function (bytes) {
  return +(uint24(bytes) / 1e4).toFixed(4);
};
uint24_1e4.BYTES = uint24.BYTES;

var uint24_1e2 = function (bytes) {
  return +(uint24(bytes) / 1e2).toFixed(2);
};
uint24_1e2.BYTES = uint24.BYTES;

var uint32 = function (bytes) {
  if (bytes.length !== uint32.BYTES) {
    throw new Error('uint32 must have exactly 4 bytes');
  }
  return bytesToInt(bytes);
};
uint32.BYTES = 4;

var uint64 = function (bytes) {
  if (bytes.length !== uint64.BYTES) {
    throw new Error('uint64 must have exactly 8 bytes');
  }
  return bytesToInt(bytes);
};
uint64.BYTES = 8;

var int8 = function (bytes) {
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

var int8_1e3 = function (bytes) {
  return +(int8(bytes) / 1e3).toFixed(3);
};
int8_1e3.BYTES = int8.BYTES;

var int8_1e2 = function (bytes) {
  return +(int8(bytes) / 1e2).toFixed(2);
};
int8_1e2.BYTES = int8.BYTES;

var int8_1e1 = function (bytes) {
  return +(int8(bytes) / 1e1).toFixed(1);
};
int8_1e1.BYTES = int8.BYTES;

var int16 = function (bytes) {
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

var int16_1e6 = function (bytes) {
  return +(int16(bytes) / 1e6).toFixed(6);
};
int16_1e6.BYTES = int16.BYTES;

var int16_1e4 = function (bytes) {
  return +(int16(bytes) / 1e4).toFixed(4);
};
int16_1e4.BYTES = int16.BYTES;

var int16_1e2 = function (bytes) {
  return +(int16(bytes) / 1e2).toFixed(2);
};
int16_1e2.BYTES = int16.BYTES;


var int24 = function (bytes) {
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

var int24_1e6 = function (bytes) {
  return +(int24(bytes) / 1e6).toFixed(6);
};
int24_1e6.BYTES = int24.BYTES;

var int24_1e4 = function (bytes) {
  return +(int24(bytes) / 1e4).toFixed(4);
};
int24_1e4.BYTES = int24.BYTES;

var int24_1e2 = function (bytes) {
  return +(int24(bytes) / 1e2).toFixed(2);
};
int24_1e2.BYTES = int24.BYTES;

var int32 = function (bytes) {
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

var int32_1e6 = function (bytes) {
  return +(int32(bytes) / 1e6).toFixed(6);
};
int32_1e6.BYTES = int32.BYTES;

var int32_1e4 = function (bytes) {
  return +(int32(bytes) / 1e4).toFixed(4);
};
int32_1e4.BYTES = int32.BYTES;

var int32_1e2 = function (bytes) {
  return +(int32(bytes) / 100).toFixed(2);
};
int32_1e2.BYTES = int32.BYTES;

var pluginid = function (bytes) {
  return +(uint8(bytes));
};
pluginid.BYTES = uint8.BYTES;


var latLng = function (bytes) {
  return +(int32_1e6(bytes));
};
latLng.BYTES = int32.BYTES;

var hdop = function (bytes) {
  
  return +(uint8(bytes) / 100).toFixed(2);
};
hdop.BYTES = uint8.BYTES;

var altitude = function (bytes) {
  // Option to increase altitude resolution (also on encoder side)
  return +(int16(bytes) / 4 - 1000).toFixed(1);
};
altitude.BYTES = int16.BYTES;


var bitmap1 = function (byte) {
  if (byte.length !== bitmap1.BYTES) {
    throw new Error('Bitmap must have exactly 1 byte');
  }
  var i = bytesToInt(byte);
  var bm = ('00000000' + Number(i).toString(2)).substr(-8).split('').map(Number).map(Boolean);
  return ['adr', 'screensaver', 'screen', 'countermode', 'blescan', 'antenna', 'filter', 'alarm']
    .reduce(function (obj, pos, index) {
      obj[pos] = +bm[index];
      return obj;
    }, {});
};
bitmap1.BYTES = 1;

var bitmap2 = function (byte) {
  if (byte.length !== bitmap2.BYTES) {
    throw new Error('Bitmap must have exactly 1 byte');
  }
  var i = bytesToInt(byte);
  var bm = ('00000000' + Number(i).toString(2)).substr(-8).split('').map(Number).map(Boolean);
  return ['gps', 'alarm', 'bme', 'counter', 'sensor1', 'sensor2', 'sensor3', 'battery']
    .reduce(function (obj, pos, index) {
      obj[pos] = +bm[index];
      return obj;
    }, {});
};
bitmap2.BYTES = 1;

var decode = function (bytes, mask, names) {

  var maskLength = mask.reduce(function (prev, cur) {
    return prev + cur.BYTES;
  }, 0);
  if (bytes.length < maskLength) {
    throw new Error('Mask length is ' + maskLength + ' whereas input is ' + bytes.length);
  }

  names = names || [];
  var offset = 0;
  return mask
    .map(function (decodeFn) {
      var current = bytes.slice(offset, offset += decodeFn.BYTES);
      return decodeFn(current);
    })
    .reduce(function (prev, cur, idx) {
      prev[names[idx] || idx] = cur;
      return prev;
    }, {});
};

if (typeof module === 'object' && typeof module.exports !== 'undefined') {
  module.exports = {
    uint8: uint8,
    uint8_1e3: uint8_1e3,
    uint8_1e2: uint8_1e2,
    uint8_1e1: uint8_1e1,
    uint16: uint16,
    uint16_1e6: uint16_1e6,
    uint16_1e4: uint16_1e4,
    uint16_1e2: uint16_1e2,
    uint24: uint24,
    uint24_1e6: uint24_1e6,
    uint24_1e4: uint24_1e4,
    uint24_1e2: uint24_1e2,
    uint32: uint32,
    int8: int8,
    int8_1e3: int8_1e3,
    int8_1e2: int8_1e2,
    int8_1e1: int8_1e1,
    int16: int16,
    int16_1e6: int16_1e6,
    int16_1e4: int16_1e4,
    int16_1e2: int16_1e2,
    int24: int24,
    int24_1e6: int24_1e6,
    int24_1e4: int24_1e4,
    int24_1e2: int24_1e2,
    int32: int32,
    int32_1e6: int32_1e6,
    int32_1e4: int32_1e4,
    int32_1e2: int32_1e2,
    pluginid: pluginid,
    latLng: latLng,
    hdop: hdop,
    altitude: altitude,
    bitmap1: bitmap1,
    bitmap2: bitmap2,
    version: version,
    decode: decode
  };
}