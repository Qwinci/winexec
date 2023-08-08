set(SRC main.c entry.c exports.c imports.c mapper.c relocs.c tls.c loader.c)

add_library(loaderlib SHARED ${SRC})
add_library(loaderlib_w32 SHARED ${SRC})
target_compile_definitions(loaderlib PRIVATE CONFIG_64BIT)
target_compile_options(loaderlib_w32 PRIVATE -m32)
target_link_options(loaderlib_w32 PRIVATE -m32)
target_include_directories(loaderlib PRIVATE ${PROJECT_SOURCE_DIR}/include/lib)
target_include_directories(loaderlib_w32 PRIVATE ${PROJECT_SOURCE_DIR}/include/lib)

add_custom_command(TARGET loaderlib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:loaderlib> "${CMAKE_BINARY_DIR}")
add_custom_command(TARGET loaderlib_w32 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:loaderlib_w32> "${CMAKE_BINARY_DIR}")