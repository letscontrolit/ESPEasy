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

env="memanalyze_ESP8266"

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



try:

    ################### start
    if len(sys.argv) <= 1:
        print("Usage: \n\t%s%s <path_to_objdump>" % sys.argv[0])
        sys.exit(1)

    # e.g.
    # ~/.platformio/packages/toolchain-xtensa/bin/xtensa-lx106-elf-objdump
    # c:/Users/gijs/.platformio/packages/toolchain-xtensa/bin/xtensa-lx106-elf-objdump.exe
    objectDumpBin = sys.argv[1]

    #get list of all plugins
    plugins=glob.glob("src/_[CPN][0-9][0-9][0-9]*.ino")
    plugins.sort()

    #which plugins to test?
    if len(sys.argv)>2:
        plugins=sys.argv[2:]
    else:
        plugins=plugins
    plugins.sort()

    print("Analysing ESPEasy memory usage for env {} ...\n".format(env))

    #### disable all plugins and to get base size
    #for plugin in plugins:
    #    disable_plugin(plugin)


    # for lib in libs:
    #     disable_lib(lib)

    #just build the core without plugins to get base memory usage
    subprocess.check_call("platformio run --silent --environment "+env, shell=True)
    # #two times, sometimes it changes a few bytes somehow
    # SEEMS TO BE NOT USEFULL
    # subprocess.check_call("platformio run --silent --environment dev_4096", shell=True)
    base=analyse_memory(".pio/build/"+env+"/firmware.elf")


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


    ##### test per plugin
    results={}
    for plugin in plugins:
        pluginname=plugin[plugin.find('_'):]
        usesflag="-DUSES{}".format(pluginname[:5])
        my_env = os.environ.copy()
        my_env["PLATFORMIO_BUILD_FLAGS"] = usesflag
        subprocess.check_call("platformio run --silent --environment {}".format(env), shell=True, env=my_env)
        results[plugin]=analyse_memory(".pio/build/"+env+"/firmware.elf")

        print(output_format.format(
            plugin,
            results[plugin]['text']-base['text'],
            results[plugin]['data']-base['data'],
            results[plugin]['rodata']-base['rodata'],
            results[plugin]['bss']-base['bss'],
            results[plugin]['irom0_text']-base['irom0_text'],
        ))

    subprocess.check_call("platformio run --silent --environment "+env, shell=True)
    total=analyse_memory(".pio/build/"+env+"/firmware.elf")

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

    raise


print("\n")
