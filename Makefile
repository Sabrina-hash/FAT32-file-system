CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRCS = src/main.c src/commands.c src/shell.c
OBJS = $(SRCS:.c=.o)
TARGET = filesys

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)
