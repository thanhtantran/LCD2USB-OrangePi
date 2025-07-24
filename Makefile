CC = gcc
CFLAGS = -Wall
LIBS = -lusb

all: sys2lcd

sys2lcd: sys2lcd.c
	$(CC) $(CFLAGS) -o sys2lcd sys2lcd.c $(LIBS)

clean:
	rm -f sys2lcd
