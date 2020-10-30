#ifndef ESPEASYWIFI_CREDENTIALS_H
#define ESPEASYWIFI_CREDENTIALS_H

const char* getLastWiFiSettingsSSID();
const char* getLastWiFiSettingsPassphrase();
bool selectNextWiFiSettings();
bool selectValidWiFiSettings();
bool wifiSettingsValid(const char *ssid, const char *pass);


#endif // ESPEASYWIFI_CREDENTIALS_H