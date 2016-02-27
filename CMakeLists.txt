cmake_minimum_required(VERSION 3.3)
# project specific options
project(event_controller)

# build specific options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# from what to build
set(SOURCE_FILES controller.cpp)

# what to build
add_library(controller ${SOURCE_FILES})

# testing
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRECTORIES})

set(TEST_FILE main.cpp)
add_executable(utest ${TEST_FILE})
target_link_libraries(utest controller ${GTEST_BOTH_LIBRARIES})

add_test(MainTest utest)