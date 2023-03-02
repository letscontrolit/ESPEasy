#include "../DataStructs/ESPEasyControllerCache_CSV_dumper.h"

#if FEATURE_RTC_CACHE_STORAGE


# include "../Globals/C016_ControllerCache.h"
# include "../Globals/MQTT.h"

# include "../../_Plugin_Helper.h"

void ESPEasyControllerCache_CSV_element::markBegin()
{
  line.clear();

  if ((endFileNr != 0) && (endPos != 0)) {
    startFileNr = endFileNr;
    startPos    = endPos;
  } else {
    startPos = ControllerCache.getPeekFilePos(startFileNr);
  }
}

void ESPEasyControllerCache_CSV_element::markEnd()
{
  endPos = ControllerCache.getPeekFilePos(endFileNr);
}

ESPEasyControllerCache_CSV_dumper::ESPEasyControllerCache_CSV_dumper(bool joinTimestamp, bool onlySetTasks, char separator, Target target)
  : _joinTimestamp(joinTimestamp), _onlySetTasks(onlySetTasks), _separator(separator), _target(target)
{
  // Initialize arrays
  const String sep_zero = String(separator);

  constexpr size_t nrTaskValues = VARS_PER_TASK * TASKS_MAX;

  for (size_t i = 0; i < nrTaskValues; ++i) {
    _csv_values[i] = sep_zero;
    _nrDecimals[i] = Cache.getTaskDeviceValueDecimals(i / VARS_PER_TASK, i % VARS_PER_TASK);
  }

  for (size_t task = 0; validTaskIndex(task); ++task) {
    _includeTask[task] = _onlySetTasks ? validPluginID(Settings.TaskDeviceNumber[task]) : true;
  }

  if (_target == Target::CSV_file) {
    // First backup the peek file positions.
    _backup_peekFilePos = ControllerCache.getPeekFilePos(_backup_peekFileNr);

    // Set peek file position to first entry:
    ControllerCache.setPeekFilePos(0, 0);
  }
  C016_flush();
}

ESPEasyControllerCache_CSV_dumper::~ESPEasyControllerCache_CSV_dumper() {
  if ((_backup_peekFileNr != 0) && (_backup_peekFilePos != 0)) {
    // Restore peek file positions.
    ControllerCache.setPeekFilePos(_backup_peekFileNr, _backup_peekFilePos);
  }
}

uint32_t ESPEasyControllerCache_CSV_dumper::writeToTarget(const String& str, bool send) const {
  if (send) {
    if (_target == Target::CSV_file) {
      addHtml(str);
    } else {
      MQTTclient.write(str);
    }
  }
  return str.length();
}

uint32_t ESPEasyControllerCache_CSV_dumper::writeToTarget(const char& c, bool send) const {
  if (send) {
    if (_target == Target::CSV_file) {
      addHtml(c);
    } else {
      MQTTclient.write(static_cast<uint8_t>(c));
    }
  }
  return 1;
}

size_t ESPEasyControllerCache_CSV_dumper::generateCSVHeader(bool send) const
{
  size_t count = 0;

  // CSV header
  String header(F("UNIX timestamp;UTC timestamp"));

  if (_joinTimestamp) {
    // Add column with nr of joined samples
    header += F(";nrJoinedSamples");
  }

  // TaskIndex and Plugin ID will be a list of numbers when lines are joined.
  header += F(";taskindex;plugin ID");

  if (_separator != ';') { header.replace(';', _separator); }
  count += writeToTarget(header, send);

  for (taskIndex_t i = 0; i < TASKS_MAX; ++i) {
    if (_includeTask[i]) {
      for (int j = 0; j < VARS_PER_TASK; ++j) {
        count += writeToTarget(_separator, send);
        count += writeToTarget(getTaskDeviceName(i), send);
        count += writeToTarget('#', send);
        count += writeToTarget(getTaskValueName(i, j), send);
      }
    }
  }

  if (_target == Target::CSV_file) {
    count += writeToTarget('\r', send);
    count += writeToTarget('\n', send);
  }

  return count;
}

bool ESPEasyControllerCache_CSV_dumper::createCSVLine()
{
  _outputLine.markBegin();


  uint32_t lastTimestamp   = 0;
  uint32_t csv_values_left = 0;

  _taskIndex_str.clear();
  _pluginID_str.clear();


  // Fetch samples from Cache Controller bin files.
  if (_element_processed) {
    if (!C016_getTaskSample(_element)) {
      return !_outputLine.line.isEmpty();
    }
    _outputLine.markEnd();
    _element_processed = false;
  }

  while (!_element_processed) {
    if (!_joinTimestamp || (lastTimestamp != static_cast<uint32_t>(_element.unixTime))) {
      // Flush the collected CSV values
      if (csv_values_left > 0) {
        flushValuesLeft(csv_values_left);
        return !_outputLine.line.isEmpty();
      }

      // Start writing a new line in the CSV file
      // Begin with the non taskvalues
      _outputLine.line += _element.unixTime;
      _outputLine.line += _separator;
      struct tm ts;
      breakTime(_element.unixTime, ts);
      _outputLine.line += formatDateTimeString(ts);

      if (!_joinTimestamp) {
        _outputLine.line += _separator;
        _outputLine.line += _element.TaskIndex;
        _outputLine.line += _separator;
        _outputLine.line += _element.pluginID;
      }

      lastTimestamp = static_cast<uint32_t>(_element.unixTime);
    }

    if (validTaskIndex(_element.TaskIndex)) {
      // Collect the task values for this row in the CSV
      size_t valindex = _element.TaskIndex * VARS_PER_TASK;

      for (size_t i = 0; i < VARS_PER_TASK; ++i) {
        _csv_values[valindex] = _separator;

        if (essentiallyZero(_element.values[i])) {
          _csv_values[valindex] += '0';
        } else {
          _csv_values[valindex] += toString(_element.values[i], static_cast<unsigned int>(_nrDecimals[valindex]));
        }
        ++valindex;
      }

      if (_joinTimestamp) {
        if (!_taskIndex_str.isEmpty()) { _taskIndex_str += '/'; }

        if (!_pluginID_str.isEmpty()) { _pluginID_str += '/'; }

        _taskIndex_str += _element.TaskIndex;
        _pluginID_str  += _element.pluginID;
      }
      ++csv_values_left;
    }
    _outputLine.markEnd();
    _element_processed = !C016_getTaskSample(_element);
  }

  if (csv_values_left > 0) {
    flushValuesLeft(csv_values_left);
  }
  return !_outputLine.line.isEmpty();
}

void ESPEasyControllerCache_CSV_dumper::setPeekFilePos(int peekFileNr, int peekReadPos)
{
  ControllerCache.setPeekFilePos(peekFileNr, peekReadPos);
  _element_processed = true;
}

void ESPEasyControllerCache_CSV_dumper::flushValuesLeft(uint32_t csv_values_left)
{
  if (_joinTimestamp) {
    _outputLine.line += _separator;
    _outputLine.line += csv_values_left; // Add column with nr of joined samples
    _outputLine.line += _separator;
    _outputLine.line += _taskIndex_str;
    _outputLine.line += _separator;
    _outputLine.line += _pluginID_str;
  }

  constexpr size_t nrTaskValues = VARS_PER_TASK * TASKS_MAX;

  for (size_t i = 0; i < nrTaskValues; ++i) {
    if (_includeTask[i / VARS_PER_TASK]) {
      _outputLine.line += _csv_values[i];
    }
  }

  if (_target == Target::CSV_file) {
    _outputLine.line += '\r';
    _outputLine.line += '\n';
  }
}

#endif // if FEATURE_RTC_CACHE_STORAGE
