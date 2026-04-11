Import("env")
import os
import glob
import filecmp

def do_concat_cpp_files(cpp_files, output_file):
    with open(output_file, 'wb') as newf:
        for filename in cpp_files:
            with open(filename, 'rb') as hf:
                newf.write(hf.read())
                newf.write(os.linesep.encode())  # append newline to separate files.


def concat_cpp_files(paths_to_concat):

    cpp_files = []

    cpp_path = paths_to_concat[0]

    cpp_path_out = '{}_tmp'.format(cpp_path)

    if not os.path.exists(cpp_path_out):
        os.makedirs(cpp_path_out)

    tmp_cpp_file = os.path.join(cpp_path_out, '__tmpfile.cpp')
    tmp2_cpp_file = os.path.join(cpp_path_out, '__tmpfile.cpp.test')

    for path_to_concat in paths_to_concat:

        print("\u001b[32m Concat {}/*.cpp to {} \u001b[0m".format(path_to_concat, tmp_cpp_file))


        for root, dirs, files in os.walk(path_to_concat):
            for file in files:
                if file.endswith('.cpp'):
                    fullname = os.path.join(root, file)
                    cpp_files.append(fullname)
                    print("\u001b[33m Add: \u001b[0m  {}".format(fullname))

        if os.path.exists(tmp_cpp_file):
            os.remove(tmp_cpp_file)

    do_concat_cpp_files(cpp_files, tmp_cpp_file)

    print("\u001b[32m ------------------------------- \u001b[0m")


concat_cpp_files(['./src/src/Commands',
                './src/src/ControllerQueue',
                './src/src/DataStructs',
                './src/src/DataTypes',
                './src/src/ESPEasyCore',
                './src/src/Globals',
                './src/src/Helpers',
                './src/src/PluginStructs',
                './src/src/WebServer'])
