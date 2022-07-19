
Import("env")
import os
import platform
import shutil
from datetime import date
import json


def create_binary_filename():
    today = date.today()
    d1 = today.strftime("%Y%m%d")
    return 'ESP_Easy_mega_{}_{}'.format(d1, env["PIOENV"])


def get_board_name():
    return env.BoardConfig().get("name").replace('"', "")


def get_git_description():
    try:
        from pygit2 import Repository
        try:
            repo = Repository('.')
            return "'{0}_{1}'".format(repo.head.shorthand, repo.revparse_single('HEAD').short_id)
        except:
            return 'No_.git_dir'
    except ImportError:
        return 'pygit2_not_installed'


def deduct_flags_from_pioenv():
    fs_str = "SPIFFS"
    if "LittleFS" in env["PIOENV"]:
        fs_str = "LittleFS"
        env.Append(CPPDEFINES=[
            "USE_LITTLEFS"])
    print("\u001b[33m File System:    \u001b[0m  {}".format(fs_str))


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
        + [("SET_BUILD_BINARY_FILENAME", create_binary_filename())]
        + [("SET_BOARD_NAME", get_board_name())]
        + [("SET_BUILD_PLATFORM", platform.platform())]
        + [("SET_BUILD_GIT_HEAD", get_git_description())],
        CCFLAGS=env["CCFLAGS"]
    )

print("\u001b[32m Compile time defines \u001b[0m")
deduct_flags_from_pioenv()

# Set the binary filename in the environment to be used in other build steps
env.Replace(PROGNAME=create_binary_filename())
print("\u001b[33m PROGNAME:       \u001b[0m  {}".format(env['PROGNAME']))
print("\u001b[33m BOARD_NAME:     \u001b[0m  {}".format(get_board_name()))
print("\u001b[33m BUILD_PLATFORM: \u001b[0m  {}".format(platform.platform()))
print("\u001b[33m GIT_HEAD:       \u001b[0m  {}".format(get_git_description()))
print("\u001b[32m ------------------------------- \u001b[0m")
print("\u001b[32m Flash configuration \u001b[0m")
print("\u001b[33m --flash-size: \u001b[0m  {}".format(env.BoardConfig().get("upload.flash_size", "4MB")))
print("\u001b[33m --flash-freq: \u001b[0m  {}".format(env.BoardConfig().get("build.f_flash")))
print("\u001b[33m --flash-mode: \u001b[0m  {}".format(env.BoardConfig().get("build.flash_mode")))


env.AddBuildMiddleware(gen_compiletime_defines)
