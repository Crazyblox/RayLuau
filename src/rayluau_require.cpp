#include <cstring>
#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "luacode.h"
#include "raylib.h"
#include "lua.h"
#include "lualib.h"
#include "luacode.h"
#include "luacodegen.h"
#include "Luau/Require.h"
#include "rayluau_require.h"

void initRequireConfig(luarequire_Configuration* config)
{
    // 1st thing Luau checks via the requirer's chunk name before proceeding to require.
    config->is_require_allowed = [](lua_State*, void* ctx, const char* requirer_chName)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            curCtx->setPath_c(requirer_chName);
            return true;
        };

    // Resets the internal state to point at the requirer module.
    config->reset = [](lua_State*, void* ctx, const char* requirer_chName)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            curCtx->setPath_c(requirer_chName);
            return NAVIGATE_SUCCESS;
        };

    // Resets the internal state to point at an aliased module, given its exact
    // path from a configuration file. This function is only called when an
    // alias's path cannot be resolved relative to its configuration file.
    config->jump_to_alias = [](lua_State*, void* ctx, const char* path)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            printf("\nconfig->jump_to_alias - In - curCtx: Root - %s, Path - %s\n", curCtx->getRoot_c(), curCtx->getPath_c());
            printf("Provided path: %s\n", path);
            printf("Returning NAVIGATE_NOT_FOUND, as this is not implemented.\n");
            // To be implemented
            return NAVIGATE_NOT_FOUND; // NAVIGATE_AMBIGUOUS, NAVIGATE_SUCCESS
        };

    // Intercepts a given alias (like '@self') before looking for alias definition via configuration files.
    config->to_alias_override = [](lua_State* L, void* ctx, const char* alias_unprefixed) 
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            if (std::string(alias_unprefixed).compare(std::string("self")) == 0) {
                std::string upPath = curCtx->trimPath();
                printf("Up path: %s\n", upPath.c_str());
                if (upPath.compare(curCtx->getPath()) == 0) {
                    printf("We didn't budge! We must be at the boundary...\n");
                    return NAVIGATE_NOT_FOUND;
                }
                curCtx->setPath(upPath);
                return NAVIGATE_SUCCESS;
            }
            // To be implemented
            printf("Match not found...\n");
            return NAVIGATE_NOT_FOUND; // NAVIGATE_AMBIGUOUS, NAVIGATE_SUCCESS
        };

    // Luau resorts to this function if no config files have info on the given alias.
    config->to_alias_fallback = [](lua_State* L, void* ctx, const char* alias_unprefixed)
        {
            // To be implemented
            return NAVIGATE_NOT_FOUND; // NAVIGATE_AMBIGUOUS, NAVIGATE_SUCCESS
        };

    // We up
    config->to_parent = [](lua_State* L, void* ctx)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            printf("\nconfig->to_parent - In - curCtx: Root - %s, Path - %s\n", curCtx->getRoot_c(), curCtx->getPath_c());
            std::string upPath = curCtx->trimPath();
            printf("Up path: %s\n", upPath.c_str());
            if (upPath.compare(curCtx->getPath()) == 0) {
                printf("We didn't budge! We must be at the boundary...");
                return NAVIGATE_NOT_FOUND;
            }

            if (DirectoryExists((curCtx->getRoot() + upPath).c_str())) {
                curCtx->setPath(upPath);
                printf("config->to_parent - Out - curCtx: %s\n", curCtx->fullPath().c_str());
                return NAVIGATE_SUCCESS;
            }
            // We land here if the directory doesn't exist
            printf("config->to_parent - Out - curCtx: Root - %s, Path - %s\n", curCtx->getRoot_c(), curCtx->getPath_c());
            return NAVIGATE_NOT_FOUND;
        };

    // We down
    config->to_child = [](lua_State*, void* ctx, const char* name)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            printf("\nIN_: config->to_child: %s, %s\n", curCtx->fullPath().c_str(), name);
            std::string searchPath = curCtx->trimPath();
            searchPath += std::string(name);
            bool dirExists = DirectoryExists((curCtx->getRoot() + searchPath).c_str());
            bool luauExists = FileExists((curCtx->getRoot() + searchPath + std::string(".luau")).c_str());
            if (dirExists || luauExists) {
                if (dirExists)
                    searchPath += std::string("/");
                curCtx->setPath(searchPath);
                printf("YES: config->to_child: %s\n", curCtx->fullPath().c_str());
                return NAVIGATE_SUCCESS;
            }
            printf("NO_: config->to_child: %s\n", curCtx->fullPath().c_str());
            return NAVIGATE_NOT_FOUND;
        };
    
    // Validate if `.luau` or `/init.luau` is present.
    config->is_module_present = [](lua_State*, void* ctx)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            printf("\nIN_: config->is_module_present: %s\n", curCtx->fullPath().c_str());
            std::string module = (curCtx->fullPath() + std::string(".luau"));
            std::string path_module_init = (curCtx->fullPath() + std::string("init.luau"));
            if (FileExists(module.c_str()) || FileExists(path_module_init.c_str())) {
                printf("Module is present\n"); return true;
            }
            printf("Module is not present\n"); return false;
        };

    // Called when 'is_module_present' returns true; also available via Luau's debug library.
    config->get_chunkname = [](lua_State*, void* ctx, char* buffer, size_t bufferSize, size_t* outSize)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            printf("\nconfig->get_chunkname - curCtx: Root - %s, Path - %s\n", curCtx->getRoot_c(), curCtx->getPath_c());
            std::string chName = curCtx->getRoot() + curCtx->getPath();
            size_t chName_size = chName.length();
            if (bufferSize < chName_size) {
                printf("WRITE_BUFFER_TOO_SMALL\n");
                return WRITE_BUFFER_TOO_SMALL; // WRITE_FAILURE   
            }
            memcpy(buffer, chName.c_str(), chName_size);
            outSize = &chName_size;
            printf("WRITE_SUCCESS\n");
            return WRITE_SUCCESS;
        };

    // Provides a loadname that identifies the current module and is passed to
    // load. This function is only called if is_module_present returns true.
    config->get_loadname = config->get_chunkname;
    // Provides a cache key representing the current module. This function is
    // only called if is_module_present returns true.
    config->get_cache_key = config->get_chunkname;

    // Returns whether a configuration file is present in the current context,
    // and if so, its syntax. If not present, require-by-string will call
    // to_parent until either a configuration file is present or
    // NAVIGATE_FAILURE is returned (at root).
    config->get_config_status = [](lua_State*, void* ctx)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            printf("\nconfig->get_config_status - curCtx: Root - %s, Path - %s\n", curCtx->getRoot_c(), curCtx->getPath_c());
            bool hasConfigScript = FileExists((curCtx->fullPath() + std::string(".config.luau")).c_str());
            bool hasLuauRc = FileExists((curCtx->fullPath() + std::string(".luaurc")).c_str());

            if (hasConfigScript && hasLuauRc)
            {
                printf("CONFIG_AMBIGUOUS\n");
                return CONFIG_AMBIGUOUS;
            }
            if (!hasConfigScript && !hasLuauRc)
            {
                printf("CONFIG_ABSENT\n");
                return CONFIG_ABSENT;
            }
            printf("CONFIG_PRESENT_LUAU or CONFIG_PRESENT_JSON\n");
            return hasConfigScript ? CONFIG_PRESENT_LUAU : CONFIG_PRESENT_JSON;
        };

    // Parses the configuration file in the current context for the given alias
    // and returns its value or WRITE_FAILURE if not found. This function is
    // only called if get_config_status returns true. If this function pointer
    // is set, get_config must not be set. Opting in to this function pointer
    // disables parsing configuration files internally and can be used for finer
    // control over the configuration file parsing process.
    // config->get_alias = [](lua_State* L, void* ctx, const char* alias, char* buffer, size_t buffer_size, size_t* sizeOut)

    // Provides the contents of the configuration file in the current context.
    // This function is only called if get_config_status does not return
    // CONFIG_ABSENT. If this function pointer is set, get_alias must not be
    // set. Opting in to this function pointer enables parsing configuration
    // files internally.    
    config->get_config = [](lua_State*, void* ctx, char* buffer, size_t bufferSize, size_t* outSize)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            printf("\nconfig->get_config - curCtx: Root - %s, Path - %s\n", curCtx->getRoot_c(), curCtx->getPath_c());
            bool hasConfigScript = FileExists((curCtx->fullPath() + std::string(".config.luau")).c_str());
            bool hasLuauRc = FileExists((curCtx->fullPath() + std::string(".luaurc")).c_str());

            int fileLen = GetFileLength(curCtx->fullPath().c_str());
            unsigned char* fileData = LoadFileData(curCtx->fullPath().c_str(), &fileLen);

            // Buffer too small to accept config
            if (bufferSize < fileLen)
            {
                printf("WRITE_BUFFER_TOO_SMALL\n");
                return WRITE_BUFFER_TOO_SMALL;
            }

            if (memcpy(buffer, fileData, fileLen) == buffer)
            {
                printf("WRITE_SUCCESS\n");
                return WRITE_SUCCESS;
            }
            
            printf("WRITE_FAILURE\n");
            return WRITE_FAILURE;
        };

    // Returns the maximum number of milliseconds to allow for executing a given
    // Luau-syntax configuration file. This function is only called if
    // get_config_status returns CONFIG_PRESENT_LUAU and can be left undefined
    // if support for Luau-syntax configuration files is not needed. A default
    // value of 2000ms is used. Negative values are treated as infinite.
    config->get_luau_config_timeout = [](lua_State* L, void* ctx)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            printf("\nconfig->get_luau_config_timeout - curCtx: Root - %s, Path - %s\n", curCtx->getRoot_c(), curCtx->getPath_c());
            return 2000;
        };

    // Executes the module and places the result on the stack. Returns the
    // number of results placed on the stack. Returning -1 directs the requiring
    // thread to yield. In this case, this thread should be resumed with the
    // module result pushed onto its stack.
    config->load = [](lua_State* L, void* ctx, const char* path, const char* chunkname, const char* loadname)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            printf("\nconfig->load - In - curCtx: Root - %s, Path - %s\n", curCtx->getRoot_c(), curCtx->getPath_c());
            // printf("Provided path: %s\n", path);
            // printf("Provided chunkname: %s\n", chunkname);
            // printf("Provided loadname: %s\n", loadname);

            // curCtx has a valid dir/luau at this point; we check for .luau first, then /init.luau.
            std::string loadPath = curCtx->fullPath();

            // Validate current path is a .luau module
            bool dirExists = DirectoryExists(loadPath.c_str());
            bool luauExists = FileExists((loadPath + std::string(".luau")).c_str());
            bool dirLuauExists = FileExists((loadPath + std::string("init.luau")).c_str());

            std::string newChunkName(curCtx->getPath());
            if (dirExists && dirLuauExists) {
                newChunkName += std::string("init.luau");
                loadPath += std::string("init.luau");
            } else if (luauExists) {
                newChunkName += std::string(".luau");
                loadPath += std::string(".luau");
            }

            printf("Decided module path is: %s\n", loadPath.c_str());

            lua_State* GL = lua_mainthread(L);
            lua_State* ML = lua_newthread(GL);
            lua_xmove(GL, L ,1); // Pushes top GL value to L value, pops top GL value
            luaL_sandboxthread(ML);

            int moduleSize = GetFileLength(loadPath.c_str());
            unsigned char* moduleData = LoadFileData(loadPath.c_str(), &moduleSize);

            // Compile, load, codegen
            lua_CompileOptions options = {.optimizationLevel = 2, .debugLevel = 0, .typeInfoLevel = 1, .coverageLevel = 0};
            size_t compileSize;
            char* bytecode = luau_compile((const char*)moduleData, (size_t)moduleSize, &options, &compileSize);
            luau_load(ML, newChunkName.c_str(), bytecode, compileSize, 0);
            if (luau_codegen_supported()) // Compile to native code if supported!
                luau_codegen_compile(ML, -1);

            int status = lua_resume(ML, L, 0);

            if (status == LUA_OK)
            {
                if (lua_gettop(ML) == 0)
                    lua_pushstring(ML, "module must return a value");
            }
            else if (status == LUA_YIELD)
                lua_pushstring(ML, "module can not yield");

            else if (!lua_isstring(ML, -1))
                lua_pushstring(ML, "unknown error while running module");

            // add ML result to L stack
    		lua_xmove(ML, L, 1);
			if (lua_status(ML) != LUA_OK)
    		    lua_error(L);

    		// remove ML thread from L stack
    		lua_remove(L, -2);

    		// added one value to L stack: module result
			return 1;
        };
    assert(!config->get_alias);
}