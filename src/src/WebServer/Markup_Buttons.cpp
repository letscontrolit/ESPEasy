#include "../WebServer/Markup_Buttons.h"

#include "../WebServer/common.h"
#include "../WebServer/HTML_wrappers.h"

#include "../Static/WebStaticData.h"


void addButton(const __FlashStringHelper * url, const __FlashStringHelper * label) {
  addButton(url, label, EMPTY_STRING);
}

void addButton(const __FlashStringHelper * url, const __FlashStringHelper * label, const __FlashStringHelper * classes, bool enabled)
{
  html_add_button_prefix(classes, enabled);
  addHtml(url);
  addHtml(F("'>"));
  addHtml(label);
  addHtml(F("</a>"));
}

void addButton(const String& url, const String& label)
{
  addButton(url, label, EMPTY_STRING);
}

void addButton(const String& url, const String& label, const String& classes, bool enabled)
{
  html_add_button_prefix(classes, enabled);
  addHtml(url);
  addHtml(F("'>"));
  addHtml(label);
  addHtml(F("</a>"));
}

void addButtonWithSvg(const String& url, const String& label)
{
  addButtonWithSvg(url, label, EMPTY_STRING, false);
}

void addButtonWithSvg(const String& url, const String& label, const String& svgPath, bool needConfirm) {
  addHtml(F("<a "));
  addHtmlAttribute(F("class"), F("button link"));
  addHtmlAttribute(F("href"),  url);
  #ifndef BUILD_MINIMAL_OTA
  bool hasSVG = svgPath.length() > 0;

  if (hasSVG)
  {
    addHtmlAttribute(F("alt"), label);
  }
  #endif // ifndef BUILD_MINIMAL_OTA

  if (needConfirm) {
    addHtmlAttribute(F("onclick"), F("return confirm(\"Are you sure?\")"));
  }
  addHtml('>');

  #ifndef BUILD_MINIMAL_OTA

  if (hasSVG) {
    addHtml(F("<svg width='24' height='24' viewBox='-1 -1 26 26' style='position: relative; top: 5px;'>"));
    addHtml(svgPath);
    addHtml(F("</svg>"));
  } else
  #endif // ifndef BUILD_MINIMAL_OTA
  {
    addHtml(label);
  }
  addHtml(F("</a>"));
}

void addSaveButton(const String& url, const String& label)
{
#ifdef BUILD_MINIMAL_OTA
  addButtonWithSvg(url, label
                   , EMPTY_STRING
                   , false);
#else // ifdef BUILD_MINIMAL_OTA
  addButtonWithSvg(url,
                   label
                   ,
                   F(
                     "<path d='M19 12v7H5v-7H3v7c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2v-7h-2zm-6 .67l2.59-2.58L17 11.5l-5 5-5-5 1.41-1.41L11 12.67V3h2v9.67z'  stroke='white' fill='white' ></path>")
                   ,
                   false);
#endif // ifdef BUILD_MINIMAL_OTA
}

void addDeleteButton(const String& url, const String& label)
{
#ifdef BUILD_MINIMAL_OTA
  addButtonWithSvg(url, label
                   , EMPTY_STRING
                   , true);
#else // ifdef BUILD_MINIMAL_OTA
  addButtonWithSvg(url,
                   label
                   ,
                   F(
                     "<path fill='none' d='M0 0h24v24H0V0z'></path><path d='M6 19c0 1.1.9 2 2 2h8c1.1 0 2-.9 2-2V7H6v12zM8 9h8v10H8V9zm7.5-5l-1-1h-5l-1 1H5v2h14V4h-3.5z' stroke='white' fill='white' ></path>")
                   ,
                   true);
#endif // ifdef BUILD_MINIMAL_OTA
}

void addWideButton(const __FlashStringHelper * url, const __FlashStringHelper * label) {
  html_add_wide_button_prefix(EMPTY_STRING, true);
  addHtml(url);
  addHtml(F("'>"));
  addHtml(label);
  addHtml(F("</a>"));
}

void addWideButton(const String& url, const String& label) {
  addWideButton(url, label, EMPTY_STRING, true);
}

void addWideButton(const String& url, const String& label, const String& classes) {
  addWideButton(url, label, classes, true);
}

void addWideButton(const String& url, const String& label, const String& classes, bool enabled)
{
  html_add_wide_button_prefix(classes, enabled);
  addHtml(url);
  addHtml(F("'>"));
  addHtml(label);
  addHtml(F("</a>"));
}

void addSubmitButton()
{
  addSubmitButton(F("Submit"), F(""));
}

// add submit button with different label and name
void addSubmitButton(const __FlashStringHelper * value, const __FlashStringHelper * name)
{
  addSubmitButton(value, name, F(""));
}

void addSubmitButton(const String& value, const String& name) {
  addSubmitButton(value, name, EMPTY_STRING);
}

void addSubmitButton(const __FlashStringHelper * value, const __FlashStringHelper * name, const __FlashStringHelper * classes)
{
  addSubmitButton(String(value), String(name), String(classes));
}

void addSubmitButton(const String& value, const String& name, const String& classes)
{
  addHtml(F("<input "));
  {
    String fullClasses;
    fullClasses.reserve(12 + classes.length());
    fullClasses = F("button link");

    if (classes.length() > 0) {
      fullClasses += ' ';
      fullClasses += classes;
    }
    addHtmlAttribute(F("class"), fullClasses);
  }
  addHtmlAttribute(F("type"),  F("submit"));
  addHtmlAttribute(F("value"), value);

  if (name.length() > 0) {
    addHtmlAttribute(F("name"), name);
  }
  addHtmlAttribute(F("onclick"), F("toasting()"));
  addHtml(F("/><div id='toastmessage'></div>"));
}

// add copy to clipboard button
void addCopyButton(const String& value, const String& delimiter, const String& name)
{
  TXBuffer.addFlashString((PGM_P)FPSTR(jsClipboardCopyPart1));
  addHtml(value);
  TXBuffer.addFlashString((PGM_P)FPSTR(jsClipboardCopyPart2));
  addHtml(delimiter);
  TXBuffer.addFlashString((PGM_P)FPSTR(jsClipboardCopyPart3));

  // Fix HTML
  addHtml(F("<button "));
  addHtmlAttribute(F("class"),   F("button link"));
  addHtmlAttribute(F("onclick"), F("setClipboard()"));
  addHtml('>');
  addHtml(name);
  addHtml(F(" ("));
  html_copyText_marker();
  addHtml(F(")</button>"));
}
