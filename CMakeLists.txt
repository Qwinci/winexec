cmake_minimum_required(VERSION 3.24)
project(winexec C ASM)

set(CMAKE_C_STANDARD 23)

include_directories(include)
add_compile_options(-fshort-wchar)

set(MAIN_SRC src/main.c)

add_executable(winexec ${MAIN_SRC})
add_executable(winexec32 ${MAIN_SRC})

target_compile_options(winexec32 PRIVATE -m32 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function)
target_compile_options(winexec PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-unused-function)

target_link_options(winexec32 PRIVATE -m32)

target_link_libraries(winexec PRIVATE loaderlib)
target_link_libraries(winexec32 PRIVATE loaderlib_w32)

target_compile_definitions(winexec PRIVATE CONFIG_64BIT)

target_include_directories(winexec PRIVATE src)
target_include_directories(winexec32 PRIVATE src)

add_dependencies(winexec winexec32)

macro(Library name)
    add_library(${name} SHARED main.c ${ARGN})
    add_library(${name}_w32 SHARED main.c ${ARGN})
    target_compile_definitions(${name} PRIVATE CONFIG_64BIT)
    target_compile_options(${name}_w32 PRIVATE -m32 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function)
    target_compile_options(${name} PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-unused-function)
    target_link_options(${name}_w32 PRIVATE -m32)
    target_include_directories(${name} PRIVATE ${PROJECT_SOURCE_DIR}/include/lib ${PROJECT_SOURCE_DIR}/src/printlib)
    target_include_directories(${name}_w32 PRIVATE ${PROJECT_SOURCE_DIR}/include/lib ${PROJECT_SOURCE_DIR}/src/printlib)
    add_dependencies(winexec ${name})
    add_dependencies(winexec32 ${name}_w32)

    add_custom_command(TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${name}> "${CMAKE_BINARY_DIR}")
    add_custom_command(TARGET ${name}_w32 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${name}_w32> "${CMAKE_BINARY_DIR}")
endmacro()

macro(Links name)
    set(extra_args ${ARGN})
    list(TRANSFORM extra_args APPEND _w32)
    target_link_libraries(${name} PRIVATE ${ARGN})
    target_link_libraries(${name}_w32 PRIVATE ${extra_args})
endmacro()

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
add_subdirectory(src)
