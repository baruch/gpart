/*
 * gm_ext2.h -- gpart ext2 guessing module header
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

#ifndef _GM_EXT2_H
#define _GM_EXT2_H

/*
 * Ext2 filesystem structures, taken from the ext2 utilities v1.10.
 */

/* imported from asm/types.h */
typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

/* imported from ext2fs/ext2_fs.h */

#define SUPERBLOCK_OFFSET	1024
#define SUPERBLOCK_SIZE		1024
#define BSIZE_1024		0
#define BSIZE_2048		1
#define BSIZE_4096		2
#define EXT2_LIB_CURRENT_REV	0
#define EXT2_SUPER_MAGIC	0xEF53
#define EXT2_VALID_FS		0x0001		/* Unmounted cleanly */
#define EXT2_ERROR_FS		0x0002		/* Errors detected */
#define EXT2_DFL_MAX_MNT_COUNT	20		/* Allow 20 mounts */
#define EXT2_DFL_CHECKINTERVAL	15552000	/* Don't use interval check */
#define EXT2_ERRORS_CONTINUE	1		/* Continue execution */
#define EXT2_ERRORS_RO		2		/* Remount fs read-only */
#define EXT2_ERRORS_PANIC	3		/* Panic */
#define EXT2_ERRORS_DEFAULT	EXT2_ERRORS_CONTINUE
#define EXT2_GOOD_OLD_REV       0       /* The good old (original) format */
#define EXT2_DYNAMIC_REV        1       /* V2 format w/ dynamic inode sizes */

struct ext2fs_sb {
	__u32	s_inodes_count;		/* Inodes count */
	__u32	s_blocks_count;		/* Blocks count */
	__u32	s_r_blocks_count;	/* Reserved blocks count */
	__u32	s_free_blocks_count;	/* Free blocks count */
	__u32	s_free_inodes_count;	/* Free inodes count */
	__u32	s_first_data_block;	/* First Data Block */
	__u32	s_log_block_size;	/* Block size */
	__s32	s_log_frag_size;	/* Fragment size */
	__u32	s_blocks_per_group;	/* # Blocks per group */
	__u32	s_frags_per_group;	/* # Fragments per group */
	__u32	s_inodes_per_group;	/* # Inodes per group */
	__u32	s_mtime;		/* Mount time */
	__u32	s_wtime;		/* Write time */
	__u16	s_mnt_count;		/* Mount count */
	__s16	s_max_mnt_count;	/* Maximal mount count */
	__u16	s_magic;		/* Magic signature */
	__u16	s_state;		/* File system state */
	__u16	s_errors;		/* Behaviour when detecting errors */
	__u16	s_minor_rev_level; 	/* minor revision level */
	__u32	s_lastcheck;		/* time of last check */
	__u32	s_checkinterval;	/* max. time between checks */
	__u32	s_creator_os;		/* OS */
	__u32	s_rev_level;		/* Revision level */
	__u16	s_def_resuid;		/* Default uid for reserved blocks */
	__u16	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 * 
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	__u32	s_first_ino; 		/* First non-reserved inode */
	__u16   s_inode_size; 		/* size of inode structure */
	__u16	s_block_group_nr; 	/* block group # of this superblock */
	__u32	s_feature_compat; 	/* compatible feature set */
	__u32	s_feature_incompat; 	/* incompatible feature set */
	__u32	s_feature_ro_compat; 	/* readonly-compatible feature set */
	__u8	s_uuid[16];		/* 128-bit uuid for volume */
	char	s_volume_name[16]; 	/* volume name */
	char	s_last_mounted[64]; 	/* directory where last mounted */
	__u32	s_reserved[206];	/* Padding to the end of the block */
};

#endif /* _GM_EXT2_H */
