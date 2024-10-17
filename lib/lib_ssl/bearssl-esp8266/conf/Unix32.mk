# Example configuration file for compiling on a Unix-like system with
# GCC, targeting a 32-bit output. Moreover, it enables the "LOMUL" setting
# to make the code select the "small" integer implementations (i15, m15,
# ctmul32...), which is not necessarily a good idea for performance, but
# handy for tests.

include conf/Unix.mk

BUILD = build32
CFLAGS = -W -Wall -Os -fPIC -m32 -DBR_LOMUL -DBR_SLOW_MUL15=1
LDFLAGS = -m32
LDDLLFLAGS = -shared -m32
