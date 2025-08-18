// platform/platform_linux.c
#ifdef __linux__

#include "platform.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h> // Para usleep

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"

// Variáveis globais para Linux/X11
static Display* g_display;
static Window g_window;
static GC g_gc;
static Pixmap g_pixmap; // Backbuffer
static int g_screen_width, g_screen_height;

bool platform_init(const char* app_name) {
    g_display = XOpenDisplay(NULL);
    if (!g_display) return false;
    return true;
}

void platform_create_window(int width, int height) {
    g_screen_width = width;
    g_screen_height = height;

    Window root = DefaultRootWindow(g_display);
    
    // Tenta encontrar um visual com canal alfa para transparência
    XVisualInfo vinfo;
    if (!XMatchVisualInfo(g_display, DefaultScreen(g_display), 32, TrueColor, &vinfo)) {
        // Fallback para visual padrão se não encontrar um de 32 bits
        XMatchVisualInfo(g_display, DefaultScreen(g_display), 24, TrueColor, &vinfo);
    }
    
    XSetWindowAttributes attrs;
    attrs.colormap = XCreateColormap(g_display, root, vinfo.visual, AllocNone);
    attrs.border_pixel = 0;
    attrs.background_pixel = 0; // Fundo transparente
    attrs.override_redirect = True; // Remove decorações da janela

    g_window = XCreateWindow(g_display, root, 0, 0, width, height, 0,
                             vinfo.depth, InputOutput, vinfo.visual,
                             CWColormap | CWBorderPixel | CWBackPixel | CWOverrideRedirect, &attrs);

    // Dicas para o gerenciador de janelas
    Atom wm_state = XInternAtom(g_display, "_NET_WM_STATE", False);
    Atom wm_state_above = XInternAtom(g_display, "_NET_WM_STATE_ABOVE", False);
    XChangeProperty(g_display, g_window, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&wm_state_above, 1);

    XSelectInput(g_display, g_window, ExposureMask | KeyPressMask);
    XMapWindow(g_display, g_window);
    XFlush(g_display);

    g_gc = XCreateGC(g_display, g_window, 0, NULL);
    g_pixmap = XCreatePixmap(g_display, g_window, width, height, vinfo.depth);
}

void platform_cleanup() {
    XFreePixmap(g_display, g_pixmap);
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
    
    // Converte os dados RGBA para o formato de pixel do X11 (BGRA)
    for (int i = 0; i < sprite->width * sprite->height * 4; i += 4) {
        unsigned char r = sprite->data[i];
        unsigned char b = sprite->data[i+2];
        sprite->data[i] = b;
        sprite->data[i+2] = r;
    }

    XImage* ximage = XCreateImage(g_display, DefaultVisual(g_display, DefaultScreen(g_display)), 
                                  32, ZPixmap, 0, (char*)sprite->data, 
                                  sprite->width, sprite->height, 32, 0);

    sprite->platform_handle = ximage;
    return sprite;
}

void platform_destroy_sprite(Sprite* sprite) {
    if (sprite) {
        XDestroyImage((XImage*)sprite->platform_handle); // stbi_image_free já está implícito aqui
        free(sprite);
    }
}

void platform_clear_screen() {
    // A janela já é transparente, mas limpamos nosso backbuffer
    XSetForeground(g_display, g_gc, 0x00000000); // 0 alpha
    XFillRectangle(g_display, g_pixmap, g_gc, 0, 0, g_screen_width, g_screen_height);
}

void platform_draw_sprite(Sprite* sprite, int x, int y) {
    XImage* ximage = (XImage*)sprite->platform_handle;
    XPutImage(g_display, g_pixmap, g_gc, ximage, 0, 0, x, y, sprite->width, sprite->height);
}

void platform_update_screen() {
    XCopyArea(g_display, g_pixmap, g_window, g_gc, 0, 0, g_screen_width, g_screen_height, 0, 0);
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

#endif // __linux__