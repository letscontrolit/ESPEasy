#include "../DataStructs/Caches.h"

#include "../Globals/Device.h"
#include "../Globals/Settings.h"
#include "../Globals/WiFi_AP_Candidates.h"

#include <ESPeasySerial.h>


void Caches::clearAllCaches()
{
  fileExistsMap.clear();
  updateTaskCaches();
  WiFi_AP_Candidates.clearCache();
}

void Caches::updateTaskCaches() {
  taskIndexName.clear();
  taskIndexValueName.clear();
  updateActiveTaskUseSerial0();

  for (int i = 0; i < (TASKS_MAX * VARS_PER_TASK); ++i) {
    taskValueDecimals[i] = -1;
  }
}

void Caches::updateActiveTaskUseSerial0() {
  activeTaskUseSerial0 = false;

  // Check to see if a task is enabled and using the pins we also use for receiving commands.
  // We're now receiving only from Serial0, so check if an enabled task is also using it.
  for (taskIndex_t task = 0; validTaskIndex(task); ++task)
  {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(task);

    if (Settings.TaskDeviceEnabled[task] && validDeviceIndex(DeviceIndex)) {
      if ((Device[DeviceIndex].Type == DEVICE_TYPE_SERIAL) ||
          (Device[DeviceIndex].Type == DEVICE_TYPE_SERIAL_PLUS1)) {
        switch (ESPeasySerialType::getSerialType(
                  ESPEasySerialPort::not_set,
                  Settings.TaskDevicePin1[task],
                  Settings.TaskDevicePin2[task]))
        {
          case ESPEasySerialPort::serial0_swap:
          case ESPEasySerialPort::serial0:
            activeTaskUseSerial0 = true;
          default:
            break;
        }
      }
    }
  }
}

int Caches::taskValueIndex(taskIndex_t taskIndex, uint8_t rel_index)
{
  //  TASKS_MAX * VARS_PER_TASK
  if (!validTaskIndex(taskIndex)) { return -1; }

  if (rel_index >= VARS_PER_TASK) { return -1; }
  return taskIndex * TASKS_MAX + rel_index;
}

int8_t Caches::getTaskValueDecimals(taskIndex_t taskIndex, uint8_t rel_index) const
{
  const int index = taskValueIndex(taskIndex, rel_index);

  if (index < 0) { return -1; }
  return taskValueDecimals[index];
}

void Caches::setTaskValueDecimals(taskIndex_t taskIndex, uint8_t rel_index, int8_t decimals)
{
  const int index = taskValueIndex(taskIndex, rel_index);

  if (index >= 0) {
    taskValueDecimals[index] = decimals;
  }
}

