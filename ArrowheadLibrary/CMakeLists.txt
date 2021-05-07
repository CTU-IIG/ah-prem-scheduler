cmake_minimum_required(VERSION 3.17)
project(ArrowheadLibrary)

set(CMAKE_CXX_STANDARD 14)

SET(BUILD_SHARED_LIBS OFF)

#option(CURL_STATICLIB "Set to ON to build libcurl with static linking."  ON)
find_package(CURL REQUIRED)

find_package(PkgConfig)
pkg_check_modules(JSON_C REQUIRED json-c)

include_directories(${CURL_INCLUDE_DIR} ${JSON_C_INCLUDE_DIRS})

add_library(ArrowheadLibrary STATIC ahlib.cpp ahlib.h)
