
#include "../Helpers/StringGenerator_Web.h"
#include "../WebServer/HTML_wrappers.h"

/**
 * start a datalist definition
 * see: datalistAddValue, datalistFinish
 */
void datalistStart(const __FlashStringHelper *id) {
  datalistStart(String(id));
}

void datalistStart(const String& id) {
  addHtml(F("<datalist id=\""));
  addHtml(id);
  addHtml(F("\">"));
}

/**
 * add a value to a datalist
 * see: datalistStart, datalistFinish
 */
void datalistAddValue(const String& value) {
  addHtml(F("<option value=\""));
  addHtml(value);
  addHtml(F("\">"));
  addHtml(value);
  addHtml(F("</option>"));
}

/**
 * finish the datalist definition
 * see: datalistStart, datalistAddValue
 */
void datalistFinish() {
  addHtml(F("</datalist>"));
}
