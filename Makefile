COMP_PATH 	= /opt/x86_64/bin
CC          = $(COMP_PATH)/x86_64-elf-gcc
LD          = $(COMP_PATH)/x86_64-elf-ld
NASM        = nasm

CFLAGS      = -Wall -Werror -g -ffreestanding -mno-red-zone -masm=intel
ASMFLAGS    = -felf64 -g
LDFLAGS     = -n -T $(LINKER_PATH) -nostdlib 

KERNEL_BIN  = kernel.bin
DISK_IMG    = os.img

SCRIPT_DIR  = scripts
SRC_DIR     = src
BUILD_DIR   = build

LINKER_PATH = $(SRC_DIR)/arch/x86_64/linker.ld
GRUB_CFG    = $(SRC_DIR)/arch/x86_64/grub.cfg
MOUNT_DIR   = /mnt/osfiles

LOOP_DISK_NUM := $(shell python3 $(SCRIPT_DIR)/loopback.py)
LOOP_DATA_NUM := $(shell python3 $(SCRIPT_DIR)/loopback.py 1)
LOOP_DISK_DEV = /dev/loop$(LOOP_DISK_NUM)
LOOP_DATA_DEV = /dev/loop$(LOOP_DATA_NUM)

ASM_SRC     := $(wildcard $(SRC_DIR)/arch/x86_64/*.asm)
ASM_OBJS    := $(patsubst $(SRC_DIR)/arch/x86_64/%.asm,$(BUILD_DIR)/%.o,$(ASM_SRC))

C_SRC       := $(wildcard $(SRC_DIR)/*.c)
C_OBJS      := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRC))

GDB					= $(COMP_PATH)/x86_64-elf-gdb
GDB_CFG			= $(shell pwd)/.gdbinit

.PHONY: all kernel disk qemu clean dirs

all: kernel

dirs:
	mkdir -p $(BUILD_DIR)

# =========================================================
# Build Kernel
# =========================================================

kernel: dirs $(BUILD_DIR)/$(KERNEL_BIN)

$(BUILD_DIR)/$(KERNEL_BIN): $(ASM_OBJS) $(C_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# build asm files
$(BUILD_DIR)/%.o: $(SRC_DIR)/arch/x86_64/%.asm
	$(NASM) $(ASMFLAGS) $< -o $@

# build c files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# =========================================================
# Build disk image and running
# =========================================================

disk: $(DISK_IMG)

$(DISK_IMG): kernel $(GRUB_CFG)
	dd if=/dev/zero of=$@ bs=512 count=32768
	parted $@ mklabel msdos
	parted $@ mkpart primary ext2 2048s 30720s
	parted $@ set 1 boot on
	sudo losetup $(LOOP_DISK_DEV) $@
	sudo losetup $(LOOP_DATA_DEV) $@ -o 1048576
	sudo mkfs.ext2 $(LOOP_DATA_DEV)
	sudo mkdir -p $(MOUNT_DIR)
	sudo mount $(LOOP_DATA_DEV) $(MOUNT_DIR)
	sudo grub-install --root-directory=$(MOUNT_DIR) --no-floppy \
		--target=i386-pc --modules="normal part_msdos ext2 multiboot2" \
		$(LOOP_DISK_DEV)
	sudo cp $(BUILD_DIR)/$(KERNEL_BIN) $(MOUNT_DIR)/boot/kernel.bin
	sudo cp $(GRUB_CFG) $(MOUNT_DIR)/boot/grub
	sudo umount $(MOUNT_DIR)
	sudo losetup -d $(LOOP_DISK_DEV)
	sudo losetup -d $(LOOP_DATA_DEV)

qemu: $(DISK_IMG)
	qemu-system-x86_64 -s -drive format=raw,file=$(DISK_IMG)


debug:
	$(GDB) -x $(GDB_CFG)

# =========================================================
# Clean up
# =========================================================

clean:
	rm -rf $(BUILD_DIR) $(DISK_IMG)
