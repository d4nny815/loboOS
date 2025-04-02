DISK_IMG = os.img
LOOPBACK_NUM_DISK := $(shell python3 loopback.py)
LOOPBACK_NUM_DATA := $(shell python3 loopback.py 1)
LOOPBACK_DISK_DEV = /dev/loop$(LOOPBACK_NUM_DISK)
LOOPBACK_DATA_DEV = /dev/loop$(LOOPBACK_NUM_DATA)
MNT_OS_DIR = /mnt/osfiles

.PHONY: all 

all: disk

disk:
	dd if=/dev/zero of=$(DISK_IMG) bs=512 count=32768
	parted $(DISK_IMG) mklabel msdos
	parted $(DISK_IMG) mkpart primary ext2 2048s 30720s
	parted $(DISK_IMG) set 1 boot on
	sudo losetup $(LOOPBACK_DISK_DEV) $(DISK_IMG)
	sudo losetup $(LOOPBACK_DATA_DEV) $(DISK_IMG) -o 1048576
	sudo mkfs.ext2 $(LOOPBACK_DATA_DEV)
	sudo mkdir -p $(MNT_OS_DIR)
	sudo mount $(LOOPBACK_DATA_DEV) $(MNT_OS_DIR)
	sudo grub-install --root-directory=$(MNT_OS_DIR) --no-floppy \
		--target=i386-pc --modules="normal part_msdos ext2 multiboot" \
		$(LOOPBACK_DISK_DEV)
	ls -a $(MNT_OS_DIR)
	sudo umount $(MNT_OS_DIR)
	sudo losetup -d $(LOOPBACK_DISK_DEV)
	sudo losetup -d $(LOOPBACK_DATA_DEV)

qemu: disk