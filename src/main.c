#include "game.h"

int main() {
    game_init_window();
    game_init();

    while (game_should_run()) {
        game_update();
    }
    
    game_shutdown();
    game_shutdown_window();
    
    return 0;
}