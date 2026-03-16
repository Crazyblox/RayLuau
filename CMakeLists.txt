cmake_minimum_required(VERSION 3.16)

# Set output directories for library and archive files
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Config Luau to C, disable compiling unused libraries 
set(LUAU_EXTERN_C ON CACHE BOOL "Use extern C for all APIs" FORCE)
set(LUAU_BUILD_CLI OFF CACHE BOOL "Build CLI" FORCE)
set(LUAU_BUILD_TESTS OFF CACHE BOOL "Build tests" FORCE)

# Make project, add executable
project(RayLuau LANGUAGES C CXX)
add_executable(RayLuau src/main.cpp)
set_target_properties(RayLuau PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add Sub-directories
add_subdirectory(extern/raylib)
add_subdirectory(extern/luau)

# Link libraries
target_link_libraries(RayLuau
    raylib
    Luau.Common
    Luau.CLI.lib
    Luau.Ast
    Luau.Compiler
    Luau.Config
    Luau.CodeGen
    Luau.VM
    Luau.Require
)