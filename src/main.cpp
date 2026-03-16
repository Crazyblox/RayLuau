#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "rayluau_lib.h"
#include "lua.h"
#include "lualib.h"
#include "luacode.h"
#include "luacodegen.h"

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
    lua_State* L = luaL_newstate();                     // Start-up Luau
    if (luau_codegen_supported()) {                     // Enable codegen if available
        luau_codegen_create(L);                         //
    }                                                   //
    luaL_openlibs(L);                                   // Register standard libraries to Luau
    openlua_raylib(L);                                  // Register 'luau_raylib' library to Luau
    luaL_sandbox(L);                                    // Sandbox the global thread
    lua_State* T = lua_newthread(L);                    // Create a new thread within the global thread 
    luaL_sandboxthread(T);                              // Sandbox the newly-created thread

    // Luau: Load Bytecode File
    const char *fileName = "script.luau";                                               // Set filename of script
    const char *filePath = TextFormat("%s%s", GetApplicationDirectory(), fileName);   // Set path to load given .luau file 
    int fileLength = GetFileLength(filePath);                                           // Get file length of .luau source
    char *source = (char *)LoadFileData(filePath, &fileLength);                         // Load .luau source code
    size_t sourceSize = (size_t)fileLength;                                             // Cast into correct type for Luau compiler

    // Luau: Compile into bytecode
    lua_CompileOptions options = {2, 0, 1, 0};
    size_t dataSize;
    char *data = luau_compile(source, sourceSize, &options, &dataSize);

    // Luau: Load bytecode
    int res = luau_load(T, "=test", data, dataSize, 0);         // Load the bytecode file into the Luau VM
    if (res != 0) {printf("Failed to load :("); return 0;}      // Exit if Luau couldn't load the bytecode
    if (luau_codegen_supported()) {                             // Compile to native code if supported!
        luau_codegen_compile(T, -1);                            //
    }                                                           //
    free(source);                                               // Free .luau source data
    lua_resume(T, L, 0);                                        // Run the created thread within Luau 

    // Loop
    while (!WindowShouldClose()) {                                  // Loop persists for the duration of the program.
        luau_raylib_loop(T);                                        // Push reference index value to Luau's stack.
        UpdateTexture(FramebufferTexture, FramebufferImage.data);   // Copy image data to texture (Raylib only allows this through Image -> Texture!) 
        BeginDrawing();                                             // Enter drawing mode
        DrawTexture(FramebufferTexture, 0, 0, WHITE);               // Draw texture to screen
        DrawText("Luau CPU draw -> Raylib :D", 0, 0, 20, WHITE);    // Draw text above all prior drawings
        EndDrawing();                                               // Finish drawing
    }
    // Close
    lua_close(L);                           // Close global Luau state; this closes all Luau threads created from this state too.
    free(data);                             // Luau: Free compiled code; might not be needed due to above 'lua_close'...?
    UnloadImage(FramebufferImage);          // Raylib: Unload Image
    UnloadTexture(FramebufferTexture);      // Raylib: Unload Texture
    GetCharPressed();                       // Raylib: I don't know why this is here
    CloseWindow();                          // Raylib: Incase the program wasn't closed via the window itself.
    return 0;                               // Program exits without issue
}