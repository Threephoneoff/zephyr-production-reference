/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <ff.h>
#include <zephyr/fs/fs.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/sys/printk.h>

#include "storage.h"

#define SD_DISK_NAME "SD"
#define SD_MOUNT_POINT "/" SD_DISK_NAME ":"

static FATFS fat_fs;

static struct fs_mount_t sd_mount = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
	.storage_dev = (void *)SD_DISK_NAME,
	.mnt_point = SD_MOUNT_POINT,
};

static void print_root_directory(void)
{
	struct fs_dir_t dir;
	struct fs_dirent entry;
	int ret;

	fs_dir_t_init(&dir);
	ret = fs_opendir(&dir, SD_MOUNT_POINT);
	if (ret < 0) {
		printk("SD: cannot open root directory (%d)\n", ret);
		return;
	}

	printk("SD: root directory:\n");
	while (true) {
		ret = fs_readdir(&dir, &entry);
		if (ret < 0 || entry.name[0] == '\0') {
			break;
		}

		printk("  %s%s", entry.name,
		       entry.type == FS_DIR_ENTRY_DIR ? "/\n" : "\n");
	}

	if (ret < 0) {
		printk("SD: directory read failed (%d)\n", ret);
	}

	(void)fs_closedir(&dir);
}

int storage_mount(void)
{
	uint32_t sector_count;
	uint32_t sector_size;
	uint64_t capacity;
	int ret;

	ret = disk_access_init(SD_DISK_NAME);
	if (ret < 0) {
		printk("SD: initialization failed (%d); is a card inserted?\n", ret);
		return ret;
	}

	ret = disk_access_ioctl(SD_DISK_NAME, DISK_IOCTL_GET_SECTOR_COUNT,
				&sector_count);
	if (ret < 0) {
		printk("SD: cannot read sector count (%d)\n", ret);
		return ret;
	}

	ret = disk_access_ioctl(SD_DISK_NAME, DISK_IOCTL_GET_SECTOR_SIZE,
				&sector_size);
	if (ret < 0) {
		printk("SD: cannot read sector size (%d)\n", ret);
		return ret;
	}

	capacity = (uint64_t)sector_count * sector_size;
	printk("SD: detected %llu MiB, %u-byte sectors\n",
	       capacity >> 20, sector_size);

	ret = fs_mount(&sd_mount);
	if (ret < 0) {
		printk("SD: FAT mount failed (%d); format the card as FAT32\n", ret);
		return ret;
	}

	printk("SD: FAT filesystem mounted at %s\n", SD_MOUNT_POINT);
	print_root_directory();

	return 0;
}
