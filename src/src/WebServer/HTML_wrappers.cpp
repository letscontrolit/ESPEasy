#include "../WebServer/HTML_wrappers.h"

#include "../Static/WebStaticData.h"

#include "../WebServer/Markup.h"

#include "../Helpers/StringConverter.h"


// ********************************************************************************
// HTML string re-use to keep the executable smaller
// Flash strings are not checked for duplication.
// ********************************************************************************
void wrap_html_tag(const String& tag, const String& text) {
  String html;

  html.reserve(6 + (2 * tag.length()) + text.length());

  html += '<';
  html += tag;
  html += '>';
  html += text;
  html += "</";
  html += tag;
  html += '>';
  addHtml(html);
}

void html_B(const String& text) {
  wrap_html_tag("b", text);
}

void html_I(const String& text) {
  wrap_html_tag("i", text);
}

void html_U(const String& text) {
  wrap_html_tag("u", text);
}

void html_TR_TD_highlight() {
  addHtml(F("<TR class=\"highlight\">"));
  html_TD();
}

void html_TR_TD() {
  html_TR();
  html_TD();
}

void html_BR() {
  addHtml(F("<BR>"));
}

void html_TR() {
  addHtml(F("<TR>"));
}

void html_TR_TD_height(int height) {
  html_TR();

  String html;

  html.reserve(20);

  html += F("<TD HEIGHT=\"");
  html += height;
  html += "\">";
  addHtml(html);
}

void html_TD() {
  html_TD(1);
}

void html_TD(int td_cnt) {
  for (int i = 0; i < td_cnt; ++i) {
    addHtml(F("<TD>"));
  }
}

int copyTextCounter = 0;

void html_reset_copyTextCounter() {
  copyTextCounter = 0;
}

void html_copyText_TD() {
  ++copyTextCounter;

  String html;

  html.reserve(24);

  html += F("<TD id='copyText_");
  html += copyTextCounter;
  html += "'>";
  addHtml(html);
}

// Add some recognizable token to show which parts will be copied.
void html_copyText_marker() {
  addHtml(F("&#x022C4;")); //   &diam; &diamond; &Diamond; &#x022C4; &#8900;
}

void html_add_estimate_symbol() {
  addHtml(F(" &#8793; ")); //   &#8793;  &#x2259;  &wedgeq;
}

void html_table_class_normal() {
  html_table(F("normal"));
}

void html_table_class_multirow() {
  html_table(F("multirow"), true);
}

void html_table_class_multirow_noborder() {
  html_table(F("multirow"), false);
}

void html_table(const String& tableclass) {
  html_table(tableclass, false);
}

void html_table(const String& tableclass, bool boxed) {
  addHtml(F("<table "));
  addHtmlAttribute(F("class"), tableclass);

  if (boxed) {
    addHtmlAttribute(F("border"), F("1px"));
    addHtmlAttribute(F("frame"),  F("box"));
    addHtmlAttribute(F("rules"),  F("all"));
  }
  addHtml('>');
}

void html_table_header(const String& label) {
  html_table_header(label, 0);
}

void html_table_header(const String& label, int width) {
  html_table_header(label, F(""), F(""), width);
}

void html_table_header(const String& label, const String& helpButton, int width) {
  html_table_header(label, helpButton, F(""), width);
}

void html_table_header(const String& label, const String& helpButton, const String& rtdHelpButton, int width) {
  String html;

  html.reserve(25 + label.length());

  html += F("<TH");

  if (width > 0) {
    html += F(" style='width:");
    html += String(width);
    html += F("px;'");
  }
  html += '>';
  html += label;
  addHtml(html);

  if (helpButton.length() > 0) {
    addHelpButton(helpButton);
  }

  if (rtdHelpButton.length() > 0) {
    addRTDHelpButton(rtdHelpButton);
  }
  addHtml(F("</TH>"));
}

void html_end_table() {
  addHtml(F("</table>"));
}

void html_end_form() {
  addHtml(F("</form>"));
}

void html_add_button_prefix() {
  html_add_button_prefix("", true);
}

void html_add_button_prefix(const String& classes, bool enabled) {
  addHtml(F(" <a class='button link"));

  if (classes.length() > 0) {
    String html;
    html.reserve(classes.length() + 1);
    html += ' ';
    html += classes;
    addHtml(html);
  }

  if (!enabled) {
    addDisabled();
  }
  addHtml("'");

  if (!enabled) {
    addDisabled();
  }
  addHtml(F(" href='"));
}

void html_add_wide_button_prefix() {
  html_add_wide_button_prefix("", true);
}

void html_add_wide_button_prefix(const String& classes, bool enabled) {
  String wide_classes;

  wide_classes.reserve(classes.length() + 5);
  wide_classes  = F("wide ");
  wide_classes += classes;
  html_add_button_prefix(wide_classes, enabled);
}

void html_add_form() {
  addHtml(F("<form name='frmselect' method='post'>"));
}

void html_add_autosubmit_form() {
  addHtml(F("<script><!--\n"
            "function dept_onchange(frmselect) {frmselect.submit();}"
            "\n//--></script>"));
}

void html_add_script(const String& script, bool defer) {
  html_add_script(defer);
  addHtml(script);
  html_add_script_end();
}

void html_add_script(bool defer) {
  addHtml(F("<script"));

  if (defer) {
    addHtml(F(" defer"));
  }
  addHtml(F(" type='text/JavaScript'>"));
}

void html_add_script_end() {
  addHtml(F("</script>"));
}

// if there is an error-string, add it to the html code with correct formatting
void addHtmlError(const String& error) {
  if (error.length() > 0)
  {
    String html;
    html.reserve(20);
    html += F("<div class=\"");

    if (error.startsWith(F("Warn"))) {
      html += F("warning");
    } else {
      html += F("alert");
    }
    addHtml(html);
    addHtml(F("\"><span class=\"closebtn\" onclick=\"this.parentElement.style.display='none';\">&times;</span>"));
    addHtml(error);
    addHtml(F("</div>"));
  }
  else
  {}
}

void addHtml(const char& html) {
  TXBuffer += html;
}

void addHtml(const String& html) {
  TXBuffer += html;
}

void addHtmlInt(int int_val) {
  addHtml(String(int_val));
}

void addEncodedHtml(const String& html) {
  // FIXME TD-er: What about the function htmlStrongEscape ??
  String copy(html);

  htmlEscape(copy);
  addHtml(copy);
}

void addHtmlAttribute(const String& label, int value) {
  addHtml(' ');
  addHtml(label);
  addHtml('=');
  addHtmlInt(value);
  addHtml(' ');
}

void addHtmlAttribute(const String& label, const String& value) {
  addHtml(' ');
  addHtml(label);
  addHtml(F("='"));
  addEncodedHtml(value);
  addHtml(F("' "));
}

void addDisabled() {
  addHtml(F(" disabled"));
}

void addHtmlLink(const String& htmlclass, const String& url, const String& label) {
  addHtml(F(" <a "));
  addHtmlAttribute(F("class"),  htmlclass);
  addHtmlAttribute(F("href"),   url);
  addHtmlAttribute(F("target"), F("_blank"));
  addHtml('>');
  addHtml(label);
  addHtml(F("</a>"));
}

void addHtmlDiv(const String& htmlclass) {
  addHtmlDiv(htmlclass, F(""));
}

void addHtmlDiv(const String& htmlclass, const String& content) {
  addHtmlDiv(htmlclass, content, F(""));
}

void addHtmlDiv(const String& htmlclass, const String& content, const String& id) {
  addHtml(F(" <div "));
  addHtmlAttribute(F("class"), htmlclass);
  if (id.length() > 0) {
    addHtmlAttribute(F("id"), id);
  }
  addHtml('>');
  addHtml(content);
  addHtml(F("</div>"));
}

void addEnabled(boolean enabled)
{
  String html;

  html.reserve(40);
  html += F("<span class='enabled ");

  if (enabled) {
    html += F("on'>&#10004;");
  }
  else {
    html += F("off'>&#10060;");
  }
  html += F("</span>");
  addHtml(html);
}
