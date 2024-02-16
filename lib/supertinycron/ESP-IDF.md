Using library from ESP-IDF
==========================

Note that only local time mode is currently supported on ESP.

Add library as a submodule, but outside components directory:

```shell
git submodule add https://github.com/mdvorak/ccronexpr.git libs/ccronexpr
```

Reference it in the project CMakeLists.txt:
```cmake
set(CRON_USE_LOCAL_TIME ON)
add_subdirectory(libs/ccronexpr)
```

Finally, link it to your component:
```cmake
idf_component_register(...)
target_link_libraries(${COMPONENT_LIB} PUBLIC ccronexpr)
```
