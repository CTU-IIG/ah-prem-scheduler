cmake_minimum_required(VERSION 3.17)
project(CompilerClient)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

include_directories(${Boost_INCLUDE_DIRS})

include_directories(../ArrowheadLibrary)
link_directories(../ArrowheadLibrary/build)

add_executable(CompilerClient main.cpp)

target_link_libraries(CompilerClient ${Boost_LIBRARIES})
target_link_libraries(CompilerClient "${CMAKE_THREAD_LIBS_INIT}")
target_link_libraries(CompilerClient ArrowheadLibrary)
target_link_libraries(CompilerClient json-c)
target_link_libraries(CompilerClient ${CURL_LIBRARIES})

