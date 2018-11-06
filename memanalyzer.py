#!/usr/bin/env python
########################################################
#
# ESPEasy plugin memory analyser
# edwin@datux.nl
#
###
#  Based on:
#  https://raw.githubusercontent.com/SmingHub/Sming/develop/tools/memanalyzer.py
#  Memory Analyzer
#  Author: Slavey Karadzhov <slav@attachix.com>
#  Based on https://github.com/Sermus/ESP8266_memory_analyzer
#
########################################################
from collections import OrderedDict
import os.path
import shlex
import subprocess
import sys
import glob
import os

TOTAL_IRAM = 32786;
TOTAL_DRAM = 81920;

env="dev_ESP8266_4096"

sections = OrderedDict([
    ("data", "Initialized Data (RAM)"),
    ("rodata", "ReadOnly Data (RAM)"),
    ("bss", "Uninitialized Data (RAM)"),
    ("text", "Cached Code (IRAM)"),
    ("irom0_text", "Uncached Code (SPI)")
])


def abort(txt):
    raise(Exception("error: "+txt))

def analyse_memory(elfFile):
    command = "%s -t '%s' " % (objectDumpBin, elfFile)
    response = subprocess.check_output(shlex.split(command))
    if isinstance(response, bytes):
        response = response.decode('utf-8')
    lines = response.split('\n')

    # print("{0: >10}|{1: >30}|{2: >12}|{3: >12}|{4: >8}".format("Section", "Description", "Start (hex)", "End (hex)", "Used space"));
    # print("------------------------------------------------------------------------------");
    ret={}
    usedRAM = 0
    usedIRAM = 0

    i = 0
    for (id, descr) in list(sections.items()):
        sectionStartToken = " _%s_start" %  id
        sectionEndToken   = " _%s_end" % id
        sectionStart = -1
        sectionEnd = -1
        for line in lines:
            if sectionStartToken in line:
                data = line.split(' ')
                sectionStart = int(data[0], 16)

            if sectionEndToken in line:
                data = line.split(' ')
                sectionEnd = int(data[0], 16)

            if sectionStart != -1 and sectionEnd != -1:
                break

        sectionLength = sectionEnd - sectionStart
        # if i < 3:
        #     usedRAM += sectionLength
        # if i == 3:
        #     usedIRAM = TOTAL_IRAM - sectionLength;

        ret[id]=sectionLength
        # print("{0: >10}|{1: >30}|{2:12X}|{3:12X}|{4:8}".format(id, descr, sectionStart, sectionEnd, sectionLength))
        # i += 1

    # print("Total Used RAM : %d" % usedRAM)
    # print("Free RAM : %d" % (TOTAL_DRAM - usedRAM))
    # print("Free IRam : %d" % usedIRAM)
    return(ret)

# reenable all plugins and libs
def enable_all():

    for plugin in glob.glob(".src/*"):
        os.rename(plugin, "src/"+os.path.basename(plugin))

    for lib in glob.glob(".lib/*"):
        os.rename(lib, "lib/"+os.path.basename(lib))

def disable_plugin(plugin):
    os.rename(plugin, ".src/"+os.path.basename(plugin))

def enable_plugin(plugin):
    os.rename(".src/"+os.path.basename(plugin), plugin)

def disable_lib(lib):
    os.rename(lib, ".lib/"+os.path.basename(lib))

def enable_lib(lib):
    os.rename(".lib/"+os.path.basename(lib), lib)


try:


    if not os.path.exists(".src"):
        os.mkdir(".src")

    if not os.path.exists(".lib"):
        os.mkdir(".lib")


    ################### start
    if len(sys.argv) < 1:
        print("Usage: \n\t%s%s <path_to_objdump>" % sys.argv[0])
        sys.exit(1)

    objectDumpBin = sys.argv[1]

    enable_all()


    #get list of all plugins
    plugins=glob.glob("src/_[CPN]*.ino")
    plugins.sort()

    #get list of all libs
    libs=glob.glob("lib/*")
    libs.remove("lib/pubsubclient")
    libs.sort()

    #which plugins to test?
    if len(sys.argv)>2:
        plugins=sys.argv[2:]
    else:
        plugins=plugins
    plugins.sort()

    print("Analysing ESPEasy memory usage for env {} ...\n".format(env))

    #### disable all plugins and to get base size
    for plugin in plugins:
        disable_plugin(plugin)


    # for lib in libs:
    #     disable_lib(lib)

    #just build the core without plugins to get base memory usage
    subprocess.check_call("platformio run --silent --environment "+env, shell=True)
    # #two times, sometimes it changes a few bytes somehow
    # SEEMS TO BE NOT USEFULL
    # subprocess.check_call("platformio run --silent --environment dev_4096", shell=True)
    base=analyse_memory(".pioenvs/"+env+"/firmware.elf")


    output_format="{:<30}|{:<11}|{:<11}|{:<11}|{:<11}|{:<11}"
    print(output_format.format(
        "module",
        "cache IRAM",
        "init RAM",
        "r.o. RAM",
        "uninit RAM",
        "Flash ROM",
    ))


    print(output_format.format(
        "CORE",
        base['text'],
        base['data'],
        base['rodata'],
        base['bss'],
        base['irom0_text'],
    ))


    # note: unused libs never use any memory, so dont have to test this
    # ##### test per lib
    # results={}
    # for lib in libs:
    #     enable_lib(lib)
    #     subprocess.check_call("platformio run --silent --environment dev_ESP8266_4096", shell=True)
    #     results[lib]=analyse_memory(".pioenvs/dev_ESP8266_4096/firmware.elf")
    #     disable_lib(lib)
    #
    #     print(output_format.format(
    #         lib,
    #         results[lib]['text']-base['text'],
    #         results[lib]['data']-base['data'],
    #         results[lib]['rodata']-base['rodata'],
    #         results[lib]['bss']-base['bss'],
    #         results[lib]['irom0_text']-base['irom0_text'],
    #     ))





    ##### test per plugin
    results={}
    for plugin in plugins:
        enable_plugin(plugin)
        subprocess.check_call("platformio run --silent --environment "+env, shell=True)
        results[plugin]=analyse_memory(".pioenvs/"+env+"/firmware.elf")
        disable_plugin(plugin)

        print(output_format.format(
            plugin,
            results[plugin]['text']-base['text'],
            results[plugin]['data']-base['data'],
            results[plugin]['rodata']-base['rodata'],
            results[plugin]['bss']-base['bss'],
            results[plugin]['irom0_text']-base['irom0_text'],
        ))



    ##### test with all test_plugins at once
    for plugin in plugins:
        enable_plugin(plugin)

    subprocess.check_call("platformio run --silent --environment "+env, shell=True)
    total=analyse_memory(".pioenvs/"+env+"/firmware.elf")

    print(output_format.format(
        "ALL PLUGINS",
        total['text']-base['text'],
        total['data']-base['data'],
        total['rodata']-base['rodata'],
        total['bss']-base['bss'],
        total['irom0_text']-base['irom0_text'],
    ))

    print(output_format.format(
        "ESPEasy",
        total['text'],
        total['data'],
        total['rodata'],
        total['bss'],
        total['irom0_text'],
    ))

except:
    enable_all()

    raise


enable_all()

print("\n")
