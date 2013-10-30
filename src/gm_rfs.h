/*
 * gm_rfs.h -- gpart ReiserFS guessing module header
 * 
 * gpart (c) 1999,2000 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   21.01.1999 <mb@ichabod.han.de>
 * Modified:  
 *
 */

#ifndef _GM_RFS_H
#define _GM_RFS_H

/* imported from asm/types.h */
typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

/*
 * taken from ReiserFS v0.91. Reiserfs Copyright 1996, 1997, 1998 Hans Reiser
 */

#define REISERFS_SUPER_MAGIC		"ReIsErFs"
#define REISERFS_FIRST_BLOCK		8
#define REISERFS_VALID_FS		1
#define REISERFS_ERROR_FS		2
#define REISERFS_MIN_BLOCK_AMOUNT	100

struct reiserfs_super_block
{
	__u32 s_block_count;		/* blocks count         */
	__u32 s_free_blocks;		/* free blocks count    */
	__u32 s_root_block;		/* root block number    */
	__u16 s_blocksize;		/* block size           */
	__u16 s_oid_maxsize;		/* max size of object id array, see get_objectid() commentary  */
	__u16 s_oid_cursize;		/* current size of object id array */
	__u16 s_state;			/* valid or error       */
	char s_magic[16];		/* reiserfs magic string indicates that file system is reiserfs */
	__u16 s_tree_height;		/* height of disk tree */
	__u16 s_bmap_nr;		/* amount of bitmap blocks needed to address each block of file system */
	__u16 s_make_tails;		/* flag specifying whether files of this file system have tails or not */
};

#define SB_SIZE (sizeof(struct reiserfs_super_block))


#endif /* _GM_RFS_H */
