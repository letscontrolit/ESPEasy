#ifndef WEBSERVER_WEBSERVER_HTML_WRAPPERS_H
#define WEBSERVER_WEBSERVER_HTML_WRAPPERS_H

#include "../WebServer/common.h"


// ********************************************************************************
// HTML string re-use to keep the executable smaller
// Flash strings are not checked for duplication.
// ********************************************************************************
void wrap_html_tag(const __FlashStringHelper * tag, const String& text);
void wrap_html_tag(const String& tag, const String& text);
void wrap_html_tag(char tag, const String& text);

void html_B(const String& text);

void html_I(const String& text);

void html_U(const String& text);

void html_TR_TD_highlight();

void html_TR_TD();

void html_BR();

void html_TR();

void html_TR_TD_height(int height);

void html_TD();

void html_TD(int td_cnt);

extern int copyTextCounter;

void html_reset_copyTextCounter();

void html_copyText_TD();

// Add some recognizable token to show which parts will be copied.
void html_copyText_marker();

void html_add_estimate_symbol();

void html_table_class_normal();

void html_table_class_multirow();

void html_table_class_multirow_noborder();

void html_table(const __FlashStringHelper * tableclass, bool boxed = false);

void html_table(const String& tableclass, bool boxed = false);

void html_table_header(const __FlashStringHelper * label);
void html_table_header(const String& label);

void html_table_header(const __FlashStringHelper * label, int width);
void html_table_header(const String& label, int width);

void html_table_header(const __FlashStringHelper * label, const __FlashStringHelper * helpButton, int width);
void html_table_header(const String& label, const __FlashStringHelper * helpButton, int width);
void html_table_header(const __FlashStringHelper * label, const String& helpButton, int width);
void html_table_header(const String& label, const String& helpButton, int width);


void html_table_header(const __FlashStringHelper * label, const __FlashStringHelper * helpButton, const String& rtdHelpButton, int width);
void html_table_header(const String& label, const __FlashStringHelper * helpButton, const String& rtdHelpButton, int width);
void html_table_header(const __FlashStringHelper * label, const String& helpButton, const String& rtdHelpButton, int width);

void html_table_header(const __FlashStringHelper * label, const __FlashStringHelper * helpButton, const __FlashStringHelper * rtdHelpButton, int width);
void html_table_header(const String& label, const __FlashStringHelper * helpButton, const __FlashStringHelper * rtdHelpButton, int width);
void html_table_header(const __FlashStringHelper * label, const String& helpButton, const __FlashStringHelper * rtdHelpButton, int width);

void html_table_header(const String& label, const String& helpButton, const String& rtdHelpButton, int width);

void html_end_table();

void html_end_form();

void html_add_button_prefix();
void html_add_button_prefix(const __FlashStringHelper * classes, bool enabled);
void html_add_button_prefix(const String& classes, bool enabled);

void html_add_wide_button_prefix();

void html_add_wide_button_prefix(const String& classes, bool enabled);

void html_add_form();

void html_add_autosubmit_form();

void html_add_script(const __FlashStringHelper * script, bool defer);
void html_add_script(const String& script, bool defer);

void html_add_script(bool defer);

void html_add_script_end();

// if there is an error-string, add it to the html code with correct formatting
void addHtmlError(const __FlashStringHelper * error);
void addHtmlError(const String& error);

void addHtml(const char& html);
void addHtml(const __FlashStringHelper * html);
void addHtml(const String& html);
void addHtmlInt(int int_val);

void addEncodedHtml(const __FlashStringHelper * html);
void addEncodedHtml(const String& html);

void addHtmlAttribute(const __FlashStringHelper * label, int value);
void addHtmlAttribute(const String& label, int value);
void addHtmlAttribute(const __FlashStringHelper * label, const __FlashStringHelper * value);
void addHtmlAttribute(const __FlashStringHelper * label, const String& value);
void addHtmlAttribute(const String& label, const String& value);

void addDisabled();

void addHtmlLink(const String& htmlclass, const String& url, const String& label);

void addHtmlDiv(const __FlashStringHelper * htmlclass, const String& content = EMPTY_STRING, const String& id = EMPTY_STRING);

void addHtmlDiv(const String& htmlclass);
void addHtmlDiv(const String& htmlclass, const String& content);
void addHtmlDiv(const String& htmlclass, const String& content, const String& id);

void addEnabled(boolean enabled);

#endif