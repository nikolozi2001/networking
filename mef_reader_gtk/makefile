CC      = gcc
CFLAGS  = -Wall -O2
GTK4    = `pkg-config --cflags --libs gtk4`

all: mef_read_gtk

mef_read_gtk: mef_read_gtk.c mef_format.h
	$(CC) $(CFLAGS) mef_read_gtk.c -o mef_read_gtk $(GTK4)

clean:
	rm -f mef_read_gtk

run:
	./mef_read_gtk

install:
	sudo apt update && sudo apt install libgtk-4-dev
