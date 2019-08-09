
void addButton(const String& url, const String& label) {
  addButton(url, label, "");
}

void addButton(const String& url, const String& label, const String& classes) {
  addButton(url, label, classes, true);
}

void addButton(const String& url, const String& label, const String& classes, bool enabled)
{
  html_add_button_prefix(classes, enabled);
  TXBuffer += url;
  TXBuffer += "'>";
  TXBuffer += label;
  TXBuffer += F("</a>");
}

void addButton(class StreamingBuffer& buffer, const String& url, const String& label)
{
  addButtonWithSvg(buffer, url, label, "", false);
}

void addButtonWithSvg(class StreamingBuffer& buffer, const String& url, const String& label, const String& svgPath, bool needConfirm) {
  bool hasSVG = svgPath.length() > 0;

  buffer += F("<a class='button link' href='");
  buffer += url;

  if (hasSVG) {
    buffer += F("' alt='");
    buffer += label;
  }

  if (needConfirm) {
    buffer += F("' onclick='return confirm(\"Are you sure?\")");
  }
  buffer += F("'>");

  if (hasSVG) {
    buffer += F("<svg width='24' height='24' viewBox='-1 -1 26 26' style='position: relative; top: 5px;'>");
    buffer += svgPath;
    buffer += F("</svg>");
  } else {
    buffer += label;
  }
  buffer += F("</a>");
}

void addSaveButton(const String& url, const String& label)
{
  addSaveButton(TXBuffer, url, label);
}

void addSaveButton(class StreamingBuffer& buffer, const String& url, const String& label)
{
#ifdef BUILD_MINIMAL_OTA
  addButtonWithSvg(buffer, url, label
                   , ""
                   , false);
#else // ifdef BUILD_MINIMAL_OTA
  addButtonWithSvg(buffer,
                   url,
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
  addSaveButton(TXBuffer, url, label);
}

void addDeleteButton(class StreamingBuffer& buffer, const String& url, const String& label)
{
#ifdef BUILD_MINIMAL_OTA
  addButtonWithSvg(buffer, url, label
                   , ""
                   , true);
#else // ifdef BUILD_MINIMAL_OTA
  addButtonWithSvg(buffer,
                   url,
                   label
                   ,
                   F(
                     "<path fill='none' d='M0 0h24v24H0V0z'></path><path d='M6 19c0 1.1.9 2 2 2h8c1.1 0 2-.9 2-2V7H6v12zM8 9h8v10H8V9zm7.5-5l-1-1h-5l-1 1H5v2h14V4h-3.5z' stroke='white' fill='white' ></path>")
                   ,
                   true);
#endif // ifdef BUILD_MINIMAL_OTA
}

void addWideButton(const String& url, const String& label) {
  addWideButton(url, label, "", true);
}

void addWideButton(const String& url, const String& label, const String& classes) {
  addWideButton(url, label, classes, true);
}

void addWideButton(const String& url, const String& label, const String& classes, bool enabled)
{
  html_add_wide_button_prefix(classes, enabled);
  TXBuffer += url;
  TXBuffer += "'>";
  TXBuffer += label;
  TXBuffer += F("</a>");
}

void addSubmitButton()
{
  addSubmitButton(F("Submit"), "");
}

// add submit button with different label and name
void addSubmitButton(const String& value, const String& name) {
  addSubmitButton(value, name, "");
}

void addSubmitButton(const String& value, const String& name, const String& classes)
{
  TXBuffer += F("<input class='button link");

  if (classes.length() > 0) {
    TXBuffer += ' ';
    TXBuffer += classes;
  }
  TXBuffer += F("' type='submit' value='");
  TXBuffer += value;

  if (name.length() > 0) {
    TXBuffer += F("' name='");
    TXBuffer += name;
  }
  TXBuffer += F("'><div id='toastmessage'></div><script type='text/javascript'>toasting();</script>");
}

// add copy to clipboard button
void addCopyButton(const String& value, const String& delimiter, const String& name)
{
  TXBuffer += jsClipboardCopyPart1;
  TXBuffer += value;
  TXBuffer += jsClipboardCopyPart2;
  TXBuffer += delimiter;
  TXBuffer += jsClipboardCopyPart3;

  // Fix HTML
  TXBuffer += F("<button class='button link' onclick='setClipboard()'>");
  TXBuffer += name;
  TXBuffer += " (";
  html_copyText_marker();
  TXBuffer += ')';
  TXBuffer += F("</button>");
}
