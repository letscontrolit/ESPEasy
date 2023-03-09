import csv
import ctypes
import datetime



def isNewSampleSet(data):
    # Each sample on a new CSV line as is done by dump8.htm
    return True

    # We trigger on the event of the GPS to flush all values
    # Thus consider this the start of a new row
    #return data.pluginID == 82 # GPS plugin
    #return data.TaskIndex == 1


def decode(bytestream):

    #    struct C016_binary_element
    #    {
    #        float             values[VARS_PER_TASK]{};
    #        unsigned long     _timestamp{}; // Unix timestamp
    #        taskIndex_t       TaskIndex{INVALID_TASK_INDEX};
    #        pluginID_t        pluginID{INVALID_PLUGIN_ID};
    #        Sensor_VType      sensorType{Sensor_VType::SENSOR_TYPE_NONE};
    #        uint8_t           valueCount{};
    #    };

    class C016_binary_element(ctypes.Structure):
        _fields_ = (
            ('val1', ctypes.c_float),
            ('val2', ctypes.c_float),
            ('val3', ctypes.c_float),
            ('val4', ctypes.c_float),
            ('timestamp', ctypes.c_ulong),
            ('TaskIndex', ctypes.c_uint8),
            ('pluginID', ctypes.c_uint8),
            ('sensorType', ctypes.c_uint8),
            ('valueCount', ctypes.c_uint8)
        )

        def __str__(self):
            return "{}: {{{}}}".format(self.__class__.__name__,
                                       ", ".join(["{}: {}".format(field[0],
                                                                  getattr(self,
                                                                          field[0]))
                                                  for field in self._fields_]))

    ex1 = C016_binary_element.from_buffer_copy(bytestream)
    return ex1


def processFile(filename):
    # Some example header, used for testing
    # Should use the output sent by cachereader.sendtaskinfo command
    header = "UNIX timestamp;UTC timestamp;task index;plugin ID;pms5003#cnt1.0;pms5003#cnt2.5;pms5003#cnt5.0;pms5003#cnt10;sunrise#co2;sunrise#T;sunrise#;sunrise#;bme280#Temperature;bme280#Humidity;bme280#Pressure;bme280#;bat#Analog;bat#;bat#;bat#;gps#long;gps#lat;gps#alt;gps#spd;batBackup#Analog;batBackup#;batBackup#;batBackup#;cachereader#FileNr;cachereader#FilePos;cachereader#;cachereader#;ADXL345#Pitch;ADXL345#Roll;ADXL345#Z;ADXL345#X;sysinfo#uptime;sysinfo#freeheap;sysinfo#rssi;sysinfo#load;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;#;bme2#Temperature;bme2#Humidity;bme2#Pressure;bme2#;pms1#pm1_0;pms1#pm2_5;pms1#pm10;pms1#cnt5_0;pms2#cnt0_3;pms2#cnt0_5;pms2#cnt1_0;pms2#cnt2_5;CO2#co2;CO2#temp;CO2#co2_prev;CO2#temp_prev;analog#carBat;analog#backupBat;analog#;analog#;gps2#hdop;gps2#satvis;gps2#sattracked;gps2#chksumfail"
    csvheader = header.split(';')
    nrcols = len(csvheader)
    fileIn = open(filename, "r")

    firstline = True

    with open('decoded.csv', 'w', newline='') as fileOut:
        writer = csv.writer(fileOut)
        writer.writerow(csvheader)

        row_out = [0] * nrcols

        for line in fileIn:
            samples = line.split(';')
            length = len(samples)

            # Skip the first 2 elements on the line
            # filenr and filepos
            i = 2

            while i < length:
                if len(samples[i]) == 48:
                    data = decode(bytes.fromhex(samples[i]))
                    if data.TaskIndex < 32: # and data.timestamp > 1674485061:
                        if isNewSampleSet(data):
                            if firstline:
                                firstline = False
                            else:
                                writer.writerow(row_out)

                        baseIndex = data.TaskIndex * 4 + 4
                        row_out[baseIndex + 0] = data.val1
                        row_out[baseIndex + 1] = data.val2
                        row_out[baseIndex + 2] = data.val3
                        row_out[baseIndex + 3] = data.val4

                        row_out[0] = data.timestamp
                        dt = datetime.datetime.fromtimestamp(data.timestamp)
                        row_out[1] = dt
                        row_out[2] = data.TaskIndex
                        row_out[3] = data.pluginID

                        print("{}: {}".format(samples[i], data))
                i += 1

        writer.writerow(row_out)
        fileOut.close()


if __name__ == '__main__':
    processFile("upload_18.csv")

