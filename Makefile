CC = gcc
CFLAGS = -Wall -Wextra -g
SOURCES = main.c platform/platform_linux.c utils/file_utils.c
LDFLAGS = -lX11 -lm
TARGET = goose

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean