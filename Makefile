IDIR=include
SDIR=src
ODIR=build
CC=gcc
CFLAGS=-I$(IDIR) -Wall -Wextra -pedantic-errors -Wno-unused-parameter -O3
LIBS=-lm -lSDL2

_DEPS=common/bitwise.h common/endianness.h asm.h cartridge.h constants.h cpu.h gpu.h joypad.h memory.h timer.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=cartridge.o cpu.o gpu.o joypad.o main.o memory.o timer.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	mkdir -p $(ODIR) $(ODIR)/common
	$(CC) -c -o $@ $< $(CFLAGS)

yobeboy: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(ODIR)/common/*.o yobeboy