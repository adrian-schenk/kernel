include ../Makefile.config

# Define variables
TARGET = kernel.elf
SRCS = $(shell find . -name '*.c') $(shell find . -name '*.S')
#OBJ = main.o kmalloc.o graphics.o font.o bitmap.o console.o printf.o string.o gdt.o idt.o
OBJ = $(SRCS:.c=.o)

.PHONY: all clean

# Default target
all: $(TARGET)

# Rule to create the target binary
$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) -e _start -Ttext 0x10000 -o $(TARGET) ../arch/x86/boot/kernelcore.o $(OBJ)
	objcopy -O binary kernel.elf kernel.bin

%.o: %.c
	${CC} ${KERNEL_CCFLAGS}  $< -o $@

%.o: %.S
	${CC} ${KERNEL_CCFLAGS}  $< -o $@
# Clean up generated files
clean:
	rm -f $(OBJ) $(TARGET)
