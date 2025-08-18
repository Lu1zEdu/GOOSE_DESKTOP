// platform/platform.h (Versão Corrigida)

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>

typedef struct {
    unsigned char* data;
    int width;
    int height;
    void* platform_handle;
} Sprite;

typedef struct { int x; int y; } Point;

// --- Funções do Ciclo de Vida ---
bool platform_init(const char* app_name); // Não precisa mais de width/height aqui
void platform_create_window(int width, int height); // Nova função dedicada
void platform_cleanup();

// --- Funções de Eventos e Loop ---
bool platform_handle_events();
void platform_sleep(int milliseconds);

// --- Funções de Desenho ---
Sprite* platform_load_sprite(const char* file_path);
void platform_destroy_sprite(Sprite* sprite);
void platform_begin_drawing();
void platform_draw_sprite_frame(Sprite* sprite, int dest_x, int dest_y, int src_x, int src_y, int src_w, int src_h);
void platform_draw_footprint(Sprite* sprite, int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y);
void platform_end_drawing();

// --- Funções de Interação (A "Irritação") ---
void platform_get_screen_dimensions(int* width, int* height);
Point platform_get_mouse_position();
void platform_set_mouse_position(int x, int y);
void platform_play_sound(const char* file_path);

#endif // PLATFORM_H