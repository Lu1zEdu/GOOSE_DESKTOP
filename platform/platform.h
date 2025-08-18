// platform/platform.h
// Esta é a nossa interface. O código principal (main.c) só vai interagir com estas funções.
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>

// Estrutura para guardar informações de um sprite (nossa imagem)
typedef struct {
    unsigned char* data;
    int width;
    int height;
    // Dados específicos da plataforma (ex: HBITMAP no Windows, Pixmap no Linux)
    void* platform_handle;
} Sprite;

// Estrutura para posição
typedef struct {
    int x;
    int y;
} Point;

// Funções do ciclo de vida da aplicação
bool platform_init(const char* app_name);
void platform_create_window(int width, int height);
void platform_cleanup();

// Funções de eventos e loop
bool platform_handle_events(); // Retorna false se o programa deve fechar
void platform_sleep(int milliseconds);

// Funções de desenho
Sprite* platform_load_sprite(const char* file_path);
void platform_destroy_sprite(Sprite* sprite);
void platform_clear_screen();
void platform_draw_sprite(Sprite* sprite, int x, int y);
void platform_update_screen();

// Funções de utilidade
void platform_get_screen_dimensions(int* width, int* height);
Point platform_get_mouse_position();

#endif // PLATFORM_H