.PHONY: all build build-kernel build-libs build-arch clean

all: build

run:
	qemu-system-x86_64 -drive id=disk0,format=raw,file=disk.img -drive id=disk1,format=raw,file=disk1.img,if=none -vga std -monitor stdio -cpu qemu64 -smp 4 -display gtk -device ahci,id=ahci0 -device ide-hd,drive=disk1,bus=ahci0.0 -netdev user,id=net0 -device e1000,netdev=net0

run-gdb:
	qemu-system-x86_64 -drive id=disk0,format=raw,file=disk.img -drive id=disk1,format=raw,file=disk1.img,if=none -vga std -monitor stdio -cpu qemu64 -smp 4 -S -s -display gtk -device ahci,id=ahci0 -device ide-hd,drive=disk1,bus=ahci0.0 -netdev user,id=net0 -device e1000,netdev=net0

build: build-arch build-kernel rebuild-boot
	# Write the boot sector to the first sector
	dd if=arch/x86/boot/boot.bin of=boot_aligned.bin bs=512 count=1 conv=notrunc

	# Write the kernel to the image starting from the second sector
	dd if=arch/x86/boot/kernelcore.bin of=kernelcore_aligned.bin bs=512 conv=sync
	dd if=arch/x86/boot/trampoline.bin of=trampoline_aligned.bin bs=512 conv=sync
	dd if=kernel/kernel.bin of=kernel_aligned.bin bs=512 conv=sync

	cat boot_aligned.bin kernelcore_aligned.bin trampoline_aligned.bin kernel_aligned.bin > disk.img
	
rebuild-disk1:
	rm disk1.img -f
	dd if=/dev/zero count=1024 bs=1024 | tr '\000' '\001' > disk1.img

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
