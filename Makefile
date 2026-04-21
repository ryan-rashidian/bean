CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O2 -std=gnu99

TARGET = bean

$(TARGET):
	$(CC) $(CFLAGS) src/bean.c -o $(TARGET)

.PHONY: clean
clean:
	rm -f $(TARGET)

