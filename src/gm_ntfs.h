/*
 * gm_ntfs.h -- gpart ntfs guessing module header
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

#ifndef _GM_NTFS_H
#define _GM_NTFS_H

/*
 * ntfs information/macros, taken from the Linux kernel sources.
 */

#define IS_MAGIC(a,b)		(*(int*)(a)==*(int*)(b))
#define IS_MFT_RECORD(a)	IS_MAGIC((a),"FILE")
#define IS_NTFS_VOLUME(a)	IS_MAGIC((a)+3,"NTFS")
#define IS_INDEX_RECORD(a)	IS_MAGIC((a),"INDX")

/* 'NTFS' in little endian */
#define NTFS_SUPER_MAGIC	0x5346544E

#if defined(i386) || defined(__i386__) || defined(__alpha__)

/* unsigned integral types */
#ifndef NTFS_INTEGRAL_TYPES
#define NTFS_INTEGRAL_TYPES
typedef unsigned char		ntfs_u8;
typedef unsigned short		ntfs_u16;
typedef unsigned int		ntfs_u32;
typedef s64_t			ntfs_u64;
#endif /* NTFS_INTEGRAL_TYPES */
#endif /* defined(i386) || defined(__i386__) || defined(__alpha__) */


/* Macros reading unsigned integers from a byte pointer */
/* these should work for all little endian machines */
#define NTFS_GETU8(p)		(*(ntfs_u8*)(p))
#define NTFS_GETU16(p)		(*(ntfs_u16*)(p))
#define NTFS_GETU24(p)		(NTFS_GETU32(p) & 0xFFFFFF)
#define NTFS_GETU32(p)		(*(ntfs_u32*)(p))
#define NTFS_GETU64(p)		(*(ntfs_u64*)(p))

/* Macros reading signed integers, returning int */
#define NTFS_GETS8(p)		((int)(*(char*)(p)))
#define NTFS_GETS16(p)		((int)(*(short*)(p)))
#define NTFS_GETS24(p)		(NTFS_GETU24(p) < 0x800000 ? (int)NTFS_GETU24(p) :



#endif /* _GM_NTFS_H */
