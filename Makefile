CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -fPIE
LDFLAGS = -pie
SRC = src/main.o src/commands.o src/shell.o
TARGET = filesys

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(LDFLAGS) -o $(TARGET) $(SRC)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(SRC) $(TARGET)

