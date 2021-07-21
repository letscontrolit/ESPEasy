#include "../WebServer/Metrics.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"


#include "../../ESPEasy-Globals.h"

#include "../Commands/Diagnostic.h"

#include "../DataStructs/RTCStruct.h"

#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"

#include "../Globals/CRCValues.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/NetworkState.h"
#include "../Globals/RTC.h"
# include "../Globals/CPlugins.h"
# include "../Globals/Device.h"
# include "../Globals/ExtraTaskSettings.h"
# include "../Globals/Nodes.h"
# include "../Globals/Plugins.h"
# include "../Globals/Protocol.h"

# include "../Static/WebStaticData.h"

# include "../Helpers/_Plugin_SensorTypeHelper.h"
# include "../Helpers/_Plugin_Helper_serial.h"


# include "../../_Plugin_Helper.h"

# include <ESPeasySerial.h>

#include "../Helpers/CompiletimeDefines.h"
#include "../Helpers/ESPEasyStatistics.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/OTA.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_GPIO.h"
#include "../Helpers/StringGenerator_System.h"

#include "../Static/WebStaticData.h"

#ifdef USES_MQTT
# include "../Globals/MQTT.h"
# include "../Helpers/PeriodicalActions.h" // For finding enabled MQTT controller
#endif

#ifdef ESP32
# include <esp_partition.h>
#endif // ifdef ESP32


void handle_metrics() {
    int result;
    String resultString;
    resultString.reserve(1000);


    //uptime
    resultString = F("# HELP espeasy_uptime current device uptime in minutes\n");
    resultString += F("# TYPE espeasy_uptime counter\n");        
    resultString += "espeasy_uptime ";
    resultString += getValue(LabelType::UPTIME);       
    resultString += "\n";
  

    //load
    resultString += F("# HELP espeasy_load device percentage load\n");
    resultString += F("# TYPE espeasy_load gauge\n");         
    resultString += "espeasy_load ";
    resultString += getValue(LabelType::LOAD_PCT);     
    resultString += "\n";

    //Free RAM
    resultString += F("# HELP espeasy_free_ram device amount of RAM free in Bytes\n");
    resultString += F("# TYPE espeasy_free_ram gauge\n");    
    resultString += "espeasy_free_ram ";
    resultString += getValue(LabelType::FREE_MEM);   
    resultString += "\n";

    //Free RAM
    resultString += F("# HELP espeasy_free_stack device amount of Stack free in Bytes\n");
    resultString += F("# TYPE espeasy_free_stack gauge\n");     
    resultString += "espeasy_free_stack ";
    resultString += getValue(LabelType::FREE_STACK);  
    resultString += "\n";

    //Wifi strength
    resultString += F("# HELP espeasy_wifi_rssi Wifi connection Strength\n");
    resultString += F("# TYPE espeasy_wifi_rssi gauge\n");   
    resultString += "espeasy_wifi_rssi ";
    resultString += getValue(LabelType::WIFI_RSSI); 
    resultString += "\n";

    //Wifi uptime
    resultString += F("# HELP espeasy_wifi_connected Time wifi has been connected in milliseconds\n");
    resultString += F("# TYPE espeasy_wifi_connected counter\n");    
    resultString += "espeasy_wifi_connected ";
    resultString += getValue(LabelType::CONNECTED_MSEC);
    resultString += "\n";

    //Wifi reconnects
    resultString += F("# HELP espeasy_wifi_reconnects Number of times Wifi has reconnected since boot\n");
    resultString += F("# TYPE espeasy_wifi_reconnects counter\n");   
    resultString += "espeasy_wifi_reconnects ";
    resultString += getValue(LabelType::NUMBER_RECONNECTS); 
    resultString += "\n";

    //devices
    resultString += handle_metrics_devices();



    web_server.send(200, F("text/plain;"), resultString);

}



String handle_metrics_value_name(const String& valName){
    return String(valName);
}

String handle_metrics_value_value(const String& valValue){
    return String(valValue);
}



String handle_metrics_devices(){
    String returnString = "";


    for (taskIndex_t x = 0; x < 128 && validTaskIndex(x); x++)
    {
        const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(x);
        const bool pluginID_set         = INVALID_PLUGIN_ID != Settings.TaskDeviceNumber[x];
         if (pluginID_set){
             LoadTaskSettings(x);
            int8_t spi_gpios[3] { -1, -1, -1 };
            struct EventStruct TempEvent(x);
            addEnabled(Settings.TaskDeviceEnabled[x]  && validDeviceIndex(DeviceIndex));
            String deviceName = ExtraTaskSettings.TaskDeviceName;
            returnString += F("# HELP espeasy_device_");
            returnString += deviceName;
            returnString += F(" Values from connected device\n");
            returnString += F("# TYPE espeasy_device_");
            returnString += deviceName;
            returnString += F(" gauge\n");
            if (validDeviceIndex(DeviceIndex)) {
                String customValuesString;
                //const bool customValues = PluginCall(PLUGIN_WEBFORM_SHOW_VALUES, &TempEvent, customValuesString);
                const bool customValues = 0;
                if (!customValues)
                {
                    const uint8_t valueCount = getValueCountForTask(x);
                    for (uint8_t varNr = 0; varNr < valueCount; varNr++)
                    {
                        if (validPluginID_fullcheck(Settings.TaskDeviceNumber[x]))
                        {
                              String valName = String(F("valuename_")) + x + '_' + varNr;
                              String valValue = String(F("value_")) + x + '_' + varNr;
                              returnString += F("espeasy_device_");
                              returnString += deviceName;
                              returnString += F("{valueName=\"");
                              returnString += ExtraTaskSettings.TaskDeviceValueNames[varNr];
                              returnString += F("\"} ");
                              returnString += formatUserVarNoCheck(x, varNr);
                              returnString += "\n";
                            
                        }
                    }
                }
            }
            
         }
    }
    return returnString;
}

