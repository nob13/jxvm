file (GLOB_RECURSE src_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file (GLOB_RECURSE header_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
set (TEST_FILES ${src_files} ${header_files})

include_directories(${GTEST_INCLUDE_DIRS})
add_executable(jx_test ${TEST_FILES})
target_link_libraries(jx_test ${LIBS} gtest)

install(TARGETS jx_test DESTINATION bin)