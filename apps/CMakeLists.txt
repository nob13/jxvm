macro (add_app name)
    message (STATUS " - Adding: ${name}")
    file (GLOB_RECURSE src_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${name}/*.cpp)
    file (GLOB_RECURSE header_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${name}/*.h)
    set (all_files ${src_files} ${header_files})

    add_executable (${name} ${all_files})
    target_link_libraries (${name} ${LIBS})
    install(TARGETS ${name} DESTINATION bin)
endmacro ()

add_app(jxvm)