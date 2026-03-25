#include <cstring>
#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "Luau/CodeGenOptions.h"
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
            // To be implemented
            return NAVIGATE_NOT_FOUND;
        };

    // Intercepts a given alias (like '@self') before looking for alias definition via configuration files.
    config->to_alias_override = [](lua_State* L, void* ctx, const char* alias_unprefixed) 
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            if (std::string(alias_unprefixed).compare(std::string("self")) == 0) {
                std::string upPath = curCtx->Parent();
                if (upPath.compare(curCtx->getPath()) == 0) {
                    return NAVIGATE_NOT_FOUND;
                }
                curCtx->setPath(upPath);
                return NAVIGATE_SUCCESS;
            }

            return NAVIGATE_NOT_FOUND;
        };

    // Luau resorts to this function if no config files have info on the given alias.
    config->to_alias_fallback = [](lua_State* L, void* ctx, const char* alias_unprefixed)
        {
            // To be implemented
            return NAVIGATE_NOT_FOUND;
        };

    // We up
    config->to_parent = [](lua_State* L, void* ctx)
        {
           RequireContext* curCtx = (RequireContext*)ctx;

            // Move curCtx to parent, and check if strings are different.
            // If they're the same, we assume to hit the path boundary, returning NAVIGATE_NOT_FOUND.
            std::string curPath = curCtx->getPath();
            curCtx->setPath(curCtx->Parent());
            if (curCtx->getPath().compare(curPath) == 0) return NAVIGATE_NOT_FOUND;

            return NAVIGATE_SUCCESS;
        };

    // We down
    config->to_child = [](lua_State*, void* ctx, const char* name)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            std::string searchPath = curCtx->Child(name);

            // Check if we are in a valid directory.
            std::string fullSearchPath = curCtx->getRoot() + searchPath;
            bool isDir = DirectoryExists(fullSearchPath.c_str());
            bool isModule = FileExists((fullSearchPath + ".luau").c_str());
            if (isDir || isModule)
            {
                curCtx->setPath(searchPath);
                return NAVIGATE_SUCCESS;
            }

            return NAVIGATE_NOT_FOUND;
        };
    
    // Validate if `.luau` or `/init.luau` is present.
    config->is_module_present = [](lua_State*, void* ctx)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            std::string searchModule = curCtx->fullPath() + ".luau";
            std::string searchInit = curCtx->getRoot() + curCtx->Child("init.luau");
            bool hasModule = FileExists(searchModule.c_str());
            bool hasInit = FileExists(searchInit.c_str());
            if (hasModule || hasInit) return true;
            return false;
        };

    // Called when 'is_module_present' returns true; also available via Luau's debug library.
    config->get_chunkname = [](lua_State*, void* ctx, char* buffer, size_t bufferSize, size_t* outSize)
        {
            RequireContext* curCtx = (RequireContext*)ctx;

            std::string searchModule = (curCtx->fullPath() + std::string(".luau"));
            std::string searchInit = curCtx->getRoot() + curCtx->Child("init.luau");

            // Check if module exists here or in directory
            bool hasModule = FileExists(searchModule.c_str());
            bool hasInit = FileExists(searchInit.c_str());

            std::string chName = curCtx->getPath();
            if (hasInit) chName += "/init.luau";
            else if (hasModule) chName += ".luau";
            else return WRITE_FAILURE;

            size_t chName_size = chName.length() + 1;

            if (bufferSize < chName_size) {
                *outSize = chName_size;
                return WRITE_BUFFER_TOO_SMALL;
            }

            char* result = (char*)memcpy(buffer, chName.c_str(), chName_size);
            if (&result == &buffer)
            {
                return WRITE_FAILURE;
            }
            
            *outSize = chName_size;
            return WRITE_SUCCESS;
        };

    // Provides a loadname that identifies the current module and is passed to
    // load. This function is only called if is_module_present returns true.
     config->get_loadname = [](lua_State*, void* ctx, char* buffer, size_t bufferSize, size_t* outSize)
        {
            RequireContext* curCtx = (RequireContext*)ctx;

            std::string searchModule = (curCtx->fullPath() + std::string(".luau"));
            std::string searchInit = curCtx->getRoot() + curCtx->Child("init.luau");

            // Check if module exists here or in directory
            bool hasModule = FileExists(searchModule.c_str());
            bool hasInit = FileExists(searchInit.c_str());

            std::string loadName = curCtx->fullPath();
            if (hasInit) loadName += "/init.luau";
            else if (hasModule) loadName += ".luau";
            else return WRITE_FAILURE;

            size_t loadName_size = loadName.length() + 1;

            if (bufferSize < loadName_size) {
                *outSize = loadName_size;
                return WRITE_BUFFER_TOO_SMALL;
            }

            char* result = (char*)memcpy(buffer, loadName.c_str(), loadName_size);
            if (&result == &buffer)
            {
                return WRITE_FAILURE;
            }
            
            *outSize = loadName_size;
            return WRITE_SUCCESS;
        };

    // Provides a cache key representing the current module. This function is
    // only called if is_module_present returns true.
     config->get_cache_key = [](lua_State*, void* ctx, char* buffer, size_t bufferSize, size_t* outSize)
        {
            RequireContext* curCtx = (RequireContext*)ctx;

            std::string searchModule = (curCtx->fullPath() + std::string(".luau"));
            std::string searchInit = curCtx->getRoot() + curCtx->Child("init.luau");

            // Check if module exists here or in directory
            bool hasModule = FileExists(searchModule.c_str());
            bool hasInit = FileExists(searchInit.c_str());

            std::string cacheName = curCtx->fullPath();
            if (hasInit) cacheName += "/init.luau";
            else if (hasModule) cacheName += ".luau";
            else return WRITE_FAILURE;

            size_t cacheName_size = cacheName.length() + 1;

            if (bufferSize < cacheName_size) {
                *outSize = cacheName_size;
                return WRITE_BUFFER_TOO_SMALL;
            }

            char* result = (char*)memcpy(buffer, cacheName.c_str(), cacheName_size);
            if (&result == &buffer)
            {
                return WRITE_FAILURE;
            }
            
            *outSize = cacheName_size;
            return WRITE_SUCCESS;
        };

    // Returns whether a configuration file is present in the current context,
    // and if so, its syntax. If not present, require-by-string will call
    // to_parent until either a configuration file is present or
    // NAVIGATE_FAILURE is returned (at root).
    config->get_config_status = [](lua_State*, void* ctx)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            bool hasConfigScript = FileExists((curCtx->fullPath() + std::string(".config.luau")).c_str());
            bool hasLuauRc = FileExists((curCtx->fullPath() + std::string(".luaurc")).c_str());

            if (hasConfigScript && hasLuauRc)
            {
                return CONFIG_AMBIGUOUS;
            }
            if (!hasConfigScript && !hasLuauRc)
            {
                return CONFIG_ABSENT;
            }
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
            // printf("\nconfig->get_config - curCtx: Root - %s, Path - %s\n", curCtx->getRoot_c(), curCtx->getPath_c());
            bool hasConfigScript = FileExists((curCtx->getRoot() + curCtx->Child(".config.luau")).c_str());
            bool hasLuauRc = FileExists((curCtx->getRoot() + curCtx->Child(".luaurc")).c_str());

            int fileLen = GetFileLength(curCtx->fullPath().c_str());
            unsigned char* fileData = LoadFileData(curCtx->fullPath().c_str(), &fileLen);

            // Buffer too small to accept config
            if (bufferSize < fileLen)
            {
                return WRITE_BUFFER_TOO_SMALL;
            }

            if (memcpy(buffer, fileData, fileLen) == buffer)
            {
                return WRITE_SUCCESS;
            }
            
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
            return 2000;
        };

    // Executes the module and places the result on the stack. Returns the
    // number of results placed on the stack. Returning -1 directs the requiring
    // thread to yield. In this case, this thread should be resumed with the
    // module result pushed onto its stack.
    config->load = [](lua_State* L, void* ctx, const char* path, const char* chunkname, const char* loadname)
        {
            RequireContext* curCtx = (RequireContext*)ctx;
            // printf("config->load - In - curCtx: Root - %s, Path - %s\n", curCtx->getRoot_c(), curCtx->getPath_c());
            printf("Require: Path: %s, Chunk: %s\n", path, chunkname);

            // Make strings
            std::string fullLoadPath = curCtx->fullPath();
            std::string newChunkName = curCtx->getPath();

            // Check for '/init.luau' first.
            std::string initPath = curCtx->getRoot() + curCtx->Child("init.luau");
            bool hasInit = FileExists(initPath.c_str());
            std::string luauPath = curCtx->fullPath() + ".luau";
            bool hasLuau = FileExists(luauPath.c_str());
            if (hasInit)
            {
                fullLoadPath = initPath;
                newChunkName += "/init.luau";
            }
            else if (hasLuau)
            {
                fullLoadPath = luauPath;
                newChunkName += ".luau";
            }

            lua_State* GL = lua_mainthread(L);
            lua_State* ML = lua_newthread(GL);
            lua_xmove(GL, L ,1);
            luaL_sandboxthread(ML);

            int moduleSize = GetFileLength(fullLoadPath.c_str());
            unsigned char* moduleData = LoadFileData(fullLoadPath.c_str(), &moduleSize);

            // Compile, load, codegen
            lua_CompileOptions options = {.optimizationLevel = 2, .debugLevel = 0, .typeInfoLevel = 1, .coverageLevel = 0};

            size_t compileSize;
            char* bytecode = luau_compile((const char*)moduleData, (size_t)moduleSize, &options, &compileSize);

            bool errored = true;
            int load_res = luau_load(ML, newChunkName.c_str(), bytecode, compileSize, 0);
            if (load_res == 0) {
                if (luau_codegen_supported()) luau_codegen_compile(ML, -1);

                int status = lua_resume(ML, L, 0);
                if (status == 0)
                {
                    if (lua_gettop(ML) == 1)
                        errored = false;
                    else
                        lua_pushfstring(ML, "module %s must return a single value, if it has no return value, you should explicitly return `nil`\n", path);
                }
                else if (status == LUA_YIELD)
                    lua_pushstring(ML, "module can not yield\n");
                else if (!lua_isstring(ML, -1))
                    lua_pushstring(ML, "unknown error while running module\n");
            }

            // add ML result to L stack
            lua_xmove(ML, L, 1);
            if (errored && lua_isstring(L, -1))
            {
                lua_pushstring(L, lua_debugtrace(ML));
                lua_concat(L, 2);
                lua_error(L);
            }

            // remove ML thread from L stack
            lua_remove(L, -2);

            // added one value to L stack: module result
            return 1;
        };
    assert(!config->get_alias);
}