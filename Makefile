# change application name here (executable output name)
TARGET=kernel
LINKERSCRIPT=link.ld

# compiler
CC=gcc
ASM=nasm
QEMU=qemu-system-i386 -kernel
LD=ld

# Specific C compiler flags
# OPT=-O2
OPT=-ggdb
WARN=-Wall -Wextra
STD=c99

# compiler, assembler, linker flags
CCFLAGS= -m32 $(WARN) -std="$(STD)" $(OPT) -fno-stack-protector #-pipe
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

.PHONY: default all run clean debug

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

debug: default
	$(QEMU) $(TARGET) -s -S &
# -s means it'll listen for gdb connection
# -S means freeze and wait for a gdb connection

gdb: debug
	gdb $(TARGET) -ex "target remote :1234"


.PRECIOUS: $(TARGET) $(OBJS)

clean:
	rm -f *.o $(TARGET)
