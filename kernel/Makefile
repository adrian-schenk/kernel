include ../Makefile.config

# FS_OFFSET_LBA is exported by the top-level Makefile; provide a default for
# direct sub-make invocations.
FS_OFFSET_LBA ?= 2048
CFLAGS += -DFS_OFFSET_LBA=$(FS_OFFSET_LBA)

# Define variables
TARGET = kernel.elf
OBJ = main.o kmalloc.o graphics.o font.o console.o printf.o string.o gdt.o idt.o page.o tss.o isr.o interrupt.o syscall.o syscall_entry.o apic.o smp.o sdt.o timer.o sleep.o task.o scheduler.o spinlock.o context.o pci.o ahci.o buddy_alloc.o ramfs.o fd.o memory.o e1000.o
DEPS = $(OBJ:.o=.d)

.PHONY: all clean FORCE

# Default target
all: $(TARGET)

# Rule to create the target binary
$(TARGET): $(OBJ) ../arch/x86/boot/kernelcore_64.o ../dev/devs.o ../fs/fs.o ../net/net.o
	# link the kernel binary
	$(LD) $(LDFLAGS) -e _start64 -Ttext 0x80000 -o $(TARGET) ../arch/x86/boot/kernelcore_64.o $(OBJ) ../dev/devs.o ../fs/fs.o ../net/net.o
	objcopy -O binary kernel.elf kernel.bin

../dev/devs.o: FORCE
	$(MAKE) -C ../dev devs.o

../fs/fs.o: FORCE
	$(MAKE) -C ../fs fs.o

../net/net.o: FORCE
	$(MAKE) -C ../net net.o

%.o: %.c
	${CC} ${CFLAGS} -MMD -MP $< -o $@

%.o: %.S
	${CC} ${CFLAGS} -MMD -MP $< -o $@

-include $(DEPS)

# Clean up generated files
clean:
	rm -f $(OBJ) $(DEPS) $(TARGET) kernel.bin
