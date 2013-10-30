/*
 * gm_minix.h -- gpart minix guessing module header
 * 
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   04.01.1999 <mb@ichabod.han.de>
 * Modified:  
 *
 */

#ifndef _GM_MINIX_H
#define _GM_MINIX_H


/*
 * Minix filesystem structures, taken from the Linux kernel.
 */


/* imported from asm/types.h */
typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;


#define BLOCK_SIZE		1024
#define MINIX_ROOT_INO		1

/* Not the same as the bogus LINK_MAX in <linux/limits.h>. Oh well. */
#define MINIX_LINK_MAX		250
#define MINIX2_LINK_MAX		65530

#define MINIX_I_MAP_SLOTS	8
#define MINIX_Z_MAP_SLOTS	64
#define MINIX_SUPER_MAGIC	0x137F		/* original minix fs */
#define MINIX_SUPER_MAGIC2	0x138F		/* minix fs, 30 char names */
#define MINIX2_SUPER_MAGIC	0x2468		/* minix V2 fs */
#define MINIX2_SUPER_MAGIC2	0x2478		/* minix V2 fs, 30 char names */
#define MINIX_VALID_FS		0x0001		/* Clean fs. */
#define MINIX_ERROR_FS		0x0002		/* fs has errors. */

#define MINIX_INODES_PER_BLOCK ((BLOCK_SIZE)/(sizeof (struct minix_inode)))
#define MINIX2_INODES_PER_BLOCK ((BLOCK_SIZE)/(sizeof (struct minix2_inode)))

#define MINIX_V1		0x0001		/* original minix fs */
#define MINIX_V2		0x0002		/* minix V2 fs */

#define INODE_VERSION(inode)	inode->i_sb->u.minix_sb.s_version

/*
 * This is the original minix inode layout on disk.
 * Note the 8-bit gid and atime and ctime.
 */
struct minix_inode {
	__u16 i_mode;
	__u16 i_uid;
	__u32 i_size;
	__u32 i_time;
	__u8  i_gid;
	__u8  i_nlinks;
	__u16 i_zone[9];
};

/*
 * The new minix inode has all the time entries, as well as
 * long block numbers and a third indirect block (7+1+1+1
 * instead of 7+1+1). Also, some previously 8-bit values are
 * now 16-bit. The inode is now 64 bytes instead of 32.
 */
struct minix2_inode {
	__u16 i_mode;
	__u16 i_nlinks;
	__u16 i_uid;
	__u16 i_gid;
	__u32 i_size;
	__u32 i_atime;
	__u32 i_mtime;
	__u32 i_ctime;
	__u32 i_zone[10];
};

/*
 * minix super-block data on disk
 */
struct minix_super_block {
	__u16 s_ninodes;
	__u16 s_nzones;
	__u16 s_imap_blocks;
	__u16 s_zmap_blocks;
	__u16 s_firstdatazone;
	__u16 s_log_zone_size;
	__u32 s_max_size;
	__u16 s_magic;
	__u16 s_state;
	__u32 s_zones;
};


#endif /* _GM_MINIX_H */
