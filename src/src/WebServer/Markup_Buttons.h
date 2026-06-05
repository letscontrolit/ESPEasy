#ifndef WEBSERVER_WEBSERVER_MARKUP_BUTTONS_H
#define WEBSERVER_WEBSERVER_MARKUP_BUTTONS_H


#include "../WebServer/common.h"

void addButton(const __FlashStringHelper * url, const __FlashStringHelper * label);

void addButton(const __FlashStringHelper * url, const __FlashStringHelper * label, const __FlashStringHelper * classes, bool enabled = true);

void addButton(const String& url, const String& label);

void addButton(const String& url, const String& label, const String& classes, bool enabled = true);

# ifdef WEBSERVER_NEW_RULES
void addButtonWithSvg(const String& url, const String& label);

void addButtonWithSvg(const String& url, const String& label, const String& svgPath, bool needConfirm);

void addSaveButton(const String& url, const String& label);

void addDeleteButton(const String& url, const String& label);
#endif
void addWideButton(const __FlashStringHelper * url, const __FlashStringHelper * label);
void addWideButton(const String& url, const String& label);

void addWideButton(const String& url, const String& label, const String& classes);

void addWideButton(const String& url, const String& label, const String& classes, bool enabled);

void addSubmitButton();

// add submit button with different label and name
void addSubmitButton(const __FlashStringHelper * value, const __FlashStringHelper * name);
void addSubmitButton(const String& value, const String& name);

void addSubmitButton(const __FlashStringHelper * value, const __FlashStringHelper * name, const __FlashStringHelper * classes);
void addSubmitButton(const String& value, const String& name, const String& classes);

#ifdef WEBSERVER_GITHUB_COPY
// add copy to clipboard button
void addCopyButton(const String& value, const String& delimiter, const String& name);
#endif

void addPlugin_Add_Edit_Button(const __FlashStringHelper * urlPrefix, size_t index, bool plugin_set, bool plugin_supported, const String& symbol = EMPTY_STRING);


#endif