Import("env")
import os



def clear_concat_cpp_files(path_to_concat):
    cpp_path_out = '{}_tmp'.format(path_to_concat)
    tmp_cpp_file = os.path.join(cpp_path_out, '__tmpfile.cpp')

    if os.path.exists(tmp_cpp_file):
        os.remove(tmp_cpp_file)

    if os.path.exists(cpp_path_out):
        os.rmdir(cpp_path_out)


def clear_all_concat_cpp_files(source, target, env):
    clear_concat_cpp_files('./src/src/Commands')
    clear_concat_cpp_files('./src/src/ControllerQueue')
    clear_concat_cpp_files('./src/src/PluginStructs')


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", [clear_all_concat_cpp_files])