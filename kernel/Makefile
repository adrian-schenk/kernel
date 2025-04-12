include ../Makefile.config

# Define variables
TARGET = kernel.elf
OBJ = main.o kmalloc.o graphics.o font.o bitmap.o console.o printf.o string.o gdt.o idt.o

.PHONY: all clean

# Default target
all: $(TARGET)

# Rule to create the target binary
$(TARGET): $(OBJ)
	# link the kernel binary
	$(LD) $(LDFLAGS) -e _start -Ttext 0x100000 -o $(TARGET) ../arch/x86/boot/kernelcore_64.o $(OBJ)
	objcopy -O binary kernel.elf kernel.bin

%.o: %.c
	${CC} ${KERNEL_CCFLAGS}  $< -o $@

%.o: %.S
	${CC} ${KERNEL_CCFLAGS}  $< -o $@
# Clean up generated files
clean:
	rm -f $(OBJ) $(TARGET)
