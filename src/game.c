#include "game.h"

#define MAX_BATCH_ELEMENTS  8192
#define MAX_BUNNIES        100000

#define TOP_BAR_HEIGHT 60

typedef struct Bunny {
    Vector2 position;
    Vector2 speed;
    float rotation;
} Bunny;

typedef struct GameMemory {
    int counter;
    bool should_run;
    Texture2D tex_bunny;
    int bunny_width;
    int bunny_height;
    Bunny bunny;
    Bunny blue_bunny;
} GameMemory;

static GameMemory* g_mem = NULL;

#ifdef HOT_RELOAD
RaylibAPI* rl = NULL;

void game_set_raylib_api(RaylibAPI* api) {
    rl = api;
}
#endif

void game_hot_reloaded(void* mem) {
    g_mem = (GameMemory*)mem;
}

void game_init() {
    g_mem = malloc(sizeof(GameMemory));

    *g_mem = (GameMemory){
        .counter = 0,
        .should_run = true,
        .tex_bunny = LoadTexture("resources/wabbit_alpha.png"),
        .bunny = (Bunny){
            .position = (Vector2){100, 100},
            .speed = (Vector2){200, 200}
        },
        .blue_bunny = (Bunny){
            .position = (Vector2){200, 200}
        }
    };

    g_mem->bunny_width = g_mem->tex_bunny.width;
    g_mem->bunny_height = g_mem->tex_bunny.height;
}

void game_init_window() {
    InitWindow(800, 450, "Raylib Hot Reload!");
    SetTargetFPS(144);
}

void game_update() {
    if (!g_mem) return;

    float dt = GetFrameTime();

    g_mem->counter++;

    if (IsKeyPressed(KEY_ESCAPE)) {
        g_mem->should_run = false;
    }

    if(IsKeyDown(KEY_W)) {
        g_mem->bunny.position.y -= g_mem->bunny.speed.y * dt;
    }
    if(IsKeyDown(KEY_S)) {
        g_mem->bunny.position.y += g_mem->bunny.speed.y * dt;
    }
    if(IsKeyDown(KEY_A)) {
        g_mem->bunny.position.x -= g_mem->bunny.speed.x * dt;
    }    
    if(IsKeyDown(KEY_D)) {
        g_mem->bunny.position.x += g_mem->bunny.speed.x * dt;
    }

    if(IsKeyDown(KEY_Q)) {
        g_mem->bunny.rotation -= g_mem->bunny.speed.y * dt;
    }
    if(IsKeyDown(KEY_E)) {
        g_mem->bunny.rotation += g_mem->bunny.speed.y * dt;
    }

    // Boundaries
    if(g_mem->bunny.position.x < 0) {
        g_mem->bunny.position.x = 0;
    }
    if(g_mem->bunny.position.y < TOP_BAR_HEIGHT - g_mem->bunny_height) {
        g_mem->bunny.position.y = TOP_BAR_HEIGHT - g_mem->bunny_height;
    }
    if(g_mem->bunny.position.y + g_mem->bunny_height > GetScreenHeight()) {
        g_mem->bunny.position.y = GetScreenHeight() - g_mem->bunny_height;
    }
    if(g_mem->bunny.position.x + g_mem->bunny_width > GetScreenWidth()) {
        g_mem->bunny.position.x = GetScreenWidth() - g_mem->bunny_width;
    }
    
    BeginDrawing();
        ClearBackground(SKYBLUE);

        DrawTextureEx(g_mem->tex_bunny, g_mem->bunny.position, g_mem->bunny.rotation, 1, WHITE);
        DrawTexture(g_mem->tex_bunny, (int)g_mem->blue_bunny.position.x, (int)g_mem->blue_bunny.position.y, BLUE);

        DrawRectangle(0, 0, GetScreenWidth(), TOP_BAR_HEIGHT, BLACK);
        DrawText(TextFormat("Counter: %i\nPosition: %.2f, %.2f", g_mem->counter, g_mem->bunny.position.x, g_mem->bunny.position.y), 120, 10, 20, WHITE);
        DrawText("F5=Reload F6=Restart ESC=Exit", 10, GetScreenHeight() - 30, 20, WHITE);
        DrawFPS(10, 10);
    EndDrawing();
}

bool game_should_run() {
    if (!g_mem) return false;

    if (WindowShouldClose()) {
        g_mem->should_run = false;
    }

    return g_mem->should_run;
}

void game_shutdown() {
    if (!g_mem) return;

    UnloadTexture(g_mem->tex_bunny);
    printf("[GAME] Game shutdown - had %d counter\n", g_mem->counter);
    free(g_mem);
    g_mem = NULL;
}

void game_shutdown_window() {
    CloseWindow();
    printf("[GAME] Window closed\n");
}

void* game_memory() {
    return g_mem;
}

int game_memory_size() {
    return sizeof(GameMemory);
}

bool game_force_reload() {
    return IsKeyPressed(KEY_F5);
}

bool game_force_restart() {
    return IsKeyPressed(KEY_F6);
}