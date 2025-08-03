#include "ImprovWiFiLibrary.h"

#if defined(ESP8266)
  # include <ESP8266WiFi.h>
#endif
#if defined(ESP32)
  # include <WiFi.h>
#endif


bool ImprovWiFi::handleSerial()
{
  if ((_serial != nullptr) && (_serial->available() > 0))
  {
    return handleSerial(_serial->read(), _serial);
  }
  return false;
}

ImprovTypes::ParseState ImprovWiFi::handleSerial(uint8_t b, Stream *serialForWrite)
{
  _serial = serialForWrite;

  if (_position >= sizeof(_buffer)) {
    _position = 0;
  }

  auto res = parseImprovSerial(_position, b, _buffer);

  switch (res)
  {
    case ImprovTypes::ParseState::VALID_INCOMPLETE:
      _buffer[_position++] = b;
      break;
    case ImprovTypes::ParseState::VALID_COMPLETE:
    case ImprovTypes::ParseState::INVALID:
      _position = 0;
      break;
  }
  return res;
}

void ImprovWiFi::onErrorCallback(ImprovTypes::Error err)
{
  if (onImprovErrorCallback)
  {
    onImprovErrorCallback(err);
  }
}

bool ImprovWiFi::onCommandCallback(ImprovTypes::ImprovCommand cmd)
{
  switch (cmd.command)
  {
    case ImprovTypes::Command::GET_CURRENT_STATE:
    {
      if (isConnected())
      {
        setState(ImprovTypes::State::STATE_PROVISIONED);
        sendDeviceUrl(cmd.command);
      }
      else
      {
        setState(ImprovTypes::State::STATE_AUTHORIZED);
      }

      break;
    }

    case ImprovTypes::Command::WIFI_SETTINGS:
    {
      if (cmd.ssid.empty())
      {
        setError(ImprovTypes::Error::ERROR_EMPTY_SSID);
        break;
      }

      setState(ImprovTypes::STATE_PROVISIONING);

      bool success = false;

      if (customTryConnectToWiFiCallback)
      {
        success = customTryConnectToWiFiCallback(cmd.ssid.c_str(), cmd.password.c_str());
      }
      else
      {
        success = tryConnectToWifi(cmd.ssid.c_str(), cmd.password.c_str());
      }

      if (success)
      {
        setError(ImprovTypes::Error::ERROR_NONE);
        setState(ImprovTypes::STATE_PROVISIONED);
        sendDeviceUrl(cmd.command);

        if (onImprovConnectedCallback)
        {
          onImprovConnectedCallback(cmd.ssid.c_str(), cmd.password.c_str());
        }
      }
      else
      {
        setState(ImprovTypes::STATE_STOPPED);
        setError(ImprovTypes::ERROR_UNABLE_TO_CONNECT);
        onErrorCallback(ImprovTypes::ERROR_UNABLE_TO_CONNECT);
      }

      break;
    }

    case ImprovTypes::Command::GET_DEVICE_INFO:
    {
      const std::vector<std::string> infos = {
        // Firmware name
        improvWiFiParams.firmwareName,

        // Firmware version
        improvWiFiParams.firmwareVersion,

        // Hardware chip/variant
        improvWiFiParams.chipVariant,

        // Device name
        improvWiFiParams.deviceName };
      sendResponse(build_rpc_response(ImprovTypes::GET_DEVICE_INFO, infos, false));
      break;
    }

    case ImprovTypes::Command::GET_WIFI_NETWORKS:
    {
      getAvailableWifiNetworks();
      break;
    }

    default:
    {
      setError(ImprovTypes::ERROR_UNKNOWN_RPC);
      return false;
    }
  }

  return true;
}

void ImprovWiFi::setDeviceInfo(const char *firmwareName,
                               const char *firmwareVersion,
                               const char *deviceName)
{
  improvWiFiParams.firmwareName    = firmwareName;
  improvWiFiParams.firmwareVersion = firmwareVersion;
  improvWiFiParams.deviceName      = deviceName;
}

void ImprovWiFi::setDeviceInfo(const char *firmwareName,
                               const char *firmwareVersion,
                               const char *deviceName,
                               const char *deviceUrl)
{
  setDeviceInfo(firmwareName, firmwareVersion, deviceName);
  improvWiFiParams.deviceUrl = deviceUrl;
}

void ImprovWiFi::setDeviceChipInfo(const char *chipVariant)
{
  improvWiFiParams.chipVariant = chipVariant;
}

bool ImprovWiFi::isConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

void ImprovWiFi::sendDeviceUrl(ImprovTypes::Command cmd)
{
  // URL where user can finish onboarding or use device
  // Recommended to use website hosted by device
  const std::string ipStr{WiFi.localIP().toString().c_str()};

  if (improvWiFiParams.deviceUrl.empty())
  {
    improvWiFiParams.deviceUrl = "http://" + ipStr;
  }
  else
  {
    replaceAll(improvWiFiParams.deviceUrl, "{LOCAL_IPV4}", ipStr);
  }

  sendResponse(build_rpc_response(cmd, { improvWiFiParams.deviceUrl }, false));
}

void ImprovWiFi::onImprovError(OnImprovError *errorCallback)
{
  onImprovErrorCallback = errorCallback;
}

void ImprovWiFi::onImprovConnected(OnImprovConnected *connectedCallback)
{
  onImprovConnectedCallback = connectedCallback;
}

void ImprovWiFi::setCustomTryConnectToWiFi(CustomConnectWiFi *connectWiFiCallBack)
{
  customTryConnectToWiFiCallback = connectWiFiCallBack;
}

bool ImprovWiFi::tryConnectToWifi(const char *ssid, const char *password)
{
  uint8_t count = 0;

  if (isConnected())
  {
    WiFi.disconnect();
    delay(100);
  }

  WiFi.begin(ssid, password);

  while (!isConnected())
  {
    delay(DELAY_MS_WAIT_WIFI_CONNECTION);

    if (count > MAX_ATTEMPTS_WIFI_CONNECTION)
    {
      WiFi.disconnect();
      return false;
    }
    count++;
  }

  return true;
}

void ImprovWiFi::getAvailableWifiNetworks()
{
  #ifdef ESP32
  const uint32_t start = millis();
  int networkNum = WiFi.scanNetworks();
  if (networkNum < 0) {
    int32_t timePassed{};
    do {
      timePassed = (int32_t) (millis() - start);
      networkNum = WiFi.scanComplete();
    }
    while (networkNum < 0 && timePassed < 60000);
  }
  #else
  const int networkNum = WiFi.scanNetworks();
  #endif

  for (int id = 0; id < networkNum; ++id)
  {
    #ifdef ESP32
    const bool openWiFi = WiFi.encryptionType(id) == WIFI_AUTH_OPEN;
    #endif 
    #ifdef ESP8266
    const bool openWiFi = WiFi.encryptionType(id) == ENC_TYPE_NONE;
    #endif 
    const std::vector<std::string> wifinetworks = {
      WiFi.SSID(id).c_str(),
      std::string{ static_cast<char>(WiFi.RSSI(id)) },
      (openWiFi ? "NO" : "YES")
    };

    sendResponse(
      build_rpc_response(
        ImprovTypes::GET_WIFI_NETWORKS,
        wifinetworks,
        false));
    delay(1);
  }
  WiFi.scanDelete();

  // final response
  sendResponse(
    build_rpc_response(
      ImprovTypes::GET_WIFI_NETWORKS,
      std::vector<std::string>{},
      false));
}

inline void ImprovWiFi::replaceAll(std::string& str, const std::string& from, const std::string& to)
{
  size_t start_pos = 0;

  while ((start_pos = str.find(from, start_pos)) != std::string::npos)
  {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
}

ImprovTypes::ParseState ImprovWiFi::parseImprovSerial(size_t position, uint8_t byte, const uint8_t *buffer)
{
  constexpr uint8_t header[] = { 'I', 'M', 'P', 'R', 'O', 'V', ImprovTypes::IMPROV_SERIAL_VERSION };

  if (position < sizeof(header)) {
    return (byte == header[position]) ? ImprovTypes::ParseState::VALID_INCOMPLETE : ImprovTypes::ParseState::INVALID;
  }

  if (position <= 8) {
    return ImprovTypes::ParseState::VALID_INCOMPLETE;
  }

  const uint8_t type     = buffer[7];
  const uint8_t data_len = buffer[8];

  if (position <= (8u + data_len)) {
    return ImprovTypes::ParseState::VALID_INCOMPLETE;
  }

  if (position == (8u + data_len + 1u))
  {
    /*
    if (computeChecksum(buffer, position - 1) != byte)
    {
      _position = 0;
      onErrorCallback(ImprovTypes::Error::ERROR_INVALID_CHECKSUM);
      return ImprovTypes::ParseState::INVALID;
    }
    */

    if (type == ImprovTypes::ImprovSerialType::TYPE_RPC)
    {
      _position = 0;
      auto command = parseImprovData(&buffer[9], data_len + 1, false);
      return onCommandCallback(command) ? ImprovTypes::ParseState::VALID_COMPLETE : ImprovTypes::ParseState::INVALID;
    }
  }

  return ImprovTypes::ParseState::INVALID;
}

ImprovTypes::ImprovCommand ImprovWiFi::parseImprovData(const std::vector<uint8_t>& data, bool check_checksum)
{
  return parseImprovData(data.data(), data.size(), check_checksum);
}

ImprovTypes::ImprovCommand ImprovWiFi::parseImprovData(const uint8_t *data, size_t length, bool check_checksum)
{
  ImprovTypes::ImprovCommand improv_command;

  improv_command.command = ImprovTypes::Command::UNKNOWN;

  if (length < 2) {
    return improv_command;
  }
  const ImprovTypes::Command command = (ImprovTypes::Command)data[0];
  const uint8_t data_length          = data[1];
  const uint8_t data_start           = 2;
  const size_t  data_end             = data_start + data_length;

//  if (data_end >= length)
  if (data_length != (length - 2 - check_checksum))
  {
//    return improv_command;
  }

  if (check_checksum)
  {
    const uint8_t checksum = data[data_end];

    if (computeChecksum(data, data_end - 1) != checksum)
    {
      improv_command.command = ImprovTypes::Command::BAD_CHECKSUM;
      setError(ImprovTypes::Error::ERROR_INVALID_CHECKSUM);
      return improv_command;
    }
  }

  if (command == ImprovTypes::Command::WIFI_SETTINGS)
  {
    const uint8_t ssid_length = data[2];
    const uint8_t ssid_start  = 3;
    const size_t  ssid_end    = ssid_start + ssid_length;

    if (ssid_end >= length) {
      return improv_command;
    }

    const uint8_t pass_length = data[ssid_end];
    const size_t  pass_start  = ssid_end + 1;
    const size_t  pass_end    = pass_start + pass_length;

    if (pass_end > length) {
      return improv_command;
    }

    std::string ssid(data + ssid_start, data + ssid_end);
    std::string password(data + pass_start, data + pass_end);

    improv_command.command  = command;
    improv_command.ssid     = ssid;
    improv_command.password = password;
    return improv_command;
  }

  improv_command.command = command;
  return improv_command;
}

void ImprovWiFi::setState(ImprovTypes::State state)
{
  if (_serial == nullptr) { return; }
  const std::vector<uint8_t> response = { state };

  send(ImprovTypes::TYPE_CURRENT_STATE, response);
}

void ImprovWiFi::setError(ImprovTypes::Error error)
{
  if (_serial == nullptr) { return; }
  const std::vector<uint8_t> response = { error };

  send(ImprovTypes::TYPE_ERROR_STATE, response);
  onErrorCallback(error);
}

void ImprovWiFi::sendResponse(const std::vector<uint8_t>& response)
{
  send(ImprovTypes::TYPE_RPC_RESPONSE, response);
}

void ImprovWiFi::send(uint8_t improvType, const std::vector<uint8_t>& response)
{
  if ((_serial == nullptr) || (response.size() == 0)) { return; }
  std::vector<uint8_t> data = { 'I', 'M', 'P', 'R', 'O', 'V' };

  data.resize(9);
  data[6] = ImprovTypes::IMPROV_SERIAL_VERSION;
  data[7] = improvType;
  data[8] = response.size();
  data.insert(data.end(), response.begin(), response.end());
  data.push_back(computeChecksum(data.data(), data.size()));

  _serial->write(data.data(), data.size());
}

uint8_t ImprovWiFi::computeChecksum(const uint8_t *data, size_t length)
{
  uint8_t checksum = 0;

  if (data != nullptr) {
    for (uint8_t i = 0; i < length; i++)
    {
      checksum += data[i];
    }
  }
  return checksum;
}

std::vector<uint8_t>ImprovWiFi::build_rpc_response(ImprovTypes::Command command, const std::vector<std::string>& datum, bool add_checksum)
{
  std::vector<uint8_t> out;
  uint32_t length = 0;

  out.push_back(command);

  for (const auto& str : datum)
  {
    uint8_t len = str.length();
    length += len + 1;
    out.push_back(len);
    out.insert(out.end(), str.begin(), str.end());
  }
  out.insert(out.begin() + 1, length);

  if (add_checksum)
  {
    out.push_back(computeChecksum(out.data(), out.size()));
  }
  return out;
}
