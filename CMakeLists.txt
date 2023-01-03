cmake_minimum_required(VERSION 3.0.0)
project(TinyTCPClient VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(./include/)

if(UNIX)
  add_library(TinyTCPClient STATIC ./src/TinyTCPClient_unix.cpp)
elseif(WIN32)
endif()

file(GLOB_RECURSE SOURCE_CPP ./src/*.cpp)
file(GLOB_RECURSE SOURCE_C ./src/*.c)
add_library(TinyHTTPClient STATIC ${SOURCE_C} ${SOURCE_CPP})

add_executable(testTinyTCPClient ./test/test.cpp)
target_link_libraries(testTinyTCPClient TinyTCPClient TinyHTTPClient)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
