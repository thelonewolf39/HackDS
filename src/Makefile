# HackDS System Makefile

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
STRIP = $(CROSS_COMPILE)strip

CFLAGS = -Wall -Wextra -O2 -std=c11
CXXFLAGS = -Wall -Wextra -O2 -std=c++17
LDFLAGS = -L/usr/lib

# SDL2 flags
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS = $(shell sdl2-config --libs) -lSDL2_ttf

# Zlib
ZLIB_LIBS = -lz

DESTDIR ?= ../rootfs
PREFIX = /system

# Targets
all: libhackds init gameloader menu

# libhackds - File format library
libhackds:
	$(CC) $(CFLAGS) -c libhackds/hackds_format.c -o libhackds/hackds_format.o
	$(AR) rcs libhackds/libhackds.a libhackds/hackds_format.o

# init system
init: libhackds
	$(CC) $(CFLAGS) -static init/init.c -o init/hackds-init
	$(STRIP) init/hackds-init

# Game loader
gameloader: libhackds
	$(CC) $(CFLAGS) gameloader/gameloader.c libhackds/libhackds.a \
		$(ZLIB_LIBS) -o gameloader/hackds-gameloader
	$(STRIP) gameloader/hackds-gameloader

# Menu system
menu: libhackds
	$(CC) $(CFLAGS) $(SDL_CFLAGS) menu/menu.c libhackds/libhackds.a \
		$(SDL_LIBS) $(ZLIB_LIBS) -o menu/hackds-menu
	$(STRIP) menu/hackds-menu

# Install
install: all
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include

	install -m 755 init/hackds-init $(DESTDIR)/sbin/init
	install -m 755 gameloader/hackds-gameloader $(DESTDIR)$(PREFIX)/bin/
	install -m 755 menu/hackds-menu $(DESTDIR)$(PREFIX)/bin/
	install -m 644 libhackds/libhackds.a $(DESTDIR)$(PREFIX)/lib/
	install -m 644 libhackds/hackds_format.h $(DESTDIR)$(PREFIX)/include/

# Clean
clean:
	rm -f libhackds/*.o libhackds/*.a
	rm -f init/hackds-init
	rm -f gameloader/hackds-gameloader
	rm -f menu/hackds-menu

.PHONY: all libhackds init gameloader menu install clean
