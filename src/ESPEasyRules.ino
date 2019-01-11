#define RULE_FILE_SEPARAROR '/'
#define RULE_MAX_FILENAME_LENGTH 24

String EventToFileName(String &eventName) {
  int size = eventName.length();
  int index = eventName.indexOf('=');
  if (index > -1) {
    size = index;
  }
#if defined(ESP8266)
  String fileName = F("rules/");
#endif
#if defined(ESP32)
  String fileName = F("/rules/");
#endif
  fileName += eventName.substring(0, size);
  fileName.replace('#', RULE_FILE_SEPARAROR);
  fileName.toLowerCase();
  return fileName;
}

String FileNameToEvent(String &fileName) {
#if defined(ESP8266)
  String eventName = fileName.substring(6);
#endif
#if defined(ESP32)
  String eventName = fileName.substring(7);
#endif
  eventName.replace(RULE_FILE_SEPARAROR, '#');
  return eventName;
}

void checkRuleSets() {
  for (byte x = 0; x < RULESETS_MAX; x++) {
#if defined(ESP8266)
    String fileName = F("rules");
#endif
#if defined(ESP32)
    String fileName = F("/rules");
#endif
    fileName += x + 1;
    fileName += F(".txt");
    if (SPIFFS.exists(fileName))
      activeRuleSets[x] = true;
    else
      activeRuleSets[x] = false;

    if (Settings.SerialLogLevel == LOG_LEVEL_DEBUG_DEV) {
      serialPrint(fileName);
      serialPrint(" ");
      serialPrintln(String(activeRuleSets[x]));
    }
  }
}

/********************************************************************************************\
  Rules processing
  \*********************************************************************************************/
void rulesProcessing(String &event) {
  if (!Settings.UseRules)
    return;
  START_TIMER
  checkRAM(F("rulesProcessing"));
  unsigned long timer = millis();
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("EVENT: ");
    log += event;
    addLog(LOG_LEVEL_INFO, log);
  }

  if (Settings.OldRulesEngine()) {
    for (byte x = 0; x < RULESETS_MAX; x++) {
#if defined(ESP8266)
      String fileName = F("rules");
#endif
#if defined(ESP32)
      String fileName = F("/rules");
#endif
      fileName += x + 1;
      fileName += F(".txt");
      if (activeRuleSets[x])
        rulesProcessingFile(fileName, event);
    }
  } else {
    String fileName = EventToFileName(event);
    // if exists processed the rule file
    if (SPIFFS.exists(fileName))
      rulesProcessingFile(fileName, event);
    else
      addLog(LOG_LEVEL_DEBUG, String(F("EVENT: ")) + event +
                                  String(F(" is ingnored. File ")) + fileName +
                                  String(F(" not found.")));
  }

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("EVENT: ");
    log += event;
    log += F(" Processing time:");
    log += timePassedSince(timer);
    log += F(" milliSeconds");
    addLog(LOG_LEVEL_DEBUG, log);
  }
  STOP_TIMER(RULES_PROCESSING);
  backgroundtasks();
}

/********************************************************************************************\
  Rules processing
  \*********************************************************************************************/
String rulesProcessingFile(const String &fileName, String &event) {
  if (!Settings.UseRules)
    return "";
  checkRAM(F("rulesProcessingFile"));
  if (Settings.SerialLogLevel == LOG_LEVEL_DEBUG_DEV) {
    serialPrint(F("RuleDebug Processing:"));
    serialPrintln(fileName);
    serialPrintln(F("     flags CMI  parse output:"));
  }

  static byte nestingLevel = 0;
  int data = 0;
  String log = "";

  nestingLevel++;
  if (nestingLevel > RULES_MAX_NESTING_LEVEL) {
    addLog(LOG_LEVEL_ERROR, F("EVENT: Error: Nesting level exceeded!"));
    nestingLevel--;
    return (log);
  }

  fs::File f = SPIFFS.open(fileName, "r+");
  SPIFFS_CHECK(f, fileName.c_str());

  String line = "";
  bool match = false;
  bool codeBlock = false;
  bool isCommand = false;
  bool condition[RULES_IF_MAX_NESTING_LEVEL];
  bool ifBranche[RULES_IF_MAX_NESTING_LEVEL];
  byte ifBlock = 0;
  byte fakeIfBlock = 0;

  byte *buf = new byte[RULES_BUFFER_SIZE]();
  int len = 0;
  while (f.available()) {
    len = f.read((byte *)buf, RULES_BUFFER_SIZE);
    for (int x = 0; x < len; x++) {
      data = buf[x];

      SPIFFS_CHECK(data >= 0, fileName.c_str());

      if (data != 10) {
        line += char(data);
      } else { // if line complete, parse this rule
        line.replace("\r", "");
        if (line.substring(0, 2) != F("//") && line.length() > 0) {
          parseCompleteNonCommentLine(line, event, log, match, codeBlock,
                                      isCommand, condition, ifBranche, ifBlock,
                                      fakeIfBlock);
          backgroundtasks();
        }

        line = "";
      }
    }
  }
  delete[] buf;
  if (f)
    f.close();

  nestingLevel--;
  checkRAM(F("rulesProcessingFile2"));
  return ("");
}

void parseCompleteNonCommentLine(String &line, String &event, String &log,
                                 bool &match, bool &codeBlock, bool &isCommand,
                                 bool condition[], bool ifBranche[],
                                 byte &ifBlock, byte &fakeIfBlock) {
  isCommand = true;

  // Strip comments
  int comment = line.indexOf(F("//"));
  if (comment > 0)
    line = line.substring(0, comment);

  if (match || !codeBlock) {
    // only parse [xxx#yyy] if we have a matching ruleblock or need to eval the
    // "on" (no codeBlock)
    // This to avoid waisting CPU time...

    if (match && !fakeIfBlock) {
      // substitution of %eventvalue% is made here so it can be used on if
      // statement too
      if (event.charAt(0) == '!') {
        line.replace(F("%eventvalue%"), event); // substitute %eventvalue% with
                                                // literal event string if
                                                // starting with '!'
      } else {
        int equalsPos = event.indexOf("=");
        if (equalsPos > 0) {
          String tmpString = event.substring(equalsPos + 1);
          // line.replace(F("%eventvalue%"), tmpString); // substitute
          // %eventvalue% with the actual value from the event
          String tmpParam;
          if (GetArgv(tmpString.c_str(), tmpParam, 1)) {
            line.replace(F("%eventvalue%"),
                         tmpParam); // for compatibility issues
            line.replace(F("%eventvalue1%"),
                         tmpParam); // substitute %eventvalue1% in actions with
                                    // the actual value from the event
          }
          if (GetArgv(tmpString.c_str(), tmpParam, 2))
            line.replace(F("%eventvalue2%"),
                         tmpParam); // substitute %eventvalue2% in actions with
                                    // the actual value from the event
          if (GetArgv(tmpString.c_str(), tmpParam, 3))
            line.replace(F("%eventvalue3%"),
                         tmpParam); // substitute %eventvalue3% in actions with
                                    // the actual value from the event
          if (GetArgv(tmpString.c_str(), tmpParam, 4))
            line.replace(F("%eventvalue4%"),
                         tmpParam); // substitute %eventvalue4% in actions with
                                    // the actual value from the event
        }
      }
    }
    line = parseTemplate(line, line.length());
  }
  line.trim();

  String lineOrg = line; // store original line for future use
  line.toLowerCase();    // convert all to lower case to make checks easier

  String eventTrigger = "";
  String action = "";

  if (!codeBlock) // do not check "on" rules if a block of actions is to be
                  // processed
  {
    if (line.startsWith(F("on "))) {
      ifBlock = 0;
      fakeIfBlock = 0;
      line = line.substring(3);
      int split = line.indexOf(F(" do"));
      if (split != -1) {
        eventTrigger = line.substring(0, split);
        action = lineOrg.substring(split + 7);
        action.trim();
      }
      if (eventTrigger == "*") // wildcard, always process
        match = true;
      else
        match = ruleMatch(event, eventTrigger);
      if (action.length() > 0) // single on/do/action line, no block
      {
        isCommand = true;
        codeBlock = false;
      } else {
        isCommand = false;
        codeBlock = true;
      }
    }
  } else {
    action = lineOrg;
  }

  String lcAction = action;
  lcAction.toLowerCase();
  if (lcAction == F("endon")) // Check if action block has ended, then we will
                              // wait for a new "on" rule
  {
    isCommand = false;
    codeBlock = false;
    match = false;
    ifBlock = 0;
    fakeIfBlock = 0;
  }

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    String log = F("RuleDebug: ");
    log += codeBlock;
    log += match;
    log += isCommand;
    log += ": ";
    log += line;
    addLog(LOG_LEVEL_DEBUG_DEV, log);
  }

  if (match) // rule matched for one action or a block of actions
  {
    processMatchedRule(lcAction, action, event, log, match, codeBlock,
                       isCommand, condition, ifBranche, ifBlock, fakeIfBlock);
  }
}

void processMatchedRule(String &lcAction, String &action, String &event,
                        String &log, bool &match, bool &codeBlock,
                        bool &isCommand, bool condition[], bool ifBranche[],
                        byte &ifBlock, byte &fakeIfBlock) {
  if (fakeIfBlock)
    isCommand = false;
  else if (ifBlock)
    if (condition[ifBlock - 1] != ifBranche[ifBlock - 1])
      isCommand = false;
  int split =
      lcAction.indexOf(F("elseif ")); // check for optional "elseif" condition
  if (split != -1) {
    isCommand = false;
    if (ifBlock && !fakeIfBlock) {
      if (ifBranche[ifBlock - 1]) {
        if (condition[ifBlock - 1])
          ifBranche[ifBlock - 1] = false;
        else {
          String check = lcAction.substring(split + 7);
          condition[ifBlock - 1] = conditionMatchExtended(check);
          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            log = F("Lev.");
            log += String(ifBlock);
            log += F(": [elseif ");
            log += check;
            log += F("]=");
            log += toString(condition[ifBlock - 1]);
            addLog(LOG_LEVEL_DEBUG, log);
          }
        }
      }
    }
  } else {
    split = lcAction.indexOf(F("if ")); // check for optional "if" condition
    if (split != -1) {
      if (ifBlock < RULES_IF_MAX_NESTING_LEVEL) {
        if (isCommand) {
          ifBlock++;
          String check = lcAction.substring(split + 3);
          condition[ifBlock - 1] = conditionMatchExtended(check);
          ifBranche[ifBlock - 1] = true;
          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            log = F("Lev.");
            log += String(ifBlock);
            log += F(": [if ");
            log += check;
            log += F("]=");
            log += toString(condition[ifBlock - 1]);
            addLog(LOG_LEVEL_DEBUG, log);
          }
        } else
          fakeIfBlock++;
      } else {
        fakeIfBlock++;
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          log = F("Lev.");
          log += String(ifBlock);
          log = F(": Error: IF Nesting level exceeded!");
          addLog(LOG_LEVEL_ERROR, log);
        }
      }
      isCommand = false;
    }
  }

  if ((lcAction == F("else")) && !fakeIfBlock) // in case of an "else" block of
                                               // actions, set ifBranche to
                                               // false
  {
    ifBranche[ifBlock - 1] = false;
    isCommand = false;
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      log = F("Lev.");
      log += String(ifBlock);
      log += F(": [else]=");
      log += toString(condition[ifBlock - 1] == ifBranche[ifBlock - 1]);
      addLog(LOG_LEVEL_DEBUG, log);
    }
  }

  if (lcAction == F("endif")) // conditional block ends here
  {
    if (fakeIfBlock)
      fakeIfBlock--;
    else if (ifBlock)
      ifBlock--;
    isCommand = false;
  }

  // process the action if it's a command and unconditional, or conditional and
  // the condition matches the if or else block.
  if (isCommand) {
    if (event.charAt(0) == '!') {
      action.replace(F("%eventvalue%"), event); // substitute %eventvalue% with
                                                // literal event string if
                                                // starting with '!'
    } else {
      int equalsPos = event.indexOf("=");
      if (equalsPos > 0) {
        String tmpString = event.substring(equalsPos + 1);

        String tmpParam;

        if (GetArgv(tmpString.c_str(), tmpParam, 1)) {
          action.replace(F("%eventvalue%"),
                         tmpParam); // for compatibility issues
          action.replace(F("%eventvalue1%"),
                         tmpParam); // substitute %eventvalue1% in actions with
                                    // the actual value from the event
        }
        if (GetArgv(tmpString.c_str(), tmpParam, 2))
          action.replace(F("%eventvalue2%"),
                         tmpParam); // substitute %eventvalue2% in actions with
                                    // the actual value from the event
        if (GetArgv(tmpString.c_str(), tmpParam, 3))
          action.replace(F("%eventvalue3%"),
                         tmpParam); // substitute %eventvalue3% in actions with
                                    // the actual value from the event
        if (GetArgv(tmpString.c_str(), tmpParam, 4))
          action.replace(F("%eventvalue4%"),
                         tmpParam); // substitute %eventvalue4% in actions with
                                    // the actual value from the event
      }
    }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("ACT  : ");
      log += action;
      addLog(LOG_LEVEL_INFO, log);
    }

    struct EventStruct TempEvent;
    parseCommandString(&TempEvent, action);

    // FIXME TD-er: This part seems a bit strange.
    // It can't schedule a call to PLUGIN_WRITE.
    // Maybe ExecuteCommand can be scheduled?
    delay(0);
    // Use a tmp string to call PLUGIN_WRITE, since PluginCall may inadvertenly
    // alter the string.
    String tmpAction(action);
    if (!PluginCall(PLUGIN_WRITE, &TempEvent, tmpAction)) {
      if (!tmpAction.equals(action)) {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = F("PLUGIN_WRITE altered the string: ");
          log += action;
          log += F(" to: ");
          log += tmpAction;
          addLog(LOG_LEVEL_ERROR, log);
        }
        // TODO: assign here modified action???
      }
      ExecuteCommand(VALUE_SOURCE_SYSTEM, action.c_str());
    }
    delay(0);
  }
}

/********************************************************************************************\
  Check if an event matches to a given rule
  \*********************************************************************************************/
boolean ruleMatch(String &event, String &rule) {
  checkRAM(F("ruleMatch"));
  boolean match = false;
  String tmpEvent = event;
  String tmpRule = rule;

  // Ignore escape char
  tmpRule.replace("[", "");
  tmpRule.replace("]", "");

  // Special handling of literal string events, they should start with '!'
  if (event.charAt(0) == '!') {
    int pos = rule.indexOf('*');
    if (pos != -1) // a * sign in rule, so use a'wildcard' match on message
    {
      tmpEvent = event.substring(0, pos - 1);
      tmpRule = rule.substring(0, pos - 1);
    } else {
      pos = rule.indexOf('#');
      if (pos ==
          -1) // no # sign in rule, use 'wildcard' match on event 'source'
      {
        tmpEvent = event.substring(0, rule.length());
        tmpRule = rule;
      }
    }

    if (tmpEvent.equalsIgnoreCase(tmpRule))
      return true;
    else
      return false;
  }

  if (event.startsWith(
          F("Clock#Time"))) // clock events need different handling...
  {
    int pos1 = event.indexOf("=");
    int pos2 = rule.indexOf("=");
    if (pos1 > 0 && pos2 > 0) {
      tmpEvent = event.substring(0, pos1);
      tmpRule = rule.substring(0, pos2);
      if (tmpRule.equalsIgnoreCase(tmpEvent)) // if this is a clock rule
      {
        tmpEvent = event.substring(pos1 + 1);
        tmpRule = rule.substring(pos2 + 1);
        unsigned long clockEvent = string2TimeLong(tmpEvent);
        unsigned long clockSet = string2TimeLong(tmpRule);
        if (matchClockEvent(clockEvent, clockSet))
          return true;
        else
          return false;
      }
    }
  }

  // parse event into verb and value
  float value = 0;
  int pos = event.indexOf("=");
  if (pos) {
    tmpEvent = event.substring(pos + 1);
    value = tmpEvent.toFloat();
    tmpEvent = event.substring(0, pos);
  }

  // parse rule
  int comparePos = 0;
  char compare = ' ';
  comparePos = rule.indexOf(">");
  if (comparePos > 0) {
    compare = '>';
  } else {
    comparePos = rule.indexOf("<");
    if (comparePos > 0) {
      compare = '<';
    } else {
      comparePos = rule.indexOf("=");
      if (comparePos > 0) {
        compare = '=';
      }
    }
  }

  float ruleValue = 0;

  if (comparePos > 0) {
    tmpRule = rule.substring(comparePos + 1);
    ruleValue = tmpRule.toFloat();
    tmpRule = rule.substring(0, comparePos);
  }

  switch (compare) {
  case '>':
    if (tmpRule.equalsIgnoreCase(tmpEvent) && value > ruleValue)
      match = true;
    break;

  case '<':
    if (tmpRule.equalsIgnoreCase(tmpEvent) && value < ruleValue)
      match = true;
    break;

  case '=':
    if (tmpRule.equalsIgnoreCase(tmpEvent) && value == ruleValue)
      match = true;
    break;

  case ' ':
    if (tmpRule.equalsIgnoreCase(tmpEvent))
      match = true;
    break;
  }
  checkRAM(F("ruleMatch2"));
  return match;
}

/********************************************************************************************\
  Check expression
  \*********************************************************************************************/

boolean conditionMatchExtended(String &check) {
  int condAnd = -1;
  int condOr = -1;
  boolean rightcond = false;
  boolean leftcond = conditionMatch(check); // initial check

  do {
    condAnd = check.indexOf(F(" and "));
    condOr = check.indexOf(F(" or "));

    if (condAnd > 0 || condOr > 0) { // we got AND/OR
      if (condAnd > 0 && ((condOr < 0 && condOr < condAnd) ||
                          (condOr > 0 && condOr > condAnd))) { // AND is first
        check = check.substring(condAnd + 5);
        rightcond = conditionMatch(check);
        leftcond = (leftcond && rightcond);
      } else { // OR is first
        check = check.substring(condOr + 4);
        rightcond = conditionMatch(check);
        leftcond = (leftcond || rightcond);
      }
    }
  } while (condAnd > 0 || condOr > 0);
  return leftcond;
}

boolean conditionMatch(const String &check) {
  boolean match = false;

  char compare = ' ';

  int posStart = check.length();
  int posEnd = posStart;
  int comparePos = 0;

  if ((comparePos = check.indexOf("!=")) > 0 && comparePos < posStart) {
    posStart = comparePos;
    posEnd = posStart + 2;
    compare = '!' + '=';
  }
  if ((comparePos = check.indexOf("<>")) > 0 && comparePos < posStart) {
    posStart = comparePos;
    posEnd = posStart + 2;
    compare = '!' + '=';
  }
  if ((comparePos = check.indexOf(">=")) > 0 && comparePos < posStart) {
    posStart = comparePos;
    posEnd = posStart + 2;
    compare = '>' + '=';
  }
  if ((comparePos = check.indexOf("<=")) > 0 && comparePos < posStart) {
    posStart = comparePos;
    posEnd = posStart + 2;
    compare = '<' + '=';
  }
  if ((comparePos = check.indexOf("<")) > 0 && comparePos < posStart) {
    posStart = comparePos;
    posEnd = posStart + 1;
    compare = '<';
  }
  if ((comparePos = check.indexOf(">")) > 0 && comparePos < posStart) {
    posStart = comparePos;
    posEnd = posStart + 1;
    compare = '>';
  }
  if ((comparePos = check.indexOf("=")) > 0 && comparePos < posStart) {
    posStart = comparePos;
    posEnd = posStart + 1;
    compare = '=';
  }

  float Value1 = 0;
  float Value2 = 0;

  if (compare > ' ') {
    String tmpCheck1 = check.substring(0, posStart);
    String tmpCheck2 = check.substring(posEnd);
    if (!isFloat(tmpCheck1) || !isFloat(tmpCheck2)) {
      Value1 = timeStringToSeconds(tmpCheck1);
      Value2 = timeStringToSeconds(tmpCheck2);
    } else {
      Value1 = tmpCheck1.toFloat();
      Value2 = tmpCheck2.toFloat();
    }
  } else
    return false;

  switch (compare) {
  case '>' + '=':
    if (Value1 >= Value2)
      match = true;
    break;

  case '<' + '=':
    if (Value1 <= Value2)
      match = true;
    break;

  case '!' + '=':
    if (Value1 != Value2)
      match = true;
    break;

  case '>':
    if (Value1 > Value2)
      match = true;
    break;

  case '<':
    if (Value1 < Value2)
      match = true;
    break;

  case '=':
    if (Value1 == Value2)
      match = true;
    break;
  }
  return match;
}

/********************************************************************************************\
  Check rule timers
  \*********************************************************************************************/
void rulesTimers() {
  if (!Settings.UseRules)
    return;
  for (byte x = 0; x < RULES_TIMER_MAX; x++) {
    if (!RulesTimer[x].paused && RulesTimer[x].timestamp != 0L) // timer active?
    {
      if (timeOutReached(RulesTimer[x].timestamp)) // timer finished?
      {
        RulesTimer[x].timestamp = 0L; // turn off this timer
        String event = F("Rules#Timer=");
        event += x + 1;
        rulesProcessing(event);
      }
    }
  }
}

/********************************************************************************************\
  Generate rule events based on task refresh
  \*********************************************************************************************/

void createRuleEvents(byte TaskIndex) {
  if (!Settings.UseRules)
    return;
  LoadTaskSettings(TaskIndex);
  byte BaseVarIndex = TaskIndex * VARS_PER_TASK;
  byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);
  byte sensorType = Device[DeviceIndex].VType;
  for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++) {
    String eventString = getTaskDeviceName(TaskIndex);
    eventString += F("#");
    eventString += ExtraTaskSettings.TaskDeviceValueNames[varNr];
    eventString += F("=");

    if (sensorType == SENSOR_TYPE_LONG)
      eventString += (unsigned long)UserVar[BaseVarIndex] +
                     ((unsigned long)UserVar[BaseVarIndex + 1] << 16);
    else
      eventString += UserVar[BaseVarIndex + varNr];

    rulesProcessing(eventString);
  }
}
