
Import('env')
import os
import platform
import shutil
from datetime import date


def create_binary_filename():
    today = date.today()
    d1 = today.strftime("%Y%m%d")
    return 'ESP_Easy_mega_{}_{}'.format(d1, env["PIOENV"])

# needed to wrap in a number of double quotes.
# one level for adding it to the list of defines
# another level to have the string quoted in the .cpp file
# somewhere along the line, another level is removed.
def wrap_quotes(str_value):
    if platform.system() == 'Windows':
        return "\"\"\"{}\"\"\"".format(str_value)
    else:
        return "\"\"\"\"{}\"\"\"\"".format(str_value)


def gen_compiletime_defines(node):
    """
    `node.name` - a name of File System Node
    `node.get_path()` - a relative path
    `node.get_abspath()` - an absolute path
    """

    # do not modify node if file name does not contain "CompiletimeDefines.cpp"
    if "CompiletimeDefines.cpp" not in node.name:
        return node

    # now, we can override ANY SCons variables (CPPDEFINES, CCFLAGS, etc.,) for the specific file
    # pass SCons variables as extra keyword arguments to `env.Object()` function
    # p.s: run `pio run -t envdump` to see a list with SCons variables

    return env.Object(
        node,
        CPPDEFINES=env["CPPDEFINES"]
        + [("BUILD_BINARY_FILENAME", wrap_quotes(create_binary_filename()))],
        CCFLAGS=env["CCFLAGS"]
    )

    #return node


env.AddBuildMiddleware(gen_compiletime_defines)
