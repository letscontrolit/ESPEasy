cmake_minimum_required(VERSION 3.4.3)
project(AnyRtttl)

# Set the output folder where your program will be created
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(   LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

##############################################################################################################################################
# Functions
##############################################################################################################################################
function(GIT_EXTERNAL DIR REPO_URL TAG)
  # Find the name of the repository
  get_filename_component(REPO_NAME ${REPO_URL} NAME_WE)
  
  # Compute output directory
  set(REPO_DIR "${DIR}/${REPO_NAME}")
  
  if (EXISTS "${REPO_DIR}")
    message(STATUS "Repository ${REPO_NAME} already exists in directory ${DIR}. Skipping git clone command.")
    return()
  endif()

  # Clone the repository
  message(STATUS "git clone ${REPO_URL} ${REPO_DIR}")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" clone ${REPO_URL} ${REPO_DIR}
    RESULT_VARIABLE returncode ERROR_VARIABLE error
    WORKING_DIRECTORY "${DIR}")
  if(returncode)
    message(FATAL_ERROR "Clone failed: ${error}\n")
  endif()
  message(STATUS "git clone completed")
  
  # Checking out the required tag
  message(STATUS "git checkout ${TAG}")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" checkout ${TAG}
    RESULT_VARIABLE returncode ERROR_VARIABLE error
    WORKING_DIRECTORY "${REPO_DIR}")
  if(returncode)
    message(FATAL_ERROR "Checkout failed: ${error}\n")
  endif()
  message(STATUS "git checkout completed")
  
  # Delete the .git folder to simulate an export of the repository
  message(STATUS "Deleting ${REPO_DIR}/.git")
  file(REMOVE_RECURSE "${REPO_DIR}/.git")
  
endfunction()

function(add_example name)
  # Create custom example.cpp file which includes the ino sketch file.
  SET(SOURCE_INO_FILE "${PROJECT_SOURCE_DIR}/examples/${name}/${name}.ino")
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/examples.cpp.in" "${PROJECT_BINARY_DIR}/${name}/examples.cpp")
 
  add_executable(${name}
    ${ARDUINO_LIBRARY_SOURCE_FILES}
    ${SOURCE_INO_FILE}
    "${PROJECT_BINARY_DIR}/${name}/examples.cpp"
  )
 
  target_include_directories(${name} PRIVATE ${PROJECT_SOURCE_DIR}/src ${BITREADER_SOURCE_DIR} win32arduino )
  target_link_libraries(${name} PRIVATE win32arduino rapidassist)
 
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  set_target_properties(${name} PROPERTIES FOLDER "examples")

  if(WIN32)
    # 1>C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include\ostream(743,1): warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
    # 1>D:\dev\AnyRtttl\master\third_parties\win32Arduino\install\include\win32arduino-2.4.0\SerialPrinter.h(202): message : see reference to function template instantiation 'std::basic_ostream<char,std::char_traits<char>> &std::operator <<<std::char_traits<char>>(std::basic_ostream<char,std::char_traits<char>> &,const char *)' being compiled
    set_target_properties(${name} PROPERTIES COMPILE_FLAGS "/wd4530")
  endif()
 
endfunction()

##############################################################################################################################################
# Dependencies
##############################################################################################################################################
find_package(GTest REQUIRED)
find_package(rapidassist 0.5.0 REQUIRED)
find_package(win32arduino 2.3.1 REQUIRED)
find_package(Git REQUIRED)

# Arduino BitReader library dependency
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external")
GIT_EXTERNAL("${CMAKE_CURRENT_SOURCE_DIR}/external" "http://github.com/end2endzone/BitReader.git" "1.3.0")
set(BITREADER_SOURCE_DIR "${PROJECT_SOURCE_DIR}/external/BitReader/src")

##############################################################################################################################################
# Project settings
##############################################################################################################################################

# Build options
option(ANYRTTTL_BUILD_EXAMPLES "Build all example projects" OFF)

# Prevents annoying warnings on MSVC
if (WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Find all library source and unit test files
file( GLOB ARDUINO_LIBRARY_SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp   ${PROJECT_SOURCE_DIR}/src/*.h   ${BITREADER_SOURCE_DIR}/*.cpp  ${BITREADER_SOURCE_DIR}/*.h)
file( GLOB ARDUINO_LIBRARY_TEST_FILES   ${PROJECT_SOURCE_DIR}/test/*.cpp  ${PROJECT_SOURCE_DIR}/test/*.h )

# Create unit test executable
add_executable(anyrtttl_unittest
  ${ARDUINO_LIBRARY_SOURCE_FILES}
  ${ARDUINO_LIBRARY_TEST_FILES}
)

#include directories
target_include_directories(anyrtttl_unittest
  PRIVATE ${PROJECT_SOURCE_DIR}/src       # Arduino Library folder
  ${GTEST_INCLUDE_DIR}
  ${BITREADER_SOURCE_DIR}
  win32arduino
)

# Unit test projects requires to link with pthread if also linking with gtest
if(NOT WIN32)
  set(PTHREAD_LIBRARIES -pthread)
endif()

#link libraries
target_link_libraries(anyrtttl_unittest PRIVATE win32arduino rapidassist ${PTHREAD_LIBRARIES} ${GTEST_LIBRARIES} )

if(WIN32)
  # 1>C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include\ostream(743,1): warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
  # 1>D:\dev\AnyRtttl\master\third_parties\win32Arduino\install\include\win32arduino-2.4.0\SerialPrinter.h(202): message : see reference to function template instantiation 'std::basic_ostream<char,std::char_traits<char>> &std::operator <<<std::char_traits<char>>(std::basic_ostream<char,std::char_traits<char>> &,const char *)' being compiled
  set_target_properties(anyrtttl_unittest PROPERTIES COMPILE_FLAGS "/wd4530")
endif()

# Copy `expected_call_stack.log` to expected locations
configure_file(${PROJECT_SOURCE_DIR}/test/expected_call_stack.log ${PROJECT_BINARY_DIR}/expected_call_stack.log COPYONLY)
set(TEST_FILES_BINARY_DIR ${EXECUTABLE_OUTPUT_PATH})
if(WIN32)
  set(TEST_FILES_BINARY_DIR ${TEST_FILES_BINARY_DIR}/${CMAKE_CFG_INTDIR})
endif()
add_custom_command(
        TARGET anyrtttl_unittest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${PROJECT_SOURCE_DIR}/test/expected_call_stack.log
                ${TEST_FILES_BINARY_DIR}/expected_call_stack.log)

##############################################################################################################################################
# Add all samples to the project unless the user has specified otherwise.
##############################################################################################################################################
if(ANYRTTTL_BUILD_EXAMPLES)
  add_example("Basic")
  add_example("BlockingProgramMemoryRtttl")
  add_example("BlockingRtttl")
  add_example("BlockingWithNonBlocking")
  add_example("NonBlockingProgramMemoryRtttl")
  add_example("NonBlockingRtttl")
  add_example("NonBlockingStopBeforeEnd")
  add_example("Play10Bits")
  add_example("Play16Bits")
  add_example("Rtttl2Code")
endif()
 