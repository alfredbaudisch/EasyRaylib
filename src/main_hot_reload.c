#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "hot_reload/platform_tools.h"
#include "hot_reload/file_watcher.h"
#include "hot_reload/raylib_api.gen.h"

#ifdef __APPLE__
    #define DLL_EXT ".dylib"
#elif _WIN32
    #define DLL_EXT ".dll"
#else
    #define DLL_EXT ".so"
#endif

#ifndef GAME_DLL_DIR
    #ifdef _WIN32
        #define GAME_DLL_DIR "build\\hot_reload\\"
        #define PATH_SEPARATOR "\\"
    #else
        #define GAME_DLL_DIR "build/hot_reload/"
        #define PATH_SEPARATOR "/"
    #endif
#endif

#define GAME_DLL_PATH GAME_DLL_DIR "game" DLL_EXT

typedef struct {
    dll_handle_t lib;
    void (*set_raylib_api)(RaylibAPI* api);
    void (*init_window)(void);
    void (*init)(void);
    void (*update)(void);
    bool (*should_run)(void);
    void (*shutdown)(void);
    void (*shutdown_window)(void);
    void* (*memory)(void);
    int (*memory_size)(void);
    void (*hot_reloaded)(void* mem);
    bool (*force_reload)(void);
    bool (*force_restart)(void);
    time_t modification_time;
    int api_version;
} GameAPI;

bool copy_dll(const char* to) {
    char cmd[512];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "copy \"%s\" \"%s\" >nul 2>nul", GAME_DLL_PATH, to);
#else
    snprintf(cmd, sizeof(cmd), "cp \"%s\" \"%s\"", GAME_DLL_PATH, to);
#endif
    int result = system(cmd);
    
    if (result != 0) {
        printf("[HOT_RELOAD] Failed to copy %s to %s\n", GAME_DLL_PATH, to);
        return false;
    }
    
    return true;
}

bool load_game_api(GameAPI* api, int api_version) {
    time_t mod_time = platform_get_modification_time(GAME_DLL_PATH);
    if (mod_time == 0) {
        printf("[HOT_RELOAD] Failed getting modification time of %s\n", GAME_DLL_PATH);
        return false;
    }
    
    // Copy DLL with versioned name
    char game_dll_name[256];
    snprintf(game_dll_name, sizeof(game_dll_name), GAME_DLL_DIR "game_%d" DLL_EXT, api_version);
    
    if (!copy_dll(game_dll_name)) {
        return false;
    }
    
    // Load the library
    api->lib = platform_load_library(game_dll_name);
    if (!api->lib) {
        printf("[HOT_RELOAD] Failed to load library: %s\n", game_dll_name);
        return false;
    }
    
    // Load symbols
    api->set_raylib_api = (void(*)(RaylibAPI*))platform_get_symbol(api->lib, "game_set_raylib_api");
    api->init_window = (void(*)(void))platform_get_symbol(api->lib, "game_init_window");
    api->init = (void(*)(void))platform_get_symbol(api->lib, "game_init");
    api->update = (void(*)(void))platform_get_symbol(api->lib, "game_update");
    api->should_run = (bool(*)(void))platform_get_symbol(api->lib, "game_should_run");
    api->shutdown = (void(*)(void))platform_get_symbol(api->lib, "game_shutdown");
    api->shutdown_window = (void(*)(void))platform_get_symbol(api->lib, "game_shutdown_window");
    api->memory = (void*(*)(void))platform_get_symbol(api->lib, "game_memory");
    api->memory_size = (int(*)(void))platform_get_symbol(api->lib, "game_memory_size");
    api->hot_reloaded = (void(*)(void*))platform_get_symbol(api->lib, "game_hot_reloaded");
    api->force_reload = (bool(*)(void))platform_get_symbol(api->lib, "game_force_reload");
    api->force_restart = (bool(*)(void))platform_get_symbol(api->lib, "game_force_restart");
    
    api->api_version = api_version;
    api->modification_time = mod_time;
    
    return true;
}

void unload_game_api(GameAPI* api) {
    if (api->lib) {
        platform_free_library(api->lib);
        api->lib = NULL;
    }
    
    // Remove the copied DLL
    char game_dll_name[256];
    snprintf(game_dll_name, sizeof(game_dll_name), GAME_DLL_DIR "game_%d" DLL_EXT, api->api_version);
    platform_delete_file(game_dll_name);
}

int main() {
    platform_create_directory(GAME_DLL_DIR);
    
    int game_api_version = 0;
    GameAPI game_api = {0};
    
    if (!load_game_api(&game_api, game_api_version)) {
        printf("[HOT_RELOAD] Failed to load Game API\n");
        return 1;
    }
    
    game_api_version++;
    
    // Set up raylib API for the shared library
    RaylibAPI* raylib_api = create_raylib_api();
    game_api.set_raylib_api(raylib_api);

    game_api.init_window();
    game_api.init();

    file_watcher_reload();
    
    printf("[HOT_RELOAD] Hot reload system started. Press F5 to force reload, F6 to restart.\n");
    
    // Keep track of old APIs for cleanup
    GameAPI old_game_apis[32];
    int old_api_count = 0;
    time_t last_rebuild_time = 0;
    const time_t rebuild_cooldown = 2;
    
    // Main game loop with hot reload
    while (game_api.should_run()) {
        time_t current_time = time(NULL);
        
#ifdef HOT_RELOAD_FILE_WATCHER
        if ((current_time - last_rebuild_time) >= rebuild_cooldown && file_watcher_check()) {
            printf("[HOT_RELOAD] Files changed, rebuilding...\n");
            last_rebuild_time = current_time;
            
        #ifdef BUILD_TYPE_DEBUG
            #ifndef _WIN32
            int build_result = system("./build_hot_reload_debug.sh");
            #endif
        #else
            #ifdef _WIN32
            int build_result = system("build_hot_reload.bat");
            #else
            int build_result = system("./build_hot_reload.sh");
            #endif
        #endif
            if (build_result == 0) {
                // Build successful, reload file versions
                //
                // This is needed, because when a file changes, the hot reload system rebuilds
                // and updates file_versions.dat with new modification times, but the running
                // application still has the old modification times in memory.
                //
                // This creates an infinite loop because the in-memory data never gets updated.
                // So a runtime-readable file that can be reloaded after a successful build is also generated.
                //
                // This also makes the hot reload loop not go bonkers when a file is deleted/moved or created.
                if (file_watcher_reload()) {
                    printf("[HOT_RELOAD] File versions reloaded successfully\n");
                } else {
                    printf("[HOT_RELOAD] Warning: Failed to reload file versions\n");
                }
            } else {
                printf("[HOT_RELOAD] Build failed with exit code: %d\n", build_result);
            }
        } else {
            game_api.update();
        }
#else
        game_api.update();
#endif
        
        bool force_reload = game_api.force_reload ? game_api.force_reload() : false;
        bool force_restart = game_api.force_restart ? game_api.force_restart() : false;
        bool reload = force_reload || force_restart;
        
        // Check if DLL/lib has been modified
        time_t current_mod_time = platform_get_modification_time(GAME_DLL_PATH);
        if (current_mod_time != 0 && game_api.modification_time != current_mod_time) {
            reload = true;
        }
        
        if (reload) {
            printf("[HOT_RELOAD] Reloading game library...\n");
            
            GameAPI new_game_api = {0};
            if (load_game_api(&new_game_api, game_api_version)) {
                // Check if we need a full restart
                bool need_restart = force_restart;
                if (game_api.memory_size && new_game_api.memory_size) {
                    need_restart = need_restart || (game_api.memory_size() != new_game_api.memory_size());
                }
                
                if (!need_restart) {
                    printf("[HOT_RELOAD] Hot reloading (preserving state)...\n");
                    
                    if (old_api_count < 32) {
                        old_game_apis[old_api_count++] = game_api;
                    } else {
                        unload_game_api(&old_game_apis[0]);
                        memmove(old_game_apis, old_game_apis + 1, sizeof(GameAPI) * 31);
                        old_game_apis[31] = game_api;
                    }
                    
                    void* game_memory = game_api.memory();
                    game_api = new_game_api;
                    game_api.set_raylib_api(raylib_api);
                    game_api.hot_reloaded(game_memory);
                } else {
                    printf("[HOT_RELOAD] Full restart (losing state)...\n");
                    
                    game_api.shutdown();
                    
                    // Clean up old APIs
                    for (int i = 0; i < old_api_count; i++) {
                        unload_game_api(&old_game_apis[i]);
                    }
                    old_api_count = 0;
                    
                    unload_game_api(&game_api);
                    game_api = new_game_api;
                    game_api.set_raylib_api(raylib_api);

                    void* game_memory = game_api.memory();
                    free(game_memory);
                    game_api.init();
                }
                
                game_api_version++;
            } else {
                printf("[HOT_RELOAD] Failed to load new game API, continuing with old one...\n");
            }
        }
    }
    
    // Cleanup
    game_api.shutdown();
    
    for (int i = 0; i < old_api_count; i++) {
        unload_game_api(&old_game_apis[i]);
    }
    
    game_api.shutdown_window();
    unload_game_api(&game_api);
    
    return 0;
}