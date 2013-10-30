/*
 * gm_hmlvm.h -- gpart Linux LVM physical volume guessing module header
 * 
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   29.08.1999 <mb@ichabod.han.de>
 * Modified:  
 *
 */

#ifndef _GM_HMLVM_H
#define _GM_HMLVM_H

/*
 * structs & defines gathered from LVM 0.7/0.9 lvm.h and liblvm.h
 */

#if !defined(__FreeBSD__)
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#endif


/*
 * Status flags
 */

/* physical volume */
#define PV_ACTIVE		0x01	/* pv_status */
#define PV_ALLOCATABLE		0x02	/* pv_allocatable */


#define LVM_PV_DISK_BASE	0L
#define LVM_PV_DISK_SIZE	1024L
#define NAME_LEN		128	/* don't change!!! */
#define UUID_LEN		16	/* don't change!!! */
#define LVM_MAX_SIZE		( 1024LU * 1024 * 1024 * 2)	/* 1TB[sectors] */
#define LVM_ID			"HM"
#define LVM_DIR_PREFIX		"/dev/"
#define MAX_LV			256
#define LVM_MIN_PE_SIZE		( 8L * 2)	/* 8 KB in sectors */
#define LVM_MAX_PE_SIZE		( 16L * 1024L * 1024L * 2)	/* 16GB in sectors */

/* disk stored pe information */
typedef struct {
	uint16_t lv_num;
	uint16_t le_num;
} disk_pe_t;

/* disk stored PV, VG, LV and PE size and offset information */
typedef struct {
	uint32_t base;
	uint32_t size;
} lvm_disk_data_t;

/*
 * Structure Physical Volume (PV) Version 2
 */

/* disk */
typedef struct {
	uint8_t id[2];		/* Identifier */
	uint16_t version;		/* HM lvm version */
	lvm_disk_data_t pv_on_disk;
	lvm_disk_data_t vg_on_disk;
	lvm_disk_data_t pv_uuidlist_on_disk;
	lvm_disk_data_t lv_on_disk;
	lvm_disk_data_t pe_on_disk;
	uint8_t pv_uuid[NAME_LEN];
	uint8_t vg_name[NAME_LEN];
	uint8_t system_id[NAME_LEN];	/* for vgexport/vgimport */
	uint32_t pv_major;
	uint32_t pv_number;
	uint32_t pv_status;
	uint32_t pv_allocatable;
	uint32_t pv_size;		/* HM */
	uint32_t lv_cur;
	uint32_t pe_size;
	uint32_t pe_total;
	uint32_t pe_allocated;
} pv_disk_v2_t;

#define pv_disk_t pv_disk_v2_t

#endif /* _GM_HMLVM_H */
