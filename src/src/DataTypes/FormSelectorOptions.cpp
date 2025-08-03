#include "../DataTypes/FormSelectorOptions.h"

#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/HTML_wrappers.h"

FormSelectorOptions::FormSelectorOptions()
  : classname(F("wide")), _onlySelectorHead(true)
{}


FormSelectorOptions::FormSelectorOptions(int optionCount)
  :   classname(F("wide")), _optionCount(optionCount)
{}

FormSelectorOptions::FormSelectorOptions(
  int          optionCount,
  const int    indices[],
  const String attr[]) :
  classname(F("wide")),
  _optionCount(optionCount),
  _indices(indices),
  _attr_str(attr)
{}

FormSelectorOptions::FormSelectorOptions(
  int          optionCount,
  const String options[],
  const int    indices[],
  const String attr[]) :
  classname(F("wide")),
  _optionCount(optionCount),
  _names_str(options),
  _indices(indices),
  _attr_str(attr)
{}

FormSelectorOptions::FormSelectorOptions(
  int                        optionCount,
  const __FlashStringHelper *options[],
  const int                  indices[],
  const String               attr[]) :
  classname(F("wide")),
  _optionCount(optionCount),
  _names_f(options),
  _indices(indices),
  _attr_str(attr)
{}

FormSelectorOptions::~FormSelectorOptions() {}

String FormSelectorOptions::getOptionString(int index) const
{
  if (index >= _optionCount) {
    return EMPTY_STRING;
  }

  if (_names_f != nullptr) {
    return String(_names_f[index]);
  }

  if (_names_str != nullptr) {
    return String(_names_str[index]);
  }

  if (_indices != nullptr) {
    return String(_indices[index]);
  }
  return String(index);
}

int FormSelectorOptions::getIndexValue(int index) const
{
  if (index >= _optionCount) {
    return -1;
  }

  if (_indices != nullptr) {
    return _indices[index];
  }
  return index;
}

bool FormSelectorOptions::isDone(int index) const
{
  return index >= _optionCount;
}

void FormSelectorOptions::clearClassName()
{
  classname = F("");
}

void FormSelectorOptions::addFormSelector(
  const __FlashStringHelper *label,
  const __FlashStringHelper *id,
  int                        selectedIndex) const
{
  addFormSelector(String(label), String(id), selectedIndex);
}

void FormSelectorOptions::addFormSelector(
  const String             & label,
  const __FlashStringHelper *id,
  int                        selectedIndex) const
{
  addFormSelector(label, String(id), selectedIndex);
}

void FormSelectorOptions::addFormSelector(
  const __FlashStringHelper *label,
  const String             & id,
  int                        selectedIndex) const
{
  addFormSelector(String(label), id, selectedIndex);
}

void FormSelectorOptions::addFormSelector(
  const String& label,
  const String& id,
  int           selectedIndex) const
{
  addRowLabel_tr_id(label, id);
  addSelector(id, selectedIndex);
}

void FormSelectorOptions::addSelector(const __FlashStringHelper *id,
                                      int                        selectedIndex) const
{
  addSelector(String(id), selectedIndex);
}

void FormSelectorOptions::addSelector(const String& id,
                                      int           selectedIndex) const
{
  // FIXME TD-er Change bool 'enabled' to disabled
  if (reloadonchange)
  {
    addSelector_Head_reloadOnChange(id, classname, !enabled
                                    #if FEATURE_TOOLTIPS
                                    , tooltip
                                    #endif // if FEATURE_TOOLTIPS
                                    );
  } else {
    do_addSelector_Head(id, classname, onChangeCall, !enabled
                        #if FEATURE_TOOLTIPS
                        , tooltip
                        #endif // if FEATURE_TOOLTIPS
                        );
  }

  if (_onlySelectorHead) { return; }

  for (int i = 0; !isDone(i); ++i)
  {
    const int index = getIndexValue(i);
    String optionString = getOptionString(i);
    if (index == default_index) {
      optionString += F(" (default)");
    }
    addSelector_Item(
      optionString,
      index,
      selectedIndex == index,
      false,
      _attr_str ? _attr_str[i] : EMPTY_STRING);

    if ((i & 0x07) == 0) { delay(0); }
  }

  addSelectorFoot();
}

void FormSelectorOptions::addSelectorFoot() const
{
  addSelector_Foot(reloadonchange);
}
