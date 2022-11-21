
Import("env")
import os
import platform
import shutil
from datetime import date
import datetime
import time
import json


def compute_version_date():
    d0 = date(2022, 8, 18)
    today = date.today()
    delta = today - d0
    return 20200 + delta.days


def create_RFC1123_build_date():
    return datetime.datetime.utcnow().strftime('%a, %d %b %Y %H:%M:%S GMT')


def create_build_unixtime():
    return int( time.time() )


def create_binary_filename():
    today = date.today()
    d1 = today.strftime("%Y%m%d")
    return 'ESP_Easy_mega_{}_{}'.format(d1, env["PIOENV"])


def get_board_name():
    return env.BoardConfig().get("name").replace('"', "")


def get_cdn_url_prefix():
    try:
        from pygit2 import Repository
        import re
        try:
            repo = Repository('.')
            try:
                # Test to see if the current checkout is a tag
                regex = re.compile('^refs/tags/mega-{0}'.format(repo.head.shorthand))
                tags = [r for r in repo.references if regex.match(r)]
                tag = tags[0]
                tag = tag.replace('refs/tags/','@')
                return "https://cdn.jsdelivr.net/gh/letscontrolit/ESPEasy{0}/static/".format(tag)
            except:
                # Not currently on a tag, thus use the last tag.
                regex = re.compile('^refs/tags/mega')
                tags = [r for r in repo.references if regex.match(r)]
                tags.sort()
                tags.reverse()
                tag = tags[0]
                # work-around to allow users to use files not yet available on a tagged version
                if '20220809' in tag:
                    return 'https://cdn.jsdelivr.net/gh/letscontrolit/ESPEasy/static/'
                    
                tag = tag.replace('refs/tags/','@')
                return "https://cdn.jsdelivr.net/gh/letscontrolit/ESPEasy{0}/static/".format(tag)
        except:
            return 'https://cdn.jsdelivr.net/gh/letscontrolit/ESPEasy/static/'
    except ImportError:
        return 'https://cdn.jsdelivr.net/gh/letscontrolit/ESPEasy/static/'


def get_git_description():
    try:
        from pygit2 import Repository
        try:
            repo = Repository('.')
            return "{0}_{1}".format(repo.head.shorthand, repo.revparse_single('HEAD').short_id)
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
        + [("SET_BUILD_BINARY_FILENAME", '\\"%s\\"' % create_binary_filename())]
        + [("SET_BOARD_NAME", '\\"%s\\"' % get_board_name())]
        + [("SET_BUILD_PLATFORM", '\\"%s\\"' % platform.platform())]
        + [("SET_BUILD_GIT_HEAD", '\\"%s\\"' % get_git_description())]
        + [("SET_BUILD_CDN_URL",  '\\"%s\\"' % get_cdn_url_prefix())]
        + [("SET_BUILD_VERSION", compute_version_date())]
        + [("SET_BUILD_UNIXTIME", create_build_unixtime())]
        + [("SET_BUILD_TIME_RFC1123", '\\"%s\\"' % create_RFC1123_build_date())],
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
print("\u001b[33m CDN_URL:        \u001b[0m  {}".format(get_cdn_url_prefix()))
print("\u001b[33m BUILD_VERSION:  \u001b[0m  {}".format(compute_version_date()))
print("\u001b[33m BUILD_UNIXTIME: \u001b[0m  {}".format(create_build_unixtime()))
print("\u001b[33m BUILD_RFC1123:  \u001b[0m  {}".format(create_RFC1123_build_date()))

print("\u001b[32m ------------------------------- \u001b[0m")
print("\u001b[32m Flash configuration \u001b[0m")
print("\u001b[33m --flash-size: \u001b[0m  {}".format(env.BoardConfig().get("upload.flash_size", "4MB")))
print("\u001b[33m --flash-freq: \u001b[0m  {}".format(env.BoardConfig().get("build.f_flash")))
print("\u001b[33m --flash-mode: \u001b[0m  {}".format(env.BoardConfig().get("build.flash_mode")))
if "esp32" in env.BoardConfig().get("build.core"):
  print("\u001b[33m  memory_type: \u001b[0m  {}".format(env.BoardConfig().get("build.arduino.memory_type", "-")))


env.AddBuildMiddleware(gen_compiletime_defines)