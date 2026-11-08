#pragma once

#include "../../ESPEasy_common.h"

#include "../DataStructs/ValueStruct.h"
#include "../DataTypes/DeviceIndex.h"

// Helper function to create formatted custom values for display in the devices overview page.
// When called from PLUGIN_WEBFORM_SHOW_VALUES, the last item should add a traling div_br class
// if the regular values should also be displayed.
// The call to PLUGIN_WEBFORM_SHOW_VALUES should only return success = true when no regular values should be displayed
// Note that the varNr of the custom values should not conflict with the existing variable numbers (e.g. start at VARS_PER_TASK)
struct TaskValuesWriterHelper {

  enum class ID_type {
    ValueName,
    Value,
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
    UoM,
#endif
  #if FEATURE_STRING_VARIABLES
    Presentation
#endif

  };


  TaskValuesWriterHelper(struct EventStruct *event);

  void   clear();

  bool   isEmpty() const;

  String format_ID(ID_type id) const;

  void   setWriteRegularTaskValuesFirst() { _writeRegularTaskValuesFirst = true; }

  void   setPreformatted()                { _preformatted = true; }


  void   write();

  void   writeTaskValues();

  void   writeCustom(uint8_t                    varNr,
                     const __FlashStringHelper *label,
                     const String             & val,
                     bool                       isLast = false);
  void writeCustom(uint8_t       varNr,
                   const String& label,
                   const String& val,
                   bool          isLast = false);

  void writeCustom(uint8_t       varNr,
                   const String& label,
                   const String& val,
                   ValueStruct&& attr,
                   bool          isLast = false);

private:

  void writeRegularTaskValues();
#if FEATURE_STRING_VARIABLES
  void writeDerivedTaskValues();
#endif

  void pluginWebformShowValue();

  bool initRegularTaskValue(uint8_t varNr);

public:

  EventStruct * const event = nullptr;
  String              valName, value;
  ValueStruct attribute;
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
  String uom;
#endif
#if FEATURE_STRING_VARIABLES
  String presentation;
  bool   hasPresentation = false;
#endif // if FEATURE_STRING_VARIABLES

  uint8_t       nrDecimals{};
  uint8_t       valueCount{};
  uint8_t       valueNumber{};
  deviceIndex_t deviceIndex = INVALID_DEVICE_INDEX;

  bool _writeRegularTaskValuesFirst{};
  bool _preformatted{};
  bool _isLast{};

};
