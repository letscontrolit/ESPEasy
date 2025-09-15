
#include "../PluginStructs/P056_data_struct.h"

#ifdef USES_P056

P056_data_struct::P056_data_struct(EventStruct *event) {
  delete SDS;

  const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
  SDS = new (std::nothrow) CjkSDS011(port, CONFIG_PIN1, CONFIG_PIN2);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("SDS  : Init OK  ESP GPIO-pin RX:%d TX:%d"), CONFIG_PIN1, CONFIG_PIN2));
  }
}

P056_data_struct::~P056_data_struct() {
  delete SDS;
  SDS = nullptr;
}

void P056_data_struct::Process() {
  if (isInitialized()) {
    this->SDS->Process();
  }
}

bool P056_data_struct::available() {
  if (isInitialized()) {
    return this->SDS->available();
  } else {
    return false;
  }
}

float P056_data_struct::GetPM2_5() {
  if (isInitialized()) {
    return this->SDS->GetPM2_5();
  } else {
    return 0.0f;
  }
}

float P056_data_struct::GetPM10_() {
  if (isInitialized()) {
    return this->SDS->GetPM10_();
  } else {
    return 0.0f;
  }
}

bool P056_data_struct::ReadAverage(float pm25, float pm10) {
  if (isInitialized()) {
    return this->SDS->ReadAverage(pm25, pm10);
  } else {
    return false;
  }
}

void P056_data_struct::SetWorkingPeriod(int minutes) {
  if (isInitialized()) {
    this->SDS->SetWorkingPeriod(minutes);
  }
}

#endif
