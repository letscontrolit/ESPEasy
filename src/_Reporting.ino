//not finished yet

#ifdef FEATURE_REPORTING


#include <ArduinoJson.h>
// NO, too big: #include <ESP8266HTTPClient.h>

#define REPORT_HOST "espeasy.datux.nl"
#define FEATURE_REPORTING

void ReportStatus()
{
  String log;
  String host=F(REPORT_HOST);

  log=F("REP  : Reporting status to ");
  log+=host;
  addLog(LOG_LEVEL_INFO, log);


  WiFiClient client;
  if (client.connect(host.c_str(), 80) != 1)
  {
    addLog(LOG_LEVEL_ERROR, F("REP  : connection failed"));
    return;
  }

  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root[F("chipId")]   = ESP.getChipId();
  root[F("flashId")]  = ESP.getFlashChipId();
  root[F("uptime")]   = wdcounter /2;



  String body;
  root.printTo(body);

  String payload = F("POST /report.php HTTP/1.1\r\n");
  payload += F("Host: ");
  payload += host;
  payload += F("\r\n");
  payload += F("Connection: close\r\n");
  payload += F("Content-Length: ");
  payload += String(body.length());
  payload += F("\r\n\r\n");
  payload += body;

  Serial.println(payload);
  client.print(payload);



  addLog(LOG_LEVEL_INFO, F("REP  : report uploaded"));

}

/*
Code below unfortunatly uses a lot of flash+r.o. ram because of the httpclient library.
So we only use wificlient and do our own http magic.

plugin                        |cache IRAM |init RAM   |r.o. RAM   |uninit RAM |Flash ROM
src/_Reporting.ino            |0          |0          |156        |-8         |4000

void ReportStatus()
{
  String log;
  String url=F(REPORT_URL);

  log=F("REP  : Reporting status to ");
  log+=url;
  addLog(LOG_LEVEL_INFO, log);

  HTTPClient http;
  http.begin(url);
  int httpCode=http.POST(F("moin"));

  if (httpCode>0)
  {
    log=F("REP  : Report uploaded, http code ");
    log+=httpCode;
  }
  else
  {
    log=F("REP  : Error, ");
    log+=http.errorToString(httpCode);
  }
  addLog(LOG_LEVEL_INFO,log);
}
*/

#endif
