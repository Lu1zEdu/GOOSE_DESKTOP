// platform/platform_linux.c
#ifdef __linux__

#include "platform.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h> // <- CORREÇÃO: Adicionado para XA_ATOM
#include <unistd.h>
#include <stdlib.h> // Para a função system()

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"

// Variáveis globais para Linux/X11
static Display* g_display;
static Window g_window;
static GC g_gc;
static Pixmap g_pixmap_backbuffer;  // Backbuffer para desenho sem flicker
static Pixmap g_pixmap_mud_buffer;  // Buffer persistente para a lama
static int g_screen_width, g_screen_height;
static int g_depth;

// CORREÇÃO: Assinatura da função corrigida para bater com o platform.h
bool platform_init(const char* app_name, int width, int height) {
    g_display = XOpenDisplay(NULL);
    if (!g_display) return false;

    g_screen_width = width;
    g_screen_height = height;

    Window root = DefaultRootWindow(g_display);
    
    XVisualInfo vinfo;
    if (!XMatchVisualInfo(g_display, DefaultScreen(g_display), 32, TrueColor, &vinfo)) {
        return false; // Exige 32-bit de cor para transparência
    }
    g_depth = vinfo.depth;
    
    XSetWindowAttributes attrs;
    attrs.colormap = XCreateColormap(g_display, root, vinfo.visual, AllocNone);
    attrs.border_pixel = 0;
    attrs.background_pixel = 0;
    attrs.override_redirect = True;

    g_window = XCreateWindow(g_display, root, 0, 0, width, height, 0,
                             vinfo.depth, InputOutput, vinfo.visual,
                             CWColormap | CWBorderPixel | CWBackPixel | CWOverrideRedirect, &attrs);

    Atom wm_state = XInternAtom(g_display, "_NET_WM_STATE", False);
    Atom wm_state_above = XInternAtom(g_display, "_NET_WM_STATE_ABOVE", False);
    XChangeProperty(g_display, g_window, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&wm_state_above, 1);

    XSelectInput(g_display, g_window, ExposureMask | KeyPressMask);
    XMapWindow(g_display, g_window);
    XFlush(g_display);

    g_gc = XCreateGC(g_display, g_window, 0, NULL);
    g_pixmap_backbuffer = XCreatePixmap(g_display, g_window, width, height, g_depth);
    g_pixmap_mud_buffer = XCreatePixmap(g_display, g_window, width, height, g_depth);
    
    // Limpa o buffer da lama com transparência
    XSetForeground(g_display, g_gc, 0x00000000);
    XFillRectangle(g_display, g_pixmap_mud_buffer, g_gc, 0, 0, width, height);

    return true;
}

void platform_cleanup() {
    XFreePixmap(g_display, g_pixmap_backbuffer);
    XFreePixmap(g_display, g_pixmap_mud_buffer);
    XFreeGC(g_display, g_gc);
    XDestroyWindow(g_display, g_window);
    XCloseDisplay(g_display);
}

bool platform_handle_events() {
    XEvent event;
    while (XPending(g_display) > 0) {
        XNextEvent(g_display, &event);
        if (event.type == KeyPress) {
            if (XLookupKeysym(&event.xkey, 0) == XK_Escape) {
                return false;
            }
        }
    }
    return true;
}

void platform_sleep(int milliseconds) {
    usleep(milliseconds * 1000);
}

Sprite* platform_load_sprite(const char* file_path) {
    Sprite* sprite = (Sprite*)malloc(sizeof(Sprite));
    if (!sprite) return NULL;

    int channels;
    sprite->data = stbi_load(file_path, &sprite->width, &sprite->height, &channels, 4);
    if (!sprite->data) {
        free(sprite);
        return NULL;
    }
    
    for (int i = 0; i < sprite->width * sprite->height * 4; i += 4) {
        unsigned char r = sprite->data[i];
        unsigned char b = sprite->data[i+2];
        sprite->data[i] = b;
        sprite->data[i+2] = r;
    }

    XImage* ximage = XCreateImage(g_display, DefaultVisual(g_display, DefaultScreen(g_display)), 
                                  g_depth, ZPixmap, 0, (char*)sprite->data, 
                                  sprite->width, sprite->height, 32, 0);

    sprite->platform_handle = ximage;
    return sprite;
}

void platform_destroy_sprite(Sprite* sprite) {
    if (sprite) {
        XDestroyImage((XImage*)sprite->platform_handle);
        free(sprite);
    }
}

void platform_begin_drawing() {
    // Copia o estado atual da lama para o backbuffer
    XCopyArea(g_display, g_pixmap_mud_buffer, g_pixmap_backbuffer, g_gc, 0, 0, g_screen_width, g_screen_height, 0, 0);
}

void platform_draw_sprite_frame(Sprite* sprite, int dest_x, int dest_y, int src_x, int src_y, int src_w, int src_h) {
    XImage* ximage = (XImage*)sprite->platform_handle;
    XPutImage(g_display, g_pixmap_backbuffer, g_gc, ximage, src_x, src_y, dest_x, dest_y, src_w, src_h);
}

void platform_draw_footprint(Sprite* sprite, int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y) {
    // Desenha a pegada diretamente no buffer da lama
    XImage* ximage = (XImage*)sprite->platform_handle;
    XPutImage(g_display, g_pixmap_mud_buffer, g_gc, ximage, src_x, src_y, dest_x, dest_y, src_w, src_h);
}

void platform_end_drawing() {
    XCopyArea(g_display, g_pixmap_backbuffer, g_window, g_gc, 0, 0, g_screen_width, g_screen_height, 0, 0);
    XFlush(g_display);
}

void platform_get_screen_dimensions(int* width, int* height) {
    Screen* screen = DefaultScreenOfDisplay(g_display);
    *width = screen->width;
    *height = screen->height;
}

Point platform_get_mouse_position() {
    Window root, child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;
    XQueryPointer(g_display, DefaultRootWindow(g_display), &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);
    return (Point){root_x, root_y};
}

void platform_set_mouse_position(int x, int y) {
    XWarpPointer(g_display, None, DefaultRootWindow(g_display), 0, 0, 0, 0, x, y);
    XFlush(g_display);
}

void platform_play_sound(const char* file_path) {
    // Usa um player de linha de comando. 'aplay' é comum.
    // O '&' no final faz com que o comando rode em background para não travar o jogo.
    char command[256];
    snprintf(command, sizeof(command), "aplay %s > /dev/null 2>&1 &", file_path);
    system(command);
}

#endif // __linux__