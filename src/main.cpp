#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "raylib.h"
#include "rayluau_lib.h"
#include "lua.h"
#include "lualib.h"
#include "luacode.h"
#include "luacodegen.h"
#include "Luau/Require.h"
#include "rayluau_require.h"

// Luau
#define RAYLUAU_SCRIPT_DIR "scripts"
// #define RAYLUAU_SCRIPT_SANDBOX true // Setting to false does not work yet via rayluau_require.cpp

// Raylib
#define FB_SCALE 1
#define GLSL_VERSION 330
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 576
#define WINDOW_TITLE "RayLuau"
unsigned char* pixels = (unsigned char *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 4 * sizeof(unsigned char));
Image FramebufferImage = {pixels, SCREEN_WIDTH/FB_SCALE, SCREEN_HEIGHT/FB_SCALE, 1, 7};
lua_CompileOptions options = {.optimizationLevel = 2, .debugLevel = 0, .typeInfoLevel = 1, .coverageLevel = 0};

// Takes a directory name and sandbox bool and spins up Luau, with execution following immediately afterwards
bool LuauIsInit = false;
lua_State* startLuauRelativeToApp(lua_State* L, RequireContext* ctx)
{
    assert(!LuauIsInit == true);
    LuauIsInit = true;

    puts("Luau: Project init...\n");

    if (luau_codegen_supported()) luau_codegen_create(L);
    luaL_openlibs(L);
    openlua_raylib(L);
    luaopen_require(L, initRequireConfig, ctx);
    luaL_sandbox(L);

    // Luau: New thread
    lua_State *T = lua_newthread(L);
    luaL_sandboxthread(T);

    // Load '/init.luau' file, compile, codegen, run
    std::string ProjectInit = ctx->fullPath() + "/init.luau";
    assert(FileExists(ProjectInit.c_str()));
    int EntryInitModule_Length = GetFileLength(ProjectInit.c_str());
    char *source = (char*)LoadFileData(ProjectInit.c_str(), &EntryInitModule_Length);
    size_t dataSize;
    char *data = luau_compile(source, std::string(source).length(), &options, &dataSize);
    int loadSuccess = luau_load(T, "/init.luau", data, dataSize, 0);
    assert(loadSuccess == 0);
    if (luau_codegen_supported()) luau_codegen_compile(T, -1);
    lua_resume(T, L, 0);

    return T;
}

// Main
int main() {
    puts("Raylib: Init...");
    // Raylib: Init
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    Texture FramebufferTexture = LoadTextureFromImage(FramebufferImage);

    // Luau: Init
    puts("Luau: Init...");
    lua_State* L = luaL_newstate();
    RequireContext ctx(std::string(GetApplicationDirectory()) + RAYLUAU_SCRIPT_DIR, true);
    lua_State *T = startLuauRelativeToApp(L, &ctx);

    // Raylib Loop
    puts("Raylib: Loop");
    while (!WindowShouldClose()) {
        luau_raylib_loop(T);
        UpdateTexture(FramebufferTexture, FramebufferImage.data);
        BeginDrawing();
        DrawTextureEx(FramebufferTexture, {0,0}, 0.0, FB_SCALE, WHITE);
        EndDrawing();
    }

    // Close
    puts("Raylib: Close");
    UnloadImage(FramebufferImage);
    UnloadTexture(FramebufferTexture);
    CloseWindow();
    return 0;
}