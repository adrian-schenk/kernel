include ../Makefile.config

# Define variables
TARGET = kernel.elf
OBJ = main.o kmalloc.o graphics.o font.o bitmap.o console.o printf.o string.o gdt.o idt.o page.o tss.o isr.o interrupt.o syscall.o apic.o smp.o sdt.o timer.o

.PHONY: all clean

# Default target
all: $(TARGET)

# Rule to create the target binary
$(TARGET): $(OBJ) ../arch/x86/boot/kernelcore_64.o
	# link the kernel binary
	$(LD) $(LDFLAGS) -e _start64 -Ttext 0x80000 -o $(TARGET) ../arch/x86/boot/kernelcore_64.o $(OBJ)
	objcopy -O binary kernel.elf kernel.bin

%.o: %.c
	${CC} ${KERNEL_CCFLAGS}  $< -o $@

%.o: %.S
	${CC} ${KERNEL_CCFLAGS}  $< -o $@
# Clean up generated files
clean:
	rm -f $(OBJ) $(TARGET)
