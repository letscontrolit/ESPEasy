bool Command_Systen_NoSleep(struct EventStruct *event, const char* Line)
{
  bool success = true;
  Settings.deepSleep = 0;
  return success;
}

bool Command_Systen_deepSleep(struct EventStruct *event, const char* Line)
{
  bool success = true;
  if (event->Par1 > 0)
    deepSleepStart(event->Par1); // call the second part of the function to avoid check and enable one-shot operation
  return success;
}

bool Command_Systen_Reboot(struct EventStruct *event, const char* Line)
{
    pinMode(0, INPUT);
    pinMode(2, INPUT);
    pinMode(15, INPUT);
    #if defined(ESP8266)
      ESP.reset();
    #endif
    #if defined(ESP32)
      ESP.restart();
    #endif
    return true;
}

bool Command_Systen_Restart(struct EventStruct *event, const char* Line)
{
    ESP.restart();
    return true;
}

