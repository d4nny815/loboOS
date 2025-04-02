CC = x86_64-elf-gcc
CFLAGS = -Wall -Werror -g 
LINKER_LD = x86_64-elf-ld

KERNEL_BIN = kernel.bin
DISK_IMG = os.img

SRC_DIR = src
BUILD_DIR = build

LOOPBACK_NUM_DISK := $(shell python3 loopback.py)
LOOPBACK_NUM_DATA := $(shell python3 loopback.py 1)
LOOPBACK_DISK_DEV = /dev/loop$(LOOPBACK_NUM_DISK)
LOOPBACK_DATA_DEV = /dev/loop$(LOOPBACK_NUM_DATA)
MNT_OS_DIR = /mnt/osfiles

LINKER_PATH = $(SRC_DIR)/arch/x86_64/linker.ld
GRUB_CFG_PATH = $(SRC_DIR)/arch/x86_64/grub.cfg

ASM_SRC_FILES := $(wildcard $(SRC_DIR)/arch/x86_64/*.asm)
ASM_OBJ_FILES := $(patsubst $(SRC_DIR)/arch/x86_64/%.asm, \
	build/%.o, $(ASM_SRC_FILES))

C_SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
C_OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, \
	build/%.o, $(C_SRC_FILES))

.PHONY: all clean kernel disk

all: kernel

disk: $(DISK_IMG) 

$(DISK_IMG): $(BUILD_DIR)/$(KERNEL_BIN) $(GRUB_CFG_PATH)
	dd if=/dev/zero of=$@ bs=512 count=32768
	parted $@ mklabel msdos
	parted $@ mkpart primary ext2 2048s 30720s
	parted $@ set 1 boot on
	sudo losetup $(LOOPBACK_DISK_DEV) $@
	sudo losetup $(LOOPBACK_DATA_DEV) $@ -o 1048576
	sudo mkfs.ext2 $(LOOPBACK_DATA_DEV)
	sudo mkdir -p $(MNT_OS_DIR)
	sudo mount $(LOOPBACK_DATA_DEV) $(MNT_OS_DIR)
	sudo grub-install --root-directory=$(MNT_OS_DIR) --no-floppy \
		--target=i386-pc --modules="normal part_msdos ext2 multiboot" \
		$(LOOPBACK_DISK_DEV)
	sudo cp $(BUILD_DIR)/$(KERNEL_BIN) $(MNT_OS_DIR)/boot/kernel.bin
	sudo cp $(GRUB_CFG_PATH) $(MNT_OS_DIR)/boot/grub
	sudo umount $(MNT_OS_DIR)
	sudo losetup -d $(LOOPBACK_DISK_DEV)
	sudo losetup -d $(LOOPBACK_DATA_DEV)


kernel: $(BUILD_DIR)/$(KERNEL_BIN)

$(BUILD_DIR)/$(KERNEL_BIN): $(ASM_OBJ_FILES) $(C_OBJ_FILES)
	$(LINKER_LD) -n -o $@ -T $(LINKER_PATH) $^

# Assembly Boot Files
build/%.o: src/arch/x86_64/%.asm
	mkdir -p $(BUILD_DIR)
	nasm -felf64 $< -o $@

# C Source Files
build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

qemu: $(DISK_IMG)
	qemu-system-x86_64 -drive format=raw,file=$(DISK_IMG)

clean: 
	rm -rf $(BUILD_DIR) $(DISK_IMG)