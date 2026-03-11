.PHONY: all build build-kernel build-libs build-arch clean

all: build

run:
	qemu-system-x86_64 -drive format=raw,file=disk.img -vga std -monitor stdio -cpu qemu64 -smp 4 -m 256M

run-gdb:
	qemu-system-x86_64 -drive format=raw,file=disk.img -vga std -monitor stdio -cpu qemu64 -smp 4 -m 256M -S -s

build: build-arch build-kernel rebuild-boot
	# Write the boot sector to the first sector
	dd if=arch/x86/boot/boot.bin of=boot_aligned.bin bs=512 count=1 conv=notrunc

	# Write the kernel to the image starting from the second sector
	dd if=arch/x86/boot/kernelcore.bin of=kernelcore_aligned.bin bs=512 conv=sync
	dd if=arch/x86/boot/trampoline.bin of=trampoline_aligned.bin bs=512 conv=sync
	dd if=kernel/kernel.bin of=kernel_aligned.bin bs=512 conv=sync

	cat boot_aligned.bin kernelcore_aligned.bin trampoline_aligned.bin kernel_aligned.bin > disk.img
	
rebuild-boot:
	python scripts/build.py

build-kernel:
	$(MAKE) -C kernel

build-libs:
	$(MAKE) -C libs

build-arch:
	$(MAKE) -C arch/x86

clean:
	find . -name "*.o" -type f -delete
	find . -name "*.bin" -type f -delete
	find . -name "*.elf" -type f -delete
	find . -name "*.img" -type f -delete
	$(MAKE) -C kernel clean
	$(MAKE) -C libs clean
	$(MAKE) -C arch clean
