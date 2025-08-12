#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef HOT_RELOAD
    #include "hot_reload/raylib_api.gen.h"

    #ifdef _WIN32
        #define EXPORT __declspec(dllexport)
    #else
        #define EXPORT
    #endif

    EXPORT void game_set_raylib_api(RaylibAPI* api);

    RaylibAPI* rl = NULL;
    void game_set_raylib_api(RaylibAPI* api) {
        rl = api;
    }
#else
    #define EXPORT
    #include "raylib.h"
    #include "raymath.h"
#endif

EXPORT void game_hot_reloaded(void* mem);
EXPORT void game_init();
EXPORT void game_init_window();
EXPORT void game_update();
EXPORT bool game_should_run();
EXPORT void game_shutdown();
EXPORT void game_shutdown_window();
EXPORT void* game_memory();
EXPORT int game_memory_size();
EXPORT bool game_force_reload();
EXPORT bool game_force_restart();

#endif