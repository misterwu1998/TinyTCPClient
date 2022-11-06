cmake_minimum_required(VERSION 3.0.0)
project(TinyTCPClient VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(./include/)

add_library(TinyTCPClient SHARED ./src/TinyTCPClient.cpp)
target_link_directories(TinyTCPClient PUBLIC ../TinyTCPServer2/build/)
target_link_libraries(TinyTCPClient TinyHTTPServer)

file(GLOB_RECURSE SOURCE_CPP ./src/*.cpp)
file(GLOB_RECURSE SOURCE_C ./src/*.c)
add_library(TinyHTTPClient SHARED ${SOURCE_C} ${SOURCE_CPP})
target_link_directories(TinyTCPClient PUBLIC ../TinyTCPServer2/build/)
target_link_libraries(TinyHTTPClient TinyTCPClient TinyHTTPServer)

add_executable(testTinyTCPClient ./test/test.cpp)
target_link_libraries(testTinyTCPClient TinyTCPClient TinyHTTPClient TinyHTTPServer)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
