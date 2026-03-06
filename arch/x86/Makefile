include ../../Makefile.config

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

INCLUDES = -I ../include -I ../../kernel

.PHONY: all clean

all: build_bootloader build_pmentry build_kernelentry build_trampoline build_kernelsig build

build: $(OBJS)

build_bootloader: boot/boot.o
	ld -m elf_i386 -Ttext 0 --oformat binary -o boot/boot.bin $<

build_pmentry: boot/kernelcore.o boot/pm.o
	ld -m elf_i386 -Ttext 0x1000 --oformat binary -o boot/kernelcore.bin $^

build_trampoline: boot/trampoline.o 
	ld -m elf_i386 -Ttext 0x8000 --oformat binary -o boot/trampoline.bin $<

build_kernelentry: boot/kernelcore_64.o

build_kernelsig: boot/kernelsig.o boot/entrysig.o
	ld -m elf_i386 --oformat binary -o boot/kernelsig.bin boot/kernelsig.o 
	ld -m elf_i386 --oformat binary -o boot/entrysig.bin boot/entrysig.o 

#Bootloader should be compiled for 32 bit, rest can be compiled for 64 bit
boot/boot.o: boot/boot.S
	gcc -m32 -c -g ${CFLAGS} ${INCLUDES} boot/boot.S -o $@

boot/kernelcore.o: boot/kernelcore.S
	gcc -m32 -c -g ${CFLAGS} ${INCLUDES} boot/kernelcore.S -o $@

boot/pm.o: boot/pm.c
	gcc -m32 -c -g ${CFLAGS} ${INCLUDES} boot/pm.c -o $@

boot/trampoline.o : boot/trampoline.S
	gcc -m32 -c -g ${CFLAGS} ${INCLUDES} boot/trampoline.S -o $@

boot/kernelcore_64.o: boot/kernelcore_64.S
	gcc -m64 -c -g ${CFLAGS} ${INCLUDES} boot/kernelcore_64.S -o $@

boot/kernelsig.o: boot/kernelsig.S
	gcc -m32 -c -g ${CFLAGS} ${INCLUDES} boot/kernelsig.S -o $@

boot/entrysig.o: boot/entrysig.S
	gcc -m32 -c -g ${CFLAGS} ${INCLUDES} boot/entrysig.S -o $@

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} $< -o $@

%.o: %.S
	${CC} ${CFLAGS} ${INCLUDES} $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
