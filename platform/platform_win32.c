// platform/platform_win32.c
#ifdef _WIN32

#include "platform.h"
#include <windows.h>
#include <gdiplus.h> // Para carregar PNGs

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"

// Variáveis globais para o Windows
static HWND g_hwnd;
static HDC g_hdc_backbuffer;
static HBITMAP g_hbm_backbuffer;
static ULONG_PTR g_gdiplus_token;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
            }
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

bool platform_init(const char* app_name) {
    // Inicializar GDI+
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplus_token, &gdiplusStartupInput, NULL);
    return true;
}

void platform_create_window(int width, int height) {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "GooseWindowClass";

    RegisterClassEx(&wc);

    g_hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        "GooseWindowClass", "Desktop Goose",
        WS_POPUP,
        0, 0, width, height,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    // Cor de fundo (ex: magenta) será transparente
    SetLayeredWindowAttributes(g_hwnd, RGB(255, 0, 255), 0, LWA_COLORKEY);
    
    ShowWindow(g_hwnd, SW_SHOW);

    // Configurar o backbuffer para desenho sem flicker
    HDC hdc = GetDC(g_hwnd);
    g_hdc_backbuffer = CreateCompatibleDC(hdc);
    g_hbm_backbuffer = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(g_hdc_backbuffer, g_hbm_backbuffer);
    ReleaseDC(g_hwnd, hdc);
}

void platform_cleanup() {
    DeleteObject(g_hbm_backbuffer);
    DeleteDC(g_hdc_backbuffer);
    GdiplusShutdown(g_gdiplus_token);
}

bool platform_handle_events() {
    MSG msg;
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

void platform_sleep(int milliseconds) {
    Sleep(milliseconds);
}

Sprite* platform_load_sprite(const char* file_path) {
    Sprite* sprite = (Sprite*)malloc(sizeof(Sprite));
    if (!sprite) return NULL;

    int channels;
    sprite->data = stbi_load(file_path, &sprite->width, &sprite->height, &channels, 4); // Força 4 canais (RGBA)
    if (!sprite->data) {
        free(sprite);
        return NULL;
    }

    // GDI+ para criar um HBITMAP a partir de dados PNG
    Gdiplus_Bitmap* bmp = NULL;
    Gdiplus_Bitmap_FromScan0(sprite->width, sprite->height, sprite->width * 4, PixelFormat32bppARGB, sprite->data, &bmp);
    
    HBITMAP hbm;
    Gdiplus_Bitmap_GetHBITMAP(bmp, NULL, &hbm);
    Gdiplus_Bitmap_Dispose(bmp);
    
    sprite->platform_handle = hbm;
    return sprite;
}

void platform_destroy_sprite(Sprite* sprite) {
    if (sprite) {
        stbi_image_free(sprite->data);
        DeleteObject((HBITMAP)sprite->platform_handle);
        free(sprite);
    }
}

void platform_clear_screen() {
    RECT rc;
    GetClientRect(g_hwnd, &rc);
    HBRUSH brush = CreateSolidBrush(RGB(255, 0, 255)); // Cor de transparência
    FillRect(g_hdc_backbuffer, &rc, brush);
    DeleteObject(brush);
}

void platform_draw_sprite(Sprite* sprite, int x, int y) {
    HDC hdc_sprite = CreateCompatibleDC(g_hdc_backbuffer);
    SelectObject(hdc_sprite, (HBITMAP)sprite->platform_handle);

    // Usa AlphaBlend para desenhar com a transparência do PNG
    BLENDFUNCTION bf = {0};
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;

    AlphaBlend(g_hdc_backbuffer, x, y, sprite->width, sprite->height,
               hdc_sprite, 0, 0, sprite->width, sprite->height, bf);
    
    DeleteDC(hdc_sprite);
}

void platform_update_screen() {
    RECT rc;
    GetClientRect(g_hwnd, &rc);
    HDC hdc = GetDC(g_hwnd);
    BitBlt(hdc, 0, 0, rc.right, rc.bottom, g_hdc_backbuffer, 0, 0, SRCCOPY);
    ReleaseDC(g_hwnd, hdc);
}

void platform_get_screen_dimensions(int* width, int* height) {
    *width = GetSystemMetrics(SM_CXSCREEN);
    *height = GetSystemMetrics(SM_CYSCREEN);
}

Point platform_get_mouse_position() {
    POINT p;
    GetCursorPos(&p);
    return (Point){p.x, p.y};
}

#endif // _WIN32