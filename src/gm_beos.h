/*
 * gm_beos.h -- gpart BeOS filesystem guessing module header
 * 
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   29.01.2001 <mb@ichabod.han.de>
 * Modified:  
 *
 */

#ifndef _GM_BEOS_H
#define _GM_BEOS_H

/* imported from asm/types.h */
typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

/*
 *  BeOS filesystem structures, taken from "BEOS filesystem for Linux  1999-05-28"
 *  by Makoto Kato <m_kato@ga2.so-net.ne.jp>
 */

/*
 * Flags of superblock
 */

#define BEOS_CLEAN 0x434c454e
#define BEOS_DIRTY 0x44495254

/*
 * Magic headers of BEOS's superblock, inode and index
 */

#define BEOS_SUPER_BLOCK_MAGIC1 0x42465331 /* BEOS1 */
#define BEOS_SUPER_BLOCK_MAGIC2 0xdd121031
#define BEOS_SUPER_BLOCK_MAGIC3 0x15b6830e
#define BEOS_INODE_MAGIC1 0x3bbe0ad9
#define BEOS_INDEX_MAGIC 0x69f6c2e8
#define BEOS_SUPER_MAGIC BEOS_SUPER_BLOCK_MAGIC1
#define BEOS_NUM_DIRECT_BLOCKS 12
#define BEOS_NAME_LENGTH 32

/*
 * BEOS filesystem type
 */

#define BEOS_PPC 1
#define BEOS_X86 2

/*
 * special type of BEOS
 */

typedef s64_t		beos_off_t;
typedef s64_t		beos_bigtime_t;
typedef void		beos_binode_etc;

typedef struct _beos_block_run {
	__u32	allocation_group;
	__u16	start;
	__u16	len;
} beos_block_run;

typedef beos_block_run	beos_inode_addr;

/*
 * The Superblock Structure
 */

typedef struct _beos_super_block {
	char	name[BEOS_NAME_LENGTH];
	__u32	magic1;
	__u32	fs_byte_order;

	__u32	block_size;
	__u32	block_shift;

	beos_off_t  num_blocks;
	beos_off_t  used_blocks;

	__u32          inode_size;

	__u32          magic2;
	__u32          blocks_per_ag;
	__u32          ag_shift;
	__u32          num_ags;

	__u32          flags;

	beos_block_run  log_blocks;
	beos_off_t      log_start;
	beos_off_t      log_end;

	__u32          magic3;
	beos_inode_addr root_dir;
	beos_inode_addr indices;

	__u32          pad[8];
} __attribute__ ((packed)) beos_super_block;


#endif /* _GM_BEOS_H */
