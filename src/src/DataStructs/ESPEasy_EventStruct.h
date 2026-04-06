#ifndef DATASTRUCTS_ESPEASY_EVENTSTRUCT_H
#define DATASTRUCTS_ESPEASY_EVENTSTRUCT_H

#include "../../ESPEasy_common.h"

#include "../DataTypes/ControllerIndex.h"
#include "../DataTypes/EventValueSource.h"
#include "../DataTypes/TaskIndex.h"
#include "../DataTypes/NotifierIndex.h"
#include "../ESPEasy/net/DataTypes/NetworkIndex.h"
#include "../DataStructs/DeviceStruct.h"
#include "../Helpers/KeyValueWriter.h"

#ifdef ESP32
# include <NetworkInterface.h>
#endif

/*********************************************************************************************\
* EventStruct
* This should not be copied, only moved.
* When copy is really needed, use deep_copy
\*********************************************************************************************/
struct EventStruct
{
  EventStruct() = default;

  // Delete the copy constructor
  EventStruct(const struct EventStruct&event) = delete;

private:

  // Hide the copy assignment operator by making it private
  EventStruct& operator=(const EventStruct&) = default;

public:

  EventStruct(struct EventStruct&&event)            = default;
  EventStruct& operator=(struct EventStruct&&other) = default;

  explicit EventStruct(taskIndex_t taskIndex);

  // Explicit deep_copy function to make sure this object is not accidentally copied using the copy-constructor
  // Copy constructor and assignment operator should not be used.
  void deep_copy(const struct EventStruct&other);
  void deep_copy(const struct EventStruct*other);

  //  explicit EventStruct(const struct EventStruct& event);
  //  EventStruct& operator=(const struct EventStruct& other);


  void         setTaskIndex(taskIndex_t taskIndex);

  void         clear();

  // Check (and update) sensorType if not set, plus return (corrected) sensorType
  Sensor_VType getSensorType();

  int64_t      getTimestamp_as_systemMicros() const;
  void         setUnixTimeTimestamp();
  void         setLocalTimeTimestamp();

  String String1;
  String String2;
  String String3;
  String String4;
  String String5;
  String String6;
  #ifdef ESP32
  NetworkInterface*networkInterface = nullptr;
  #endif

  KeyValueWriter* kvWriter = nullptr;

  uint32_t timestamp_sec  = 0u;
  uint32_t timestamp_frac = 0u;
  uint8_t *Data           = nullptr;
  int      idx            = 0;
  union {
    struct {
      int Par1;
      int Par2;
      int Par3;
      int Par4;
      int Par5;
      int Par6;
      int Par7;
      int Par8;
    };

    struct {
      float Parf_1;
      float Parf_2;
      float Parf_3;
      float Parf_4;
      float Parf_5;
      float Parf_6;
      float Parf_7;
      float Parf_8;
    };

    float ParfN[8];
    struct {
      int64_t Par64_1;
      int64_t Par64_2;
      int64_t Par64_3;
      int64_t Par64_4;
    };
    int64_t Par64N[4];

    int ParN[8] = { 0 };

  };

  // The origin of the values in the event. See EventValueSource.h
  EventValueSource::Enum Source          = EventValueSource::Enum::VALUE_SOURCE_NOT_SET;
  taskIndex_t            TaskIndex       = INVALID_TASK_INDEX;                        // index position in TaskSettings array, 0-11
  controllerIndex_t      ControllerIndex = INVALID_CONTROLLER_INDEX;                  // index position in Settings.Controller, 0-3
#if FEATURE_NOTIFIER
  notifierIndex_t NotificationIndex = INVALID_NOTIFIER_INDEX;                         // index position in Settings.Notification, 0-3
#endif
  ESPEasy::net::networkIndex_t NetworkIndex    = ESPEasy::net::INVALID_NETWORK_INDEX; // index position in Settings.NWPluginID, 0-7
  uint8_t                      BaseVarIndex    = 0;
  Sensor_VType                 sensorType      = Sensor_VType::SENSOR_TYPE_NOT_SET;
  uint8_t                      OriginTaskIndex = 0;

};

#endif // DATASTRUCTS_ESPEASY_EVENTSTRUCT_H
