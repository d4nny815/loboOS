KERNEL_BIN = kernel.bin
DISK_IMG = os.img

LOOPBACK_NUM_DISK := $(shell python3 loopback.py)
LOOPBACK_NUM_DATA := $(shell python3 loopback.py 1)
LOOPBACK_DISK_DEV = /dev/loop$(LOOPBACK_NUM_DISK)
LOOPBACK_DATA_DEV = /dev/loop$(LOOPBACK_NUM_DATA)
MNT_OS_DIR = /mnt/osfiles

SRC_DIR = src
BUILD_DIR = build

.PHONY: all clean dirs kernel disk

all: $(DISK_IMG)
kernel: $(BUILD_DIR)/$(KERNEL_BIN)
disk: $(DISK_IMG) 

$(DISK_IMG): $(BUILD_DIR)/$(KERNEL_BIN) $(SRC_DIR)/arch/x86_64/grub.cfg
	dd if=/dev/zero of=$@ bs=512 count=32768
	parted $@ mklabel msdos
	parted $@ mkpart primary ext2 2048s 30720s
	parted $@ set 1 boot on
	sudo losetup $(LOOPBACK_DISK_DEV) $@
	sudo losetup $(LOOPBACK_DATA_DEV) $@ -o 1048576
	sudo mkfs.ext2 $(LOOPBACK_DATA_DEV)
	sudo mount $(LOOPBACK_DATA_DEV) $(MNT_OS_DIR)
	sudo grub-install --root-directory=$(MNT_OS_DIR) --no-floppy \
		--target=i386-pc --modules="normal part_msdos ext2 multiboot" \
		$(LOOPBACK_DISK_DEV)
	sudo cp $(BUILD_DIR)/$(KERNEL_BIN) $(MNT_OS_DIR)/boot/kernel.bin
	sudo cp $(SRC_DIR)/arch/x86_64/grub.cfg $(MNT_OS_DIR)/boot/grub
	sudo umount $(MNT_OS_DIR)
	sudo losetup -d $(LOOPBACK_DISK_DEV)
	sudo losetup -d $(LOOPBACK_DATA_DEV)

$(BUILD_DIR)/$(KERNEL_BIN): boot_files
	ld -n -o $(BUILD_DIR)/kernel.bin -T $(SRC_DIR)/arch/x86_64/linker.ld \
	 $(BUILD_DIR)/multiboot_header.o $(BUILD_DIR)/boot.o $(BUILD_DIR)/long_mode_init.o

boot_files: dirs
	nasm -f elf64 $(SRC_DIR)/arch/x86_64/multiboot_header.asm \
	 -o $(BUILD_DIR)/multiboot_header.o
	nasm -f elf64 $(SRC_DIR)/arch/x86_64/boot.asm -o $(BUILD_DIR)/boot.o
	nasm -f elf64 $(SRC_DIR)/arch/x86_64/long_mode_init.asm \
	 -o $(BUILD_DIR)/long_mode_init.o

qemu: $(DISK_IMG)
	qemu-system-x86_64 -drive format=raw,file=$(DISK_IMG)

dirs:
	mkdir -p $(BUILD_DIR)
	sudo mkdir -p $(MNT_OS_DIR)

clean: 
	rm -rf $(BUILD_DIR) $(DISK_IMG)