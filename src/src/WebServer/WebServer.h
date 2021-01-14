#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H

#include "../WebServer/common.h"


#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataTypes/SettingsType.h"
#include "../Globals/Plugins.h"
#include "../Helpers/StringConverter.h"

#define _HEAD false
#define _TAIL true

#define TASKS_PER_PAGE TASKS_MAX


#define MENU_INDEX_MAIN          0
#define MENU_INDEX_CONFIG        1
#define MENU_INDEX_CONTROLLERS   2
#define MENU_INDEX_HARDWARE      3
#define MENU_INDEX_DEVICES       4
#define MENU_INDEX_RULES         5
#define MENU_INDEX_NOTIFICATIONS 6
#define MENU_INDEX_TOOLS         7
extern byte navMenuIndex;

// Uncrustify must not be used on macros, so turn it off.
// *INDENT-OFF*
#define strncpy_webserver_arg(D, N) safe_strncpy_webserver_arg(D, N, sizeof(D));
// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*

void safe_strncpy_webserver_arg(char *dest, const String& arg, size_t max_size);

void sendHeadandTail(const String& tmplName,
                     boolean       Tail      = false,
                     boolean       rebooting = false);

void   sendHeadandTail_stdtemplate(boolean Tail      = false,
                                   boolean rebooting = false);

size_t streamFile_htmlEscape(const String& fileName);

void   WebServerInit();

void   setWebserverRunning(bool state);

void   getWebPageTemplateDefault(const String& tmplName,
                                 String      & tmpl);

void   getWebPageTemplateDefaultHead(String& tmpl,
                                     bool    addMeta,
                                     bool    addJS);

void getWebPageTemplateDefaultHeader(String      & tmpl,
                                     const String& title,
                                     bool          addMenu);

void   getWebPageTemplateDefaultContentSection(String& tmpl);

void   getWebPageTemplateDefaultFooter(String& tmpl);

void   getErrorNotifications();

String getGpMenuIcon(byte index);

String getGpMenuLabel(byte index);

String getGpMenuURL(byte index);

void   getWebPageTemplateVar(const String& varName);

void   writeDefaultCSS(void);


// ********************************************************************************
// Functions to stream JSON directly to TXBuffer
// FIXME TD-er: replace stream_xxx_json_object* into this code.
// N.B. handling of numerical values differs (string vs. no string)
// ********************************************************************************

extern int8_t level;
extern int8_t lastLevel;

void json_quote_name(const String& val);

void json_quote_val(const String& val);

void json_open();

void json_open(bool arr);

void json_open(bool          arr,
               const String& name);

void json_init();

void json_close();

void json_close(bool arr);

void json_number(const String& name,
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
boolean isLoggedIn();

String  getControllerSymbol(byte index);

/*
   String getValueSymbol(byte index);
 */
void    addSVG_param(const String& key,
                     float         value);

void    addSVG_param(const String& key,
                     const String& value);

void    createSvgRect_noStroke(unsigned int fillColor,
                               float        xoffset,
                               float        yoffset,
                               float        width,
                               float        height,
                               float        rx,
                               float        ry);

void createSvgRect(unsigned int fillColor,
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

void write_SVG_image_header(int width,
                            int height);

void write_SVG_image_header(int  width,
                            int  height,
                            bool useViewbox);

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

int  getPartionCount(byte pType);

void getPartitionTableSVG(byte         pType,
                          unsigned int partitionColor);

#endif // ifdef ESP32

bool webArg2ip(const String& arg,
               byte         *IP);

#endif // ifndef WEBSERVER_WEBSERVER_H
