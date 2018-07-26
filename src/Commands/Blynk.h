#ifndef COMMAND_BLYNK_H
#define COMMAND_BLYNK_H

#ifdef CPLUGIN_012
  //FIXME: this should go to PLUGIN_WRITE in _C012.ino
bool Command_Blynk_Get(struct EventStruct *event, const char* Line)
{
  byte first_enabled_blynk_controller = firstEnabledBlynkController();
  if (first_enabled_blynk_controller == -1) {
    status = F("Controller not enabled");
  } else {
    String strLine = Line;
    strLine = strLine.substring(9);
    int index = strLine.indexOf(',');
    if (index > 0)
    {
      int index = strLine.lastIndexOf(',');
      String blynkcommand = strLine.substring(index+1);
      float value = 0;
      if (Blynk_get(blynkcommand, first_enabled_blynk_controller, &value))
      {
        UserVar[(VARS_PER_TASK * (event->Par1 - 1)) + event->Par2 - 1] = value;
      }
      else
        status = F("Error getting data");
    }
    else
    {
      if (!Blynk_get(strLine, first_enabled_blynk_controller))
      {
        status = F("Error getting data");
      }
    }
    
  }
  return true;
}
#endif

#endif // COMMAND_BLYNK_H