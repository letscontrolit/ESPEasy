
// ********************************************************************************
// Add a separator as row start
// ********************************************************************************
void addFormSeparator(int clspan)
{
  TXBuffer += F("<TR><TD colspan='");
  TXBuffer += clspan;
  TXBuffer += F("'><hr>");
}

// ********************************************************************************
// Add a note as row start
// ********************************************************************************
void addFormNote(const String& text)
{
  html_TR_TD();
  html_TD();
  TXBuffer += F("<div class='note'>Note: ");
  TXBuffer += text;
  TXBuffer += F("</div>");
}

// ********************************************************************************
// Create Forms
// ********************************************************************************


// ********************************************************************************
// Add a checkbox Form
// ********************************************************************************

void addFormCheckBox(const String& label, const String& id, boolean checked) {
  addFormCheckBox(label, id, checked, false);
}

void addFormCheckBox_disabled(const String& label, const String& id, boolean checked) {
  addFormCheckBox(label, id, checked, true);
}

void addFormCheckBox(const String& label, const String& id, boolean checked, bool disabled)
{
  addRowLabel(label);
  addCheckBox(id, checked, disabled);
}

void addFormCheckBox(LabelType::Enum label, boolean checked, bool disabled) {
  addFormCheckBox(getLabel(label), getInternalLabel(label), checked, disabled);
}

void addFormCheckBox_disabled(LabelType::Enum label, boolean checked) {
  addFormCheckBox(label, checked, true);
}

// ********************************************************************************
// Add a Numeric Box form
// ********************************************************************************

void addFormNumericBox(const String& label, const String& id, int value, int min, int max)
{
  addRowLabel(label);
  addNumericBox(id, value, min, max);
}

void addFormNumericBox(const String& label, const String& id, int value)
{
  addFormNumericBox(label, id, value, INT_MIN, INT_MAX);
}

void addFormFloatNumberBox(const String& label, const String& id, float value, float min, float max)
{
  addRowLabel(label);
  addFloatNumberBox(id, value, min, max);
}

// ********************************************************************************
// Add a Text Box form
// ********************************************************************************

void addFormTextBox(const String& label, const String& id, const String&  value, int maxlength)
{
  addRowLabel(label);
  addTextBox(id, value, maxlength);
}

void addFormTextBox(const String& label, const String& id, const String&  value, int maxlength, bool readonly)
{
  addRowLabel(label);
  addTextBox(id, value, maxlength, readonly);
}

void addFormTextBox(const String& label, const String& id, const String&  value, int maxlength, bool readonly, bool required)
{
  addRowLabel(label);
  addTextBox(id, value, maxlength, readonly, required);
}

void addFormTextBox(const String& label,
                    const String& id,
                    const String& value,
                    int           maxlength,
                    bool          readonly,
                    bool          required,
                    const String& pattern)
{
  addRowLabel(label);
  addTextBox(id, value, maxlength, readonly, required, pattern);
}

// ********************************************************************************
// Add a Password Box form
// ********************************************************************************

void addFormPasswordBox(const String& label, const String& id, const String& password, int maxlength)
{
  addRowLabel(label);
  TXBuffer += F("<input class='wide' type='password' name='");
  TXBuffer += id;
  TXBuffer += F("' maxlength=");
  TXBuffer += maxlength;
  TXBuffer += F(" value='");

  if (password != "") { // no password?
    TXBuffer += F("*****");
  }

  // TXBuffer += password;   //password will not published over HTTP
  TXBuffer += "'>";
}

// ********************************************************************************
// Add a IP Box form
// ********************************************************************************

void addFormIPBox(const String& label, const String& id, const byte ip[4])
{
  bool empty_IP = (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0);

  addRowLabel(label);
  TXBuffer += F("<input class='wide' type='text' name='");
  TXBuffer += id;
  TXBuffer += F("' value='");

  if (!empty_IP) {
    TXBuffer += formatIP(ip);
  }
  TXBuffer += "'>";
}

// ********************************************************************************
// Add a IP Access Control select dropdown list
// ********************************************************************************
void addFormIPaccessControlSelect(const String& label, const String& id, int choice)
{
  addRowLabel(label);
  addIPaccessControlSelect(id, choice);
}

// ********************************************************************************
// Add a selector form
// ********************************************************************************

void addFormPinSelect(const String& label, const String& id, int choice)
{
  addRowLabel(label, String("tr_") + id);
  addPinSelect(false, id, choice);
}

void addFormPinSelectI2C(const String& label, const String& id, int choice)
{
  addRowLabel(label, String("tr_") + id);
  addPinSelect(true, id, choice);
}

void addFormSelectorI2C(const String& id, int addressCount, const int addresses[], int selectedIndex)
{
  String options[addressCount];

  for (byte x = 0; x < addressCount; x++)
  {
    options[x] = formatToHex_decimal(addresses[x]);

    if (x == 0) {
      options[x] += F(" - (default)");
    }
  }
  addFormSelector(F("I2C Address"), id, addressCount, options, addresses, NULL, selectedIndex, false);
}

void addFormSelector(const String& label, const String& id, int optionCount, const String options[], const int indices[], int selectedIndex)
{
  addFormSelector(label, id, optionCount, options, indices, NULL, selectedIndex, false);
}

void addFormSelector(const String& label,
                     const String& id,
                     int           optionCount,
                     const String  options[],
                     const int     indices[],
                     int           selectedIndex,
                     bool          reloadonchange)
{
  addFormSelector(label, id, optionCount, options, indices, NULL, selectedIndex, reloadonchange);
}

void addFormSelector(const String& label,
                     const String& id,
                     int           optionCount,
                     const String  options[],
                     const int     indices[],
                     const String  attr[],
                     int           selectedIndex,
                     boolean       reloadonchange)
{
  addRowLabel(label);
  addSelector(id, optionCount, options, indices, attr, selectedIndex, reloadonchange);
}

void addFormSelector_script(const String& label,
                            const String& id,
                            int           optionCount,
                            const String  options[],
                            const int     indices[],
                            const String  attr[],
                            int           selectedIndex,
                            const String& onChangeCall)
{
  addRowLabel(label);
  addSelector_Head(id, onChangeCall, false);
  addSelector_options(optionCount, options, indices, attr, selectedIndex);
  addSelector_Foot();
}

// ********************************************************************************
// Add a GPIO pin select dropdown list
// ********************************************************************************
void addFormPinStateSelect(const String& label, const String& id, int choice, bool enabled)
{
  addRowLabel(label);
  String options[4] = { F("Default"), F("Output Low"), F("Output High"), F("Input") };
  addSelector(id, 4, options, NULL, NULL, choice, false, enabled);
}

// ********************************************************************************
// Retrieve return values from form/checkbox.
// ********************************************************************************


int getFormItemInt(const String& key, int defaultValue) {
  int value = defaultValue;

  getCheckWebserverArg_int(key, value);
  return value;
}

bool getCheckWebserverArg_int(const String& key, int& value) {
  String valueStr = WebServer.arg(key);

  if (!isInt(valueStr)) { return false; }
  value = valueStr.toInt();
  return true;
}

// Note: Checkbox values will not appear in POST Form data if unchecked.
// So if webserver does not have an argument for a checkbox form, it means it should be considered unchecked.
bool isFormItemChecked(const String& id)
{
  return WebServer.arg(id) == F("on");
}

int getFormItemInt(const String& id)
{
  return getFormItemInt(id, 0);
}

float getFormItemFloat(const String& id)
{
  String val = WebServer.arg(id);

  if (!isFloat(val)) { return 0.0; }
  return val.toFloat();
}

bool isFormItem(const String& id)
{
  return WebServer.arg(id).length() != 0;
}

void copyFormPassword(const String& id, char *pPassword, int maxlength)
{
  String password = WebServer.arg(id);

  if (password == F("*****")) { // no change?
    return;
  }
  safe_strncpy(pPassword, password.c_str(), maxlength);
}
