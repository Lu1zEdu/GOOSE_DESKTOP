// main.c
#include "platform/platform.h"
#include "utils/file_utils.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#define FRAME_WIDTH 34
#define FRAME_HEIGHT 34
#define GOOSE_SPEED 3
#define GOOSE_RUN_SPEED 6
#define MOUSE_CATCH_DISTANCE 20
#define FOOTPRINT_INTERVAL 200
#define WINDOW_INTERVAL_MIN 15000
#define WINDOW_INTERVAL_MAX 30000
#define MAX_WINDOWS 5
#define MAX_FOOTPRINTS 100

typedef struct { int x; int y; } Frame;
Frame anim_walk_down[]  = {{0, 0}, {1, 0}, {2, 0}, {3, 0}};
Frame anim_walk_up[]    = {{0, 1}, {1, 1}, {2, 1}, {3, 1}};
Frame anim_walk_left[]  = {{0, 2}, {1, 2}, {2, 2}, {3, 2}};
Frame anim_walk_right[] = {{0, 3}, {1, 3}, {2, 3}, {3, 3}};
Frame footprint_frame   = {12, 14};

typedef enum {
    IDLE, WALKING, CHASING_MOUSE, DRAGGING_MOUSE, SPAWNING_WINDOW
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

typedef struct {
    int x, y;
    bool active;
    Sprite* content_sprite;
} BroughtWindow;

typedef struct { int x, y; bool active; } Footprint;

typedef struct { int x, y, w, h; } Rect;
const Rect close_button_rect = {297, 7, 17, 17};

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
        case SPAWNING_WINDOW:
            goose->target_x = -350;
            goose->target_y = rand() % (screen_height - 300);
            goose->state_timer_ms = 20000;
            break;
        default: break;
    }
}

int main() {
    srand(time(NULL));

    if (!platform_init("Desktop Goose XP")) return 1;

    int screen_width, screen_height;
    platform_get_screen_dimensions(&screen_width, &screen_height);
    platform_create_window(screen_width, screen_height);
    
    int image_count = 0;
    char** image_files = find_png_files_in_user_pictures(&image_count);
    if (image_count > 0) {
        printf("%d imagens encontradas!\n", image_count);
    } else {
        printf("Nenhuma imagem .png encontrada na pasta de Imagens. O ganso não trará janelas.\n");
    }

    Sprite* goose_spritesheet = platform_load_sprite("assets/SpriteSheet.png");
    if (!goose_spritesheet) {
        fprintf(stderr, "AVISO: 'assets/SpriteSheet.png' não encontrado. As pegadas e o ganso não aparecerão.\n");
    }

    extern const unsigned char xp_frame_png[];
    extern const unsigned int xp_frame_png_len;
    Sprite* xp_frame_sprite = platform_load_sprite_from_memory(xp_frame_png, xp_frame_png_len);
    if (!xp_frame_sprite) {
        fprintf(stderr, "Erro fatal: Não foi possível carregar a moldura da janela XP da memória.\n");
        return 1;
    }
    
    Goose goose = {0};
    goose.x = screen_width / 2;
    goose.y = screen_height / 2;
    goose.current_anim = anim_walk_down;
    set_goose_state(&goose, WALKING, screen_width, screen_height);
    
    BroughtWindow windows[MAX_WINDOWS] = {0};
    Footprint footprints[MAX_FOOTPRINTS] = {0};
    int footprint_index = 0;
    long long next_window_timer = (rand() % (WINDOW_INTERVAL_MAX - WINDOW_INTERVAL_MIN)) + WINDOW_INTERVAL_MIN;
    long long footprint_timer = FOOTPRINT_INTERVAL;

    bool running = true;
    while (running) {
        AppEvent event = platform_handle_events();
        if (event.type == EVENT_QUIT) {
            running = false;
        }
        if (event.type == EVENT_MOUSE_CLICK) {
            for (int i = MAX_WINDOWS - 1; i >= 0; i--) {
                if (windows[i].active) {
                    int btn_abs_x = windows[i].x + close_button_rect.x;
                    int btn_abs_y = windows[i].y + close_button_rect.y;

                    if (event.mouse_x >= btn_abs_x && event.mouse_x <= btn_abs_x + close_button_rect.w &&
                        event.mouse_y >= btn_abs_y && event.mouse_y <= btn_abs_y + close_button_rect.h) {
                        
                        windows[i].active = false;
                        platform_destroy_sprite(windows[i].content_sprite);
                        windows[i].content_sprite = NULL;
                        break;
                    }
                }
            }
        }
        
        goose.state_timer_ms -= 16;
        goose.anim_timer_ms -= 16;
        next_window_timer -= 16;
        if(goose_spritesheet) footprint_timer -= 16;
        
        Point mouse_pos = platform_get_mouse_position();
        float dist_to_mouse = hypot(goose.x - mouse_pos.x, goose.y - mouse_pos.y);

        if (goose.state_timer_ms <= 0) {
            set_goose_state(&goose, (rand() % 3 == 0) ? CHASING_MOUSE : WALKING, screen_width, screen_height);
        }
        if (image_count > 0 && next_window_timer <= 0) {
             set_goose_state(&goose, SPAWNING_WINDOW, screen_width, screen_height);
             next_window_timer = (rand() % (WINDOW_INTERVAL_MAX - WINDOW_INTERVAL_MIN)) + WINDOW_INTERVAL_MIN;
        }

        if (goose.state == WALKING || goose.state == CHASING_MOUSE || goose.state == SPAWNING_WINDOW) {
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
            if (goose.state == SPAWNING_WINDOW && goose.x < 10) {
                int free_slot = -1;
                for (int i = 0; i < MAX_WINDOWS; i++) { if (!windows[i].active) { free_slot = i; break; } }
                
                if (free_slot != -1) {
                    const char* random_image_path = image_files[rand() % image_count];
                    windows[free_slot].content_sprite = platform_load_sprite(random_image_path);
                    if (windows[free_slot].content_sprite) {
                        windows[free_slot].active = true;
                        windows[free_slot].x = -xp_frame_sprite->width;
                        windows[free_slot].y = goose.y;
                    }
                }
                set_goose_state(&goose, WALKING, screen_width, screen_height);
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

        if (goose_spritesheet && (goose.state == WALKING || goose.state == CHASING_MOUSE) && footprint_timer <= 0) {
            footprints[footprint_index].active = true;
            footprints[footprint_index].x = goose.x;
            footprints[footprint_index].y = goose.y;
            footprint_index = (footprint_index + 1) % MAX_FOOTPRINTS;
            footprint_timer = FOOTPRINT_INTERVAL;
        }
        
        platform_begin_drawing();
        if (goose_spritesheet) {
            for (int i = 0; i < MAX_FOOTPRINTS; ++i) {
                if (footprints[i].active) {
                    platform_draw_footprint(goose_spritesheet, 
                        footprint_frame.x * FRAME_WIDTH, footprint_frame.y * FRAME_HEIGHT, FRAME_WIDTH, FRAME_HEIGHT,
                        footprints[i].x, footprints[i].y);
                }
            }
        }
        for (int i = 0; i < MAX_WINDOWS; i++) {
            if (windows[i].active) {
                if(windows[i].x < 20 * (i + 1)) windows[i].x += 5;
                platform_draw_sprite_frame(xp_frame_sprite, windows[i].x, windows[i].y, 0, 0, xp_frame_sprite->width, xp_frame_sprite->height);
                platform_draw_sprite_frame(windows[i].content_sprite, windows[i].x + 8, windows[i].y + 27, 
                                           0, 0, windows[i].content_sprite->width, windows[i].content_sprite->height);
            }
        }
        if (goose_spritesheet) {
            Frame current_frame = goose.current_anim[goose.current_frame_index];
            platform_draw_sprite_frame(goose_spritesheet, goose.x, goose.y, 
                current_frame.x * FRAME_WIDTH, current_frame.y * FRAME_HEIGHT, FRAME_WIDTH, FRAME_HEIGHT);
        }
        platform_end_drawing();
        platform_sleep(16);
    }

    free_file_list(image_files, image_count);
    if(goose_spritesheet) platform_destroy_sprite(goose_spritesheet);
    platform_destroy_sprite(xp_frame_sprite);
    for(int i = 0; i < MAX_WINDOWS; ++i) {
        if(windows[i].content_sprite) platform_destroy_sprite(windows[i].content_sprite);
    }
    platform_cleanup();
    return 0;
}