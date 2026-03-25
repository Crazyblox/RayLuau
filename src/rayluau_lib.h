// bindings.h - Header/API for RayLuau library
// Makes extensive use of Luau's own library
#pragma once
#include "raylib.h"
#include "lua.h"
#include "lualib.h"
extern unsigned char* pixels;
int luau_raylib_loop(lua_State* L);
int openlua_raylib(lua_State* L);