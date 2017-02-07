cmake_minimum_required (VERSION 2.8.11)
project (HPCN_Network_Library)
# The version number.
execute_process(
    COMMAND git describe --always --tags
    OUTPUT_VARIABLE netlib_VERSION)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# C_STANDARD
set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 11)

include_directories("include")
add_definitions(-Wall -Wextra -Werror -O3 -g -pipe -flto -ffat-lto-objects)

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -O3 -g -pipe")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto -O3 -g -pipe")

#Function to make output a bit more quiet
function(message)
    if (NOT MESSAGE_QUIET)
        _message(${ARGN})
    endif()
endfunction()

#Function to remove imported executables
function(add_executable)
    if (NOT NO_COMPILE_EXECUTABLES)
        _add_executable(${ARGN})
    else()
        _add_executable(${ARGN})
        set_target_properties(${ARGV0} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
    endif()
endfunction()

#Function to remove custom targets
function(add_custom_target)
    if (NOT NO_ADD_CUSTOM)
        _add_custom_target(${ARGN})
    else()
    endif()
endfunction()

function(INSTALL)
    if (NOT NO_COMPILE_EXECUTABLES)
        _INSTALL(${ARGN})
    else()
    endif()
endfunction()

#Import all .hpp
file(GLOB lib_INCLUDE
    "include/*.h"
    "include/*.hpp"
)

#Import all .cpp
file(GLOB lib_SRC
    "src/*.c"
    "src/*.cpp"
)

include(Config.cmake)

#The library objects

# this is the "object library" target: compiles the sources only once
add_library(netlib_obj OBJECT ${lib_SRC} ${lib_INCLUDE})
# shared libraries need PIC
set_property(TARGET ${hptl_obj} PROPERTY POSITION_INDEPENDENT_CODE 1)
# actual shared and static libraries built from the same object files
add_library(netlib_shared SHARED $<TARGET_OBJECTS:netlib_obj>)
add_library(netlib_static STATIC $<TARGET_OBJECTS:netlib_obj>)

#Library Dependencies
target_link_libraries(netlib_static rt pthread ssl crypto)
target_link_libraries(netlib_static rt pthread ssl crypto)

#Tests
add_executable(cli src/examples/cli)
add_executable(srv src/examples/srv)

#Link library executables
target_link_libraries(cli netlib_static)
target_link_libraries(srv netlib_static)
