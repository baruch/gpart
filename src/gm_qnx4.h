/*
 * gm_qnx4.h -- gpart qnx4 guessing module header
 * 
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   04.01.2001 <mb@ichabod.han.de>
 * Modified:  
 *
 */

#ifndef _GM_QNX4_H
#define _GM_QNX4_H

/* imported from asm/types.h */
typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;


#define QNX4_BOOTSECT_SIG	"QNX4FS"
#define QNX4_BITMAP_NAME	".bitmap"

/*
 * QNX4 filesystem structures, taken from the Linux kernel.
 */

#define QNX4_VALID_FS		0x0001	/* Clean fs. */
#define QNX4_ERROR_FS		0x0002	/* fs has errors. */
#define QNX4_BLOCK_SIZE		0x200	/* blocksize of 512 bytes */
#define QNX4_INODES_PER_BLOCK	0x08	/* 512 / 64 */
#define QNX4_DIR_ENTRY_SIZE	0x040	/* dir entry size of 64 bytes */

/* for filenames */
#define QNX4_SHORT_NAME_MAX     16
#define QNX4_NAME_MAX           48

typedef __u16 qnx4_nxtnt_t;
typedef __u8  qnx4_ftype_t;

typedef struct {
	__u32 xtnt_blk;
	__u32 xtnt_size;
} qnx4_xtnt_t;

typedef __u16 qnx4_mode_t;
typedef __u16 qnx4_muid_t;
typedef __u16 qnx4_mgid_t;
typedef __u32 qnx4_off_t;
typedef __u16 qnx4_nlink_t;

/*
 * This is the original qnx4 inode layout on disk.
 */
struct qnx4_inode_entry {
	char		di_fname[QNX4_SHORT_NAME_MAX];
	qnx4_off_t	di_size;
	qnx4_xtnt_t	di_first_xtnt;
	__u32		di_xblk;
	__s32		di_ftime;
	__s32		di_mtime;
	__s32		di_atime;
	__s32		di_ctime;
	qnx4_nxtnt_t	di_num_xtnts;
	qnx4_mode_t	di_mode;
	qnx4_muid_t	di_uid;
	qnx4_mgid_t	di_gid;
	qnx4_nlink_t	di_nlink;
	__u8		di_zero[4];
	qnx4_ftype_t	di_type;
	__u8		di_status;
};

struct qnx4_super_block {
	struct qnx4_inode_entry RootDir;
	struct qnx4_inode_entry Inode;
	struct qnx4_inode_entry Boot;
	struct qnx4_inode_entry AltBoot;
};


#endif /* _GM_QNX4_H */
