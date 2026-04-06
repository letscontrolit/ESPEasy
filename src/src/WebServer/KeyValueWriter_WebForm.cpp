#include "../WebServer/KeyValueWriter_WebForm.h"

#include "../Helpers/StringConverter.h"

#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"


KeyValueWriter_WebForm::KeyValueWriter_WebForm(bool emptyHeader, PrintToString *toStr)
  : KeyValueWriter(emptyHeader, toStr)
{}

KeyValueWriter_WebForm::KeyValueWriter_WebForm(KeyValueWriter_WebForm*parent, PrintToString *toStr)
  : KeyValueWriter(parent, toStr)
{}

KeyValueWriter_WebForm::KeyValueWriter_WebForm(bool emptyHeader, KeyValueWriter_WebForm*parent, PrintToString *toStr)
  : KeyValueWriter(emptyHeader, parent, toStr)
{}

KeyValueWriter_WebForm::KeyValueWriter_WebForm(const String& header, PrintToString *toStr)
  : KeyValueWriter(header, nullptr, toStr)
{}

KeyValueWriter_WebForm::KeyValueWriter_WebForm(const __FlashStringHelper *header, PrintToString *toStr)
  : KeyValueWriter(String(header), nullptr, toStr)
{}


KeyValueWriter_WebForm::KeyValueWriter_WebForm(const String& header, KeyValueWriter_WebForm*parent, PrintToString *toStr)
  : KeyValueWriter(header, parent, toStr)
{}

KeyValueWriter_WebForm::KeyValueWriter_WebForm(const __FlashStringHelper *header, KeyValueWriter_WebForm*parent, PrintToString *toStr)
  : KeyValueWriter(String(header), parent, toStr)
{}

KeyValueWriter_WebForm::~KeyValueWriter_WebForm()
{
  if (!summaryValueOnly() && !_isEmpty && _hasHeader && _header.isEmpty()) {
    // TODO TD-er: Should we add a separator line here?
  }

}

void KeyValueWriter_WebForm::write()
{
  if (_isEmpty) {
    if (_parent != nullptr) { _parent->write(); }

    if (!summaryValueOnly() && _hasHeader && !_header.isEmpty()) {
      if (plainText()) {
        getPrint().print(concat(_header, F(": ")));
      } else {
        addFormSubHeader(_header);
      }
    }
    _isEmpty = false;
  }
}

void KeyValueWriter_WebForm::write(const KeyValueStruct& kv)
{
  write();
  const bool plain_text         = plainText();
  const bool summary_value_only = summaryValueOnly();

  const size_t nrValues   = kv._values.size();
  const bool   format_pre =
    !plain_text &&
    !summary_value_only &&
    (nrValues > 1 || kv._format == KeyValueStruct::Format::PreFormatted);

  const bool format_note =
    !plain_text &&
    !summary_value_only &&
    (nrValues == 1 && kv._format == KeyValueStruct::Format::Note);

  if (!summary_value_only) {
    if (format_note) { addRowLabel_tr_id(EMPTY_STRING, EMPTY_STRING); }
    else {
      // Use the __id value, not the getID function, so we only output the set value, not a generated value based on the key.
      addRowLabel(kv._key.toString(), kv.__id.toString());
    }
  }

  if (format_note) { getPrint().print(F(" <div class='note'>Note: ")); }

  if (format_pre) { getPrint().print(F("<pre>")); }

  for (size_t i = 0; i < nrValues; ++i) {
    if (i != 0) {
      if (plain_text) {
        getPrint().write('\n');
      }
      else {
        getPrint().print(F("<br>"));
      }
    }

    if (kv._values[i].isSet()) {

      String str(kv._values[i].toString());

      if (!plain_text) {

        const auto vtype = kv._values[i].getValueType();

        if (vtype == ValueStruct::ValueType::Bool) {
          str = concat(F("<span class='enabled "), str.equals("0")
          ? F("off'>&#10060;")
          : F("on'>&#10004;"));
          str += F("</span>");
        } else {
          if (str.indexOf('\n') != -1)
          {
            str.replace(F("\n"), F("<br>"));
          }
        }
      }
      getPrint().print(str);
    }
  }

#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
  // May need to include the unit before ending </pre>
  // or else it will be shown on the next line
  if (!plain_text && (nrValues == 1) && !summary_value_only) {
    addUnit(kv.getUnit());
  }
#endif

  if (format_pre) { getPrint().print(F("</pre>")); }

  if (format_note) { getPrint().print(F("</div>")); }

  if (summary_value_only) {
    // Must add a newline
    if (plain_text) {
      getPrint().write('\n');
    }
    else {
      getPrint().print(F("<br>"));
    }
  }
}

UP_KeyValueWriter KeyValueWriter_WebForm::createChild()
{
  UP_KeyValueWriter_WebForm  child(new (std::nothrow) KeyValueWriter_WebForm(this));

  return std::move(child);

  // return std::make_unique<KeyValueWriter_WebForm>(this);
}

UP_KeyValueWriter KeyValueWriter_WebForm::createChild(const String& header)
{
  UP_KeyValueWriter_WebForm  child(new (std::nothrow) KeyValueWriter_WebForm(header, this));

  return std::move(child);

  // return std::make_unique<KeyValueWriter_WebForm>(header, this);
}

UP_KeyValueWriter KeyValueWriter_WebForm::createChildArray(const String& header)
{
  auto child = createChild(header);

  if (child) {
    child->setIsArray();
  }

  // return std::move(child);
  return child;
}

UP_KeyValueWriter KeyValueWriter_WebForm::createNew()
{
  UP_KeyValueWriter_WebForm  child(new (std::nothrow) KeyValueWriter_WebForm());

  return std::move(child);

  // return std::make_unique<KeyValueWriter_WebForm>();
}

UP_KeyValueWriter KeyValueWriter_WebForm::createNew(const String& header)
{
  UP_KeyValueWriter_WebForm  child(new (std::nothrow) KeyValueWriter_WebForm(header));

  return std::move(child);

  // return std::make_unique<KeyValueWriter_WebForm>(header);
}

bool KeyValueWriter_WebForm::allowHTML() const {
  return true;
}