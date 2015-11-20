/*
 * gm_btrfs.h -- gpart Btrfs volume guessing module header
 *
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   20.11.2015 <mwilck@arcor.de>
 * Modified:
 *
 */

#ifndef _GM_BTRFS_H
#define _GM_BTRFS_H

/*
 * structs & defines gathered from btrfs-progs
 */

#if !defined(__FreeBSD__)
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#endif

typedef uint64_t __le64;
typedef uint32_t __le32;
typedef uint16_t __le16;
typedef uint8_t u8;

#define BTRFS_SUPER_INFO_OFFSET (64 * 1024)
#define BTRFS_SUPER_INFO_SIZE 4096
#define btrfs_sb_offset(i) ((i) ? ((16*1024) << (BTRFS_SUPER_MIRROR_SHIFT * (i))) : BTRFS_SUPER_INFO_SIZE)
#define BTRFS_SUPER_MIRROR_SHIFT 12
#define BTRFS_CSUM_SIZE 32
#define BTRFS_FSID_SIZE 16
#define BTRFS_UUID_SIZE 16
#define BTRFS_MAGIC 0x4D5F53665248425FULL
#define BTRFS_SYSTEM_CHUNK_ARRAY_SIZE 2048
#define BTRFS_LABEL_SIZE 256

#define BTRFS_NUM_BACKUP_ROOTS 4
struct btrfs_root_backup {
	__le64 tree_root;
	__le64 tree_root_gen;

	__le64 chunk_root;
	__le64 chunk_root_gen;

	__le64 extent_root;
	__le64 extent_root_gen;

	__le64 fs_root;
	__le64 fs_root_gen;

	__le64 dev_root;
	__le64 dev_root_gen;

	__le64 csum_root;
	__le64 csum_root_gen;

	__le64 total_bytes;
	__le64 bytes_used;
	__le64 num_devices;
	/* future */
	__le64 unsed_64[4];

	u8 tree_root_level;
	u8 chunk_root_level;
	u8 extent_root_level;
	u8 fs_root_level;
	u8 dev_root_level;
	u8 csum_root_level;
	/* future and to align */
	u8 unused_8[10];
} __attribute__ ((__packed__));

#define BTRFS_UUID_SIZE 16
struct btrfs_dev_item {
	/* the internal btrfs device id */
	__le64 devid;

	/* size of the device */
	__le64 total_bytes;

	/* bytes used */
	__le64 bytes_used;

	/* optimal io alignment for this device */
	__le32 io_align;

	/* optimal io width for this device */
	__le32 io_width;

	/* minimal io size for this device */
	__le32 sector_size;

	/* type and info about this device */
	__le64 type;

	/* expected generation for this device */
	__le64 generation;

	/*
	 * starting byte of this partition on the device,
	 * to allowr for stripe alignment in the future
	 */
	__le64 start_offset;

	/* grouping information for allocation decisions */
	__le32 dev_group;

	/* seek speed 0-100 where 100 is fastest */
	u8 seek_speed;

	/* bandwidth 0-100 where 100 is fastest */
	u8 bandwidth;

	/* btrfs generated uuid for this device */
	u8 uuid[BTRFS_UUID_SIZE];

	/* uuid of FS who owns this device */
	u8 fsid[BTRFS_UUID_SIZE];
} __attribute__ ((__packed__));

struct btrfs_super_block {
	u8 csum[BTRFS_CSUM_SIZE];
	/* the first 3 fields must match struct btrfs_header */
	u8 fsid[BTRFS_FSID_SIZE];    /* FS specific uuid */
	__le64 bytenr; /* this block number */
	__le64 flags;
	/* allowed to be different from the btrfs_header from here own down */
	__le64 magic;
	__le64 generation;
	__le64 root;
	__le64 chunk_root;
	__le64 log_root;

	/* this will help find the new super based on the log root */
	__le64 log_root_transid;
	__le64 total_bytes;
	__le64 bytes_used;
	__le64 root_dir_objectid;
	__le64 num_devices;
	__le32 sectorsize;
	__le32 nodesize;
	__le32 leafsize;
	__le32 stripesize;
	__le32 sys_chunk_array_size;
	__le64 chunk_root_generation;
	__le64 compat_flags;
	__le64 compat_ro_flags;
	__le64 incompat_flags;
	__le16 csum_type;
	u8 root_level;
	u8 chunk_root_level;
	u8 log_root_level;
	struct btrfs_dev_item dev_item;

	char label[BTRFS_LABEL_SIZE];

	__le64 cache_generation;
	__le64 uuid_tree_generation;

	/* future expansion */
	__le64 reserved[30];
	u8 sys_chunk_array[BTRFS_SYSTEM_CHUNK_ARRAY_SIZE];
	struct btrfs_root_backup super_roots[BTRFS_NUM_BACKUP_ROOTS];
} __attribute__ ((__packed__));

#endif /* _GM_BTRFS_H */
