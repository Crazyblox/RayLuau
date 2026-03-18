#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "rayluau_lib.h"
#include "lua.h"
#include "lualib.h"
#include "luacode.h"
#include "luacodegen.h"
#include "Luau/Require.h"
#include "rayluau_require.h"

#define GLSL_VERSION 330
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 576
#define WINDOW_TITLE "RayLuau"

unsigned char* pixels = (unsigned char *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 4 * sizeof(unsigned char));
Image FramebufferImage = {pixels, SCREEN_WIDTH, SCREEN_HEIGHT, 1, 7};

// Main
int main() {
    // Raylib: Init
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    Texture FramebufferTexture = LoadTextureFromImage(FramebufferImage);
    // Luau: Init
    lua_State* L = luaL_newstate();
    if (luau_codegen_supported())
        luau_codegen_create(L);
    luaL_openlibs(L);
    openlua_raylib(L);
    const char *sDir = TextFormat("%s%s", GetApplicationDirectory(), "scripts");
    int sDir_B = strlen(sDir) + 1;
    RequireContext ctx(sDir, "");
    luaopen_require(L, initRequireConfig, (void *)&ctx);
    luaL_sandbox(L);
    lua_State* T = lua_newthread(L);
    luaL_sandboxthread(T);

    // Load 1st script file (/init.luau)
    const char *fileName = "/init.luau";
    const char *filePath = TextFormat("%s%s", sDir, fileName);
    int fileLength = GetFileLength(filePath);
    char *source = (char*)LoadFileData(filePath, &fileLength);
    size_t sourceSize = (size_t)fileLength;

    // Compile, load, codegen
    lua_CompileOptions options = {.optimizationLevel = 2, .debugLevel = 0, .typeInfoLevel = 1, .coverageLevel = 0};
    size_t dataSize;
    char *data = luau_compile(source, sourceSize, &options, &dataSize);
    int res = luau_load(T, "/init.luau", data, dataSize, 0);
    if (res != 0) {
        printf("Failed to load :("); return 0;
    }
    if (luau_codegen_supported())
        luau_codegen_compile(T, -1);
    free(source);
    lua_resume(T, L, 0);

    // Raylib Loop
    while (!WindowShouldClose()) {
        luau_raylib_loop(T);
        UpdateTexture(FramebufferTexture, FramebufferImage.data);
        BeginDrawing();
        DrawTextureEx(FramebufferTexture, {0,0}, 0.0, 1.0, WHITE);
        EndDrawing();
    }

    // Close
    lua_close(L);
    free(data);
    UnloadImage(FramebufferImage);
    UnloadTexture(FramebufferTexture);
    GetCharPressed();
    CloseWindow();
    return 0;
}