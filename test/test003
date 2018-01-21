#!/usr/bin/env python3

from esptest import *

# hardware requirements:
# - node 0
# - framed oled display connected to default I2C

# tests:
# - only configures it, nothing is verfied yet

espeasy[0].post_device(1, data="""
        TDNUM:36
        TDN:
        TDE:on
        plugin_036_adr:60
        plugin_036_rotate:1
        plugin_036_nlines:1
        plugin_036_scroll:1
        Plugin_036_template1:espeasy
        Plugin_036_template2:test
        Plugin_036_template3:suite
        Plugin_036_template4:test1
        Plugin_036_template5:test2
        Plugin_036_template6:test3
        Plugin_036_template7:test4
        Plugin_036_template8:test5
        Plugin_036_template9:test6
        Plugin_036_template10:test7
        Plugin_036_template11:test8
        Plugin_036_template12:test9
        taskdevicepin3:-1
        plugin_036_timer:0
        TDT:1
        edit:1
        page:1
""")


if __name__=='__main__':
    completed()
