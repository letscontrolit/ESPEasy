Import("env")
import os



def clear_concat_cpp_files(path_to_concat):
    cpp_path_out = '{}_tmp'.format(path_to_concat)
    tmp_cpp_file = os.path.join(cpp_path_out, '__tmpfile.cpp')

    if os.path.exists(tmp_cpp_file):
        print("\u001b[33m Delete: \u001b[0m  {}".format(tmp_cpp_file))
        os.remove(tmp_cpp_file)

    if os.path.exists(cpp_path_out):
        os.rmdir(cpp_path_out)


def remove_concat_cpp_files(paths_to_remove_concat_files):
    print("\u001b[32m Remove temp concatenated files \u001b[0m")
    for path_to_remove in paths_to_remove_concat_files:
        clear_concat_cpp_files(path_to_remove)
    print("\u001b[32m ------------------------------ \u001b[0m")


remove_concat_cpp_files(['./src/src/Commands',
                         './src/src/ControllerQueue',
                         './src/src/DataStructs',
                         './src/src/DataTypes',
                         './src/src/ESPEasyCore',
                         './src/src/Globals',
                         './src/src/Helpers',
                         './src/src/PluginStructs',
                         './src/src/WebServer'])

