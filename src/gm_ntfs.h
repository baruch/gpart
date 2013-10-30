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

#include <stdint.h>
#include <asm/byteorder.h>

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

/* unsigned integral types */
#ifndef NTFS_INTEGRAL_TYPES
#define NTFS_INTEGRAL_TYPES
typedef uint8_t      		ntfs_u8;
typedef uint16_t      		ntfs_u16;
typedef uint32_t    		ntfs_u32;
typedef uint64_t            ntfs_u64;
typedef int8_t              ntfs_s8;
typedef int16_t             ntfs_s16;
#endif /* NTFS_INTEGRAL_TYPES */


/* Macros reading unsigned integers from a byte pointer */
#define NTFS_GETU8(p)		(*(ntfs_u8*)(p))
#define NTFS_GETU16(p)		((ntfs_u16)__cpu_to_le16(*(ntfs_u16*)(p)))
#define NTFS_GETU24(p)		((ntfs_u32)NTFS_GETU16(p) | \
		                     ((ntfs_u32)NTFS_GETU8(((char*)p)+2))<<16)
#define NTFS_GETU32(p)		((ntfs_u32)__cpu_to_le32(*(ntfs_u32*)(p)))
#define NTFS_GETU64(p)		((ntfs_u64)__cpu_to_le64(*(ntfs_u64*)(p)))

/* Macros reading signed integers, returning int */
#define NTFS_GETS8(p)		(*(ntfs_s8*)(p))
#define NTFS_GETS16(p)		((ntfs_s16)__cpu_to_le16(*(ntfs_s16*)(p)))
#define NTFS_GETS24(p)		(NTFS_GETU24(p) < 0x800000 ? \
								(int)NTFS_GETU24(p) : \
								(int)(NTFS_GETU24(p) - 0x1000000))

#endif /* _GM_NTFS_H */
