@echo off
REM Script para compilar com o compilador do Visual Studio (cl.exe)
REM Execute este script a partir de um "Developer Command Prompt for VS"

REM Limpa compilações anteriores
del goose.exe *.obj > nul 2> nul

echo Compilando para Windows...

REM Compila o código. gdiplus.lib é necessária para carregar PNGs.
cl /Zi /EHsc /Fe:goose.exe main.c platform/platform_win32.c /link user32.lib gdi32.lib gdiplus.lib

if %errorlevel% == 0 (
    echo Compilacao bem-sucedida! Execute com: goose.exe
) else (
    echo Erro na compilacao.
)