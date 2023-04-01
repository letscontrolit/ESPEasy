#include "../Helpers/RulesMatcher.h"

#include "../Helpers/ESPEasy_math.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"


bool ruleMatch(String event, String rule) {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ruleMatch"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  if (rule.equals("*")) {
    // wildcard, always process
    return true;
  }

  rule.trim();
  parseStandardConversions(rule, false);
  rule = parseTemplate(rule);

  event.trim();

  if (event.equalsIgnoreCase(rule)) {
    return true;
  }

  // clock events need different handling...
  if (event.substring(0, 10).equalsIgnoreCase(F("Clock#Time")))
  {
    int pos1 = event.indexOf('=');
    int pos2 = rule.indexOf('=');

    if ((pos1 > 0) && (pos2 > 0)) {
      if (event.substring(0, pos1).equalsIgnoreCase(rule.substring(0, pos2))) // if this is a clock rule
      {
//        addLog(LOG_LEVEL_INFO, concat(F("Clock#Time="), rule.substring(pos2 + 1)));
        unsigned long clockEvent = string2TimeLong(event.substring(pos1 + 1));
        unsigned long clockSet   = string2TimeLong(rule.substring(pos2 + 1));

        return matchClockEvent(clockEvent, clockSet);
      }
    } else {
      // Not supported yet, see: https://github.com/letscontrolit/ESPEasy/issues/2640
      return false;
    }
  }

  // Handling wildcard in event
  {
    const int asterisk_pos = rule.indexOf('*');

    if (asterisk_pos != -1) // a * sign in rule, so use a 'wildcard' match on message
    {
      return event.substring(0, asterisk_pos).equalsIgnoreCase(rule.substring(0, asterisk_pos));
    }
  }

  // Special handling of literal string events, they should start with '!'
  if (event.charAt(0) == '!') {
    const bool pound_char_found = rule.indexOf('#') != -1;

    if (!pound_char_found)
    {
      // no # sign in rule, use 'wildcard' match on event 'source'
      return event.substring(0, rule.length()).equalsIgnoreCase(rule);
    }
    return event.equalsIgnoreCase(rule);
  }


  // parse event into verb and value
  double value = 0;
  int    equal_pos   = event.indexOf('=');

  if (equal_pos >= 0) {
    if (!validDoubleFromString(event.substring(equal_pos + 1), value)) {
      return false;

      // FIXME TD-er: What to do when trying to match NaN values?
    }
    event = event.substring(0, equal_pos);
  }

  // parse rule
  int  posStart, posEnd;
  char compare;

  if (!findCompareCondition(rule, compare, posStart, posEnd)) {
    // No compare condition found, so just check if the event- and rule string match.
    return event.equalsIgnoreCase(rule);
  }

  const bool stringMatch = event.equalsIgnoreCase(rule.substring(0, posStart));
  double     ruleValue   = 0;

  if (!validDoubleFromString(rule.substring(posEnd), ruleValue)) {
    return false;

    // FIXME TD-er: What to do when trying to match NaN values?
  }

  bool match = false;

  if (stringMatch) {
    match = compareDoubleValues(compare, value, ruleValue);
  }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ruleMatch2"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  return match;
}

bool compareIntValues(char compare, const int& Value1, const int& Value2)
{
  switch (compare) {
    case '>' + '=': return Value1 >= Value2;
    case '<' + '=': return Value1 <= Value2;
    case '<' + '>': return Value1 != Value2;
    case '>':       return Value1 > Value2;
    case '<':       return Value1 < Value2;
    case '=':       return Value1 == Value2;
  }
  return false;
}

bool compareDoubleValues(char compare, const double& Value1, const double& Value2)
{
  switch (compare) {
    case '>' + '=': return !definitelyLessThan(Value1, Value2);
    case '<' + '=': return !definitelyGreaterThan(Value1, Value2);
    case '<' + '>': return !essentiallyEqual(Value1, Value2);
    case '>':       return definitelyGreaterThan(Value1, Value2);
    case '<':       return definitelyLessThan(Value1, Value2);
    case '=':       return essentiallyEqual(Value1, Value2);
  }
  return false;
}

// Find the compare condition.
// @param posStart = first position of the compare condition in the string
// @param posEnd   = first position rest of the string, right after the compare condition.
bool findCompareCondition(const String& check, char& compare, int& posStart, int& posEnd)
{
  posStart = check.length();
  posEnd   = posStart;
  int  comparePos = 0;
  bool found      = false;

  if (((comparePos = check.indexOf(F("!="))) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 2;
    compare  = '<' + '>';
    found    = true;
  }

  if (((comparePos = check.indexOf(F("<>"))) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 2;
    compare  = '<' + '>';
    found    = true;
  }

  if (((comparePos = check.indexOf(F(">="))) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 2;
    compare  = '>' + '=';
    found    = true;
  }

  if (((comparePos = check.indexOf(F("<="))) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 2;
    compare  = '<' + '=';
    found    = true;
  }

  if (((comparePos = check.indexOf(F("=="))) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 2;
    compare  = '=';
    found    = true;
  }

  if (((comparePos = check.indexOf('<')) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 1;
    compare  = '<';
    found    = true;
  }

  if (((comparePos = check.indexOf('>')) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 1;
    compare  = '>';
    found    = true;
  }

  if (((comparePos = check.indexOf('=')) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 1;
    compare  = '=';
    found    = true;
  }
  return found;
}

bool getEventFromRulesLine(const String& line, String& event, String& action)
{
  if (line.length() == 0) {
    return false;
  }

  if (!line.substring(0, 3).equalsIgnoreCase(F("on "))) {
    return false;
  }

  // Make sure to skip any number of spaces between 'on' and the event.
  int startpos_event = 2;
  while (static_cast<int>(line.length()) > startpos_event && line[startpos_event] == ' ') {
    ++startpos_event;
  }

  // Need to look for the " do" part starting after "on " (3 chars)
  // and the event needs to be at least 1 char.
  String line_lc = line;

  line_lc.toLowerCase();
  const int pos_do = line_lc.indexOf(F(" do"), startpos_event + 1);

  if (pos_do == -1) {
    return false;
  }

  // event: The part between on ... do
  event = line.substring(startpos_event, pos_do);
  event.trim();

  // Ignore escape char
//  removeChar(event, '[');
//  removeChar(event, ']');

  // action: The optional part after the " do"
  action = line.substring(pos_do + 3);
  action.trim();
  // Remove optional "endon"
  if (action.endsWith(F("endon"))) {
    action = action.substring(0, action.lastIndexOf(F("endon")));
  }
  return true;
}
