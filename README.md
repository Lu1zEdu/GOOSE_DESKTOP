# Goose de Desktop em C

Um exemplo de "Desktop Goose" feito em C puro, com suporte para Windows e Linux.

## Dependências

### Windows
- Um compilador C (MinGW-w64 ou o compilador do Visual Studio).
- O script `build.bat` assume que você está usando o compilador `cl.exe` do Visual Studio. Se estiver usando o GCC (MinGW), ajuste o script.

### Linux
- `gcc`
- Bibliotecas de desenvolvimento do X11: `sudo apt-get install libx11-dev libxext-dev` (em sistemas baseados em Debian/Ubuntu).

## Como Compilar

### Windows
1. Abra um "Developer Command Prompt for VS".
2. Navegue até a pasta do projeto.
3. Execute o comando: `build.bat`

### Linux
1. Abra um terminal.
2. Navegue até a pasta do projeto.
3. Execute o comando: `make`

## Como Executar
- No Windows: `./goose.exe`
- No Linux: `./goose`

Pressione `ESC` para fechar.