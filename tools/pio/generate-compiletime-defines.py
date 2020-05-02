
Import('env')
import os
import shutil
from datetime import date


def get_build_filename():
    today = date.today()
    d1 = today.strftime("%Y%m%d")
    return 'ESP_Easy_mega_{}_{}'.format(d1, env["PIOENV"])


def gen_compiletime_defines(node):
    """
    `node.name` - a name of File System Node
    `node.get_path()` - a relative path
    `node.get_abspath()` - an absolute path
    """

    # do not modify node if file name does not contain "CRCStruct"
    if "CompiletimeDefines.cpp" not in node.name:
        return node

    # now, we can override ANY SCons variables (CPPDEFINES, CCFLAGS, etc.,) for the specific file
    # pass SCons variables as extra keyword arguments to `env.Object()` function
    # p.s: run `pio run -t envdump` to see a list with SCons variables

    return env.Object(
        node,
        CPPDEFINES=env["CPPDEFINES"]
        + [("BUILD_BINARY_FILENAME", '\"\"\"{}\"\"\"'.format(get_build_filename()))],
        CCFLAGS=env["CCFLAGS"]
    )

    #return node


env.AddBuildMiddleware(gen_compiletime_defines)
