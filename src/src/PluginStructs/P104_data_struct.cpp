#include "../PluginStructs/P104_data_struct.h"

#ifdef USES_P104

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Numerical.h"
# include "../WebServer/Markup_Forms.h"
# include "../WebServer/WebServer.h"
# include "../WebServer/Markup.h"
# include "../WebServer/HTML_wrappers.h"
# include "../ESPEasyCore/ESPEasyRules.h"
# include "../Globals/ESPEasy_time.h"

# include <vector>
# include <MD_Parola.h>
# include <MD_MAX72xx.h>

// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter

# if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
void createHString(String& string);
# endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

P104_data_struct::P104_data_struct(MD_MAX72XX::moduleType_t _mod,
                                   taskIndex_t              _taskIndex,
                                   int8_t                   _cs_pin,
                                   uint8_t                  _modulesize)
  : mod(_mod), taskIndex(_taskIndex), cs_pin(_cs_pin), modules(_modulesize) {
  if (Settings.InitSPI < 1) {
    addLog(LOG_LEVEL_ERROR, F("DOTMATRIX: Required SPI not enabled. Initialization aborted!"));
  } else {
    P = new MD_Parola(mod, cs_pin, modules);
  }
}

bool P104_data_struct::begin() {
  if (!initialized) {
    loadSettings();
    initialized = true;
  }

  if ((P != nullptr) && (cs_pin > -1)) {
    addLog(LOG_LEVEL_INFO, F("dotmatrix: P->begin() called"));
    P->begin(expectedZones);
    return true;
  }
  return false;
}

# define P104_ZONE_SEP   '\x02'
# define P104_FIELD_SEP  '\x01'
# define P104_ZONE_DISP  ';'
# define P104_FIELD_DISP ','

/**************************************
 * loadSettings
 *************************************/
void P104_data_struct::loadSettings() {
  if (taskIndex < TASKS_MAX) {
    LoadCustomTaskSettings(taskIndex, (byte *)&StoredSettings, sizeof(StoredSettings.bufferSize));
    uint16_t structDataSize = StoredSettings.bufferSize + sizeof(StoredSettings.bufferSize);
    # ifdef P104_DEBUG
    String log;

    if (loglevelActiveFor(LOG_LEVEL_INFO) &&
        log.reserve(54)) {
      log  = F("P104: loadSettings stored Size: ");
      log += structDataSize;
      log += F(" taskindex: ");
      log += taskIndex;
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG
    LoadCustomTaskSettings(taskIndex, (byte *)&StoredSettings, structDataSize); // Read only actual data
    StoredSettings.buffer[StoredSettings.bufferSize + 1] = '\0';                // Terminate string

    uint8_t zoneIndex = 0;

    {
      String buffer;
      buffer.reserve(StoredSettings.bufferSize + 1);
      buffer = String(StoredSettings.buffer);
      # ifdef P104_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log  = F("P104: loadSettings bufferSize: ");
        log += StoredSettings.bufferSize;
        log += F(" untrimmed: ");
        log += buffer.length();
      }
      # endif // ifdef P104_DEBUG
      buffer.trim();
      # ifdef P104_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F(" trimmed: ");
        log += buffer.length();
        addLog(LOG_LEVEL_INFO, log);
      }
      # endif // ifdef P104_DEBUG

      zones.clear();
      zonesInitialized = false;
      numDevices       = 0;

      int16_t  offset2;
      uint16_t prev2 = 0;
      String   tmp, fld, tmp_val;
      int tmp_int;
      offset2 = buffer.indexOf(P104_ZONE_SEP);

      while (offset2 > -1) {
        tmp.reserve(offset2 - prev2);
        tmp = buffer.substring(prev2, offset2);

        zones.push_back(P104_zone_struct(zoneIndex + 1));

        tmp_int = 0;

        // WARNING: Order of values should match the numeric order of P104_OFFSET_* values

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_SIZE, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].size = tmp_int;
        }

        zones[zoneIndex].text = parseStringKeepCase(tmp, 1 + P104_OFFSET_TEXT, P104_FIELD_SEP);

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_ALIGNMENT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].alignment = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_ANIM_IN, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].animationIn = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_SPEED, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].speed = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_ANIM_OUT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].animationOut = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_PAUSE, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].pause = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_FONT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].font = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_CONTENT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].content = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_LAYOUT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].layout = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_SPEC_EFFECT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].specialEffect = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_OFFSET, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].offset = tmp_int;
        }

        tmp_val = parseString(tmp, 1 + P104_OFFSET_BRIGHTNESS, P104_FIELD_SEP);

        if (tmp_val.isEmpty() || !validIntFromString(tmp_val, tmp_int)) {
          zones[zoneIndex].brightness = 7;
        } else {
          zones[zoneIndex].brightness = tmp_int;
        }

        tmp_val = parseString(tmp, 1 + P104_OFFSET_REPEATDELAY, P104_FIELD_SEP);

        if (tmp_val.isEmpty() || !validIntFromString(tmp_val, tmp_int)) {
          zones[zoneIndex].repeatDelay = -1;
        } else {
          zones[zoneIndex].repeatDelay = tmp_int;
        }

        delay(0);
        prev2       = offset2 + 1;
        offset2     = buffer.indexOf(P104_ZONE_SEP, prev2);
        numDevices += zones[zoneIndex].size + zones[zoneIndex].offset;
        zoneIndex++;
      }
      buffer.reserve(0); // Free some memory
    }
    # ifdef P104_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("P104: read zones from config: ");
      log += zoneIndex;

      // log += F(" struct size: ");
      // log += sizeof(P104_zone_struct);
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG

    if (expectedZones == -1) { expectedZones = zoneIndex; }

    while (zoneIndex < expectedZones) {
      zones.push_back(P104_zone_struct(zoneIndex + 1));

      zones[zoneIndex].size          = 0u;
      zones[zoneIndex].alignment     = 0u;
      zones[zoneIndex].animationIn   = 1u; // Doesn't allow 'None'
      zones[zoneIndex].speed         = 0u;
      zones[zoneIndex].animationOut  = 0u;
      zones[zoneIndex].pause         = 0u;
      zones[zoneIndex].font          = 0u;
      zones[zoneIndex].content       = 0u;
      zones[zoneIndex].layout        = 0u;
      zones[zoneIndex].specialEffect = 0u;
      zones[zoneIndex].offset        = 0u;
      zones[zoneIndex].brightness    = 7u; // 32Just below average brightness 1..15
      zones[zoneIndex].repeatDelay   = -1; // Off by default
      zoneIndex++;
      delay(0);
    }
    zonesInitialized = true;
    # ifdef P104_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("P104: total zones initialized: ");
      log += zoneIndex;
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG
  }
}

/****************************************************
 * configureZones: initialize Zones setup
 ***************************************************/
void P104_data_struct::configureZones() {
  if (!zonesInitialized) {
    loadSettings();
  }

  uint8_t currentZone = 0;
  uint8_t zoneOffset  = 0;

  # ifdef P104_DEBUG
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO) &&
      log.reserve(45)) {
    log  = F("P104: configureZones to do: ");
    log += zones.size();
    addLog(LOG_LEVEL_INFO, log);
  }
  # endif // ifdef P104_DEBUG

  P->displayClear();

  for (auto it = zones.begin(); it != zones.end(); ++it) {
    if (it->zone <= expectedZones) {
      zoneOffset += it->offset;
      P->setZone(currentZone, zoneOffset, zoneOffset + it->size - 1);
      zoneOffset += it->size;

      switch (it->font) {
        case P104_DEFAULT_FONT_ID: {
          P->setFont(nullptr); // default font
          break;
        }
        # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
        case P104_DOUBLE_HEIGHT_FONT_ID: {
          P->setFont(currentZone, numeric7SegDouble);
          P->setCharSpacing(P->getCharSpacing() * 2); // double spacing as well
          break;
        }
        # endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
        # ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
        case P104_FULL_DOUBLEHEIGHT_FONT_ID: {
          P->setFont(currentZone, BigFont);
          P->setCharSpacing(P->getCharSpacing() * 2); // double spacing as well
          break;
        }
        # endif // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
        # ifdef P104_USE_VERTICAL_FONT
        case P104_VERTICAL_FONT_ID: {
          P->setFont(currentZone, _fontVertical);
          break;
        }
        # endif // ifdef P104_USE_VERTICAL_FONT
        # ifdef P104_USE_EXT_ASCII_FONT
        case P104_EXT_ASCII_FONT_ID: {
          P->setFont(currentZone, ExtASCII);
          break;
        }
        # endif // ifdef P104_USE_EXT_ASCII_FONT
        # ifdef P104_USE_ARABIC_FONT
        case P104_ARABIC_FONT_ID: {
          P->setFont(currentZone, fontArabic);
          break;
        }
        # endif // ifdef P104_USE_ARABIC_FONT
        # ifdef P104_USE_GREEK_FONT
        case P104_GREEK_FONT_ID: {
          P->setFont(currentZone, fontGreek);
          break;
        }
        # endif // ifdef P104_USE_GREEK_FONT
        # ifdef P104_USE_KATAKANA_FONT
        case P104_KATAKANA_FONT_ID: {
          P->setFont(currentZone, fontKatakana);
          break;
        }
        # endif // ifdef P104_USE_KATAKANA_FONT

          // Extend here with more fonts if/when available
      }

      // Special Effects
      P->setZoneEffect(currentZone, (it->specialEffect & P104_SPECIAL_EFFECT_UP_DOWN) == P104_SPECIAL_EFFECT_UP_DOWN,       PA_FLIP_UD);
      P->setZoneEffect(currentZone, (it->specialEffect & P104_SPECIAL_EFFECT_LEFT_RIGHT) == P104_SPECIAL_EFFECT_LEFT_RIGHT, PA_FLIP_LR);

      // Brightness
      P->setIntensity(currentZone, it->brightness);

      # ifdef P104_DEBUG
      log  = F("P104: configureZones #");
      log += (currentZone + 1);
      log += '/';
      log += expectedZones;
      log += F(" offset: ");
      log += zoneOffset;
      addLog(LOG_LEVEL_INFO, log);
      # endif // ifdef P104_DEBUG

      // Content == text && text != ""
      if ((it->content == P104_CONTENT_TEXT) && (!it->text.isEmpty())) {
        displayOneZoneText(currentZone, *it, it->text);
      }
      currentZone++;
    }
  }
}

/**********************************************************
 * Display the text with attributes for a specific zone
 *********************************************************/
void P104_data_struct::displayOneZoneText(uint8_t                 zone,
                                          const P104_zone_struct& zstruct,
                                          const String          & text) {
  if ((zone < 0) || (zone > P104_MAX_ZONES)) { return; } // double check
  sZoneBuffers[zone] = String(text); // We explicitly want a copy here so it can be modified by parseTemplate()

  parseTemplate(sZoneBuffers[zone]);

  # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

  if (zstruct.layout == P104_LAYOUT_DOUBLE_LOWER) {
    createHString(sZoneBuffers[zone]);
  }
  # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO) &&
      logAllText &&
      log.reserve(28 + text.length() + sZoneBuffers[zone].length())) {
    log  = F("dotmatrix: ZoneText: ");
    log += zone + 1; // UI-number
    log += F(", '");
    log += text;
    log += F("' -> '");
    log += sZoneBuffers[zone];
    log += '\'';
    addLog(LOG_LEVEL_INFO, log);
  }

  P->displayZoneText(zone,
                     sZoneBuffers[zone].c_str(),
                     static_cast<textPosition_t>(zstruct.alignment),
                     zstruct.speed,
                     zstruct.pause,
                     static_cast<textEffect_t>(zstruct.animationIn),
                     static_cast<textEffect_t>(zstruct.animationOut));
}

/*******************************************************
 * handlePluginWrite : process commands
 ******************************************************/
bool P104_data_struct::handlePluginWrite(taskIndex_t   taskIndex,
                                         const String& string) {
  bool   reconfigure = false;
  bool   success     = false;
  String command     = parseString(string, 1);

  if (command.equals(F("dotmatrix"))) { // main command: dotmatrix
    String sub = parseString(string, 2);

    int zoneIndex;
    int value4;
    String string4 = parseStringKeepCase(string, 4);
    validIntFromString(string4, value4);

    if (validIntFromString(parseString(string, 3), zoneIndex) &&
        (zoneIndex > 0) &&
        (static_cast<unsigned int>(zoneIndex) <= zones.size())) {
      if (sub.equals(F("clear")) && // subcommand: clear[,all]
          (string4.isEmpty() ||
           string4.equalsIgnoreCase(F("all")))) {
        P->displayClear();
        success = true;
      }

      for (auto it = zones.begin(); it != zones.end() && !success; ++it) {
        if ((it->zone == zoneIndex) &&  // This zone, and animation is done
            P->getZoneStatus(zoneIndex - 1)) {
          if (sub.equals(F("clear"))) { // subcommand: clear,<zone>
            P->displayClear(zoneIndex - 1);
            success = true;
            break;
          }

          if (sub.equals(F("txt")) &&               // subcommand: txt,<zone>,<text> (only allowed for zones with Text content)
              (it->content == P104_CONTENT_TEXT)) { // no length check, so longer than the UI allows is made possible
            displayOneZoneText(zoneIndex - 1, *it, string4);
            success = true;
            break;
          }

          if (sub.equals(F("content")) && // subcommand: content,zone,<contenttype> (0..<P104_CONTENT_count>-1)
              (value4 >= 0) &&
              (value4 < P104_CONTENT_count)) {
            it->content = value4;
            reconfigure = true;
            break;
          }

          if (sub.equals(F("font")) && // subcommand: font,zone,<font id> (only for incuded font id's)
              (
                (value4 == 0)
                # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
                || (value4 == P104_DOUBLE_HEIGHT_FONT_ID)
                # endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
                # ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
                || (value4 == P104_FULL_DOUBLEHEIGHT_FONT_ID)
                # endif // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
                # ifdef P104_USE_VERTICAL_FONT
                || (value4 == P104_VERTICAL_FONT_ID)
                # endif // ifdef P104_USE_VERTICAL_FONT
                # ifdef P104_USE_EXT_ASCII_FONT
                || (value4 == P104_EXT_ASCII_FONT_ID)
                # endif // ifdef P104_USE_EXT_ASCII_FONT
                # ifdef P104_USE_ARABIC_FONT
                || (value4 == P104_ARABIC_FONT_ID)
                # endif // ifdef P104_USE_ARABIC_FONT
                # ifdef P104_USE_GREEK_FONT
                || (value4 == P104_GREEK_FONT_ID)
                # endif // ifdef P104_USE_GREEK_FONT
                # ifdef P104_USE_KATAKANA_FONT
                || (value4 == P104_KATAKANA_FONT_ID)
                # endif // ifdef P104_USE_KATAKANA_FONT
              )
              ) {
            it->content = value4;
            reconfigure = true;
            break;
          }

          # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

          if (sub.equals(F("layout")) && // subcommand: layout,zone,<layout> (0..2), only when double-height font is available
              (value4 >= 0) &&
              (value4 <= P104_LAYOUT_DOUBLE_LOWER)) {
            it->layout  = value4;
            reconfigure = true;
            break;
          }
          # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

          if (sub.equals(F("size")) && // subcommand: size,zone,<size> (1..)
              (value4 > 0) &&
              (value4 <= P104_MAX_MODULES_PER_ZONE)) {
            it->size    = value4;
            reconfigure = true;
            break;
          }

          if (sub.equals(F("offset")) && // subcommand: offset,zone,<size> (0..<size>-1)
              (value4 >= 0) &&
              (value4 < P104_MAX_MODULES_PER_ZONE) &&
              (value4 < it->size)) {
            it->offset  = value4;
            reconfigure = true;
            break;
          }

          if (sub.equals(F("alignment")) &&                             // subcommand: alignment,zone,<alignment> (0..3)
              (value4 >= 0) &&
              (value4 <= static_cast<int>(textPosition_t::PA_RIGHT))) { // last item in the enum
            it->alignment = value4;
            reconfigure   = true;
            break;
          }

          if (sub.equals(F("anim.in")) && // subcommand: anim.in,zone,<animation> (1..<PA_last_value>-1)
              (value4 > 0) &&
              (value4 < static_cast<int>(textEffect_t::PA_last_value))) {
            it->animationIn = value4;
            reconfigure     = true;
            break;
          }

          if (sub.equals(F("anim.out")) && // subcommand: anim.out,zone,<animation> (0..<PA_last_value>-1)
              (value4 > 0) &&
              (value4 < static_cast<int>(textEffect_t::PA_last_value))) {
            it->animationOut = value4;
            reconfigure      = true;
            break;
          }

          if (sub.equals(F("speed")) && // subcommand: speed,zone,<speed_ms> (0..65535)
              (value4 >= 0) &&
              (value4 <= 65535)) {
            it->speed   = value4;
            reconfigure = true;
            break;
          }

          if (sub.equals(F("pause")) && // subcommand: pause,zone,<pause_ms> (0..65535)
              (value4 >= 0) &&
              (value4 <= 65535)) {
            it->pause   = value4;
            reconfigure = true;
            break;
          }

          if (sub.equals(F("specialeffect")) && // subcommand: specialeffect,zone,<effect> (0..3)
              (value4 >= 0) &&
              (value4 <= P104_SPECIAL_EFFECT_BOTH)) {
            it->specialEffect = value4;
            reconfigure       = true;
            break;
          }

          if (sub.equals(F("brightness")) && // subcommand: brightness,zone,<brightness> (0..15)
              (value4 >= 0) &&
              (value4 <= P104_BRIGHTNESS_MAX)) {
            it->brightness = value4;
            P->setIntensity(zoneIndex - 1, it->brightness); // Change brightness directly
            success = true;
            break;
          }

          if (sub.equals(F("repeat")) && // subcommand: repeaat,zone,<repeat_sec> (-1..86400 = 24h)
              (value4 >= -1) &&
              (value4 <= 86400)) {
            it->repeatDelay = value4;
            reconfigure     = true;
            break;
          }
        }
      }
    }
  }

  if (reconfigure) {
    configureZones(); // Re-initialize
    success = true;   // Successful
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;

    if (log.reserve(32 + string.length())) {
      log = F("dotmatrix: command ");

      if (!success) { log += F("UN"); }
      log += F("succesful: ");
      log += string;
      addLog(LOG_LEVEL_INFO, log);
    }
  }

  return success; // Default: unknown command
}

void getTime(char *psz,
             bool  seconds = false,
             bool  colon   = true) {
  uint16_t h, m, s;

  h = node_time.hour();
  m = node_time.minute();

  if (!seconds) {
    sprintf(psz, "%02d%c%02d", h, (colon ? ':' : ' '), m);
  } else {
    s = node_time.second();
    sprintf(psz, "%02d%c%02d %02d", h, (colon ? ':' : ' '), m, s);
  }
}

void getDate(char *psz,
             bool  year    = true,
             bool  fourDgt = false) {
  uint16_t d, m, y;

  d = node_time.day();
  m = node_time.month();

  if (year) {
    y = node_time.year() - (fourDgt ? 0 : 2000); // Will we survive into 21xx?
    sprintf(psz, "%02d %02d %02d", d, m, y);     // last %02d will expand to 04 when needed
  } else {
    sprintf(psz, "%02d %02d", d, m);
  }
}

void getDateTime(char *psz,
                 bool  colon = true) {
  uint16_t d, M, y, h, m;

  d = node_time.day();
  M = node_time.month();

  y = node_time.year() - 2000; // Will we survive into 21xx?
  h = node_time.hour();
  m = node_time.minute();
  sprintf(psz, "%02d %02d %02d %02d%c%02d", d, M, y, h, (colon ? ':' : ' '), m);
}

# if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
void createHString(String& string) {
  for (uint32_t i = 0; i < string.length(); i++) {
    string[i] |= 0x80;
  }
}

# endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

bool P104_data_struct::handlePluginOncePerSecond(taskIndex_t taskIndex) {
  bool retval     = false;
  bool newFlasher = !flasher;

  for (auto it = zones.begin(); it != zones.end(); ++it) {
    retval = false;

    switch (it->content) {
      case P104_CONTENT_TIME:     // time
      case P104_CONTENT_TIME_SEC: // time sec
      {
        getTime(szTimeL, it->content == 2, flasher);
        flasher = newFlasher;
        retval  = true;
        break;
      }
      case P104_CONTENT_DATE4:      // date/4
      case P104_CONTENT_DATE6:      // date/6
      case P104_CONTENT_DATE6_YYYY: // date/7
      {
        if (lastDay != node_time.day()) {
          getDate(szTimeL, it->content != 3, it->content == 5);
          retval  = true;
          lastDay = node_time.day();
        }
        break;                     // TODO
      }
      case P104_CONTENT_DATE_TIME: // date-time/9
      {
        getDateTime(szTimeL, flasher);
        flasher = newFlasher;
        retval  = true;
        break; // TODO
      }
      default:
        break;
    }

    if (retval) {
      displayOneZoneText(it->zone - 1, *it, String(szTimeL));
      P->displayReset(it->zone - 1);
    }
  }

  if (retval) {
    // synchronise the start
    P->synchZoneStart();
  }
  return retval;
}

/******************************************
 * enquoteString wrap in ", ' or ` unless all 3 quote types are used
 *****************************************/
String enquoteString(const String& input) {
  char quoteChar = '"';

  if (input.indexOf(quoteChar) > -1) {
    quoteChar = '\'';

    if (input.indexOf(quoteChar) > -1) {
      quoteChar = '`';

      if (input.indexOf(quoteChar) > -1) {
        return input; // All types of supported quotes used, return original string
      }
    }
  }
  String result;

  result.reserve(input.length() + 2);
  result  = quoteChar;
  result += input;
  result += quoteChar;

  return result;
}

/***************************************
 * saveSettings gather the zones data from the UI and store in customsettings
 **************************************/
bool P104_data_struct::saveSettings() {
  error = EMPTY_STRING; // Clear
  String zbuffer;

  # ifdef P104_DEBUG
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO) &&
      log.reserve(64)) {
    log  = F("P104: saving zones, count: ");
    log += expectedZones;
    addLog(LOG_LEVEL_INFO, log);
  }
  # endif // ifdef P104_DEBUG

  uint8_t zoneIndexP1;
  uint8_t index = 0;

  zones.clear(); // Start afresh

  for (uint8_t zoneIndex = 0; zoneIndex < expectedZones; zoneIndex++) {
    zoneIndexP1 = zoneIndex + 1;
    zones.push_back(P104_zone_struct(zoneIndexP1));

    zones[zoneIndex].size          = getFormItemIntCustomArgName(index + P104_OFFSET_SIZE);
    zones[zoneIndex].text          = enquoteString(webArg(getPluginCustomArgName(index + P104_OFFSET_TEXT)));
    zones[zoneIndex].content       = getFormItemIntCustomArgName(index + P104_OFFSET_CONTENT);
    zones[zoneIndex].alignment     = getFormItemIntCustomArgName(index + P104_OFFSET_ALIGNMENT);
    zones[zoneIndex].animationIn   = getFormItemIntCustomArgName(index + P104_OFFSET_ANIM_IN);
    zones[zoneIndex].speed         = getFormItemIntCustomArgName(index + P104_OFFSET_SPEED);
    zones[zoneIndex].animationOut  = getFormItemIntCustomArgName(index + P104_OFFSET_ANIM_OUT);
    zones[zoneIndex].pause         = getFormItemIntCustomArgName(index + P104_OFFSET_PAUSE);
    zones[zoneIndex].font          = getFormItemIntCustomArgName(index + P104_OFFSET_FONT);
    zones[zoneIndex].layout        = getFormItemIntCustomArgName(index + P104_OFFSET_LAYOUT);
    zones[zoneIndex].specialEffect = getFormItemIntCustomArgName(index + P104_OFFSET_SPEC_EFFECT);
    zones[zoneIndex].offset        = getFormItemIntCustomArgName(index + P104_OFFSET_OFFSET);

    if (previousZones == expectedZones) {
      zones[zoneIndex].brightness  = getFormItemIntCustomArgName(index + P104_OFFSET_BRIGHTNESS);
      zones[zoneIndex].repeatDelay = getFormItemIntCustomArgName(index + P104_OFFSET_REPEATDELAY);
    } else {
      zones[zoneIndex].brightness  = 7u;
      zones[zoneIndex].repeatDelay = -1;
    }
    # ifdef P104_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("P104: add zone: ");
      log += zoneIndexP1;
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG

    index += P104_OFFSET_COUNT;
    delay(0);
  }

  uint16_t bufSize = (zones.size() * 58) + 50; // Use actual size and add a little spare
  numDevices = 0;                              // Count the number of connected display units

  if (zbuffer.reserve(bufSize)) {
    for (auto it = zones.begin(); it != zones.end(); ++it) {
      if (it->zone <= expectedZones) { // sizes: (estimated)
        // WARNING: Order of values should match the numeric order of P104_OFFSET_* values
        zbuffer += it->size;           // 2
        zbuffer += P104_FIELD_SEP;     // 1
        zbuffer += it->text;           // ~15
        zbuffer += P104_FIELD_SEP;     // 1
        zbuffer += it->content;        // 1
        zbuffer += P104_FIELD_SEP;     // 1
        zbuffer += it->alignment;      // 1
        zbuffer += P104_FIELD_SEP;     // 1
        zbuffer += it->animationIn;    // 2
        zbuffer += P104_FIELD_SEP;     // 1
        zbuffer += it->speed;          // 5
        zbuffer += P104_FIELD_SEP;     // 1
        zbuffer += it->animationOut;   // 2
        zbuffer += P104_FIELD_SEP;     // 1
        zbuffer += it->pause;          // 5
        zbuffer += P104_FIELD_SEP;     // 1
        zbuffer += it->font;           // 1
        zbuffer += P104_FIELD_SEP;     // 1
        zbuffer += it->layout;         // 1
        zbuffer += P104_FIELD_SEP;     // 1
        zbuffer += it->specialEffect;  // 1
        zbuffer += P104_FIELD_SEP;     // 1
        zbuffer += it->offset;         // 2
        zbuffer += P104_FIELD_SEP;     // 1

        if (it->brightness != 7) {
          zbuffer += it->brightness;   // 2
        }
        zbuffer += P104_FIELD_SEP;     // 1

        if (it->repeatDelay > -1) {
          zbuffer += it->repeatDelay;  // 4
        }
        zbuffer += P104_FIELD_SEP;     // 1

        zbuffer += P104_ZONE_SEP;      // 1
                                       // 58 total
        numDevices += it->size + it->offset;
        # ifdef P104_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          log  = F("P104: append zone: ");
          log += it->zone;
          log += F(" buffer len: ");
          log += zbuffer.length();
          log += F(" size: ");
          log += it->size;
          addLog(LOG_LEVEL_INFO, log);
        }
        # endif // ifdef P104_DEBUG
      }
      delay(0);
    }
    zbuffer.trim();
    StoredSettings.bufferSize = zbuffer.length();
    # ifdef P104_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("P104: saveSettings zones: ");
      log += expectedZones;
      log += F(" buffer length: ");
      log += StoredSettings.bufferSize;
      log += F(" numDevices: ");
      log += numDevices;
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG
  } else {
    addLog(LOG_LEVEL_ERROR, F("DOTMATRIX: Can't allocate string for saving settings, insufficient memory!"));
    return false; // Don't continue
  }

  if (StoredSettings.bufferSize < sizeof(StoredSettings.buffer)) {
    if (!safe_strncpy(StoredSettings.buffer, zbuffer.c_str(), StoredSettings.bufferSize + 1)) {
      error.reserve(42);
      error += F("Moving settings to config buffer failed.\n");
      addLog(LOG_LEVEL_ERROR, error);
      addLog(LOG_LEVEL_ERROR, zbuffer);
      return false;
    }
    uint16_t structDataSize = StoredSettings.bufferSize + sizeof(StoredSettings.bufferSize);
    # ifdef P104_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("P104: saveSettings structSize: ");
      log += structDataSize;
      log += F(" taskindex: ");
      log += taskIndex;
      addLog(LOG_LEVEL_INFO, log);
      zbuffer.replace(P104_FIELD_SEP, P104_FIELD_DISP);
      zbuffer.replace(P104_ZONE_SEP,  P104_ZONE_DISP);
      addLog(LOG_LEVEL_INFO, zbuffer);
    }
    # endif // ifdef P104_DEBUG
    zbuffer.clear(); // Clear string after reporting any errors
    error += SaveCustomTaskSettings(taskIndex, (byte *)&StoredSettings, structDataSize);
    return error.isEmpty();
  }
  error.reserve(55);
  error += F("Total combination of Zones & text too long to store.\n");
  addLog(LOG_LEVEL_ERROR, error);
  return false;
}

/**************************************************************
* webform_load
**************************************************************/
bool P104_data_struct::webform_load(struct EventStruct *event) {
  addFormSubHeader(F("Device settings"));

  {                                       // Hardware types
    # define P104_hardwareTypeCount 8
    const __FlashStringHelper *hardwareTypes[P104_hardwareTypeCount] = {
      F("Generic (DR:0, CR:1, RR:0)"),    // 010
      F("Parola (DR:1, CR:1, RR:0)"),     // 110
      F("FC16 (DR:1, CR:0, RR:0)"),       // 100
      F("IC Station (DR:1, CR:1, RR:1)"), // 111
      F("Other 1 (DR:0, CR:0, RR:0)"),    // 000
      F("Other 2 (DR:0, CR:0, RR:1)"),    // 001
      F("Other 3 (DR:0, CR:1, RR:1)"),    // 011
      F("Other 4 (DR:1, CR:0, RR:1)")     // 101
    };
    int hardwareOptions[P104_hardwareTypeCount] = {
      static_cast<int>(MD_MAX72XX::moduleType_t::GENERIC_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::PAROLA_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::FC16_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::ICSTATION_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::DR0CR0RR0_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::DR0CR0RR1_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::DR0CR1RR1_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::DR1CR0RR1_HW)
    };
    addFormSelector(F("Hardware type"),
                    F("plugin_104_hardware"),
                    P104_hardwareTypeCount,
                    hardwareTypes,
                    hardwareOptions,
                    P104_CONFIG_HARDWARETYPE);
    addFormNote(F("DR = Digits as Rows, CR = Column Reversed, RR = Row Reversed; 0 = no, 1 = yes."));
  }

  {
    addFormCheckBox(F("Clear display on disable"), F("plugin_104_cleardisable"), bitRead(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_CLEAR_DISABLE));

    addFormCheckBox(F("Log all text"),             F("plugin_104_logalltext"),   bitRead(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_LOG_ALL_TEXT));
  }

  { // Zones
    String zonesList[P104_MAX_ZONES];
    int    zonesOptions[P104_MAX_ZONES];

    for (uint8_t i = 0; i < P104_MAX_ZONES; i++) {
      zonesList[i]    = String(i + 1);
      zonesOptions[i] = i + 1; // No 0 needed or wanted
    }
    String zonetip;

    if (zonetip.reserve(58)) {
      zonetip  = F("Select between 1 and ");
      zonetip += P104_MAX_ZONES;
      zonetip += F(" zones, changing will save and reload the page.");
    }
    addFormSelector(F("Zones"), F("plugin_104_zone"), P104_MAX_ZONES, zonesList, zonesOptions, NULL, P104_CONFIG_ZONE_COUNT, true
                    # ifdef P104_USE_TOOLTIPS
                    , zonetip
                    # endif // ifdef P104_USE_TOOLTIPS
                    );
    addFormNote(zonetip);
  }
  expectedZones = P104_CONFIG_ZONE_COUNT;

  if (expectedZones == 0) { expectedZones++; } // Minimum of 1 zone

  loadSettings();

  {
    const __FlashStringHelper *alignmentTypes[3] = {
      F("Left"),
      F("Center"),
      F("Right")
    };
    int alignmentOptions[3] = {
      static_cast<int>(textPosition_t::PA_LEFT),
      static_cast<int>(textPosition_t::PA_CENTER),
      static_cast<int>(textPosition_t::PA_RIGHT)
    };
    int animationCount = 6;
    # if ENA_SPRITE
    animationCount += 1;
    # endif // ENA_SPRITE
    # if ENA_MISC
    animationCount += 6;
    # endif // ENA_MISC
    # if ENA_WIPE
    animationCount += 2;
    # endif // ENA_WIPE
    # if ENA_SCAN
    animationCount += 4;
    # endif // ENA_SCAN
    # if ENA_OPNCLS
    animationCount += 4;
    # endif // ENA_OPNCLS
    # if ENA_SCR_DIA
    animationCount += 4;
    # endif // ENA_SCR_DIA
    # if ENA_GROW
    animationCount += 2;
    # endif // ENA_GROW
    String animationTypes[] {
      F("None")
      , F("Print")
      , F("Scroll up")
      , F("Scroll down")
      , F("Scroll left *")
      , F("Scroll right *")
    # if ENA_SPRITE
      , F("Sprite")
    # endif // ENA_SPRITE
    # if ENA_MISC
      , F("Slice *")
      , F("Mesh")
      , F("Fade")
      , F("Dissolve")
      , F("Blinds")
      , F("Random")
    # endif // ENA_MISC
    # if ENA_WIPE
      , F("Wipe")
      , F("Wipe w. cursor")
    # endif // ENA_WIPE
    # if ENA_SCAN
      , F("Scan horiz.")
      , F("Scan horiz. cursor")
      , F("Scan vert.")
      , F("Scan vert. cursor")
    # endif // ENA_SCAN
    # if ENA_OPNCLS
      , F("Opening")
      , F("Opening w. cursor")
      , F("Closing")
      , F("Closing w. cursor")
    # endif // ENA_OPNCLS
    # if ENA_SCR_DIA
      , F("Scroll up left *")
      , F("Scroll up right *")
      , F("Scroll down left *")
      , F("Scroll down right *")
    # endif // ENA_SCR_DIA
    # if ENA_GROW
      , F("Grow up")
      , F("Grow down")
    # endif // ENA_GROW
    };
    int animationOptions[] = {
      static_cast<int>(textEffect_t::PA_NO_EFFECT)
      , static_cast<int>(textEffect_t::PA_PRINT)
      , static_cast<int>(textEffect_t::PA_SCROLL_UP)
      , static_cast<int>(textEffect_t::PA_SCROLL_DOWN)
      , static_cast<int>(textEffect_t::PA_SCROLL_LEFT)
      , static_cast<int>(textEffect_t::PA_SCROLL_RIGHT)
    # if ENA_SPRITE
      , static_cast<int>(textEffect_t::PA_SPRITE)
    # endif // ENA_SPRITE
    # if ENA_MISC
      , static_cast<int>(textEffect_t::PA_SLICE)
      , static_cast<int>(textEffect_t::PA_MESH)
      , static_cast<int>(textEffect_t::PA_FADE)
      , static_cast<int>(textEffect_t::PA_DISSOLVE)
      , static_cast<int>(textEffect_t::PA_BLINDS)
      , static_cast<int>(textEffect_t::PA_RANDOM)
    # endif // ENA_MISC
    # if ENA_WIPE
      , static_cast<int>(textEffect_t::PA_WIPE)
      , static_cast<int>(textEffect_t::PA_WIPE_CURSOR)
    # endif // ENA_WIPE
    # if ENA_SCAN
      , static_cast<int>(textEffect_t::PA_SCAN_HORIZ)
      , static_cast<int>(textEffect_t::PA_SCAN_HORIZX)
      , static_cast<int>(textEffect_t::PA_SCAN_VERT)
      , static_cast<int>(textEffect_t::PA_SCAN_VERTX)
    # endif // ENA_SCAN
    # if ENA_OPNCLS
      , static_cast<int>(textEffect_t::PA_OPENING)
      , static_cast<int>(textEffect_t::PA_OPENING_CURSOR)
      , static_cast<int>(textEffect_t::PA_CLOSING)
      , static_cast<int>(textEffect_t::PA_CLOSING_CURSOR)
    # endif // ENA_OPNCLS
    # if ENA_SCR_DIA
      , static_cast<int>(textEffect_t::PA_SCROLL_UP_LEFT)
      , static_cast<int>(textEffect_t::PA_SCROLL_UP_RIGHT)
      , static_cast<int>(textEffect_t::PA_SCROLL_DOWN_LEFT)
      , static_cast<int>(textEffect_t::PA_SCROLL_DOWN_RIGHT)
    # endif // ENA_SCR_DIA
    # if ENA_GROW
      , static_cast<int>(textEffect_t::PA_GROW_UP)
      , static_cast<int>(textEffect_t::PA_GROW_DOWN)
    # endif // ENA_GROW
    };

    // Append the numeric value as a reference for the 'anim.in' and 'anim.out' subcommands
    for (uint8_t a = 0; a < animationCount; a++) {
      animationTypes[a] += F(" (");
      animationTypes[a] += a;
      animationTypes[a] += ')';
    }
    delay(0);

    int fontCount = 1;
    # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
    fontCount++;
    # endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
    fontCount++;
    # endif // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_VERTICAL_FONT
    fontCount++;
    # endif // ifdef P104_USE_VERTICAL_FONT
    # ifdef P104_USE_EXT_ASCII_FONT
    fontCount++;
    # endif // ifdef P104_USE_EXT_ASCII_FONT
    # ifdef P104_USE_ARABIC_FONT
    fontCount++;
    # endif // ifdef P104_USE_ARABIC_FONT
    # ifdef P104_USE_GREEK_FONT
    fontCount++;
    # endif // ifdef P104_USE_GREEK_FONT
    # ifdef P104_USE_KATAKANA_FONT
    fontCount++;
    # endif // ifdef P104_USE_KATAKANA_FONT
    const __FlashStringHelper *fontTypes[] = {
      F("Default")
    # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
      , F("Numeric, double height")
    # endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
      , F("Full, double height")
    # endif // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_VERTICAL_FONT
      , F("Vertical")
    # endif // ifdef P104_USE_VERTICAL_FONT
    # ifdef P104_USE_EXT_ASCII_FONT
      , F("Ext. ASCII")
      # endif // ifdef P104_USE_EXT_ASCII_FONT
    # ifdef P104_USE_ARABIC_FONT
      , F("Arabic")
    # endif // ifdef P104_USE_ARABIC_FONT
    # ifdef P104_USE_GREEK_FONT
      , F("Greek")
    # endif // ifdef P104_USE_GREEK_FONT
    # ifdef P104_USE_KATAKANA_FONT
      , F("Katakana")
    # endif // ifdef P104_USE_KATAKANA_FONT
    };
    int fontOptions[] = {
      P104_DEFAULT_FONT_ID
    # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
      , P104_DOUBLE_HEIGHT_FONT_ID
    # endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
      , P104_FULL_DOUBLEHEIGHT_FONT_ID
    # endif // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_VERTICAL_FONT
      , P104_VERTICAL_FONT_ID
    # endif // ifdef P104_USE_VERTICAL_FONT
    # ifdef P104_USE_EXT_ASCII_FONT
      , P104_EXT_ASCII_FONT_ID
      # endif // ifdef P104_USE_EXT_ASCII_FONT
    # ifdef P104_USE_ARABIC_FONT
      , P104_ARABIC_FONT_ID
    # endif // ifdef P104_USE_ARABIC_FONT
    # ifdef P104_USE_GREEK_FONT
      , P104_GREEK_FONT_ID
    # endif // ifdef P104_USE_GREEK_FONT
    # ifdef P104_USE_KATAKANA_FONT
      , P104_KATAKANA_FONT_ID
    # endif // ifdef P104_USE_KATAKANA_FONT
    };

    int layoutCount = 1;
    # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
    layoutCount += 2;
    # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
    const __FlashStringHelper *layoutTypes[] = {
      F("Standard")
    # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
      , F("Double, upper")
      , F("Double, lower")
    # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
    };
    int layoutOptions[] = {
      P104_LAYOUT_STANDARD
    # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
      , P104_LAYOUT_DOUBLE_UPPER
      , P104_LAYOUT_DOUBLE_LOWER
    # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
    };

    int specialEffectCount                          = 4;
    const __FlashStringHelper *specialEffectTypes[] = {
      F("None"),
      F("Flip up/down"),
      F("Flip left/right *"),
      F("Flip u/d &amp; l/r *")
    };
    int specialEffectOptions[] = {
      P104_SPECIAL_EFFECT_NONE,
      P104_SPECIAL_EFFECT_UP_DOWN,
      P104_SPECIAL_EFFECT_LEFT_RIGHT,
      P104_SPECIAL_EFFECT_BOTH
    };

    const __FlashStringHelper *contentTypes[] = {
      F("Text"),
      F("Clock (4 mod.)"),
      F("Clock sec (6 mod.)"),
      F("Date (4 mod.)"),
      F("Date yy (6 mod.)"),
      F("Date yyyy (7 mod.)"),
      F("Date/time (9 mod.)")
    };
    int contentOptions[] {
      P104_CONTENT_TEXT,
      P104_CONTENT_TIME,
      P104_CONTENT_TIME_SEC,
      P104_CONTENT_DATE4,
      P104_CONTENT_DATE6,
      P104_CONTENT_DATE6_YYYY,
      P104_CONTENT_DATE_TIME
    };

    delay(0);

    addFormSubHeader(F("Zone configuration"));

    {
      html_table(EMPTY_STRING); // Sub-table
      html_table_header(F("Zone #&nbsp;"));
      html_table_header(F("Size"));
      html_table_header(F("Text"), 180);
      html_table_header(F("Content"));
      html_table_header(F("Alignment"));
      html_table_header(F("Animation In/Out")); // 1st and 2nd row title
      html_table_header(F("Speed/Pause"));      // 1st and 2nd row title
      html_table_header(F("Font"));
      html_table_header(F("Layout"));
      html_table_header(F("Special Effects"));
      html_table_header(F("Offset"));
      html_table_header(F("Brightness"));
      html_table_header(F("Repeat (sec)"));
    }

    uint8_t index = 0;
    uint8_t zone  = 0;

    for (auto it = zones.begin(); it != zones.end(); ++it) {
      if (it->zone <= expectedZones) {
        html_TR_TD(); // All columns use max. width available
        addHtml(F("&nbsp;"));
        addHtmlInt(zone + 1);
        html_TD();    // Size
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_SIZE), it->size, 1, P104_MAX_MODULES_PER_ZONE);
        html_TD();    // text
        addTextBox(getPluginCustomArgName(index + P104_OFFSET_TEXT),
                   it->text,
                   P104_MAX_TEXT_LENGTH_PER_ZONE,
                   false,
                   false,
                   EMPTY_STRING,
                   EMPTY_STRING);
        html_TD(); // Content
        addSelector(getPluginCustomArgName(index + P104_OFFSET_CONTENT),
                    P104_CONTENT_count,
                    contentTypes,
                    contentOptions,
                    NULL,
                    it->content,
                    false,
                    true,
                    EMPTY_STRING);
        html_TD(); // Alignment
        addSelector(getPluginCustomArgName(index + P104_OFFSET_ALIGNMENT),
                    3,
                    alignmentTypes,
                    alignmentOptions,
                    NULL,
                    it->alignment,
                    false,
                    true,
                    EMPTY_STRING);
        html_TD(); // Animation In (without None by passing the second element index)
        addSelector(getPluginCustomArgName(index + P104_OFFSET_ANIM_IN),
                    animationCount - 1,
                    &animationTypes[1],
                    &animationOptions[1],
                    NULL,
                    it->animationIn,
                    false,
                    true,
                    F("")
                    # ifdef P104_USE_TOOLTIPS
                    , F("Animation In")
                    # endif // ifdef P104_USE_TOOLTIPS
                    );
        html_TD(); // Speed In
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_SPEED), it->speed, 0, 65535
                      # ifdef P104_USE_TOOLTIPS
                      , EMPTY_STRING // classname
                      , F("Speed")   // title
                      # endif // ifdef P104_USE_TOOLTIPS
                      );

        html_TD(); // Fill columns
        html_TD();
        html_TD();
        html_TD();
        html_TD();
        html_TD();

        // Split here
        html_TR_TD(); // Start new row
        html_TD();    // Start with some blank columns
        html_TD();
        html_TD();
        html_TD();

        html_TD(); // Animation Out
        addSelector(getPluginCustomArgName(index + P104_OFFSET_ANIM_OUT),
                    animationCount,
                    animationTypes,
                    animationOptions,
                    NULL,
                    it->animationOut,
                    false,
                    true,
                    EMPTY_STRING
                    # ifdef P104_USE_TOOLTIPS
                    , F("Animation Out")
                    # endif // ifdef P104_USE_TOOLTIPS
                    );

        html_TD(); // Speed Out
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_PAUSE), it->pause, 0, 65535
                      # ifdef P104_USE_TOOLTIPS
                      , EMPTY_STRING // classname
                      , F("Pause")   // title
                      # endif // ifdef P104_USE_TOOLTIPS
                      );

        html_TD(); // Font
        addSelector(getPluginCustomArgName(index + P104_OFFSET_FONT),
                    fontCount,
                    fontTypes,
                    fontOptions,
                    NULL,
                    it->font,
                    false,
                    true,
                    EMPTY_STRING);

        html_TD(); // Layout
        addSelector(getPluginCustomArgName(index + P104_OFFSET_LAYOUT),
                    layoutCount,
                    layoutTypes,
                    layoutOptions,
                    NULL,
                    it->layout,
                    false,
                    true,
                    EMPTY_STRING);

        html_TD(); // Special effects
        addSelector(getPluginCustomArgName(index + P104_OFFSET_SPEC_EFFECT),
                    specialEffectCount,
                    specialEffectTypes,
                    specialEffectOptions,
                    NULL,
                    it->specialEffect,
                    false,
                    true,
                    EMPTY_STRING);

        html_TD(); // Offset
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_OFFSET), it->offset, 0, 254);
        html_TD(); // Brightness

        if (it->brightness == 0) { it->brightness = 7; }
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_BRIGHTNESS), it->brightness, 0, P104_BRIGHTNESS_MAX);

        html_TD();                                                                                        // Repeat (sec)
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_REPEATDELAY), it->repeatDelay, -1, 86400 // max delay 24 hours
                      # ifdef P104_USE_TOOLTIPS
                      , EMPTY_STRING                                                                      // classname
                      , F("Repeat after this delay (sec), -1 = off")                                      // tooltip
                      # endif // ifdef P104_USE_TOOLTIPS
                      );
        delay(0);
        index += P104_OFFSET_COUNT;
        zone++;
      }
    }
    html_end_table();
  }

  String devicesMsg;

  if (devicesMsg.reserve(80)) {
    devicesMsg  = F("- Maximum nr. of modules possible (Zones * Size + Offset) = 255. Last saved: ");
    devicesMsg += numDevices;
    addFormNote(devicesMsg);
  }
  addFormNote(F("- 'Animation In' or 'Animation Out' and 'Special Effects' marked with <b>*</b> should <b>not</b> be combined in a Zone."));
  # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) && !defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
  addFormNote(F("- 'Layout' 'Double upper' and 'Double lower' are only supported for 'Content' types 'Clock' and 'Date'."));
  # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) && !defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

  return true;
}

/**************************************************************
* webform_save
**************************************************************/
bool P104_data_struct::webform_save(struct EventStruct *event) {
  P104_CONFIG_ZONE_COUNT   = getFormItemInt(F("plugin_104_zone"));
  P104_CONFIG_HARDWARETYPE = getFormItemInt(F("plugin_104_hardware"));

  bitWrite(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_CLEAR_DISABLE, isFormItemChecked(F("plugin_104_cleardisable")));
  bitWrite(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_LOG_ALL_TEXT,  isFormItemChecked(F("plugin_104_logalltext")));

  previousZones = expectedZones;
  expectedZones = P104_CONFIG_ZONE_COUNT;

  saveSettings();                       // Determines numDevices

  P104_CONFIG_TOTAL_UNITS = numDevices; // Store counted number of devices

  return true;
}

#endif // ifdef USES_P104
