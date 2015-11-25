/*
 * gm_lvm2.h -- gpart Linux LVM physical volume guessing module header
 *
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   19.11.2015 <mwilck@arcor.de>
 * Modified:
 *
 */

#ifndef _GM_LVM2_H
#define _GM_LVM2_H
#include <endian.h>

/*
 * structs & defines gathered from LVM2
 */

#if !defined(__FreeBSD__)
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#endif

#define ID_LEN 32
#define SECTOR_SHIFT 9
#define SECTOR_SIZE (1 << SECTOR_SHIFT)
#define LABEL_ID "LABELONE"
#define LABEL_SIZE SECTOR_SIZE
#define LABEL_SCAN_SECTORS 4L
#define LABEL_SCAN_SIZE (LABEL_SCAN_SECTORS << SECTOR_SHIFT)
#define LVM2_LABEL "LVM2 001"

/* On disk - 32 bytes */
struct label_header {
	int8_t id[8];		/* LABELONE */
	uint64_t sector_xl; /* Sector number of this label */
	uint32_t crc_xl;	/* From next field to end of sector */
	uint32_t offset_xl; /* Offset from start of struct to contents */
	int8_t type[8];		/* LVM2 001 */
} __attribute__((packed));

struct disk_locn {
	uint64_t offset; /* Offset in bytes to start sector */
	uint64_t size;   /* Bytes */
} __attribute__((packed));

struct pv_header {
	int8_t pv_uuid[ID_LEN];

	/* This size can be overridden if PV belongs to a VG */
	uint64_t device_size_xl; /* Bytes */

	/* NULL-terminated list of data areas followed by */
	/* NULL-terminated list of metadata area headers */
	struct disk_locn disk_areas_xl[0]; /* Two lists */
} __attribute__((packed));

#endif /* _GM_LVM2_H */
