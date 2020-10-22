#ifndef WEBSERVER_WEBSERVER_MARKUP_H
#define WEBSERVER_WEBSERVER_MARKUP_H

#include "../WebServer/common.h"
#include "../Globals/Plugins.h"

// ********************************************************************************
// Add Selector
// ********************************************************************************
void addSelector(const String& id,
                 int           optionCount,
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex);

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
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex,
                 boolean       reloadonchange,
                 bool          enabled,
                 const String& classname);

void addSelector_options(int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex);

void addSelector_option(const int index, const String& option, const String& attr, bool isSelected);

void addSelector_Head(const String& id);

void addSelector_Head_reloadOnChange(const String& id);

void addSelector_Head_reloadOnChange(const String& id, const String& classname, bool disabled);

void addSelector_Head_reloadOnChange(const String& id, const String& classname, const String& onChangeCall, bool disabled);

void do_addSelector_Head(const String& id, const String& classname, const String& onChangeCall, const bool& disabled);

void addSelector_Item(const String& option, int index, boolean selected, boolean disabled, const String& attr);

void addSelector_Foot();

void addUnit(const String& unit);

void addRowLabel_tr_id(const String& label, const String& id);

void addRowLabel(const String& label, const String& id = "");

// Add a row label and mark it with copy markers to copy it to clipboard.
void addRowLabel_copy(const String& label);

void addRowLabelValue(LabelType::Enum label);

void addRowLabelValue_copy(LabelType::Enum label);

// ********************************************************************************
// Add a header
// ********************************************************************************
void addTableSeparator(const String& label, int colspan, int h_size, const String& helpButton = "");

void addFormHeader(const String& header, const String& helpButton = "");
void addFormHeader(const String& header, const String& helpButton, const String& rtdHelpButton);

// ********************************************************************************
// Add a sub header
// ********************************************************************************
void addFormSubHeader(const String& header);

// ********************************************************************************
// Add a checkbox
// ********************************************************************************
void addCheckBox(const String& id, boolean checked, bool disabled = false);

// ********************************************************************************
// Add a numeric box
// ********************************************************************************
void addNumericBox(const String& id, int value, int min, int max);

void addFloatNumberBox(const String& id, float value, float min, float max);

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
void addHelpButton(const String& url);

void addRTDHelpButton(const String& url);

void addHelpButton(const String& url, bool isRTD);

void addRTDPluginButton(pluginID_t taskDeviceNumber);

String makeDocLink(const String& url, bool isRTD);

void addPinSelect(boolean forI2C, const String& id,  int choice);


#ifdef ESP32
void addADC_PinSelect(bool touchOnly, const String& id,  int choice);
#endif


// ********************************************************************************
// Helper function actually rendering dropdown list for addPinSelect()
// ********************************************************************************
void renderHTMLForPinSelect(String options[], int optionValues[], boolean forI2C, const String& id,  int choice, int count);

#endif