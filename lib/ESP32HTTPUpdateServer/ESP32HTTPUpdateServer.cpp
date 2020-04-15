
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WebServer.h>
#include <WiFiUdp.h>
//#include <flash_hal.h>
#include <FS.h>
#include <Update.h>
#include "StreamString.h"

#include <ESP32HTTPUpdateServer.h>
  
  static const char ESP32HTTPUpdateServer_serverIndex[] PROGMEM =
  R"(<!DOCTYPE html>
     <html lang='en'>
     <head>
         <meta charset='utf-8'>
         <meta name='viewport' content='width=device-width,initial-scale=1'/>
     </head>
     <body>
     <form method='POST' action='' enctype='multipart/form-data'>
         Firmware:<br>
         <input type='file' accept='.bin' name='firmware'>
         <input type='submit' value='Update Firmware'>
     </form>
     </body>
     </html>)";

 static const char ESP32HTTPUpdateServer_successResponse[] PROGMEM = 
  "<META http-equiv=\"refresh\" content=\"15;URL=/\">Update Success! Rebooting...";

ESP32HTTPUpdateServer::ESP32HTTPUpdateServer(bool serial_debug)
{
  _serial_output = serial_debug;
  _server = NULL;
  _username = emptyString;
  _password = emptyString;
  _authenticated = false;
}

void ESP32HTTPUpdateServer::setup(WebServer *server, const String& path, const String& username, const String& password)
{
    _server = server;
    _username = username;
    _password = password;

    // handler for the /update form page
    _server->on(path.c_str(), HTTP_GET, [&](){
      if(_username != emptyString && _password != emptyString && !_server->authenticate(_username.c_str(), _password.c_str()))
        return _server->requestAuthentication();
      _server->send_P(200, PSTR("text/html"), ESP32HTTPUpdateServer_serverIndex);
    });

    // handler for the /update form POST (once file upload finishes)
    _server->on(path.c_str(), HTTP_POST, [&](){
      if(!_authenticated)
        return _server->requestAuthentication();
      if (Update.hasError()) {
        _server->send(200, F("text/html"), String(F("Update error: ")) + _updaterError);
      } else {
        _server->client().setNoDelay(true);
        _server->send_P(200, PSTR("text/html"), ESP32HTTPUpdateServer_successResponse);
        delay(100);
        _server->client().stop();
        ESP.restart();
      }
    },[&](){
      // handler for the file upload, get's the sketch bytes, and writes
      // them through the Update object
      HTTPUpload& upload = _server->upload();

      if(upload.status == UPLOAD_FILE_START){
        _updaterError.clear();
        if (_serial_output)
          Serial.setDebugOutput(true);

        _authenticated = (_username == emptyString || _password == emptyString || _server->authenticate(_username.c_str(), _password.c_str()));
        if(!_authenticated){
          if (_serial_output)
            Serial.printf("Unauthenticated Update\n");
          return;
        }

        //WiFiUDP::stopAll();
        if (_serial_output)
          Serial.printf("Update: %s\n", upload.filename.c_str());
        if (upload.name == "filesystem") {
          /* TODO: add filesystem support
          size_t fsSize = ((size_t) &_FS_end - (size_t) &_FS_start);
          close_all_fs();
          if (!Update.begin(fsSize, U_SPIFFS)){//start with max available size
            if (_serial_output) Update.printError(Serial);
          }*/
        } else {
          uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
          if (!Update.begin(maxSketchSpace, U_FLASH)){//start with max available size
            _setUpdaterError();
          }
        }
      } else if(_authenticated && upload.status == UPLOAD_FILE_WRITE && !_updaterError.length()){
        if (_serial_output) Serial.printf(".");
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          _setUpdaterError();
        }
      } else if(_authenticated && upload.status == UPLOAD_FILE_END && !_updaterError.length()){
        if(Update.end(true)){ //true to set the size to the current progress
          if (_serial_output) Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          _setUpdaterError();
        }
        if (_serial_output) Serial.setDebugOutput(false);
      } else if(_authenticated && upload.status == UPLOAD_FILE_ABORTED){
        Update.end();
        if (_serial_output) Serial.println("Update was aborted");
      }
      delay(0);
    });
}

void ESP32HTTPUpdateServer::_setUpdaterError()
{
  if (_serial_output) Update.printError(Serial);
  StreamString str;
  Update.printError(str);
  _updaterError = str.c_str();
}
    

