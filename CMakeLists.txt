cmake_minimum_required(VERSION 3.0)
# project specific options
project(event_controller CXX)

# build specific options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

# from what to build
set(SOURCE_FILES controller.cpp)

# what to build
add_library(${PROJECT_NAME} ${SOURCE_FILES})

# testing
add_subdirectory(googletest/googletest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(GTEST_LIBRARIES gtest gtest_main)

set(TEST_FILE test.cpp)
add_executable(first_test ${TEST_FILE})
target_link_libraries(first_test ${PROJECT_NAME} ${GTEST_LIBRARIES})

add_test(MainTest first_test)