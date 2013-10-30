/*
 * gm_s86dl.h -- gpart solaris/x86 disklabel guessing module header
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

#ifndef _GM_S86DL_H
#define _GM_S86DL_H


#define SOLARIS_X86_NUMSLICE		8
#define SOLARIS_X86_VTOC_SANE		(0x600DDEEEUL)
#define SOLARIS_X86_V_VERSION		(0x01)
#define	SOLARIS_X86_V_UNASSIGNED	0x00	/* unassigned partition */
#define	SOLARIS_X86_V_BOOT		0x01	/* Boot partition */
#define	SOLARIS_X86_V_ROOT		0x02	/* Root filesystem */
#define	SOLARIS_X86_V_SWAP		0x03	/* Swap filesystem */
#define	SOLARIS_X86_V_USR		0x04	/* Usr filesystem */
#define	SOLARIS_X86_V_BACKUP		0x05	/* full disk */
#define	SOLARIS_X86_V_STAND		0x06	/* Stand partition */
#define	SOLARIS_X86_V_VAR		0x07	/* Var partition */
#define	SOLARIS_X86_V_HOME		0x08	/* Home partition */
#define	SOLARIS_X86_V_ALTSCTR		0x09	/* Alternate sector partition */
#define	SOLARIS_X86_V_CACHE		0x0a	/* Cache (cachefs) partition */
#define SOLARIS_X86_V_UNMNT		0x01	/* Unmountable partition */
#define SOLARIS_X86_V_RONLY		0x10	/* Read only */



struct solaris_x86_slice {
	ushort	s_tag;			/* ID tag of partition */
	ushort	s_flag;			/* permision flags */
	daddr_t s_start;		/* start sector no of partition */
	long	s_size;			/* # of blocks in partition */
};

struct solaris_x86_vtoc {
		unsigned long v_bootinfo[3];	/* info needed by mboot (unsupported) */
	unsigned long v_sanity;		/* to verify vtoc sanity */
	unsigned long v_version;	/* layout version */
	char	v_volume[8];		/* volume name */
	ushort	v_sectorsz;		/* sector size in bytes */
	ushort	v_nparts;		/* number of partitions */
	unsigned long v_reserved[10];	/* free space */
	struct solaris_x86_slice
		v_slice[SOLARIS_X86_NUMSLICE]; /* slice headers */
	time_t	timestamp[SOLARIS_X86_NUMSLICE]; /* timestamp (unsupported) */
	char	v_asciilabel[128];	/* for compatibility */
};


#endif /* _GM_S86DL_H */
