IDIR=include
SDIR=src
ODIR=build
CC=gcc
CFLAGS=-I$(IDIR) -Wall -Wextra
LIBS=-lSDL2

_DEPS=common/bitwise.h asm.h constants.h cpu.h gpu.h joypad.h memory.h timer.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=common/bitwise.o asm.o cpu.o gpu.o joypad.o main.o memory.o timer.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	mkdir -p $(ODIR) $(ODIR)/common
	$(CC) -c -o $@ $< $(CFLAGS)

yobeboy: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(ODIR)/common/*.o yobeboy