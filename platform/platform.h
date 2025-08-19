// platform/platform.h
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>

typedef struct {
    unsigned char* data;
    int width;
    int height;
    void* platform_handle;
} Sprite;

typedef enum { EVENT_NONE, EVENT_QUIT, EVENT_MOUSE_CLICK } EventType;
typedef struct {
    EventType type;
    int mouse_x;
    int mouse_y;
} AppEvent;

bool platform_init(const char* app_name);
void platform_create_window(int width, int height);
void platform_cleanup();

AppEvent platform_handle_events();
void platform_sleep(int milliseconds);

Sprite* platform_load_sprite(const char* file_path);
Sprite* platform_load_sprite_from_memory(const unsigned char* data, int len);
void platform_destroy_sprite(Sprite* sprite);
void platform_begin_drawing();
void platform_draw_sprite_frame(Sprite* sprite, int dest_x, int dest_y, int src_x, int src_y, int src_w, int src_h);
void platform_draw_footprint(Sprite* sprite, int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y);
void platform_end_drawing();

void platform_get_screen_dimensions(int* width, int* height);
void platform_set_mouse_position(int x, int y);
void platform_play_sound(const char* file_path);

#endif // PLATFORM_H