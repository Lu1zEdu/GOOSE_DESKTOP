# Makefile para compilar no Linux

CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lX11 -lm

# Define os arquivos fonte baseado no SO
SOURCES = main.c platform/platform_linux.c
TARGET = goose

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean