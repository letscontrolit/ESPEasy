cmake_minimum_required(VERSION 3.0)
project(ccronexpr)

# Library
add_library(ccronexpr STATIC ccronexpr.c)
target_include_directories(ccronexpr PUBLIC .)

# Supertinycron binary
#add_executable(supertinycron supertinycron.c)
#target_link_libraries(supertinycron ccronexpr)

if (ESP_PLATFORM)
    target_compile_definitions(ccronexpr PRIVATE ESP_PLATFORM=1)
    set(CRON_DISABLE_TESTING ON) # disable tests automatically
endif ()

if (CRON_USE_LOCAL_TIME)
    target_compile_definitions(ccronexpr PUBLIC CRON_USE_LOCAL_TIME=1)
endif ()

if (CRON_COMPILE_AS_CXX)
    target_compile_definitions(ccronexpr PUBLIC CRON_COMPILE_AS_CXX=1)
endif ()

if (MSVC)
    # Strict compilation
    target_compile_options(ccronexpr PRIVATE /W4 /WX)
    # But ignore _s functions
    target_compile_definitions(ccronexpr PRIVATE _CRT_SECURE_NO_WARNINGS)
else ()
    # Strict compilation
    target_compile_options(ccronexpr PRIVATE -ansi -Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wcast-qual -Wconversion -Wno-unused-parameter -pedantic-errors)
endif ()

# Tests
if (NOT CRON_DISABLE_TESTING)
    enable_testing()
    add_subdirectory(test)
endif ()
