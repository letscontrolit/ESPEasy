CC = musl-gcc
VERSION := $(shell git rev-parse --short HEAD)-$(shell date +%Y%m%d%H%M%S)
UNCOMMITTED_CHANGES := $(shell git status --porcelain)
ifeq ($(strip $(UNCOMMITTED_CHANGES)),)
    VERSION := $(VERSION)
else
    VERSION := $(VERSION)-dev-build
endif
CFLAGS = -Wextra -std=c89 -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -Wl,-z,norelro -Wno-unused-parameter -static -DVERSION=\"$(VERSION)\" -DCRON_USE_LOCAL_TIME
CFLAGS = -Wextra -std=c89 -s -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -Wl,-z,norelro -static -DVERSION=\"$(VERSION)\" -DCRON_USE_LOCAL_TIME -fPIC
SOURCES = supertinycron.c ccronexpr.c ccronexpr_test.c
OBJECTS = supertinycron.o ccronexpr.o
OBJECTS_TEST = ccronexpr_test.o ccronexpr.o
OBJECTS_SHARED = ccronexpr.o
EXECUTABLE = supertinycron
EXECUTABLE_TEST = ccronexpr_test
SHARED = libccronexpr.so
LDFLAGS = -shared -Wl,-soname,$(SHARED) -fPIC

all: $(EXECUTABLE) $(EXECUTABLE_TEST)

shared: $(SHARED)

$(SHARED): $(OBJECTS_SHARED)
	$(CC) $(LDFLAGS) $^ -o $@

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@
	upx -9 $(EXECUTABLE)

$(EXECUTABLE_TEST): $(OBJECTS_TEST)
	$(CC) $(CFLAGS) $(OBJECTS_TEST) -o $@

clean:
	rm -f $(OBJECTS) $(OBJECTS_TEST) $(EXECUTABLE) $(EXECUTABLE_TEST) $(SHARED)
