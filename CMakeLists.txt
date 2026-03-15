cmake_minimum_required(VERSION 3.16)

project(RayLuau LANGUAGES C CXX)

# ---- Add External Subdirectories ----
add_subdirectory(extern/raylib)
add_subdirectory(extern/luau)

# ---- Set build output directories ----
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ---- Your executable ----
add_executable(RayLuau src/main.cpp)

# ---- Link libraries ----
target_link_libraries(RayLuau
    raylib
    Luau.Common
    Luau.CLI.lib
    Luau.Ast
    Luau.Compiler
    Luau.Config
    Luau.VM
    Luau.Require
)