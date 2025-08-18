// main.c
// Este código é 100% multiplataforma. Ele não sabe se está rodando em Windows ou Linux.
#include "platform/platform.h"
#include <stdlib.h>
#include <time.h>

#define GOOSE_WIDTH 64
#define GOOSE_HEIGHT 64
#define GOOSE_SPEED 4

// Estados do Ganso
typedef enum {
    IDLE,
    WALKING
} GooseState;

// Estrutura do nosso ganso
typedef struct {
    int x, y;
    int dx, dy; // Direção
    GooseState state;
    int state_timer_ms;
} Goose;

// Função para mudar o estado do ganso
void set_goose_state(Goose* goose, GooseState new_state) {
    goose->state = new_state;
    if (new_state == IDLE) {
        goose->state_timer_ms = (rand() % 3000) + 1000; // Fica parado por 1-4 segundos
        goose->dx = 0;
        goose->dy = 0;
    } else if (new_state == WALKING) {
        goose->state_timer_ms = (rand() % 5000) + 2000; // Anda por 2-7 segundos
        // Sorteia uma nova direção
        int angle = rand() % 360;
        if (angle < 45 || angle >= 315) { goose->dx = 1; goose->dy = 0; }
        else if (angle < 135) { goose->dx = 0; goose->dy = 1; }
        else if (angle < 225) { goose->dx = -1; goose->dy = 0; }
        else { goose->dx = 0; goose->dy = -1; }
    }
}

int main() {
    srand(time(NULL));

    if (!platform_init("Desktop Goose C")) {
        return 1;
    }

    int screen_width, screen_height;
    platform_get_screen_dimensions(&screen_width, &screen_height);

    // Cria a janela do tamanho da tela
    platform_create_window(screen_width, screen_height);

    Sprite* goose_sprite = platform_load_sprite("assets/goose.png");
    if (!goose_sprite) {
        platform_cleanup();
        return 1;
    }

    Goose goose = {0};
    goose.x = screen_width / 2;
    goose.y = screen_height / 2;
    set_goose_state(&goose, WALKING);

    bool running = true;
    while (running) {
        running = platform_handle_events();

        // --- ATUALIZAÇÃO DA LÓGICA DO GANSO ---
        goose.state_timer_ms -= 16; // Aproximadamente 16ms por frame (~60 FPS)
        if (goose.state_timer_ms <= 0) {
            set_goose_state(&goose, (rand() % 2 == 0) ? IDLE : WALKING);
        }

        goose.x += goose.dx * GOOSE_SPEED;
        goose.y += goose.dy * GOOSE_SPEED;

        // Manter o ganso dentro da tela
        if (goose.x < 0) { goose.x = 0; set_goose_state(&goose, IDLE); }
        if (goose.y < 0) { goose.y = 0; set_goose_state(&goose, IDLE); }
        if (goose.x > screen_width - GOOSE_WIDTH) { goose.x = screen_width - GOOSE_WIDTH; set_goose_state(&goose, IDLE); }
        if (goose.y > screen_height - GOOSE_HEIGHT) { goose.y = screen_height - GOOSE_HEIGHT; set_goose_state(&goose, IDLE); }
        
        // --- DESENHO ---
        platform_clear_screen();
        platform_draw_sprite(goose_sprite, goose.x, goose.y);
        platform_update_screen();
        
        platform_sleep(16); // Controla o FPS
    }

    platform_destroy_sprite(goose_sprite);
    platform_cleanup();

    return 0;
}