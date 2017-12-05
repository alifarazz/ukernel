# change application name here (executable output name)
TARGET=kernel
LINKERSCRIPT=link.ld

# compiler
CC=gcc
ASM=nasm
QEMU=qemu-system-i386 -kernel
LD=ld

# Specific C compiler flags
OPT=-O2
# OPT=-ggdb
WARN=-Wall -Wextra
STD=c99

# compiler, assembler, linker flags
CCFLAGS= -m32 $(WARN) -std="$(STD)" $(OPT) #-pipe
ASMFLAGS= -f elf32
LINKERFLAGS= -m elf_i386 -T $(LINKERSCRIPT)

# source files
CCSRCS  = $(wildcard *.c)
ASMSRCS = $(wildcard *.asm)
SRCS = $(CCSRCS) $(ASMSRCS)

# object files
CCOBJS  = $(patsubst %.c, %_c.o, $(CCSRCS))
ASMOBJS = $(patsubst %.asm, %_asm.o, $(ASMSRCS))
OBJS = $(CCOBJS) $(ASMOBJS)

.PHONY: default all run clean

default: $(TARGET)

all: default

%_c.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

%_asm.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(LD)  $(LINKERFLAGS) -o $(TARGET) $(OBJS)

run: all
	$(QEMU) $(TARGET)

.PRECIOUS: $(TARGET) $(OBJS)

clean:
	rm -f *.o $(TARGET)
