.PHONY: all build build-kernel build-libs build-arch clean

all: build

run:
	qemu-system-x86_64 -drive format=raw,file=floppy.img -vga std -monitor stdio -cpu qemu64 

build: build-arch build-kernel
	# Write the boot sector to the first sector
	dd if=arch/x86/boot/boot.bin of=boot_aligned.bin bs=512 count=1 conv=notrunc

	# Write the kernel to the image starting from the second sector
	# put kernelcore (32bit) into second block. assume its not bigger than 512 bytes
	dd if=arch/x86/boot/kernelcore.bin of=kernelcore_aligned.bin bs=512 conv=sync
	dd if=arch/x86/boot/entrysig.bin of=entrysig_aligned.bin bs=512 conv=sync
	dd if=arch/x86/boot/kernelsig.bin of=kernelsig_aligned.bin bs=512 conv=sync
	dd if=kernel/kernel.bin of=kernel_aligned.bin bs=512 conv=sync

	# Write the kernel signature after the kernel
	cat boot_aligned.bin kernelcore_aligned.bin entrysig_aligned.bin kernel_aligned.bin kernelsig_aligned.bin > floppy.img
	
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
