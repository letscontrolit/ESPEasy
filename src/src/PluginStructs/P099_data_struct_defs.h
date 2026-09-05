#pragma once

#include "../../_Plugin_Helper.h"

#ifdef USES_P099

# if !defined(P099_ENABLE_OLD_CONFIG) && defined(ESP32)
#  define P099_ENABLE_OLD_CONFIG  1 // When set to 0 will also inhibit the 1-time settings conversion in ESPEasy_TouchHandler
# endif // if !defined(P099_ENABLE_OLD_CONFIG) && defined(ESP32)
# ifdef ESP8266
#  if defined(P099_ENABLE_OLD_CONFIG)
#   undef P099_ENABLE_OLD_CONFIG
#  endif // if defined(P099_ENABLE_OLD_CONFIG)
#  define P099_ENABLE_OLD_CONFIG  0 // Using old implementation, so shouldn't convert anything
# endif // ifdef ESP8266


# if P099_ENABLE_OLD_CONFIG || defined(ESP8266)
#  define P099_FLAGS_ON_OFF_BUTTON 0   // TouchObjects.flags On/Off Button function
#  define P099_FLAGS_INVERT_BUTTON 1   // TouchObjects.flags Inverted On/Off Button function
# endif // if P099_ENABLE_OLD_CONFIG || defined(ESP8266)

# define P099_FLAGS_SEND_XY          0 // Set in P099_CONFIG_FLAGS
# define P099_FLAGS_SEND_Z           1 // Set in P099_CONFIG_FLAGS
# define P099_FLAGS_SEND_OBJECTNAME  2 // Set in P099_CONFIG_FLAGS
# define P099_FLAGS_USE_CALIBRATION  3 // Set in P099_CONFIG_FLAGS
# define P099_FLAGS_LOG_CALIBRATION  4 // Set in P099_CONFIG_FLAGS
# define P099_FLAGS_ROTATION_FLIPPED 5 // Set in P099_CONFIG_FLAGS
# define P099_FLAGS_CONFIG_VTYPE     6 // 4 bits to store the VType
# define P099_FLAGS_CONFIG_DISPLAY  10 // 8 bits to store the Display task

# ifdef ESP32
#  define P099_CONFIG_VERSION     PCONFIG(0)
# endif // ifdef ESP32
# ifdef ESP8266
#  define P099_CONFIG_STATE       PCONFIG(0)
# endif // ifdef ESP8266
# define P099_CONFIG_CS_PIN       PIN(0)
# define P099_CONFIG_THRESHOLD    PCONFIG(1)
# define P099_CONFIG_ROTATION     PCONFIG(2)
# define P099_CONFIG_X_RES        PCONFIG(3)
# define P099_CONFIG_Y_RES        PCONFIG(4)
# if P099_ENABLE_OLD_CONFIG || defined(ESP8266)
#  define P099_CONFIG_OBJECTCOUNT  PCONFIG(5)
#  define P099_CONFIG_DEBOUNCE_MS  PCONFIG(6)
# endif // if P099_ENABLE_OLD_CONFIG || defined(ESP8266)
# define P099_CONFIG_FLAGS        PCONFIG_LONG(0) // 0-31 flags
# ifdef ESP32
#  define P099_CONFIG_DISPLAY_PREV PCONFIG(7)
#  define P099_COLOR_DEPTH         PCONFIG_LONG(1)

#  define P099_GET_CONFIG_VTYPE    get4BitFromUL(P099_CONFIG_FLAGS, P099_FLAGS_CONFIG_VTYPE)
#  define P099_SET_CONFIG_VTYPE(v) set4BitToUL(P099_CONFIG_FLAGS, P099_FLAGS_CONFIG_VTYPE, v)
#  define P099_GET_CONFIG_DISPLAY    get8BitFromUL(P099_CONFIG_FLAGS, P099_FLAGS_CONFIG_DISPLAY)
#  define P099_SET_CONFIG_DISPLAY(v) set8BitToUL(P099_CONFIG_FLAGS, P099_FLAGS_CONFIG_DISPLAY, v)
# endif // ifdef ESP32

# define P099_VALUE_X UserVar.getFloat(event->TaskIndex, 0)
# define P099_VALUE_Y UserVar.getFloat(event->TaskIndex, 1)
# define P099_VALUE_Z UserVar.getFloat(event->TaskIndex, 2)

# define P099_SET_VALUE_X(v) UserVar.setFloat(event->TaskIndex, 0, v)
# define P099_SET_VALUE_Y(v) UserVar.setFloat(event->TaskIndex, 1, v)
# define P099_SET_VALUE_Z(v) UserVar.setFloat(event->TaskIndex, 2, v)

# if P099_ENABLE_OLD_CONFIG || defined(ESP8266)
#  define P099_INIT_OBJECTCOUNT    8   // Initial setting
#  define P099_DEBOUNCE_MILLIS     150 // Debounce delay for On/Off button function
# endif // if P099_ENABLE_OLD_CONFIG || defined(ESP8266)


# if P099_ENABLE_OLD_CONFIG || defined(ESP8266)
#  define P099_MaxObjectNameLength 15 // 14 character objectnames + terminating 0
#  define P099_MaxObjectCount      40 // This count of touchobjects should be enough, because of limited settings storage, 960 bytes + 8
                                      // bytes calibration coordinates


// The settings structures
// Lets define our own coordinate point
struct tP099_Point
{
  uint16_t x = 0;
  uint16_t y = 0;
};

// For touch objects we store a name and 2 coordinates
struct tP099_Touchobjects
{
  char        objectname[P099_MaxObjectNameLength] = { 0 };
  uint8_t     flags                                = 0;
  tP099_Point top_left;
  tP099_Point bottom_right;
};

// Only 2 coordinates used for calibration (we must assume that the touch panel is mounted straight on to tft)
struct tP099_Calibration
{
  tP099_Point top_left;
  tP099_Point bottom_right;
};

// The stuff we want to save between settings (Calibration coordinates and touchable objects)
struct tP099_StoredSettings_struct
{
  tP099_Calibration  Calibration;
  tP099_Touchobjects TouchObjects[P099_MaxObjectCount];
};
# endif // if P099_ENABLE_OLD_CONFIG || defined(ESP8266)

#endif // ifdef USES_P099
