#ifndef PLUGINSTRUCTS_P016_DATA_STRUCT_H
#define PLUGINSTRUCTS_P016_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P016

# include <IRremoteESP8266.h>

# include <vector>

# define PLUGIN_016_DEBUG // additional debug messages in the log
# if defined(LIMIT_BUILD_SIZE) && defined(PLUGIN_016_DEBUG)
#  undef PLUGIN_016_DEBUG
# endif // if defined(LIMIT_BUILD_SIZE) && defined(PLUGIN_016_DEBUG)

// bit definition in PCONFIG_LONG(0)
# define P016_BitAddNewCode  0        // Add automatically new code into Code of the command structure
# define P016_BitExecuteCmd  1        // Execute command if received code matches Code or AlternativeCode of the command structure
# define P016_BitAcceptUnknownType  2 // Accept unknown DecodeType as valid IR code (will be set to RAW before calling AddCode() or
                                      // ExecuteCode())

# define P16_Nlines   10              // The number of different lines which can be displayed - each line is 64 chars max
# define P16_Nchars   64              // max chars per command line
# define P16_Cchars   20              // max chars per code
# define P016_DEFAULT_BUFFERSIZE 150  // default buffer size, range: 100..1024
# define P016_MIN_BUFFERSIZE     100  // Minimum
# define P016_MAX_BUFFERSIZE     1024 // Maximum buffer size

# define P16_SETTINGS_V1              // Settings v1 original settings when enabled, settings conversion is also enabled
// Settings v2 includes 64 bit codes and some separated flags

# ifdef P16_SETTINGS_V1
#  define P16_CMDBIT_REPEAT 23 // Only used for V1 settings
# endif // ifdef P16_SETTINGS_V1

# define P16_SETTINGS_LATEST 2 // Latest version is V2

// Settings flags, max. 16 bits (0..15) can be used
# define P16_FLAGS_REPEAT    0 // Repeat code
// # define P16_FLAGS_HASH      1  // Code is a Hash

# ifndef P016_FEATURE_COMMAND_HANDLING
#  define P016_FEATURE_COMMAND_HANDLING 1 // Enable Command handling and table
# endif // ifndef P016_FEATURE_COMMAND_HANDLING

# if !P016_FEATURE_COMMAND_HANDLING && defined(P16_SETTINGS_V1)
#  undef P16_SETTINGS_V1
# endif // if !P016_FEATURE_COMMAND_HANDLING && defined(P16_SETTINGS_V1)

# ifdef P16_SETTINGS_V1
typedef struct {
  char     Command[P16_Nchars] = { 0 };
  uint32_t Code                = 0; // received code (can be added automatically)
  uint32_t AlternativeCode     = 0; // alternative code fpr the same command
} tCommandLinesV1;
# endif // ifdef P16_SETTINGS_V1

# if P016_FEATURE_COMMAND_HANDLING
struct tCommandLinesV2 {
  #  ifdef P16_SETTINGS_V1
  tCommandLinesV2() = default;
  tCommandLinesV2(const tCommandLinesV1& lineV1,
                  uint8_t                i);
  #  endif // ifdef P16_SETTINGS_V1

  char          Command[P16_Nchars]       = { 0 };
  uint64_t      Code                      = 0; // received code (can be added automatically)
  uint64_t      AlternativeCode           = 0; // alternative code fpr the same command
  decode_type_t CodeDecodeType            = decode_type_t::UNKNOWN;
  decode_type_t AlternativeCodeDecodeType = decode_type_t::UNKNOWN;
  uint16_t      CodeFlags                 = 0;
  uint16_t      AlternativeCodeFlags      = 0;
};
# endif // if P016_FEATURE_COMMAND_HANDLING

struct P016_data_struct : public PluginTaskData_base {
public:

  P016_data_struct() = default;
  virtual ~P016_data_struct() = default;

  void        init(struct EventStruct *event,
                   uint16_t            CmdInhibitTime);
  # if P016_FEATURE_COMMAND_HANDLING
  void        loadCommandLines(struct EventStruct *event);
  void        saveCommandLines(struct EventStruct *event);

  static void loadCommandLine(struct EventStruct *event,
                              tCommandLinesV2   & line,
                              uint8_t             lineNr);
  #  ifdef P16_SETTINGS_V1
  static void loadCommandLinev1(struct EventStruct *event,
                                tCommandLinesV2   & line,
                                uint8_t             lineNr);
  #  endif // ifdef P16_SETTINGS_V1
  static void saveCommandLine(struct EventStruct    *event,
                              const tCommandLinesV2& line,
                              uint8_t                lineNr);

  void AddCode(uint64_t      Code,
               decode_type_t DecodeType = decode_type_t::UNKNOWN,
               uint16_t      CodeFlags  = 0u);
  void ExecuteCode(uint64_t      Code,
                   decode_type_t DecodeType = decode_type_t::UNKNOWN,
                   uint16_t      CodeFlags  = 0u);

  // CustomTaskSettings
  std::vector<tCommandLinesV2>CommandLines; // holds the CustomTaskSettings V2

  bool bCodeChanged = false;                // set if code has been added and CommandLines need to be saved (in PLUGIN_ONCE_A_SECOND)
  # endif // if P016_FEATURE_COMMAND_HANDLING

private:

  # if P016_FEATURE_COMMAND_HANDLING
  bool validateCode(int           i,
                    uint64_t      Code,
                    decode_type_t DecodeType,
                    uint16_t      CodeFlags);

  uint64_t      iLastCmd        = 0;                      // last command send
  uint32_t      iLastCmdTime    = 0;                      // time while last command was send
  decode_type_t iLastDecodeType = decode_type_t::UNKNOWN; // last decode_type sent
  uint16_t      iCmdInhibitTime = 0;                      // inhibit time for sending the same command again
  uint16_t      iLastCodeFlags  = 0;                      // last flags sent
  # endif // if P016_FEATURE_COMMAND_HANDLING
};

#endif // ifdef USES_P016
#endif // ifndef PLUGINSTRUCTS_P016_DATA_STRUCT_H
