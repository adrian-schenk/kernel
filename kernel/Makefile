include ../Makefile.config

# Define variables
TARGET = kernel.elf
OBJ = main.o kmalloc.o graphics.o font.o console.o printf.o string.o gdt.o idt.o page.o tss.o isr.o interrupt.o syscall.o apic.o smp.o sdt.o timer.o sleep.o task.o scheduler.o spinlock.o context.o pci.o ahci.o buddy_alloc.o ramfs.o fd.o memory.o
DEV_DEPS = $(wildcard ../dev/*.c ../dev/*.S ../dev/*.h)
FS_DEPS = $(wildcard ../fs/*.c ../fs/*.S ../fs/*.h ../fs/ext4/*.c ../fs/ext4/*.S ../fs/ext4/*.h)

.PHONY: all clean

# Default target
all: $(TARGET)

# Rule to create the target binary
$(TARGET): $(OBJ) ../arch/x86/boot/kernelcore_64.o ../dev/devs.o ../fs/fs.o
	# link the kernel binary
	$(LD) $(LDFLAGS) -e _start64 -Ttext 0x80000 -o $(TARGET) ../arch/x86/boot/kernelcore_64.o $(OBJ) ../dev/devs.o ../fs/fs.o
	objcopy -O binary kernel.elf kernel.bin

../dev/devs.o: $(DEV_DEPS)
	$(MAKE) -C ../dev devs.o

../fs/fs.o: ../fs/Makefile $(FS_DEPS)
	$(MAKE) -C ../fs fs.o

%.o: %.c
	${CC} ${CFLAGS}  $< -o $@ -I../dev -I../fs

%.o: %.S
	${CC} ${CFLAGS}  $< -o $@
# Clean up generated files
clean:
	rm -f $(OBJ) $(TARGET)
