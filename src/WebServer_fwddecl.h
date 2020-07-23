#ifndef WEBSERVER_FWD_DECL_H
#define WEBSERVER_FWD_DECL_H

#include "limits.h"
#include "StringProviderTypes.h"
#include "src/Helpers/SystemVariables.h"


// ********************************************************************************
// Add a note as row start
// ********************************************************************************
void addFormNote(const String& text, const String& id = "");

// ********************************************************************************
// Add a Numeric Box form
// ********************************************************************************

void addFormNumericBox(const String& label, const String& id, int value, int min = INT_MIN, int max = INT_MAX);

// ********************************************************************************
// Add a Text Box form
// ********************************************************************************
void addFormTextBox(const String& label,
                    const String& id,
                    const String& value,
                    int           maxlength,
                    bool          readonly = false,
                    bool          required = false,
                    const String& pattern = "");


void addFormTextArea(const String& label,
                    const String& id,
                    const String& value,
                    int           maxlength,
                    int           rows, 
                    int           columns,
                    bool          readonly = false,
                    bool          required = false);


// ********************************************************************************
// Add a checkbox Form
// ********************************************************************************
void addFormCheckBox(const String& label, const String& id, boolean checked, bool disabled = false);
void addFormCheckBox(LabelType::Enum label, boolean checked, bool disabled = false);
void addFormCheckBox_disabled(LabelType::Enum label, boolean checked);



// FIXME TD-er: replace stream_xxx_json_object* into this code.
// N.B. handling of numerical values differs (string vs. no string)
void stream_next_json_object_value(LabelType::Enum label);
void stream_last_json_object_value(LabelType::Enum label);
void addRowLabelValue(LabelType::Enum label);
void addRowLabelValue_copy(LabelType::Enum label);



// ********************************************************************************
// ********************************************************************************
//                             WebServer Markup
// ********************************************************************************
// ********************************************************************************



// ********************************************************************************
// Add a Row label
// ********************************************************************************
void addRowLabel(const String& label, const String& id = "");

// ********************************************************************************
// Add a header
// ********************************************************************************
void addTableSeparator(const String& label, int colspan, int h_size, const String& helpButton = "");

// ********************************************************************************
// Add a form header
// ********************************************************************************
void addFormHeader(const String& header, const String& helpButton = "");

// ********************************************************************************
// Add a checkbox
// ********************************************************************************
void addCheckBox(const String& id, boolean checked, bool disabled = false);

// ********************************************************************************
// Add a numeric box
// ********************************************************************************
void addNumericBox(const String& id, int value, int min = INT_MIN, int max = INT_MAX);

// ********************************************************************************
// Add Textbox
// ********************************************************************************
void addTextBox(const String& id, const String&  value, int maxlength, bool readonly = false, bool required = false, const String& pattern = "");

// ********************************************************************************
// Add Textarea
// ********************************************************************************
void addTextArea(const String& id, const String& value, int maxlength, int rows, int columns, bool readonly=false, bool required = false);


// ********************************************************************************
// Add Selector
// ********************************************************************************
void addSelector(const String& id,
                 int           optionCount,
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex,
                 boolean       reloadonchange,
                 bool          enabled);

#endif // WEBSERVER_FWD_DECL_H