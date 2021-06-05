#ifndef WEBSERVER_WEBSERVER_MARKUP_H
#define WEBSERVER_WEBSERVER_MARKUP_H

#include "../WebServer/common.h"
#include "../Globals/Plugins.h"
#include "../Helpers/StringGenerator_GPIO.h"

// ********************************************************************************
// Add Selector
// ********************************************************************************
void addSelector(const String& id,
                 int           optionCount,
                 const __FlashStringHelper * options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex);

void addSelector(const String& id,
                 int           optionCount,
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex);

void addSelector(const String& id,
                 int           optionCount,
                 const __FlashStringHelper * options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex,
                 boolean       reloadonchange,
                 bool          enabled);

void addSelector(const String& id,
                 int           optionCount,
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex,
                 boolean       reloadonchange,
                 bool          enabled);


void addSelector(const String& id,
                 int           optionCount,
                 const __FlashStringHelper * options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex,
                 boolean       reloadonchange,
                 bool          enabled,
                 const String& classname);

void addSelector(const String& id,
                 int           optionCount,
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex,
                 boolean       reloadonchange,
                 bool          enabled,
                 const String& classname);

void addSelector_options(int optionCount, const __FlashStringHelper * options[], const int indices[], const String attr[], int selectedIndex);
void addSelector_options(int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex);

void addSelector_Head(const String& id);

void addSelector_Head_reloadOnChange(const String& id);

void addSelector_Head_reloadOnChange(const String& id, const String& classname, bool disabled);

void addSelector_Head_reloadOnChange(const String& id, const String& classname, const String& onChangeCall, bool disabled);

void do_addSelector_Head(const String& id, const String& classname, const String& onChangeCall, const bool& disabled);

void addPinSelector_Item(PinSelectPurpose purpose, const String& gpio_label, int gpio, boolean selected, boolean disabled = false, const String& attr = EMPTY_STRING);

void addSelector_Item(const __FlashStringHelper * option, int index, boolean selected, boolean disabled = false, const String& attr = EMPTY_STRING);
void addSelector_Item(const String& option, int index, boolean selected, boolean disabled = false, const String& attr = EMPTY_STRING);

void addSelector_Foot();

void addUnit(const __FlashStringHelper * unit);
void addUnit(const String& unit);
void addUnit(char unit);

void addRowLabel_tr_id(const __FlashStringHelper * label, const __FlashStringHelper * id);
void addRowLabel_tr_id(const __FlashStringHelper * label, const String& id);
void addRowLabel_tr_id(const String& label, const String& id);

void addRowLabel(const __FlashStringHelper * label);
void addRowLabel(const String& label, const String& id = "");

// Add a row label and mark it with copy markers to copy it to clipboard.
void addRowLabel_copy(const __FlashStringHelper * label);
void addRowLabel_copy(const String& label);

void addRowLabel(LabelType::Enum label);

void addRowLabelValue(LabelType::Enum label);

void addRowLabelValue_copy(LabelType::Enum label);

// ********************************************************************************
// Add a header
// ********************************************************************************
void addTableSeparator(const __FlashStringHelper *label, int colspan, int h_size);
void addTableSeparator(const __FlashStringHelper *label, int colspan, int h_size, const __FlashStringHelper * helpButton);
void addTableSeparator(const String& label, int colspan, int h_size, const String& helpButton = "");

void addFormHeader(const __FlashStringHelper * header);
void addFormHeader(const String& header, const String& helpButton = "");
void addFormHeader(const String& header, const String& helpButton, const String& rtdHelpButton);

// ********************************************************************************
// Add a sub header
// ********************************************************************************
void addFormSubHeader(const __FlashStringHelper *  header);
void addFormSubHeader(const String& header);

// ********************************************************************************
// Add a checkbox
// ********************************************************************************
void addCheckBox(const __FlashStringHelper * id, boolean checked, bool disabled = false);
void addCheckBox(const String& id, boolean checked, bool disabled = false);

// ********************************************************************************
// Add a numeric box
// ********************************************************************************
void addNumericBox(const __FlashStringHelper * id, int value, int min, int max);
void addNumericBox(const String& id, int value, int min, int max);

void addFloatNumberBox(const String& id, float value, float min, float max, byte nrDecimals = 6, float stepsize = 0.0f);

// ********************************************************************************
// Add Textbox
// ********************************************************************************
void addTextBox(const String& id, const String&  value, int maxlength, bool readonly = false, bool required = false, const String& pattern = "");
void addTextBox(const String& id, const String&  value, int maxlength, bool readonly, bool required, const String& pattern, const String& classname);

// ********************************************************************************
// Add Textarea
// ********************************************************************************
void addTextArea(const String& id, const String& value, int maxlength, int rows, int columns, bool readonly, bool required);

// ********************************************************************************
// Add Help Buttons
// ********************************************************************************

// adds a Help Button with points to the the given Wiki Subpage
// If url starts with "RTD", it will be considered as a Read-the-docs link
void addHelpButton(const __FlashStringHelper * url);
void addHelpButton(const String& url);

void addRTDHelpButton(const String& url);

void addHelpButton(const String& url, bool isRTD);

void addRTDPluginButton(pluginID_t taskDeviceNumber);

String makeDocLink(const String& url, bool isRTD);


void addPinSelect(PinSelectPurpose purpose, const __FlashStringHelper * id,  int choice);
void addPinSelect(PinSelectPurpose purpose, const String& id,  int choice);


#ifdef ESP32
enum class AdcPinSelectPurpose {
    TouchOnly,
    ADC_Touch,
    ADC_Touch_HallEffect,
    ADC_Touch_Optional
};
void addADC_PinSelect(AdcPinSelectPurpose purpose, const String& id,  int choice);
#endif


#endif