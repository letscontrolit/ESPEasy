#include "_Plugin_Helper_CAN.h"
#include "../Globals/Settings.h"
#include "../Globals/RuntimeData.h"
#include "../Commands/InternalCommands.h"
# include <CAN.h>

# define CAN_HELPER_MSG_DATA     ((uint8_t)0)
# define CAN_HELPER_MSG_CMD_GPIO ((uint8_t)1)

union canHelper_byte_field {
    float float_var;
    uint32_t uint_var;
    byte byte_var[4];
};

bool canHelper_sendData(const uint8_t taskIndex, const uint8_t valIndex, const uint8_t sensorType, const uint32_t val32)
{
  canHelper_byte_field val;
  val.uint_var = val32;

  CAN.beginPacket(Settings.CAN_node_id & 0x7FF);
  CAN.write(CAN_HELPER_MSG_DATA);
  CAN.write(taskIndex);
  CAN.write(valIndex);
  CAN.write(sensorType);
  CAN.write(val.byte_var[0]);
  CAN.write(val.byte_var[1]);
  CAN.write(val.byte_var[2]);
  CAN.write(val.byte_var[3]);
  int ret = CAN.endPacket(200);

  if (ret == CANControllerClass::SEND_OK) {
    String log = "CAN  : Send nd:" + String(uint8_t(Settings.CAN_node_id & 0x7FF)) + 
      " tId:" + String(uint8_t(taskIndex)+1) + 
      " vId:" + String(uint8_t(valIndex)) + 
      " (" + String(val.float_var, 2) + ")";
    addLogMove(LOG_LEVEL_DEBUG, log);
    return true;
  } else if (ret == CANControllerClass::SEND_ACK) {
    addLog(LOG_LEVEL_ERROR, F("CAN  : CAN ACK error"));
  } else {
    String log = F("CAN  : send message error ");
    log += ret;
    log += String(sensorType);
    addLogMove(LOG_LEVEL_ERROR, log);
  }

  return false;
}

static void canHelper_sendTaskValue(EventStruct *event, const uint8_t val_index, Sensor_VType sensorType)
{
  canHelper_byte_field u;

  if (isFloatOutputDataType(sensorType)) {
    u.float_var = UserVar.getFloat(event->TaskIndex, val_index);
  } else {
    u.uint_var = UserVar.getUint32(event->TaskIndex, val_index);
  }

  canHelper_sendData(event->TaskIndex, val_index, static_cast<uint8_t>(event->getSensorType()), u.uint_var);
}

static void canHelper_recvData()
{
  EventStruct tmp;
  String dummy;
  canHelper_byte_field u;

  tmp.Par2 = int(CAN.read()); // TaskIndex
  tmp.idx = int(CAN.read()); // ValueIndex
  tmp.sensorType = (Sensor_VType)CAN.read();
  u.byte_var[0] = CAN.read();
  u.byte_var[1] = CAN.read();
  u.byte_var[2] = CAN.read();
  u.byte_var[3] = CAN.read();

  tmp.Par1 = CAN.packetId();

  String log = "CAN: Recv nd:" + String(tmp.Par1) + 
    " tId:" + String(tmp.Par2 + 1) + 
    " vId:" + String(tmp.idx) +
    " sTp:" + static_cast<uint8_t>(tmp.sensorType);
  if (isFloatOutputDataType(tmp.sensorType)) {
    log += " (" + String(u.float_var, 2) + ")";
  } else {
    log += " (" + String(u.uint_var) + ")";
  }
  addLogMove(LOG_LEVEL_INFO, log);

  for (taskIndex_t x = 0; x < TASKS_MAX; x++) {
    constexpr pluginID_t PLUGIN_ID_CAN_HELPER(155);
    if (Settings.TaskDeviceEnabled[x] 
        && (Settings.getPluginID_for_task(x) == PLUGIN_ID_CAN_HELPER)
        && (Settings.TaskDevicePluginConfig[x][0] == tmp.Par1)
        && (Settings.TaskDevicePluginConfig[x][2] == tmp.Par2)
        ) {
      tmp.TaskIndex = x;

      if (isFloatOutputDataType(tmp.sensorType)) {
        UserVar.setFloat(tmp.TaskIndex, tmp.idx, u.float_var);
      } else {
        UserVar.setUint32(tmp.TaskIndex, tmp.idx, u.uint_var);
      }

      PluginCall(PLUGIN_READ, &tmp, dummy);
    }
  }
}

static void canHelper_recvGPIO()
{
  if (CAN.available() != 3) {
    addLog(LOG_LEVEL_INFO, "helperCAN: Received CAN gpio packet with wrong size");
    return;
  }

  int node = CAN.read();

  if (node == Settings.CAN_node_id) {
    const uint8_t pin = CAN.read();
    const uint8_t value = CAN.read();

    char cmd[32];
    sprintf(cmd, "GPIO,%u,%u", pin, value);
    ExecuteCommand(0, EventValueSource::Enum::VALUE_SOURCE_SERIAL, cmd, false, true, false);
  }
}

void canHelper_recv()
{
  const int packetSize = CAN.parsePacket();
  if (packetSize == 0 && CAN.packetId() == -1)
  {
    return;
  }

  uint8_t msg_type = CAN.read();

  if (msg_type == CAN_HELPER_MSG_DATA) {
    if (CAN.available() == 7) {
      canHelper_recvData();
    } else {
      addLog(LOG_LEVEL_ERROR, " CAN   : Received CAN data packet with wrong size");
    }
  } else if (msg_type == CAN_HELPER_MSG_CMD_GPIO) {
    canHelper_recvGPIO();
  }
}

void canHelper_sendTaskData(struct EventStruct *event)
{
  Sensor_VType sensorType = event->getSensorType();

  if (isFloatOutputDataType(sensorType)
      || isUInt32OutputDataType(sensorType)
      || isInt32OutputDataType(sensorType)
      ) {
    const uint8_t valueCount = getValueCountFromSensorType(sensorType);

    for (uint8_t i = 0; i < valueCount; ++i) {
      canHelper_sendTaskValue(event, i, sensorType);
    }
  } else {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("canHelper: Not yet implemented sensor type: ");
      log += static_cast<uint8_t>(event->sensorType);
      log += F(" idx: ");
      log += event->idx;
      addLogMove(LOG_LEVEL_ERROR, log);
    }
  }
}

static bool canHelper_sendGPIO(struct EventStruct *event)
{
  CAN.beginPacket(Settings.CAN_node_id & 0x7FF);
  CAN.write(CAN_HELPER_MSG_CMD_GPIO);
  CAN.write(uint8_t(event->Par2));
  CAN.write(uint8_t(event->Par3));
  CAN.write(uint8_t(event->Par4));
  int ret = CAN.endPacket(200);

  if (ret == CANControllerClass::SEND_OK) {
    // String log = "CAN  : Send nd:" + String(uint8_t(Settings.CAN_node_id & 0x7FF)) + 
    //   " tId:" + String(uint8_t(event->TaskIndex)+1) + 
    //   " vId:" + String(uint8_t(val_index)) + 
    //   " (" + String(u.float_var, 2) + ")";
    // addLogMove(LOG_LEVEL_INFO, log);
    return true;
  } else if (ret == CANControllerClass::SEND_ACK) {
    addLog(LOG_LEVEL_ERROR, F("canHelper: CAN ACK error"));
  } else {
    String log = F("canHelper: send message error ");
    log += ret;
    log += String(int(event->getSensorType()));
    addLogMove(LOG_LEVEL_ERROR, log);
  }
  return false;
}

bool canHelper_sendCmd(EventStruct *event)
{
  if (event->Par1 == CAN_HELPER_MSG_CMD_GPIO) {
    return canHelper_sendGPIO(event);
  }

  return false;
}
