#ifndef PLUGINSTRUCTS_P104_DATA_STRUCT_H
#define PLUGINSTRUCTS_P104_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P104

# define P104_DEBUG // Log some extra (tech) data, useful during development

# include "../CustomBuild/StorageLayout.h"
# include "src/Globals/EventQueue.h"
# include "src/Globals/MQTT.h"
# include "src/Globals/CPlugins.h"
# include "src/Globals/Plugins.h"
# include "src/Helpers/ESPEasy_Storage.h"
# include "src/Helpers/Hardware.h"
# include "src/Helpers/Misc.h"
# include "src/Helpers/StringParser.h"

# define P104_USE_NUMERIC_DOUBLEHEIGHT_FONT // Enables double height numeric font used by double-height time/date display
// # define P104_MINIMAL_ANIMATIONS            // disable most animations
// # define P104_MEDIUM_ANIMATIONS             // disable some complex animations


# define P104_MAX_MESG  15           // Message size for time/date

# ifdef ESP32
#  define P104_MAX_ZONES        16u  // 1..P104_MAX_ZONES zones selectable
#  define P104_SETTINGS_BUFFER  1020 // Bigger buffer possible on ESP32
# else // ifdef ESP32
#  define P104_MAX_ZONES        8u   // 1..P104_MAX_ZONES zones selectable
#  define P104_SETTINGS_BUFFER  512
# endif // ifdef ESP32

# define P104_MAX_MODULES_PER_ZONE 64      // Maximum allowed modules per zone
# define P104_MAX_TEXT_LENGTH_PER_ZONE 100 // Limit the Text content length

# define P104_USE_TOOLTIPS                 // Enable tooltips in UI

# ifdef LIMIT_BUILD_SIZE
  #  ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT

// #   undef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
  #  endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
  #  ifdef P104_DEBUG
  #   undef P104_DEBUG
  #  endif // ifdef P104_DEBUG
  #  ifdef P104_USE_TOOLTIPS
  #   undef P104_USE_TOOLTIPS
  #  endif // ifdef P104_USE_TOOLTIPS
# endif    // ifdef LIMIT_BUILD_SIZE

# if defined(P104_USE_TOOLTIPS) && !defined(ENABLE_TOOLTIPS)
  #  undef P104_USE_TOOLTIPS
# endif // if defined(P104_USE_TOOLTIPS) && !defined(ENABLE_TOOLTIPS)


# ifdef P104_MINIMAL_ANIMATIONS
#  define ENA_MISC 0 // Disabling some MD_Parola features
#  define ENA_WIPE 0
#  define ENA_SCAN 0
#  define P104_MEDIUM_ANIMATIONS
# endif // ifdef P104_MINIMAL_ANIMATIONS

# ifdef P104_MEDIUM_ANIMATIONS
#  define ENA_SPRITE  0 // Disabling more MD_Parola features
#  define ENA_OPNCLS  0
#  define ENA_SCR_DIA 0
#  define ENA_GROW    0
# endif // ifdef P104_MEDIUM_ANIMATIONS

# include <MD_Parola.h>
# include <MD_MAX72xx.h>

// WARNING: Order of values should match the numeric order of P104_OFFSET_* values
# define P104_OFFSET_SIZE         0u
# define P104_OFFSET_TEXT         1u
# define P104_OFFSET_CONTENT      2u
# define P104_OFFSET_ALIGNMENT    3u
# define P104_OFFSET_ANIM_IN      4u
# define P104_OFFSET_SPEED        5u
# define P104_OFFSET_ANIM_OUT     6u
# define P104_OFFSET_PAUSE        7u
# define P104_OFFSET_FONT         8u
# define P104_OFFSET_LAYOUT       9u
# define P104_OFFSET_SPEC_EFFECT  10u
# define P104_OFFSET_OFFSET       11u
# define P104_OFFSET_BRIGHTNESS   12u
# define P104_OFFSET_REPEATDELAY  13u

# define P104_OFFSET_COUNT        14u // Highest P104_OFFSET_* defined + 1

# define P104_CONFIG_ZONE_COUNT   PCONFIG(0)
# define P104_CONFIG_TOTAL_UNITS  PCONFIG(1)
# define P104_CONFIG_HARDWARETYPE PCONFIG(2)

# define P104_CONTENT_TEXT        0
# define P104_CONTENT_TIME        1
# define P104_CONTENT_TIME_SEC    2
# define P104_CONTENT_DATE4       3
# define P104_CONTENT_DATE6       4
# define P104_CONTENT_DATE6_YYYY  5
# define P104_CONTENT_DATE_TIME   6

# define P104_SPECIAL_EFFECT_NONE       0
# define P104_SPECIAL_EFFECT_UP_DOWN    1
# define P104_SPECIAL_EFFECT_LEFT_RIGHT 2
# define P104_SPECIAL_EFFECT_BOTH       P104_SPECIAL_EFFECT_UP_DOWN + P104_SPECIAL_EFFECT_LEFT_RIGHT // Used as a bitmap

# define P104_LAYOUT_STANDARD     0
# define P104_LAYOUT_DOUBLE_UPPER 1
# define P104_LAYOUT_DOUBLE_LOWER 2

# define P104_DEFAULT_FONT_ID     0

# ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT

#  define P104_DOUBLE_HEIGHT_FONT_ID 1

// each char table is specific for each display and maps all numbers/symbols
MD_MAX72XX::fontType_t numeric7SegDouble[] PROGMEM =
{
  0,                                                                                      // 0
  0,                                                                                      // 1
  0,                                                                                      // 2
  0,                                                                                      // 3
  0,                                                                                      // 4
  0,                                                                                      // 5
  0,                                                                                      // 6
  0,                                                                                      // 7
  0,                                                                                      // 8
  0,                                                                                      // 9
  0,                                                                                      // 10
  0,                                                                                      // 11
  0,                                                                                      // 12
  0,                                                                                      // 13
  0,                                                                                      // 14
  0,                                                                                      // 15
  0,                                                                                      // 16
  0,                                                                                      // 17
  0,                                                                                      // 18
  0,                                                                                      // 19
  0,                                                                                      // 20
  0,                                                                                      // 21
  0,                                                                                      // 22
  0,                                                                                      // 23
  0,                                                                                      // 24
  0,                                                                                      // 25
  0,                                                                                      // 26
  0,                                                                                      // 27
  0,                                                                                      // 28
  0,                                                                                      // 29
  0,                                                                                      // 30
  0,                                                                                      // 31
  2,  0,    0,                                                                            // 32 - 'Space'
  0,                                                                                      // 33 - '!'
  0,                                                                                      // 34 - '"'
  0,                                                                                      // 35 - '#'
  0,                                                                                      // 36 - '$'
  0,                                                                                      // 37 - '%'
  0,                                                                                      // 38 - '&'
  0,                                                                                      // 39 - '''
  0,                                                                                      // 40 - '('
  0,                                                                                      // 41 - ')'
  0,                                                                                      // 42 - '*'
  0,                                                                                      // 43 - '+'
  0,                                                                                      // 44 - ','
  0,                                                                                      // 45 - '-'
  2,  48,   48,                                                                           // 46 - '.'
  0,                                                                                      // 47 - '/'
  10, 62,   127,    192,      192,      192,      192,      192,      192,      127, 62,  // 48 - '0'
  10, 0,    0,      0,        0,        0,        0,        0,        0,        127, 62,  // 49 - '1'
  10, 62,   127,    193,      193,      193,      193,      193,      193,      64, 0,    // 50 - '2'
  10, 0,    65,     193,      193,      193,      193,      193,      193,      127, 62,  // 51 - '3'
  10, 0,    0,      1,        1,        1,        1,        1,        1,        127, 62,  // 52 - '4'
  10, 0,    64,     193,      193,      193,      193,      193,      193,      127, 62,  // 53 - '5'
  10, 62,   127,    193,      193,      193,      193,      193,      193,      127, 62,  // 54 - '6'
  10, 0,    0,      0,        0,        0,        0,        0,        0,        127, 62,  // 55 - '7'
  10, 62,   127,    193,      193,      193,      193,      193,      193,      127, 62,  // 56 - '8'
  10, 0,    64,     193,      193,      193,      193,      193,      193,      127, 62,  // 57 - '9'
  2,  6,    6,                                                                            // 58 - ':'
  0,                                                                                      // 59 - ';'
  0,                                                                                      // 60 - '<'
  0,                                                                                      // 61 - '='
  0,                                                                                      // 62 - '>'
  0,                                                                                      // 63 - '?'
  0,                                                                                      // 64 - '@'
  10, 62,   127,    1,        1,        1,        1,        1,        1,        127, 62,  // 65 - 'A'
  10, 62,   127,    193,      193,      193,      193,      193,      193,      127, 62,  // 66 - 'B'
  10, 62,   127,    193,      193,      193,      193,      193,      193,      65, 0,    // 67 - 'C'
  10, 62,   127,    193,      193,      193,      193,      193,      193,      127, 62,  // 68 - 'D'
  10, 62,   127,    193,      193,      193,      193,      193,      193,      65, 0,    // 69 - 'E'
  10, 62,   127,    1,        1,        1,        1,        1,        1,        1, 0,     // 70 - 'F'
  0,                                                                                      // 71 - 'G'
  0,                                                                                      // 72 - 'H'
  0,                                                                                      // 73 - 'I'
  0,                                                                                      // 74 - 'J'
  0,                                                                                      // 75 - 'K'
  0,                                                                                      // 76 - 'L'
  0,                                                                                      // 77 - 'M'
  0,                                                                                      // 78 - 'N'
  0,                                                                                      // 79 - 'O'
  0,                                                                                      // 80 - 'P'
  0,                                                                                      // 81 - 'Q'
  0,                                                                                      // 82 - 'R'
  0,                                                                                      // 83 - 'S'
  0,                                                                                      // 84 - 'T'
  0,                                                                                      // 85 - 'U'
  0,                                                                                      // 86 - 'V'
  0,                                                                                      // 87 - 'W'
  0,                                                                                      // 88 - 'X'
  0,                                                                                      // 89 - 'Y'
  0,                                                                                      // 90 - 'Z'
  0,                                                                                      // 91 - '['
  0,                                                                                      // 92 - '\'
  0,                                                                                      // 93 - ']'
  0,                                                                                      // 94 - '^'
  0,                                                                                      // 95 - '_'
  0,                                                                                      // 96 - '`'
  10, 62,   127,    1,        1,        1,        1,        1,        1,        127, 62,  // 97 - 'a'
  10, 62,   127,    193,      193,      193,      193,      193,      193,      127, 62,  // 98 - 'b'
  10, 62,   127,    193,      193,      193,      193,      193,      193,      65, 0,    // 99 - 'c'
  10, 62,   127,    193,      193,      193,      193,      193,      193,      127, 62,  // 100 - 'd'
  10, 62,   127,    193,      193,      193,      193,      193,      193,      65, 0,    // 101 - 'e'
  10, 62,   127,    1,        1,        1,        1,        1,        1,        1, 0,     // 102 - 'f'
  0,                                                                                      // 103 - 'g'
  0,                                                                                      // 104 - 'h'
  0,                                                                                      // 105 - 'i'
  0,                                                                                      // 106 - 'j'
  0,                                                                                      // 107 - 'k'
  0,                                                                                      // 108 - 'l'
  0,                                                                                      // 109 - 'm'
  0,                                                                                      // 110 - 'n'
  0,                                                                                      // 111 - 'o'
  0,                                                                                      // 112 - 'p'
  0,                                                                                      // 113 - 'q'
  0,                                                                                      // 114 - 'r'
  0,                                                                                      // 115 - 's'
  0,                                                                                      // 116 - 't'
  0,                                                                                      // 117 - 'u'
  0,                                                                                      // 118 - 'v'
  0,                                                                                      // 119 - 'w'
  0,                                                                                      // 120 - 'x'
  0,                                                                                      // 121 - 'y'
  0,                                                                                      // 122 - 'z'
  0,                                                                                      // 123 - '{'
  2,  255,  255,                                                                          // 124 - '|'
  0,                                                                                      // 125
  0,                                                                                      // 126
  0,                                                                                      // 127
  0,                                                                                      // 128
  0,                                                                                      // 129
  0,                                                                                      // 130
  0,                                                                                      // 131
  0,                                                                                      // 132
  0,                                                                                      // 133
  0,                                                                                      // 134
  0,                                                                                      // 135
  0,                                                                                      // 136
  0,                                                                                      // 137
  0,                                                                                      // 138
  0,                                                                                      // 139
  0,                                                                                      // 140
  0,                                                                                      // 141
  0,                                                                                      // 142
  0,                                                                                      // 143
  0,                                                                                      // 144
  0,                                                                                      // 145
  0,                                                                                      // 146
  0,                                                                                      // 147
  0,                                                                                      // 148
  0,                                                                                      // 149
  0,                                                                                      // 150
  0,                                                                                      // 151
  0,                                                                                      // 152
  0,                                                                                      // 153
  0,                                                                                      // 154
  0,                                                                                      // 155
  0,                                                                                      // 156
  0,                                                                                      // 157
  0,                                                                                      // 158
  0,                                                                                      // 159
  2,  0,    0,                                                                            // 160 high space
  0,                                                                                      // 161
  0,                                                                                      // 162
  0,                                                                                      // 163
  0,                                                                                      // 164
  0,                                                                                      // 165
  0,                                                                                      // 166
  0,                                                                                      // 167
  0,                                                                                      // 168
  0,                                                                                      // 169
  0,                                                                                      // 170
  0,                                                                                      // 171
  0,                                                                                      // 172
  0,                                                                                      // 173
  2,  0,    0,                                                                            // 174 high period
  0,                                                                                      // 175
  10, 124,  254,    3,        3,        3,        3,        3,        3,        254, 124, // 176 - '0'
  10, 0,    0,      0,        0,        0,        0,        0,        0,        254, 124, // 177 - '1'
  10, 0,    2,      131,      131,      131,      131,      131,      131,      254, 124, // 178 - '2'
  10, 0,    130,    131,      131,      131,      131,      131,      131,      254, 124, // 179 - '3'
  10, 124,  254,    128,      128,      128,      128,      128,      128,      254, 124, // 180 - '4'
  10, 124,  254,    131,      131,      131,      131,      131,      131,      2, 0,     // 181 - '5'
  10, 124,  254,    131,      131,      131,      131,      131,      131,      2, 0,     // 182 - '6'
  10, 0,    2,      3,        3,        3,        3,        3,        3,        254, 124, // 183 - '7'
  10, 124,  254,    131,      131,      131,      131,      131,      131,      254, 124, // 184 - '8'
  10, 124,  254,    131,      131,      131,      131,      131,      131,      254, 124, // 185 - '9'
  2,  96,   96,                                                                           // 186 - ':'
  0,                                                                                      // 187
  0,                                                                                      // 188
  0,                                                                                      // 189
  0,                                                                                      // 190
  0,                                                                                      // 191
  0,                                                                                      // 192
  10, 124,  254,    131,      131,      131,      131,      131,      131,      254, 124, // 193 - 'A'
  10, 124,  254,    128,      128,      128,      128,      128,      128,      0, 0,     // 194 - 'B'
  10, 0,    0,      128,      128,      128,      128,      128,      128,      0, 0,     // 195 - 'C'
  10, 0,    0,      128,      128,      128,      128,      128,      128,      254, 124, // 196 - 'D'
  10, 124,  254,    131,      131,      131,      131,      131,      131,      130, 0,   // 197 - 'E'
  10, 124,  254,    131,      131,      131,      131,      131,      131,      130, 0,   // 198 - 'F'
  0,                                                                                      // 199
  0,                                                                                      // 200
  0,                                                                                      // 201
  0,                                                                                      // 202
  0,                                                                                      // 203
  0,                                                                                      // 204
  0,                                                                                      // 205
  0,                                                                                      // 206
  0,                                                                                      // 207
  0,                                                                                      // 208
  0,                                                                                      // 209
  0,                                                                                      // 210
  0,                                                                                      // 211
  0,                                                                                      // 212
  0,                                                                                      // 213
  0,                                                                                      // 214
  0,                                                                                      // 215
  0,                                                                                      // 216
  0,                                                                                      // 217
  0,                                                                                      // 218
  0,                                                                                      // 219
  0,                                                                                      // 220
  0,                                                                                      // 221
  0,                                                                                      // 222
  0,                                                                                      // 223
  0,                                                                                      // 224
  10, 124,  254,    131,      131,      131,      131,      131,      131,      254, 124, // 225 - 'a'
  10, 124,  254,    128,      128,      128,      128,      128,      128,      0, 0,     // 226 - 'b'
  10, 0,    0,      128,      128,      128,      128,      128,      128,      0, 0,     // 227 - 'c'
  10, 0,    0,      128,      128,      128,      128,      128,      128,      254, 124, // 228 - 'd'
  10, 124,  254,    131,      131,      131,      131,      131,      131,      130, 0,   // 229 - 'e'
  10, 124,  254,    131,      131,      131,      131,      131,      131,      130, 0,   // 230 - 'f'
  0,                                                                                      // 231
  0,                                                                                      // 232
  0,                                                                                      // 233
  0,                                                                                      // 234
  0,                                                                                      // 235
  0,                                                                                      // 236
  0,                                                                                      // 237
  0,                                                                                      // 238
  0,                                                                                      // 239
  0,                                                                                      // 240
  0,                                                                                      // 241
  0,                                                                                      // 242
  0,                                                                                      // 243
  0,                                                                                      // 244
  0,                                                                                      // 245
  0,                                                                                      // 246
  0,                                                                                      // 247
  0,                                                                                      // 104
  0,                                                                                      // 249
  0,                                                                                      // 250
  0,                                                                                      // 251
  2,  255,  255,                                                                          // 252 - '|'
  0,                                                                                      // 253
  0,                                                                                      // 254
  0,                                                                                      // 255
};
# endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT

struct P104_zone_struct {
  P104_zone_struct(uint8_t _zone) : zone(_zone) {}

  uint8_t  zone;
  uint8_t  size;
  String   text;
  uint8_t  alignment;
  uint8_t  animationIn, animationOut;
  uint8_t  font;
  uint8_t  content;
  uint8_t  layout;
  uint8_t  specialEffect;
  uint8_t  offset;
  uint8_t  brightness;
  uint16_t speed, pause;
  int32_t  repeatDelay;
};

struct tP104_StoredSettings {
  uint16_t bufferSize = 0u;
  char     buffer[P104_SETTINGS_BUFFER]; // Max. acceptable value would be ~1020
};

struct P104_data_struct : public PluginTaskData_base {
  P104_data_struct(MD_MAX72XX::moduleType_t _mod,
                   taskIndex_t              _taskIndex,
                   int8_t                   _cs_pin,
                   byte                     _modules);

  bool   begin();
  void   loadSettings();
  bool   saveSettings();
  bool   webform_load(struct EventStruct *event);
  bool   webform_save(struct EventStruct *event);
  String getError() {
    return error;
  }

  void configureZones();

  void setZones(uint16_t _zones) {
    expectedZones = _zones;
  }

  bool handlePluginWrite(taskIndex_t   taskIndex,
                         const String& string);
  bool handlePluginOncePerSecond(taskIndex_t taskIndex);

  MD_Parola *P; // = MD_Parola(mod, /*din_pin, clk_pin,*/ cs_pin, modules);

private:

  void displayOneZoneText(uint8_t                 currentZone,
                          const P104_zone_struct& idx,
                          const String          & text);

  MD_MAX72XX::moduleType_t mod;

  taskIndex_t taskIndex;
  int8_t      din_pin, clk_pin, cs_pin;
  uint8_t     modules = 1u;

  bool    initialized   = false;
  int8_t  expectedZones = -1;
  int8_t  previousZones = -1;
  uint8_t numDevices    = 0;

  String error;

  std::vector<P104_zone_struct>zones;
  bool                         zonesInitialized = false;

  // time/date stuff
  bool   flasher = false;        // seconds passing flasher
  char   szTimeL[P104_MAX_MESG]; // dd-mm-yy mm:ss\0
  char   szTimeH[P104_MAX_MESG];
  String sZoneBuffers[P104_MAX_ZONES];
  int8_t lastDay = -1;

  // Stored settings
  tP104_StoredSettings StoredSettings;
};

#endif // ifdef USES_P104
#endif // ifndef PLUGINSTRUCTS_P104_DATA_STRUCT_H
