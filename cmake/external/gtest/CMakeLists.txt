set (GTEST_DIRECTORY ${CMAKE_SOURCE_DIR}/dependencies_source/googletest-release-1.7.0)

message (STATUS "GTest")
set (GTEST_INCLUDE_DIRS "${GTEST_DIRECTORY}/include")
set (GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} PARENT_SCOPE)

message(STATUS "GtestInclude ${GTEST_INCLUDE_DIRS}")

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${GTEST_DIRECTORY}) # only needed for compilation here

add_library(gtest ${GTEST_DIRECTORY}/src/gtest-all.cc)
add_library(gtest_main ${GTEST_DIRECTORY}/src/gtest_main.cc)