#ifndef PLUGINSTRUCTS_P021_DATA_STRUCT_H
#define PLUGINSTRUCTS_P021_DATA_STRUCT_H

// Moved here from _P021_Level.ino as the Arduino compiler doesn't (properly) support #ifdef in .ino files
#define PLUGIN_021_DEBUG

// For additional debugging information use PLUGIN_021_DEBUG (see note)
#ifdef BUILD_NO_DEBUG
# undef PLUGIN_021_DEBUG
#endif // ifndef/else BUILD_NO_DEBUG

// See note at top of file
#ifdef LIMIT_BUILD_SIZE
# define FEATURE_P021_EXTRAS  0
# define P021_MIN_BUILD_SIZE
# undef PLUGIN_021_DEBUG
#else // ifdef LIMIT_BUILD_SIZE
# define FEATURE_P021_EXTRAS  1
#endif // ifdef LIMIT_BUILD_SIZE

# define P021_GPIO_RELAY            CONFIG_PIN1

// Define the configuration options using ESPeasy standard structures
// For unique identification on the HTML form use P021_GUID_<option>
// ------------------------------------------------------------------

// Input signal is read from another task (plugin) by accessing its TASK & VALUE
# define P021_CHECK_TASK            PCONFIG(0)
# define P021_GUID_CHECK_TASK       "f0"
# define P021_CHECK_VALUE           PCONFIG(1)
# define P021_GUID_CHECK_VALUE      "f1"

// Flag to prevent excessive writing. Backwards compatible with previous version
# define P021_DONT_ALWAYS_SAVE      PCONFIG(2)
# define P021_GUID_DONT_ALWAYS_SAVE "f2"

// Set of individual bits packed in a single PCONFIG(3)
// Use bitRead() and bitWrite() to access the individual bits
// For backwards compatibility first bit shall be equal original P021_INVERT_OUTPUT
# define P021_FLAGS                 PCONFIG(3)
# define P021_INV_OUTPUT            0 // Invert output signal
# define P021_GUID_INV_OUTPUT       "b0"
# define P021_EXT_FUNCT             1 // Extended functionality on setup screen
# define P021_GUID_EXT_FUNCT        "b1"
# define P021_INV_INPUT             2 // Invert input comparator
# define P021_GUID_INV_INPUT        "b2"
# define P021_SYM_HYSTERESIS        3 // Use symetrical hysteresis
# define P021_GUID_SYM_HYSTERESIS   "b3"
# define P021_SLOW_EVAL             4 // 1Hz evaluation i.s.o. 10Hz
# define P021_GUID_SLOW_EVAL        "b4"
# define P021_STATE_OUTP            5 // Add state as additional output value
# define P021_GUID_STATE_OUTP       "b5"
# define P021_EXTEND_END            6 // Extend state timed from start
# define P021_GUID_EXTEND_END       "b6"
# define P021_LONG_TIMER_UNIT       7 // Use longer timer units on setup screen
# define P021_GUID_LONG_TIMER_UNIT  "b7"

// Operation mode [enum]
# define P021_OPMODE                PCONFIG(4)
# define P021_GUID_OPMODE           "f4"

// Setpoint (set level)
# define P021_SETPOINT              PCONFIG_FLOAT(0)
# define P021_GUID_SETPOINT         "f5"

// Hysteresis
# define P021_HYSTERESIS            PCONFIG_FLOAT(1)
# define P021_GUID_HYSTERESIS       "f6"

// SETP_LAST_STORED is not available on form. Used as storage for autosave function
# define P021_SETP_LAST_STORED      PCONFIG_FLOAT(2)
# define P021_GUID_SETP_LAST_STORED "f7"

// Auto save time interval to update remotely changed setpoints to flash
# define P021_AUTOSAVE_TIMER        PCONFIG_ULONG(0)
# define P021_GUID_AUTOSAVE_TIMER   "f8"

// Minimum time output shall be active [sec]/[min]
# define P021_MIN_TIME              PCONFIG_ULONG(1)
# define P021_GUID_MIN_TIME         "f9"

// Maximum time output may be idle [sec]/[hour]
# define P021_INTERVAL_TIME         PCONFIG_ULONG(2)
# define P021_GUID_INTERVAL_TIME    "f10"

// Duration for forced circulation [sec]/[min]
# define P021_FORCE_TIME            PCONFIG_ULONG(3)
# define P021_GUID_FORCE_TIME       "f11"

// Positions in userVar array for the output values of this plugin
// For now we only advertise output & state. Autosave bookkeeping is hidden and used as static storage
# define P021_VALUE_OUTPUT           0 // Switch output, logical state [inactive/active]
# define P021_VALUE_STATE            1 // Control state, see P021_control_state
# define P021_VALUE_AUTOSAVE_FLAG    2 // Autosave bookkeeping
# define P021_VALUE_AUTOSAVE_TIME    3 // Autosave bookkeeping

// Simple conversion from parameter settings in hours/minutes to seconds (administration unit)
// And from seconds to the millis domain used for the actual control
// Note that these simple conversion may lose precision due to rough rounding


#endif // ifndef PLUGINSTRUCTS_P021_DATA_STRUCT_H
