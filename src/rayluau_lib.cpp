// Allows Luau to interface with the program.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rayluau_lib.h"
#include "lua.h"
#include "lualib.h"

static int loopRef = -1;

static int lPrint(lua_State* L)
{
    printf("luaPrint: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    return 1;
}

static int lDraw(lua_State* L)
{
    size_t len;
    unsigned char* b = (unsigned char*)lua_tobuffer(L, 1, &len);
    memcpy(pixels, b, len);
    return 1;
}

static int lLoop(lua_State* L)
{
    loopRef = lua_ref(L, -1);
    lua_pop(L, 1);
    printf("Got loopRef: %d\n", loopRef);
    return 1;
}

static int lMultiply(lua_State* L)
{
    lua_pushnumber(L, lua_tonumber(L, 1) * lua_tonumber(L, 2));
    return 1;
}

static const luaL_Reg lib_raylib_luau[] =
{
    {"DrawFramebuffer", &lDraw},
    {"Loop", &lLoop},
    {"Multiply", &lMultiply},
    {"Print", &lPrint},
    {NULL, NULL}
};

// Perform loop function
int luau_raylib_loop(lua_State* L)
{
    if (loopRef < 1) return 0;
    
    // Run on main thread
     lua_State* ML = lua_mainthread(L);
    lua_getref(ML, loopRef);
    lua_call(ML, 0, 0);

    return 1;
}

// Provide raylib-based library to Luau
int openlua_raylib(lua_State* L)
{
    luaL_register(L, "raylib", lib_raylib_luau);
    return 1;
}