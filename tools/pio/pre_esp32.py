# Part of ESPEasy build toolchain.
#
# ESP8266 builds need to concatenate the files to a single .cpp file.
# However for ESP32 we should ignore this file.
# So when building for ESP32, we simply delete this temporary file.

Import("env")

platform = env.PioPlatform()

import os
import glob
import filecmp


def delete_concat_cpp_files(paths_to_concat):
    cpp_files = []

    cpp_path = paths_to_concat[0]

    cpp_path_out = '{}_tmp'.format(cpp_path)

    tmp_cpp_file = os.path.join(cpp_path_out, '__tmpfile.cpp')
    tmp2_cpp_file = os.path.join(cpp_path_out, '__tmpfile.cpp.test')
    if os.path.exists(tmp_cpp_file):
        os.remove(tmp_cpp_file)
    if os.path.exists(tmp2_cpp_file):
        os.remove(tmp2_cpp_file)


delete_concat_cpp_files(['./src/src/Commands',
                        './src/src/ControllerQueue',
                        './src/src/DataStructs',
                        './src/src/DataTypes',
                        './src/src/ESPEasyCore',
                        './src/src/Globals',
                        './src/src/Helpers',
                        './src/src/PluginStructs',
                        './src/src/WebServer'])
