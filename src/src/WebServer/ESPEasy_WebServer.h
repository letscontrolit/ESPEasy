#ifndef WEBSERVER_ESPEASY_WEBSERVER_H
#define WEBSERVER_ESPEASY_WEBSERVER_H

#include "../WebServer/common.h"


#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataTypes/SettingsType.h"
#include "../Globals/Plugins.h"
#include "../Helpers/StringConverter.h"

#include "../WebServer/WebTemplateParser.h"


// Uncrustify must not be used on macros, so turn it off.
// *INDENT-OFF*
#define strncpy_webserver_arg(D, N) safe_strncpy_webserver_arg(D, N, sizeof(D));
// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*

void safe_strncpy_webserver_arg(char *dest, const String& arg, size_t max_size);

void safe_strncpy_webserver_arg(char *dest, const __FlashStringHelper * arg, size_t max_size);

void sendHeadandTail(const __FlashStringHelper * tmplName,
                     bool       Tail      = false,
                     bool       rebooting = false);

void   sendHeadandTail_stdtemplate(bool Tail,
                                   bool rebooting = false);


void   WebServerInit();

// ********************************************************************************
// Redirect to captive portal if we got a request for another domain. 
// Return true in that case so the page handler does not try to handle the request again.
// ********************************************************************************
bool   captivePortal();

void   setWebserverRunning(bool state);

void   getWebPageTemplateDefault(const String& tmplName,
                                 WebTemplateParser& parser);

void   getWebPageTemplateDefaultHead(WebTemplateParser& parser,
                                     bool    addMeta,
                                     bool    addJS);

void getWebPageTemplateDefaultHeader(WebTemplateParser& parser,
                                     const __FlashStringHelper * title,
                                     bool          addMenu);

void   getWebPageTemplateDefaultContentSection(WebTemplateParser& parser);

void   getWebPageTemplateDefaultFooter(WebTemplateParser& parser);


void   writeDefaultCSS(void);


// ********************************************************************************
// Functions to stream JSON directly to TXBuffer
// FIXME TD-er: replace stream_xxx_json_object* into this code.
// N.B. handling of numerical values differs (string vs. no string)
// ********************************************************************************

extern int8_t level;
extern int8_t lastLevel;

void json_quote_name(const __FlashStringHelper * val);
void json_quote_name(const String& val);

void json_quote_val(const String& val);

void json_open(bool arr = false);

void json_open(bool          arr,
               const __FlashStringHelper * name);

void json_open(bool          arr,
               const String& name);

void json_init();

void json_close();

void json_close(bool arr);

void json_number(const __FlashStringHelper * name,
                 const String& value);

void json_number(const String& name,
                 const String& value);

void json_prop(const __FlashStringHelper * name,
               const String& value);

void json_prop(const String& name,
               const String& value);

void json_prop(LabelType::Enum label);

// ********************************************************************************
// Add a task select dropdown list
// This allows to select a task index based on the existing tasks.
// ********************************************************************************
void addTaskSelect(const String& name,
                   taskIndex_t   choice);

// ********************************************************************************
// Add a Value select dropdown list, based on TaskIndex
// This allows to select a task value, based on the existing tasks.
// ********************************************************************************
void addTaskValueSelect(const String& name,
                        int           choice,
                        taskIndex_t   TaskIndex);

// ********************************************************************************
// Login state check
// ********************************************************************************
bool isLoggedIn(bool mustProvideLogin = true);

String  getControllerSymbol(uint8_t index);

/*
   String getValueSymbol(uint8_t index);
 */
void    addSVG_param(const __FlashStringHelper * key,
                     int         value);

void    addSVG_param(const __FlashStringHelper * key,
                     float         value);

void    addSVG_param(const __FlashStringHelper * key,
                     const String& value);

void    createSvgRect_noStroke(const __FlashStringHelper * classname,
                               unsigned int fillColor,
                               float        xoffset,
                               float        yoffset,
                               float        width,
                               float        height,
                               float        rx,
                               float        ry);

void createSvgRect(const String& classname,
                   unsigned int fillColor,
                   unsigned int strokeColor,
                   float        xoffset,
                   float        yoffset,
                   float        width,
                   float        height,
                   float        strokeWidth,
                   float        rx,
                   float        ry);

void createSvgHorRectPath(unsigned int color,
                          int          xoffset,
                          int          yoffset,
                          int          size,
                          int          height,
                          int          range,
                          float        SVG_BAR_WIDTH);

void createSvgTextElement(const String& text,
                          float         textXoffset,
                          float         textYoffset);

void write_SVG_image_header(int  width,
                            int  height,
                            bool useViewbox = false);

/*
   void getESPeasyLogo(int width_pixels);
 */
void getWiFi_RSSI_icon(int rssi,
                       int width_pixels);

#ifndef BUILD_MINIMAL_OTA
void getConfig_dat_file_layout();

void getStorageTableSVG(SettingsType::Enum settingsType);

#endif // ifndef BUILD_MINIMAL_OTA

#ifdef ESP32

void getPartitionTableSVG(uint8_t         pType,
                          unsigned int partitionColor);

#endif // ifdef ESP32

bool webArg2ip(const __FlashStringHelper * arg,
               uint8_t         *IP);


#endif // ifndef WEBSERVER_ESPEASY_WEBSERVER_H