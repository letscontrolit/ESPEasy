#pragma once

#ifndef __HTTP_UPDATE_SERVER_H
#define __HTTP_UPDATE_SERVER_H

#include <Arduino.h>

class ESP32HTTPUpdateServer
{
  public:
    ESP32HTTPUpdateServer(bool serial_debug=false);

    void setup(WebServer *server)
    {
      setup(server, emptyString, emptyString);
    }

    void setup(WebServer *server, const String& path)
    {
      setup(server, path, emptyString, emptyString);
    }

    void setup(WebServer *server, const String& username, const String& password)
    {
      setup(server, "/update", username, password);
    }

    void setup(WebServer *server, const String& path, const String& username, const String& password);

    void updateCredentials(const String& username, const String& password)
    {
      _username = username;
      _password = password;
    }

  protected:
    void _setUpdaterError();

  private:
    bool _serial_output;
    WebServer *_server;
    String _username;
    String _password;
    bool _authenticated;
    String _updaterError;

};

//using ESP32HTTPUpdateServer = ESP32HTTPUpdateServer;

#endif
