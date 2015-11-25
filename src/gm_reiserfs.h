/*
 * gm_reiserfs.h -- gpart ReiserFS guessing module header
 *
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   21.01.1999 <mb@ichabod.han.de>
 * Modified:  26.12.2000 Francis Devereux <francis@devereux.tc>
 *            Update support reiserfs version 3.5.28
 * Modified:  10.01.2003 Yury Umanets <umka@namesys.com>
 *            Added reiserfs 3.6.x support.
 *
 */

#ifndef _GM_reiserfs_H
#define _GM_reiserfs_H

/* imported from asm/types.h */
typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

/*
 * taken from ReiserFS v3.5.28, v3.6.x. Reiserfs Copyright 1996-2000 Hans Reiser
 */

#define REISERFS_SUPER_V35_MAGIC	"ReIsErFs"
#define REISERFS_SUPER_V36_MAGIC	"ReIsEr2Fs"

#define REISERFS_FIRST_BLOCK		64
#define REISERFS_VALID_FS		1
#define REISERFS_ERROR_FS		2
#define REISERFS_MIN_BLOCK_AMOUNT	100

struct reiserfs_super_block_v35
{
	__u32 s_block_count;		/* blocks count         */
	__u32 s_free_blocks;		/* free blocks count    */
	__u32 s_root_block;		/* root block number    */
	__u32 s_journal_block;		/* journal block number    */
	__u32 s_journal_dev;		/* journal device number  */
	__u32 s_orig_journal_size;	/* size of the journal on FS creation.  used to make sure they don't overflow it */
	__u32 s_journal_trans_max;	/* max number of blocks in a transaction.  */
	__u32 s_journal_block_count;	/* total size of the journal. can change over time  */
	__u32 s_journal_max_batch;	/* max number of blocks to batch into a trans */
	__u32 s_journal_max_commit_age;	/* in seconds, how old can an async commit be */
	__u32 s_journal_max_trans_age;	/* in seconds, how old can a transaction be */
	__u16 s_blocksize;		/* block size           */
	__u16 s_oid_maxsize;		/* max size of object id array, see get_objectid() commentary  */
	__u16 s_oid_cursize;		/* current size of object id array */
	__u16 s_state;			/* valid or error       */
	char s_magic[12];		/* reiserfs magic string indicates that file system is reiserfs */
	__u32 s_hash_function_code;	/* indicate, what hash fuction is being use to sort names in a directory*/
	__u16 s_tree_height;		/* height of disk tree */
	__u16 s_bmap_nr;		/* amount of bitmap blocks needed to address each block of file system */
	__u16 s_reserved;
};

#define SB_V35_SIZE (sizeof(struct reiserfs_super_block_v35))

struct reiserfs_super_block_v36 {
	struct reiserfs_super_block_v35 s_v35;
	__u32 s_inode_generation;
	__u32 s_flags;
	char s_uuid[16];
	char s_label[16];
	char s_unused[88];
};

#define SB_V36_SIZE (sizeof(struct reiserfs_super_block_v36))

#endif /* _GM_REISERFS_H */
