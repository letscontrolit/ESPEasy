#include "src/Static/WebStaticData.h"


// ********************************************************************************
// HTML string re-use to keep the executable smaller
// Flash strings are not checked for duplication.
// ********************************************************************************
void wrap_html_tag(const String& tag, const String& text) {
  TXBuffer += '<';
  TXBuffer += tag;
  TXBuffer += '>';
  TXBuffer += text;
  TXBuffer += "</";
  TXBuffer += tag;
  TXBuffer += '>';
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

void html_TR_TD_highlight(void) {
  TXBuffer += F("<TR class=\"highlight\">");
  html_TD(void);
}

void html_TR_TD(void) {
  html_TR(void);
  html_TD(void);
}

void html_BR(void) {
  TXBuffer += F("<BR>");
}

void html_TR(void) {
  TXBuffer += F("<TR>");
}

void html_TR_TD_height(int height) {
  html_TR(void);
  TXBuffer += F("<TD HEIGHT=\"");
  TXBuffer += height;
  TXBuffer += "\">";
}

void html_TD(void) {
  html_TD(1);
}

void html_TD(int td_cnt) {
  for (int i = 0; i < td_cnt; ++i) {
    TXBuffer += F("<TD>");
  }
}

static int copyTextCounter = 0;

void html_reset_copyTextCounter(void) {
  copyTextCounter = 0;
}

void html_copyText_TD(void) {
  ++copyTextCounter;
  TXBuffer += F("<TD id='copyText_");
  TXBuffer += copyTextCounter;
  TXBuffer += "'>";
}

// Add some recognizable token to show which parts will be copied.
void html_copyText_marker(void) {
  TXBuffer += F("&#x022C4;"); //   &diam; &diamond; &Diamond; &#x022C4; &#8900;
}

void html_add_estimate_symbol(void) {
  TXBuffer += F(" &#8793; "); //   &#8793;  &#x2259;  &wedgeq;
}

void html_table_class_normal(void) {
  html_table(F("normal"));
}

void html_table_class_multirow(void) {
  html_table(F("multirow"), true);
}

void html_table_class_multirow_noborder(void) {
  html_table(F("multirow"), false);
}

void html_table(const String& tableclass) {
  html_table(tableclass, false);
}

void html_table(const String& tableclass, bool boxed) {
  TXBuffer += F("<table class='");
  TXBuffer += tableclass;
  TXBuffer += '\'';

  if (boxed) {
    TXBuffer += F("' border=1px frame='box' rules='all'");
  }
  TXBuffer += '>';
}

void html_table_header(const String& label) {
  html_table_header(label, 0);
}

void html_table_header(const String& label, int width) {
  html_table_header(label, "", width);
}

void html_table_header(const String& label, const String& helpButton, int width) {
  TXBuffer += F("<TH");

  if (width > 0) {
    TXBuffer += F(" style='width:");
    TXBuffer += String(width);
    TXBuffer += F("px;'");
  }
  TXBuffer += '>';
  TXBuffer += label;

  if (helpButton.length(void) > 0) {
    addHelpButton(helpButton);
  }
  TXBuffer += F("</TH>");
}

void html_end_table(void) {
  TXBuffer += F("</table>");
}

void html_end_form(void) {
  TXBuffer += F("</form>");
}

void html_add_button_prefix(void) {
  html_add_button_prefix("", true);
}

void html_add_button_prefix(const String& classes, bool enabled) {
  TXBuffer += F(" <a class='button link");

  if (classes.length(void) > 0) {
    TXBuffer += ' ';
    TXBuffer += classes;
  }

  if (!enabled) {
    addDisabled(void);
  }
  TXBuffer += '\'';

  if (!enabled) {
    addDisabled(void);
  }
  TXBuffer += F(" href='");
}

void html_add_wide_button_prefix(void) {
  html_add_wide_button_prefix("", true);
}

void html_add_wide_button_prefix(const String& classes, bool enabled) {
  String wide_classes;

  wide_classes.reserve(classes.length(void) + 5);
  wide_classes  = F("wide ");
  wide_classes += classes;
  html_add_button_prefix(wide_classes, enabled);
}

void html_add_form(void) {
  TXBuffer += F("<form name='frmselect' method='post'>");
}

void html_add_autosubmit_form(void) {
  TXBuffer += F("<script><!--\n"
                "function dept_onchange(frmselect) {frmselect.submit(void);}"
                "\n//--></script>");
}

void html_add_script(const String& script, bool defer) {
  html_add_script(defer);
  addHtml(script);
  html_add_script_end(void);
}

void html_add_script(bool defer) {
  TXBuffer += F("<script");

  if (defer) {
    TXBuffer += F(" defer");
  }
  TXBuffer += F(" type='text/JavaScript'>");
}

void html_add_script_end(void) {
  TXBuffer += F("</script>");
}

// if there is an error-string, add it to the html code with correct formatting
void addHtmlError(const String& error) {
  if (error.length(void) > 0)
  {
    TXBuffer += F("<div class=\"");

    if (error.startsWith(F("Warn"))) {
      TXBuffer += F("warning");
    } else {
      TXBuffer += F("alert");
    }
    TXBuffer += F("\"><span class=\"closebtn\" onclick=\"this.parentElement.style.display='none';\">&times;</span>");
    TXBuffer += error;
    TXBuffer += F("</div>");
  }
  else
  {
    TXBuffer += jsToastMessageBegin;

    // we can push custom messages here in future releases...
    TXBuffer += F("Submitted");
    TXBuffer += jsToastMessageEnd;
  }
}

void addHtml(const String& html) {
  TXBuffer += html;
}

void addDisabled(void) {
  TXBuffer += F(" disabled");
}

void addHtmlLink(const String& htmlclass, const String& url, const String& label) {
  TXBuffer += F(" <a class='");
  TXBuffer += htmlclass;
  TXBuffer += F("' href='");
  TXBuffer += url;
  TXBuffer += F("' target='_blank'>");
  TXBuffer += label;
  TXBuffer += F("</a>");
}

void addEnabled(boolean enabled)
{
  TXBuffer += F("<span class='enabled ");

  if (enabled) {
    TXBuffer += F("on'>&#10004;");
  }
  else {
    TXBuffer += F("off'>&#10060;");
  }
  TXBuffer += F("</span>");
}
