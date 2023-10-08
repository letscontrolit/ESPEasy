# Install #

The library can be found, installed, or updated from the Arduino IDE using the official Arduino Library Manager (available from IDE version 1.6.2).


The library can be installed on the system by following the same steps as with other Arduino library.

Refer to [Installing Additional Arduino Libraries](https://www.arduino.cc/en/Guide/Libraries) tutorial for details on how to install a third party library.




# Dependencies #

The AnyRtttl library have no dependencies on other Arduino library. However, some examples of the library requires external dependencies.

The following Arduino Library must be installed on the system to use the library examples:

* [BitReader v1.3.0](https://github.com/end2endzone/BitReader/tree/1.3.0)




# Build #

The library unit tests can be build on Windows/Linux platform to maintain the product stability and level of quality.

This section explains how to compile and build the software and how to get a test environment ready.



## Prerequisites ##



### Software Requirements ###

The following software must be installed on the system before compiling unit test source code:

* [Google C++ Testing Framework v1.8.0](https://github.com/google/googletest/tree/release-1.8.0)
* [RapidAssist v0.5.0](https://github.com/end2endzone/RapidAssist/tree/0.5.0)
* [win32Arduino v2.3.1](https://github.com/end2endzone/win32Arduino/tree/2.3.1)
* [CMake](http://www.cmake.org/) v3.4.3 (or newer)



### Linux Requirements ###

These are the base requirements to build source code:

  * GNU-compatible Make or gmake
  * POSIX-standard shell
  * A C++98-standard-compliant compiler



### Windows Requirements ###

* Microsoft Visual C++ 2010 or newer



## Build steps ##

The AnyRtttl unit test uses the CMake build system to generate a platform-specific build environment. CMake reads the CMakeLists.txt files, checks for installed dependencies and then generates files for the selected build system.

The following steps show how to build the library:

1) Download the source code from an existing [tags](https://github.com/end2endzone/AnyRtttl/tags) and extract the content to a local directory (for example `c:\projects\AnyRtttl` or `~/dev/AnyRtttl`).

2) Open a Command Prompt (Windows) or Terminal (Linux) and browse to the project directory.

3) Enter the following commands to generate the project files for your build system:
```
mkdir build
cd build
cmake ..
```

4) Build the source code.

**Windows**
```
cmake --build . --config Release
```

**Linux**
```
make
```




# Testing #

AnyRtttl comes with unit tests which help maintaining the product stability and level of quality.

Test are build using the Google Test v1.8.0 framework. For more information on how googletest is working, see the [google test documentation primer](https://github.com/google/googletest/blob/release-1.8.0/googletest/docs/V1_6_Primer.md).  

To run tests, open a shell prompt and browse to the `build/bin` folder and run `anyrtttl_unittest` executable. For Windows users, the executable is located in `build\bin\Release`.

Test results are saved in junit format in file `anyrtttl_unittest.release.xml`.

The latest test results are available at the beginning of the [README.md](README.md) file.
