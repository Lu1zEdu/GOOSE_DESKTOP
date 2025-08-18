// platform/platform_win32.c
// (Adiciona controle do mouse, som, e desenho de frames específicos)
#ifdef _WIN32

#include "platform.h"
#include <windows.h>
#include <gdiplus.h>
#include <mmsystem.h> // Para PlaySound

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winmm.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"

static HWND g_hwnd;
static HDC g_hdc_backbuffer;
static HBITMAP g_hbm_backbuffer;
static HDC g_hdc_mud_buffer; // Buffer persistente para a lama
static HBITMAP g_hbm_mud_buffer;
static ULONG_PTR g_gdiplus_token;
static int g_width, g_height;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) { /* ... (sem alterações) ... */ }

bool platform_init(const char* app_name, int width, int height) {
    g_width = width; g_height = height;
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplus_token, &gdiplusStartupInput, NULL);

    WNDCLASSEX wc = { /* ... (sem alterações) ... */ };
    RegisterClassEx(&wc);
    g_hwnd = CreateWindowEx(/* ... (sem alterações) ... */);
    SetLayeredWindowAttributes(g_hwnd, RGB(255, 0, 255), 0, LWA_COLORKEY);
    ShowWindow(g_hwnd, SW_SHOW);

    HDC hdc = GetDC(g_hwnd);
    g_hdc_backbuffer = CreateCompatibleDC(hdc);
    g_hbm_backbuffer = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(g_hdc_backbuffer, g_hbm_backbuffer);
    
    // Criar o buffer da lama
    g_hdc_mud_buffer = CreateCompatibleDC(hdc);
    g_hbm_mud_buffer = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(g_hdc_mud_buffer, g_hbm_mud_buffer);
    // Limpar o buffer da lama com a cor de transparência
    HBRUSH mud_brush = CreateSolidBrush(RGB(255, 0, 255));
    RECT rc_mud = {0, 0, width, height};
    FillRect(g_hdc_mud_buffer, &rc_mud, mud_brush);
    DeleteObject(mud_brush);
    
    ReleaseDC(g_hwnd, hdc);
    return true;
}

void platform_cleanup() { /* ... */ }
bool platform_handle_events() { /* ... */ }
void platform_sleep(int ms) { /* ... */ }

// Carregar Sprite não muda
Sprite* platform_load_sprite(const char* file_path) { /* ... */ }
void platform_destroy_sprite(Sprite* sprite) { /* ... */ }

void platform_begin_drawing() {
    // Copia o estado atual da lama para o backbuffer
    BitBlt(g_hdc_backbuffer, 0, 0, g_width, g_height, g_hdc_mud_buffer, 0, 0, SRCCOPY);
}

void platform_draw_sprite_frame(Sprite* sprite, int dest_x, int dest_y, int src_x, int src_y, int src_w, int src_h) {
    HDC hdc_sprite = CreateCompatibleDC(g_hdc_backbuffer);
    SelectObject(hdc_sprite, (HBITMAP)sprite->platform_handle);

    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    AlphaBlend(g_hdc_backbuffer, dest_x, dest_y, src_w, src_h,
               hdc_sprite, src_x, src_y, src_w, src_h, bf);
    
    DeleteDC(hdc_sprite);
}

void platform_draw_footprint(Sprite* sprite, int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y) {
    // Desenha a pegada diretamente no buffer da lama para que ela persista
    HDC hdc_sprite = CreateCompatibleDC(g_hdc_mud_buffer);
    SelectObject(hdc_sprite, (HBITMAP)sprite->platform_handle);
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    AlphaBlend(g_hdc_mud_buffer, dest_x, dest_y, src_w, src_h,
               hdc_sprite, src_x, src_y, src_w, src_h, bf);
    DeleteDC(hdc_sprite);
}

void platform_end_drawing() {
    HDC hdc = GetDC(g_hwnd);
    BitBlt(hdc, 0, 0, g_width, g_height, g_hdc_backbuffer, 0, 0, SRCCOPY);
    ReleaseDC(g_hwnd, hdc);
}

void platform_get_screen_dimensions(int* width, int* height) { /* ... */ }
Point platform_get_mouse_position() { /* ... */ }

// --- Novas Funções de Irritação ---
void platform_set_mouse_position(int x, int y) {
    SetCursorPos(x, y);
}

void platform_play_sound(const char* file_path) {
    PlaySound(TEXT(file_path), NULL, SND_FILENAME | SND_ASYNC);
}

#endif // _WIN32