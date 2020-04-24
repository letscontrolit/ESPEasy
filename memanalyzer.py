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

env="spec_memanalyze_ESP8266"

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
        print(" e.g.")
        print("  ~/.platformio/packages/toolchain-xtensa/bin/xtensa-lx106-elf-objdump")
        print("  c:/Users/gijs/.platformio/packages/toolchain-xtensa/bin/xtensa-lx106-elf-objdump.exe")

        sys.exit(1)

    # e.g.
    # ~/.platformio/packages/toolchain-xtensa/bin/xtensa-lx106-elf-objdump
    # c:/Users/gijs/.platformio/packages/toolchain-xtensa/bin/xtensa-lx106-elf-objdump.exe
    objectDumpBin = sys.argv[1]

    #get list of all plugins
    #which plugins to test?


    tmpplugins = []
    plugins = []
    pluginnames = {}
    plugins.append('CORE_ONLY')
    if len(sys.argv)>2:
        tmpplugins=sys.argv[2:]
    else:
        tmpplugins=glob.glob("src/_[CPN][0-9][0-9][0-9]*.ino")
    tmpplugins.sort()

    for plugin in tmpplugins:
        pluginname=plugin[plugin.find('_'):]
        buildflag= "USES{}".format(pluginname[:5])
        pluginnames[buildflag] = plugin
        plugins.append(buildflag)

    plugins.append('MQTT_ONLY')
    plugins.append('USE_SETTINGS_ARCHIVE')
    plugins.append('WEBSERVER_RULES_DEBUG=1')
    plugins.append('WEBSERVER_TIMINGSTATS')
    plugins.append('WEBSERVER_NEW_UI')

    

    print("Analysing ESPEasy memory usage for env {} ...\n".format(env))

    output_format="{:<30}|{:<11}|{:<11}|{:<11}|{:<11}|{:<11}"
    print(output_format.format(
        "module",
        "cache IRAM",
        "init RAM",
        "r.o. RAM",
        "uninit RAM",
        "Flash ROM",
    ))


    ##### test per plugin
    results={}
    base = {}
    for plugin in plugins:
        buildflag= "-D{}".format(plugin)
        my_env = os.environ.copy()
        my_env["PLATFORMIO_BUILD_FLAGS"] = buildflag
        subprocess.check_call("platformio run --silent --environment {}".format(env), shell=True, env=my_env)
        res = analyse_memory(".pio/build/"+env+"/firmware.elf")
        if plugin == 'CORE_ONLY':
            base = res
            print(output_format.format(
                "CORE",
                base['text'],
                base['data'],
                base['rodata'],
                base['bss'],
                base['irom0_text'],
            ))
        else:
            results[plugin] = res
            name = plugin
            if plugin in pluginnames:
                name = pluginnames[plugin]
            print(output_format.format(
                name,
                results[plugin]['text']-base['text'],
                results[plugin]['data']-base['data'],
                results[plugin]['rodata']-base['rodata'],
                results[plugin]['bss']-base['bss'],
                results[plugin]['irom0_text']-base['irom0_text'],
        ))

except:

    raise


print("\n")
