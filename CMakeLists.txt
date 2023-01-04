cmake_minimum_required(VERSION 3.0.0)
project(TinyTCPClient VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(./include/)

# TinyTCPClient
if(UNIX)
  add_library(TinyTCPClient STATIC ./src/TinyTCPClient_unix.cpp)
elseif(WIN32)
  add_library(TinyTCPClient STATIC ./src/TinyTCPClient_win.cpp)
  target_link_libraries(TinyTCPClient ws2_32)
endif()

# TinyHTTPClient
if(UNIX)
  file(GLOB_RECURSE SOURCE_CPP ./src/*_unix.cpp)
  file(GLOB_RECURSE SOURCE_C ./src/*_unix.c)  
elseif(WIN32)
  file(GLOB_RECURSE SOURCE_CPP ./src/*_win.cpp)
  file(GLOB_RECURSE SOURCE_C ./src/*_win.c)
endif()
add_library(TinyHTTPClient STATIC ${SOURCE_C} ${SOURCE_CPP})

# test
if(UNIX)
  add_executable(testTinyHTTPClient ./test/test_unix.cpp)
elseif(WIN32)
  add_executable(testTinyHTTPClient ./test/test_win.cpp)
endif()
target_link_libraries(testTinyHTTPClient TinyTCPClient TinyHTTPClient)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
