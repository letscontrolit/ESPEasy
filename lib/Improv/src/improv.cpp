#include "improv.h"

namespace improv {

ImprovCommand parse_improv_data(const std::vector<uint8_t> &data, bool check_checksum) {
  return parse_improv_data(data.data(), data.size(), check_checksum);
}

ImprovCommand parse_improv_data(const uint8_t *data, size_t length, bool check_checksum) {
  ImprovCommand improv_command;
  Command command = (Command) data[0];
  uint8_t data_length = data[1];

  if (data_length != length - 2 - check_checksum) {
    improv_command.command = UNKNOWN;
    return improv_command;
  }

  if (check_checksum) {
    uint8_t checksum = data[length - 1];

    uint32_t calculated_checksum = 0;
    for (uint8_t i = 0; i < length - 1; i++) {
      calculated_checksum += data[i];
    }

    if ((uint8_t) calculated_checksum != checksum) {
      improv_command.command = BAD_CHECKSUM;
      return improv_command;
    }
  }

  if (command == WIFI_SETTINGS) {
    uint8_t ssid_length = data[2];
    uint8_t ssid_start = 3;
    size_t ssid_end = ssid_start + ssid_length;

    uint8_t pass_length = data[ssid_end];
    size_t pass_start = ssid_end + 1;
    size_t pass_end = pass_start + pass_length;

    std::string ssid(data + ssid_start, data + ssid_end);
    std::string password(data + pass_start, data + pass_end);
    return {.command = command, .ssid = ssid, .password = password};
  }

  improv_command.command = command;
  return improv_command;
}

bool parse_improv_serial_byte(size_t position, uint8_t byte, const uint8_t *buffer,
                              std::function<bool(ImprovCommand)> &&callback, std::function<void(Error)> &&on_error) {
  if (position == 0)
    return byte == 'I';
  if (position == 1)
    return byte == 'M';
  if (position == 2)
    return byte == 'P';
  if (position == 3)
    return byte == 'R';
  if (position == 4)
    return byte == 'O';
  if (position == 5)
    return byte == 'V';

  if (position == 6)
    return byte == IMPROV_SERIAL_VERSION;

  if (position <= 8)
    return true;

  uint8_t type = buffer[7];
  uint8_t data_len = buffer[8];

  if (position <= 8 + data_len)
    return true;

  if (position == 8 + data_len + 1) {
    uint8_t checksum = 0x00;
    for (size_t i = 0; i < position; i++)
      checksum += buffer[i];

    if (checksum != byte) {
      on_error(ERROR_INVALID_RPC);
      return false;
    }

    if (type == TYPE_RPC) {
      auto command = parse_improv_data(&buffer[9], data_len, false);
      return callback(command);
    }
  }

  return false;
}

std::vector<uint8_t> build_rpc_response(Command command, const std::vector<std::string> &datum, bool add_checksum) {
  std::vector<uint8_t> out;
  uint32_t length = 0;
  out.push_back(command);
  for (const auto &str : datum) {
    uint8_t len = str.length();
    length += len + 1;
    out.push_back(len);
    out.insert(out.end(), str.begin(), str.end());
  }
  out.insert(out.begin() + 1, length);

  if (add_checksum) {
    uint32_t calculated_checksum = 0;

    for (uint8_t byte : out) {
      calculated_checksum += byte;
    }
    out.push_back(calculated_checksum);
  }
  return out;
}

#ifdef ARDUINO
std::vector<uint8_t> build_rpc_response(Command command, const std::vector<String> &datum, bool add_checksum) {
  std::vector<uint8_t> out;
  uint32_t length = 0;
  out.push_back(command);
  for (const auto &str : datum) {
    uint8_t len = str.length();
    length += len;
    out.push_back(len);
    out.insert(out.end(), str.begin(), str.end());
  }
  out.insert(out.begin() + 1, length);

  if (add_checksum) {
    uint32_t calculated_checksum = 0;

    for (uint8_t byte : out) {
      calculated_checksum += byte;
    }
    out.push_back(calculated_checksum);
  }
  return out;
}
#endif  // ARDUINO

}  // namespace improv
