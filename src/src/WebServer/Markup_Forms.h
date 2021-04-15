#ifndef WEBSERVER_WEBSERVER_MARKUP_FORMS_H
#define WEBSERVER_WEBSERVER_MARKUP_FORMS_H

#include "../WebServer/common.h"

#include "../Globals/Plugins.h"


// ********************************************************************************
// Add a separator as row start
// ********************************************************************************
void addFormSeparator(int clspan);

// ********************************************************************************
// Add a note as row start
// ********************************************************************************
void addFormNote(const String& text, const String& id = "");

// ********************************************************************************
// Create Forms
// ********************************************************************************


// ********************************************************************************
// Add a checkbox Form
// ********************************************************************************

void addFormCheckBox_disabled(const String& label, const String& id, boolean checked);
void addFormCheckBox(const String& label, const String& id, boolean checked, bool disabled = false);
void addFormCheckBox(LabelType::Enum label, boolean checked, bool disabled = false);
void addFormCheckBox_disabled(LabelType::Enum label, boolean checked);

// ********************************************************************************
// Add a Numeric Box form
// ********************************************************************************
void addFormNumericBox(LabelType::Enum label, int value, int min = INT_MIN, int max = INT_MAX);
void addFormNumericBox(const String& label, const String& id, int value, int min = INT_MIN, int max = INT_MAX);

void addFormFloatNumberBox(LabelType::Enum label, float value, float min, float max, byte nrDecimals = 6, float stepsize = 0.0f);
void addFormFloatNumberBox(const String& label, const String& id, float value, float min, float max, byte nrDecimals = 6, float stepsize = 0.0f);

// ********************************************************************************
// Add a task selector form
// ********************************************************************************
void addTaskSelectBox(const String& label, const String& id, taskIndex_t choice);

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
// Add a Password Box form
// ********************************************************************************

void addFormPasswordBox(const String& label, const String& id, const String& password, int maxlength);

bool getFormPassword(const String& id, String& password);

// ********************************************************************************
// Add a IP Box form
// ********************************************************************************

void addFormIPBox(const String& label, const String& id, const byte ip[4]);

// ********************************************************************************
// Add a IP Access Control select dropdown list
// ********************************************************************************
void addFormIPaccessControlSelect(const String& label, const String& id, int choice);

// ********************************************************************************
// Add a selector form
// ********************************************************************************

void addFormPinSelect(const String& label, const String& id, int choice);

void addFormPinSelectI2C(const String& label, const String& id, int choice);

void addFormSelectorI2C(const String& id, int addressCount, const int addresses[], int selectedIndex);

void addFormSelector(const String& label, const String& id, int optionCount, const String options[], const int indices[], int selectedIndex);

void addFormSelector(const String& label,
                     const String& id,
                     int           optionCount,
                     const String  options[],
                     const int     indices[],
                     int           selectedIndex,
                     bool          reloadonchange);

void addFormSelector(const String& label,
                     const String& id,
                     int           optionCount,
                     const String  options[],
                     const int     indices[],
                     const String  attr[],
                     int           selectedIndex,
                     boolean       reloadonchange);

void addFormSelector_script(const String& label,
                            const String& id,
                            int           optionCount,
                            const String  options[],
                            const int     indices[],
                            const String  attr[],
                            int           selectedIndex,
                            const String& onChangeCall);

// ********************************************************************************
// Add a GPIO pin select dropdown list
// ********************************************************************************
void addFormPinStateSelect(int gpio, int choice);

// ********************************************************************************
// Retrieve return values from form/checkbox.
// ********************************************************************************


int getFormItemInt(const String& key, int defaultValue);

bool getCheckWebserverArg_int(const String& key, int& value);

bool update_whenset_FormItemInt(const String& key, int& value);

bool update_whenset_FormItemInt(const String& key, byte& value);

// Note: Checkbox values will not appear in POST Form data if unchecked.
// So if webserver does not have an argument for a checkbox form, it means it should be considered unchecked.
bool isFormItemChecked(const String& id);

int getFormItemInt(const String& id);

float getFormItemFloat(const String& id);

bool isFormItem(const String& id);

void copyFormPassword(const String& id, char *pPassword, int maxlength);



#endif