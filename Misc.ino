 /********************************************************************************************\
 * Een float en een unsigned long zijn beide 4bytes groot. Deze zijn niet te casten naar 
 * elkaar. Onderstaande twee funkties converteren de unsigned long
 * en de float.
 \*********************************************************************************************/
unsigned long float2ul(float f)
  {
  unsigned long ul;
  memcpy(&ul, &f,4);
  return ul;
  }

float ul2float(unsigned long ul)
  {
  float f;
  memcpy(&f, &ul,4);
  return f;
  }
  
void addLog(byte loglevel, char *line)
{
  if (loglevel <= Settings.SerialLogLevel)
    Serial.println(line);
    
  if (loglevel <= Settings.SyslogLevel)
    syslog(line);

  if (loglevel <= Settings.WebLogLevel)
    {
      logcount++;
      if (logcount > 9)
        logcount = 0;
      Logging[logcount].timeStamp = millis();
      strncpy(Logging[logcount].Message, line, 80);
      Logging[logcount].Message[79] = 0;
    }
}


void delayedReboot(int rebootDelay)
  {
    while (rebootDelay !=0 )
    {
      Serial.print(F("Delayed Reset "));
      Serial.println(rebootDelay);
      rebootDelay--;
      delay(1000);
    }
    ESP.reset();
  }
