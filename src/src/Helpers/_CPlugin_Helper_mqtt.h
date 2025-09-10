#ifndef CPLUGIN_HELPER_MQTT_H
#define CPLUGIN_HELPER_MQTT_H

#if FEATURE_MQTT
# include "../Helpers/_CPlugin_Helper.h"

bool MQTT_handle_topic_commands(struct EventStruct *event,
                                bool                handleCmd       = true,
                                bool                handleSet       = true,
                                bool                tryRemoteConfig = false);
void MQTT_execute_command(String& command,
                          bool    tryRemoteConfig = false);
bool MQTT_protocol_send(EventStruct *event,
                        String       pubname,
                        bool         retainFlag);

# if FEATURE_MQTT_DISCOVER

typedef int (*QueryVType_ptr)(uint8_t);

bool getDiscoveryVType(struct EventStruct *event,
                       QueryVType_ptr      func_ptr,
                       uint8_t             pConfigOffset,
                       uint8_t             nrVars);

int                        Plugin_QueryVType_BinarySensor(uint8_t value_nr);
int                        Plugin_QueryVType_BinarySensorInv(uint8_t value_nr);
int                        Plugin_QueryVType_Analog(uint8_t value_nr);
int                        Plugin_QueryVType_CO2(uint8_t value_nr);
int                        Plugin_QueryVType_Distance(uint8_t value_nr);
int                        Plugin_QueryVType_DustPM2_5(uint8_t value_nr);
int                        Plugin_QueryVType_Lux(uint8_t value_nr);
int                        Plugin_QueryVType_Temperature(uint8_t value_nr);
int                        Plugin_QueryVType_Weight(uint8_t value_nr);

String                     makeHomeAssistantCompliantName(const String& name);

#  if FEATURE_MQTT_DEVICECLASS
String                     MQTT_binary_deviceClassName(int devClassIndex);
bool                       MQTT_binary_deviceClassTwoWay(int devClassIndex);
int                        MQTT_binary_deviceClassIndex(const String& deviceClassName);
#  endif // if FEATURE_MQTT_DEVICECLASS
#  if FEATURE_MQTT_STATE_CLASS
const __FlashStringHelper* MQTT_sensor_StateClass(uint8_t index,
                                                  bool    display = true);
#  endif // if FEATURE_MQTT_STATE_CLASS
struct DiscoveryItem {
  DiscoveryItem(Sensor_VType _VType, int _valueCount, taskVarIndex_t _varIndex, const bool _canSet = false)
    : VType(_VType), valueCount(_valueCount), varIndex(_varIndex), canSet(_canSet) {}

  #  if FEATURE_STRING_VARIABLES
  DiscoveryItem(Sensor_VType _VType, int _valueCount, taskVarIndex_t _varIndex, const String& _varName, const String& _uom,
                const bool _canSet)
    : VType(_VType), valueCount(_valueCount), varIndex(_varIndex), varName(_varName), uom(_uom), canSet(_canSet) {}

  #  endif // if FEATURE_STRING_VARIABLES

  Sensor_VType   VType;
  int            valueCount;
  taskVarIndex_t varIndex;
  #  if FEATURE_STRING_VARIABLES
  String varName;
  String uom;
  #  endif // if FEATURE_STRING_VARIABLES
  bool canSet{};
};

bool MQTT_SendAutoDiscovery(controllerIndex_t ControllerIndex,
                            cpluginID_t       CPluginID);
bool MQTT_HomeAssistant_SendAutoDiscovery(controllerIndex_t         ControllerIndex,
                                          ControllerSettingsStruct& ControllerSettings);
bool MQTT_DiscoveryGetDeviceVType(taskIndex_t                 TaskIndex,
                                  std::vector<DiscoveryItem>& discoveryItems,
                                  int                         valueCount,
                                  String                    & deviceClass);
String MQTT_TaskValueUniqueName(const String& taskName,
                                const String& valueName);
String MQTT_DiscoveryBuildValueTopic(const String            & topic,
                                     struct EventStruct       *event,
                                     uint8_t                   taskValueIndex,
                                     const __FlashStringHelper*deviceClass,
                                     const String            & uniqueId,
                                     const String            & elementId,
                                     const String            & valueName);

bool MQTT_DiscoveryPublish(controllerIndex_t ControllerIndex,
                           const String    & topic,
                           const String    & discoveryMessage,
                           taskIndex_t       x,
                           uint8_t           v,
                           bool              retained = false);

bool MQTT_DiscoveryPublishWithStatusAndSet(taskIndex_t               taskIndex,
                                           uint8_t                   taskValue,
                                           const String            & valueName,
                                           controllerIndex_t         ControllerIndex,
                                           ControllerSettingsStruct& ControllerSettings,
                                           const __FlashStringHelper*componentClass,
                                           String                    deviceClass,
                                           String                    unitOfMeasure,
                                           struct EventStruct       *event,
                                           const String              deviceElement,
                                           const String              stateClass,
                                           bool                      success,
                                           bool                      hasSet,
                                           bool                      hasIcon,
                                           const String            & elementName,
                                           const String            & elementId,
                                           bool                      sendTrigger = false);

String MQTT_DiscoveryHelperGetValueName(taskIndex_t   taskIndex,
                                        uint8_t       taskVarIndex,
                                        DiscoveryItem discoveryItem);

String MQTT_DiscoveryHelperGetValueUoM(taskIndex_t   taskIndex,
                                       uint8_t       taskVarIndex,
                                       DiscoveryItem discoveryItem,
                                       const String& defaultUoM = EMPTY_STRING);

# endif // if FEATURE_MQTT_DISCOVER
#endif // if FEATURE_MQTT
#endif // ifndef CPLUGIN_HELPER_MQTT_H
