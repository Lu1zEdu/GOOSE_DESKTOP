// main.c (Versão Final e Corrigida)

#include "platform/platform.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

// --- Configurações do Ganso e outras definições...
// (TODA ESTA PARTE FICA IGUAL, NÃO PRECISA MUDAR)
#define FRAME_WIDTH 34
#define FRAME_HEIGHT 34
#define GOOSE_SPEED 3
#define GOOSE_RUN_SPEED 6
#define MOUSE_CATCH_DISTANCE 20
#define FOOTPRINT_INTERVAL 200 // ms
#define MEME_INTERVAL_MIN 15000 // 15s
#define MEME_INTERVAL_MAX 30000 // 30s
#define HONK_INTERVAL_MIN 5000  // 5s
#define HONK_INTERVAL_MAX 15000 // 15s
#define MAX_MEMES 5
#define MAX_FOOTPRINTS 100

typedef struct { int x; int y; } Frame;
Frame anim_walk_down[]  = {{0, 0}, {1, 0}, {2, 0}, {3, 0}};
Frame anim_walk_up[]    = {{0, 1}, {1, 1}, {2, 1}, {3, 1}};
Frame anim_walk_left[]  = {{0, 2}, {1, 2}, {2, 2}, {3, 2}};
Frame anim_walk_right[] = {{0, 3}, {1, 3}, {2, 3}, {3, 3}};
Frame footprint_frame   = {12, 14};

typedef enum {
    IDLE, WALKING, CHASING_MOUSE, DRAGGING_MOUSE, SPAWNING_MEME, HONKING
} GooseState;

typedef struct {
    int x, y, dx, dy;
    float target_x, target_y;
    GooseState state;
    long long state_timer_ms;
    Frame* current_anim;
    int current_frame_index;
    long long anim_timer_ms;
} Goose;

typedef struct { int x, y; bool active; } Meme;
typedef struct { int x, y; bool active; } Footprint;

void set_goose_state(Goose* goose, GooseState new_state, int screen_width, int screen_height) {
    goose->state = new_state;
    goose->state_timer_ms = (rand() % 3000) + 2000;
    switch (new_state) {
        case IDLE: goose->dx = 0; goose->dy = 0; break;
        case WALKING:
            goose->target_x = rand() % screen_width;
            goose->target_y = rand() % screen_height;
            break;
        case CHASING_MOUSE: goose->state_timer_ms = (rand() % 8000) + 5000; break;
        case SPAWNING_MEME:
            goose->target_x = -FRAME_WIDTH * 2;
            goose->target_y = rand() % (screen_height - 100) + 50;
            goose->state_timer_ms = 20000;
            break;
        case HONKING:
            platform_play_sound("assets/honk.wav");
            goose->dx = 0; goose->dy = 0;
            goose->state_timer_ms = 1000;
            break;
        default: break;
    }
}
// (FIM DA PARTE QUE FICA IGUAL)


// --- Função principal ---
int main() {
    srand(time(NULL));

    // ================== A CORREÇÃO ESTÁ AQUI ==================
    // 1. Primeiro, inicializamos a plataforma para estabelecer a conexão
    if (!platform_init("Irritating Goose")) {
         return 1;
    }

    // 2. Agora que a conexão existe, pegamos o tamanho da tela
    int screen_width, screen_height;
    platform_get_screen_dimensions(&screen_width, &screen_height);

    // 3. Com o tamanho, finalmente criamos nossa janela
    platform_create_window(screen_width, screen_height);
    // ==========================================================

    // Carregar assets
    Sprite* goose_spritesheet = platform_load_sprite("assets/SpriteSheet.png");
    Sprite* meme_sprite = platform_load_sprite("assets/meme.png");
    if (!goose_spritesheet || !meme_sprite) {
        platform_cleanup();
        return 1;
    }

    // O resto do código é o mesmo de antes...
    Goose goose = {0};
    goose.x = screen_width / 2;
    goose.y = screen_height / 2;
    goose.current_anim = anim_walk_down;
    set_goose_state(&goose, WALKING, screen_width, screen_height);
    
    Meme memes[MAX_MEMES] = {0};
    int meme_count = 0;
    Footprint footprints[MAX_FOOTPRINTS] = {0};
    int footprint_index = 0;
    long long next_meme_timer = (rand() % (MEME_INTERVAL_MAX - MEME_INTERVAL_MIN)) + MEME_INTERVAL_MIN;
    long long next_honk_timer = (rand() % (HONK_INTERVAL_MAX - HONK_INTERVAL_MIN)) + HONK_INTERVAL_MIN;
    long long footprint_timer = FOOTPRINT_INTERVAL;

    bool running = true;
    while (running) {
        running = platform_handle_events();
        
        goose.state_timer_ms -= 16;
        goose.anim_timer_ms -= 16;
        next_meme_timer -= 16;
        next_honk_timer -= 16;
        footprint_timer -= 16;
        
        Point mouse_pos = platform_get_mouse_position();
        float dist_to_mouse = hypot(goose.x - mouse_pos.x, goose.y - mouse_pos.y);

        if (goose.state_timer_ms <= 0) {
            int next_state = rand() % 100;
            if (next_state < 60) set_goose_state(&goose, WALKING, screen_width, screen_height);
            else if (next_state < 90) set_goose_state(&goose, CHASING_MOUSE, screen_width, screen_height);
            else set_goose_state(&goose, IDLE, screen_width, screen_height);
        }
        if (next_meme_timer <= 0 && meme_count < MAX_MEMES) {
             set_goose_state(&goose, SPAWNING_MEME, screen_width, screen_height);
             next_meme_timer = (rand() % (MEME_INTERVAL_MAX - MEME_INTERVAL_MIN)) + MEME_INTERVAL_MIN;
        }
         if (next_honk_timer <= 0 && goose.state != DRAGGING_MOUSE) {
            set_goose_state(&goose, HONKING, screen_width, screen_height);
            next_honk_timer = (rand() % (HONK_INTERVAL_MAX - HONK_INTERVAL_MIN)) + HONK_INTERVAL_MIN;
        }

        if (goose.state == WALKING || goose.state == CHASING_MOUSE || goose.state == SPAWNING_MEME) {
            if(goose.state == CHASING_MOUSE) {
                goose.target_x = mouse_pos.x;
                goose.target_y = mouse_pos.y;
                if (dist_to_mouse < MOUSE_CATCH_DISTANCE) {
                    goose.state = DRAGGING_MOUSE;
                }
            }
            float angle = atan2(goose.target_y - goose.y, goose.target_x - goose.x);
            int speed = (goose.state == CHASING_MOUSE) ? GOOSE_RUN_SPEED : GOOSE_SPEED;
            goose.dx = cos(angle) * speed;
            goose.dy = sin(angle) * speed;
            if (abs(goose.dx) > abs(goose.dy)) {
                goose.current_anim = (goose.dx > 0) ? anim_walk_right : anim_walk_left;
            } else {
                goose.current_anim = (goose.dy > 0) ? anim_walk_down : anim_walk_up;
            }
            if (goose.state == SPAWNING_MEME && goose.x < 10 && meme_count < MAX_MEMES) {
                memes[meme_count].active = true;
                memes[meme_count].x = -meme_sprite->width;
                memes[meme_count].y = goose.y;
                set_goose_state(&goose, WALKING, screen_width, screen_height);
                meme_count++;
            }
        } else if (goose.state == DRAGGING_MOUSE) {
            platform_set_mouse_position(goose.x, goose.y);
            if (goose.state_timer_ms % 1000 < 20) {
                goose.target_x = rand() % screen_width;
                goose.target_y = rand() % screen_height;
            }
            float angle = atan2(goose.target_y - goose.y, goose.target_x - goose.x);
            goose.dx = cos(angle) * GOOSE_SPEED;
            goose.dy = sin(angle) * GOOSE_SPEED;
        }

        goose.x += goose.dx;
        goose.y += goose.dy;
        
        if (goose.anim_timer_ms <= 0) {
            goose.current_frame_index = (goose.current_frame_index + 1) % 4;
            goose.anim_timer_ms = 100;
        }

        if ((goose.state == WALKING || goose.state == CHASING_MOUSE) && footprint_timer <= 0) {
            footprints[footprint_index].active = true;
            footprints[footprint_index].x = goose.x;
            footprints[footprint_index].y = goose.y;
            footprint_index = (footprint_index + 1) % MAX_FOOTPRINTS;
            footprint_timer = FOOTPRINT_INTERVAL;
        }
        
        platform_begin_drawing();
        for (int i = 0; i < MAX_FOOTPRINTS; ++i) {
            if (footprints[i].active) {
                platform_draw_footprint(goose_spritesheet, 
                    footprint_frame.x * FRAME_WIDTH, footprint_frame.y * FRAME_HEIGHT, FRAME_WIDTH, FRAME_HEIGHT,
                    footprints[i].x, footprints[i].y);
            }
        }
        for (int i = 0; i < MAX_MEMES; ++i) {
            if (memes[i].active) {
                if(memes[i].x < 20 * (i + 1)) memes[i].x += 5;
                platform_draw_sprite_frame(meme_sprite, memes[i].x, memes[i].y, 0, 0, meme_sprite->width, meme_sprite->height);
            }
        }
        Frame current_frame = goose.current_anim[goose.current_frame_index];
        platform_draw_sprite_frame(goose_spritesheet, goose.x, goose.y, 
            current_frame.x * FRAME_WIDTH, current_frame.y * FRAME_HEIGHT, FRAME_WIDTH, FRAME_HEIGHT);
        platform_end_drawing();
        platform_sleep(16);
    }

    platform_destroy_sprite(goose_spritesheet);
    platform_destroy_sprite(meme_sprite);
    platform_cleanup();
    return 0;
}