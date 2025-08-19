@echo off
REM Script para compilar com o compilador do Visual Studio (cl.exe)
REM Adiciona utils/file_utils.c à compilação e a biblioteca Shell32.lib para encontrar a pasta de Imagens.

cl /Zi /EHsc /Fe:goose.exe main.c platform/platform_win32.c utils/file_utils.c /link user32.lib gdi32.lib gdiplus.lib winmm.lib Shell32.lib OLE32.Lib