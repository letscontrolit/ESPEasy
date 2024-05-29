#ifndef DATASTRUCTS_SCHEDULER_SYSTEMEVENTQUEUETIMERID_H
#define DATASTRUCTS_SCHEDULER_SYSTEMEVENTQUEUETIMERID_H

#include "../DataStructs/SchedulerTimerID.h"

#include "../DataTypes/SchedulerPluginPtrType.h"

// Create mixed ID for scheduling a system event to be handled by the scheduler.
// ptr_type: Indicating whether it should be handled by controller, plugin or notifier
// Index   : DeviceIndex / ProtocolIndex / NotificationProtocolIndex  (thus not the Plugin_ID/CPlugin_ID/NPlugin_ID, saving an extra lookup
// when processing)
// Function: The function to be called for handling the event.
struct SystemEventQueueTimerID : SchedulerTimerID {
  SystemEventQueueTimerID(SchedulerPluginPtrType_e ptr_type,
                          uint8_t                  Index,
                          uint8_t                  Function);

  uint8_t getFunction() const {
    return static_cast<uint8_t>((getId()) & 0xFF);
  }

  uint8_t getIndex() const {
    return static_cast<uint8_t>((getId() >> 8) & 0xFF);
  }

  SchedulerPluginPtrType_e getPtrType() const {
    return static_cast<SchedulerPluginPtrType_e>((getId() >> 16) & 0xFF);
  }

#ifndef BUILD_NO_DEBUG
  String decode() const;
#endif // ifndef BUILD_NO_DEBUG
};


#endif // ifndef DATASTRUCTS_SCHEDULER_SYSTEMEVENTQUEUETIMERID_H
