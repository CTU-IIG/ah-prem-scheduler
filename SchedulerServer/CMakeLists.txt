cmake_minimum_required(VERSION 3.17)
project(SchedulerServer)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

include_directories(../ArrowheadLibrary)
link_directories(../ArrowheadLibrary/build)

add_executable(SchedulerServer main.cpp)

target_link_libraries(SchedulerServer ${Boost_LIBRARIES})
target_link_libraries(SchedulerServer "${CMAKE_THREAD_LIBS_INIT}")
target_link_libraries(SchedulerServer ArrowheadLibrary)
target_link_libraries(SchedulerServer ${CURL_LIBRARIES})
target_link_libraries(SchedulerServer json-c)
