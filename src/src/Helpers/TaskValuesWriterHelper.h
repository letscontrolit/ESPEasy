#pragma once

#include "../../ESPEasy_common.h"

#include "../DataTypes/DeviceIndex.h"

// Helper function to create formatted custom values for display in the devices overview page.
// When called from PLUGIN_WEBFORM_SHOW_VALUES, the last item should add a traling div_br class
// if the regular values should also be displayed.
// The call to PLUGIN_WEBFORM_SHOW_VALUES should only return success = true when no regular values should be displayed
// Note that the varNr of the custom values should not conflict with the existing variable numbers (e.g. start at VARS_PER_TASK)
struct TaskValuesWriterHelper {
  TaskValuesWriterHelper(struct EventStruct *event);

  void clear();

  void writeRegularTaskValues();
#if FEATURE_STRING_VARIABLES
  void writeDerivedTaskValues();
#endif

  bool initRegularTaskValue(uint8_t varNr);

  void setID(uint8_t varNr);

  void write();


  EventStruct * const event = nullptr;
  String              valName, valName_id, value, value_id;
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
  String uom, uom_id;
#endif
#if FEATURE_STRING_VARIABLES
  String presentation, presentation_id;
  bool   hasPresentation = false;
#endif // if FEATURE_STRING_VARIABLES

  uint8_t       nrDecimals{};
  uint8_t       valueCount{};
  uint8_t       valueNumber{};
  deviceIndex_t deviceIndex      = INVALID_DEVICE_INDEX;
  bool          addTrailingBreak = false;

};
