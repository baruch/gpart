/*
 * gm_hpfs.h -- gpart hpfs guessing module header
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

#ifndef _GM_HPFS_H
#define _GM_HPFS_H


/*
 * hpfs information/macros, taken from the Linux kernel sources.
 */


/* Notation */

typedef unsigned secno;			/* sector number, partition relative */

typedef secno dnode_secno;		/* sector number of a dnode */
typedef secno fnode_secno;		/* sector number of an fnode */
typedef secno anode_secno;		/* sector number of an anode */



/* sector 0 */

/* The boot block is very like a FAT boot block, except that the
   29h signature byte is 28h instead, and the ID string is "HPFS". */

struct hpfs_boot_block
{
  unsigned char jmp[3];
  unsigned char oem_id[8];
  unsigned char bytes_per_sector[2];	/* 512 */
  unsigned char sectors_per_cluster;
  unsigned char n_reserved_sectors[2];
  unsigned char n_fats;
  unsigned char n_rootdir_entries[2];
  unsigned char n_sectors_s[2];
  unsigned char media_byte;
  unsigned short sectors_per_fat;
  unsigned short sectors_per_track;
  unsigned short heads_per_cyl;
  unsigned int n_hidden_sectors;
  unsigned int n_sectors_l;		/* size of partition */
  unsigned char drive_number;
  unsigned char mbz;
  unsigned char sig_28h;		/* 28h */
  unsigned char vol_serno[4];
  unsigned char vol_label[11];
  unsigned char sig_hpfs[8];		/* "HPFS    " */
  unsigned char pad[448];
  unsigned short magic;			/* aa55 */
};


/* sector 16 */

/* The super block has the pointer to the root directory. */

#define SB_MAGIC 0xf995e849

struct hpfs_super_block
{
  unsigned magic;			/* f995 e849 */
  unsigned magic1;			/* fa53 e9c5, more magic? */
  unsigned huh202;			/* ?? 202 = N. of B. in 1.00390625 S.*/
  fnode_secno root;			/* fnode of root directory */
  secno n_sectors;			/* size of filesystem */
  unsigned n_badblocks;			/* number of bad blocks */
  secno bitmaps;			/* pointers to free space bit maps */
  unsigned zero1;			/* 0 */
  secno badblocks;			/* bad block list */
  unsigned zero3;			/* 0 */
  time_t last_chkdsk;			/* date last checked, 0 if never */
  unsigned zero4;			/* 0 */
  secno n_dir_band;			/* number of sectors in dir band */
  secno dir_band_start;			/* first sector in dir band */
  secno dir_band_end;			/* last sector in dir band */
  secno dir_band_bitmap;		/* free space map, 1 dnode per bit */
  unsigned zero5[8];			/* 0 */
  secno scratch_dnodes;			/* ?? 8 preallocated sectors near dir
					   band, 4-aligned. */
  unsigned zero6[103];			/* 0 */
};



#endif /* _GM_HPFS_H */
