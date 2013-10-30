/*
 * gm_xfs.h -- gpart SGI xfs guessing module header
 * 
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   30.01.2001 <mb@ichabod.han.de>
 * Modified:
 *
 */

#ifndef _GM_XFS_H
#define _GM_XFS_H

/* imported from asm/types.h */
typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

typedef __signed__ long long int __s64;
typedef unsigned long long int __u64;

/*
 * Taken from SGI's Jan192001prerelease.patch for Linux kernel 2.4.0
 */

typedef __u32		xfs_agblock_t;	/* blockno in alloc. group */
typedef	__u32		xfs_extlen_t;	/* extent length in blocks */
typedef	__u32		xfs_agnumber_t;	/* allocation group number */
typedef __s32		xfs_extnum_t;	/* # of extents in a file */
typedef __s16		xfs_aextnum_t;	/* # extents in an attribute fork */
typedef	__s64		xfs_fsize_t;	/* bytes in a file */
typedef __u64		xfs_ufsize_t;	/* unsigned bytes in a file */

typedef	__s32		xfs_suminfo_t;	/* type of bitmap summary info */
typedef	__s32		xfs_rtword_t;	/* word type for bitmap manipulations */

typedef	__s64		xfs_lsn_t;	/* log sequence number */
typedef	__s32		xfs_tid_t;	/* transaction identifier */

typedef	__u32		xfs_dablk_t;	/* dir/attr block number (in file) */
typedef	__u32		xfs_dahash_t;	/* dir/attr hash value */

typedef __u16		xfs_prid_t;	/* prid_t truncated to 16bits in XFS */

/*
 * These types are 64 bits on disk but are either 32 or 64 bits in memory.
 * Disk based types:
 */
typedef __u64		xfs_dfsbno_t;	/* blockno in filesystem (agno|agbno) */
typedef __u64		xfs_drfsbno_t;	/* blockno in filesystem (raw) */
typedef	__u64		xfs_drtbno_t;	/* extent (block) in realtime area */
typedef	__u64		xfs_dfiloff_t;	/* block number in a file */
typedef	__u64		xfs_dfilblks_t;	/* number of blocks in a file */

typedef __u64		xfs_off_t;
typedef __s32		xfs32_off_t;
typedef __u64		xfs_ino_t;      /* <inode> type */
typedef __s32		xfs_daddr_t;    /* <disk address> type */
typedef char *		xfs_caddr_t;    /* <core address> type */
typedef __u32		xfs_dev_t;

typedef struct {
	        unsigned char   __u_bits[16];
} uuid_t;

#define	XFS_SB_MAGIC		0x58465342	/* 'XFSB' */
#define	XFS_SB_VERSION_1	1		/* 5.3, 6.0.1, 6.1 */
#define	XFS_SB_VERSION_2	2		/* 6.2 - attributes */
#define	XFS_SB_VERSION_3	3		/* 6.2 - new inode version */
#define	XFS_SB_VERSION_4	4		/* 6.2+ - bitmask version */

/*
 * Inode minimum and maximum sizes.
 */
#define XFS_DINODE_MIN_LOG	8
#define XFS_DINODE_MAX_LOG	11
#define XFS_DINODE_MIN_SIZE	(1 << XFS_DINODE_MIN_LOG)
#define XFS_DINODE_MAX_SIZE	(1 << XFS_DINODE_MAX_LOG)

typedef struct xfs_sb
{
	__u32		sb_magicnum;	/* magic number == XFS_SB_MAGIC */
	__u32		sb_blocksize;	/* logical block size, bytes */
	xfs_drfsbno_t	sb_dblocks;	/* number of data blocks */
	xfs_drfsbno_t	sb_rblocks;	/* number of realtime blocks */
	xfs_drtbno_t	sb_rextents;	/* number of realtime extents */
	uuid_t		sb_uuid;	/* file system unique id */
	xfs_dfsbno_t	sb_logstart;	/* starting block of log if internal */
	xfs_ino_t	sb_rootino;	/* root inode number */
	xfs_ino_t	sb_rbmino;	/* bitmap inode for realtime extents */
	xfs_ino_t	sb_rsumino;	/* summary inode for rt bitmap */
	xfs_agblock_t	sb_rextsize;	/* realtime extent size, blocks */
	xfs_agblock_t	sb_agblocks;	/* size of an allocation group */
	xfs_agnumber_t	sb_agcount;	/* number of allocation groups */
	xfs_extlen_t	sb_rbmblocks;	/* number of rt bitmap blocks */
	xfs_extlen_t	sb_logblocks;	/* number of log blocks */
	__u16		sb_versionnum;	/* header version == XFS_SB_VERSION */
	__u16		sb_sectsize;	/* volume sector size, bytes */
	__u16		sb_inodesize;	/* inode size, bytes */
	__u16		sb_inopblock;	/* inodes per block */
	char		sb_fname[12];	/* file system name */
	__u8		sb_blocklog;	/* log2 of sb_blocksize */
	__u8		sb_sectlog;	/* log2 of sb_sectsize */
	__u8		sb_inodelog;	/* log2 of sb_inodesize */
	__u8		sb_inopblog;	/* log2 of sb_inopblock */
	__u8		sb_agblklog;	/* log2 of sb_agblocks (rounded up) */
	__u8		sb_rextslog;	/* log2 of sb_rextents */
	__u8		sb_inprogress;	/* mkfs is in progress, don't mount */
	__u8		sb_imax_pct;	/* max % of fs for inode space */
					/* statistics */
	/*
	 * These fields must remain contiguous.  If you really
	 * want to change their layout, make sure you fix the
	 * code in xfs_trans_apply_sb_deltas().
	 */
	__u64		sb_icount;	/* allocated inodes */
	__u64		sb_ifree;	/* free inodes */
	__u64		sb_fdblocks;	/* free data blocks */
	__u64		sb_frextents;	/* free realtime extents */
	/*
	 * End contiguous fields.
	 */
	xfs_ino_t	sb_uquotino;	/* user quota inode */
	xfs_ino_t	sb_pquotino;	/* project quota inode */
	__u16		sb_qflags;	/* quota flags */
	__u8		sb_flags;	/* misc. flags */
	__u8		sb_shared_vn;	/* shared version number */
	xfs_extlen_t	sb_inoalignmt;	/* inode chunk alignment, fsblocks */
	__u32		sb_unit;	/* stripe or raid unit */
	__u32		sb_width;	/* stripe or raid width */	
	__u8		sb_dirblklog;	/* log2 of dir block size (fsbs) */
        __u8		sb_dummy[7];    /* padding */
} xfs_sb_t;

#endif /* _GM_XFS_H */
