// Converter for device payload encoder "PACKED"
// copy&paste to TTN Console -> Applications -> PayloadFormat -> Converter

function Converter(decoded, port) {

    var converted = decoded;
    var name = "";

    if (port === 1) {
      if('plugin_id' in converted) {
          switch(converted.plugin_id) {
              case 26:
                  converted.name = "SysInfo";
                  break;
            case 28:
                converted.name = "BME280";
                converted.temp = converted.val_1;
                converted.hum = converted.val_2;
                converted.pressure = converted.val_3;
                break;      
          }
          
      }
       
    }

    return converted;
}