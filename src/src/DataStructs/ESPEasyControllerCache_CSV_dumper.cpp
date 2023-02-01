#include "../DataStructs/ESPEasyControllerCache_CSV_dumper.h"

#if FEATURE_RTC_CACHE_STORAGE


# include "../Globals/C016_ControllerCache.h"
# include "../Globals/MQTT.h"

# include "../../_Plugin_Helper.h"


ESPEasyControllerCache_CSV_dumper::ESPEasyControllerCache_CSV_dumper(bool joinTimestamp, bool onlySetTasks, char separator)
  : _joinTimestamp(joinTimestamp), _onlySetTasks(onlySetTasks), _separator(separator)
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

  // First backup the peek file positions.
  _backup_peekFilePos = ControllerCache.getPeekFilePos(_backup_peekFileNr);

  // Set peek file position to first entry:
  ControllerCache.setPeekFilePos(0, 0);

  C016_flush();
}

ESPEasyControllerCache_CSV_dumper::~ESPEasyControllerCache_CSV_dumper() {
  // Restore peek file positions.
  ControllerCache.setPeekFilePos(_backup_peekFileNr, _backup_peekFilePos);
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
  } else {
    // Does not make sense to have taskindex and plugin ID
    // in a table where separate samples may have been combined.
    header += F(";taskindex;plugin ID");
  }

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
  _outputLine.clear();

  uint32_t lastTimestamp   = 0;
  uint32_t csv_values_left = 0;


  // Fetch samples from Cache Controller bin files.
  constexpr size_t nrTaskValues = VARS_PER_TASK * TASKS_MAX;

  if (_element_processed) {
    if (!C016_getTaskSample(element)) {
      return !_outputLine.isEmpty();
    }
    _element_processed = false;
  }

  while (!_element_processed) {
    if (!_joinTimestamp || (lastTimestamp != static_cast<uint32_t>(element._timestamp))) {
      // Flush the collected CSV values
      if (csv_values_left > 0) {
        if (_joinTimestamp) {
          // Add column with nr of joined samples
          _outputLine += _separator;
          _outputLine += csv_values_left;
        }

        for (size_t i = 0; i < nrTaskValues; ++i) {
          if (_includeTask[i / VARS_PER_TASK]) {
            _outputLine += _csv_values[i];
          }
        }

        if (_target == Target::CSV_file) {
          _outputLine += '\r';
          _outputLine += '\n';
        }
        return !_outputLine.isEmpty();
      }

      // Start writing a new line in the CSV file
      // Begin with the non taskvalues
      _outputLine += element._timestamp;
      _outputLine += _separator;
      struct tm ts;
      breakTime(element._timestamp, ts);
      _outputLine += formatDateTimeString(ts);

      if (!_joinTimestamp) {
        _outputLine += _separator;
        _outputLine += element.TaskIndex;
        _outputLine += _separator;
        _outputLine += element.pluginID;
      }

      lastTimestamp = static_cast<uint32_t>(element._timestamp);
    }

    if (validTaskIndex(element.TaskIndex)) {
      // Collect the task values for this row in the CSV
      size_t valindex = element.TaskIndex * VARS_PER_TASK;

      for (size_t i = 0; i < VARS_PER_TASK; ++i) {
        _csv_values[valindex] = _separator;

        if (essentiallyZero(element.values[i])) {
          _csv_values[valindex] += '0';
        } else {
          _csv_values[valindex] += toString(element.values[i], static_cast<unsigned int>(_nrDecimals[valindex]));
        }
        ++valindex;
      }
      ++csv_values_left;
    }
    _element_processed = !C016_getTaskSample(element);
  }

  if (csv_values_left > 0) {
    if (_joinTimestamp) {
      // Add column with nr of joined samples
      _outputLine += _separator;
      _outputLine += csv_values_left;
    }

    for (size_t i = 0; i < nrTaskValues; ++i) {
      if (_includeTask[i / VARS_PER_TASK]) {
        _outputLine += _csv_values[i];
      }
    }

    if (_target == Target::CSV_file) {
      _outputLine += '\r';
      _outputLine += '\n';
    }
  }
  return !_outputLine.isEmpty();
}

#endif // if FEATURE_RTC_CACHE_STORAGE
