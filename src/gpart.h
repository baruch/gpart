/*
 * gpart.h -- gpart main header file
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

#ifndef _GPART_H
#define _GPART_H

#include "config.h"

#include "errmsgs.h"
#include "l64seek.h"

#include <stdint.h>

typedef uint8_t byte_t;



#define le16(x) htole16(x)
#define be16(x) htobe16(x)
#define le32(x) htole32(x)
#define be32(x) htobe32(x)
#define le64(x) htole64(x)
#define be64(x) htobe64(x)


#ifndef max
#	define max(a,b)	((a)>(b)?(a):(b))
#	define min(a,b)	((a)<(b)?(a):(b))
#endif

#define MINSSIZE	(512)		/* min. sector size */
#define MAXSSIZE	(16384)

#define FATAL		1		/* fatal error, exit */
#define ERROR		2		/* non-fatal error */
#define WARN		3
#define MSG		4		/* normal message */

void pr(int,char *,...);
ssize_t bread(int,byte_t *,size_t,size_t);
byte_t *alloc(ssize_t);



/*
 * dos partition table stuff
 */

#define DOSMBSECTOR	0		/* absolute sector # of mbr */
#define NDOSPARTS	4		/* # of primary partitions */
#define DOSPARTOFF	446		/* offset of part-table in mbr */
#define DOSPARTACTIVE	0x80		/* active (boot) flag */
#define DOSPTMAGIC	0xaa55		/* signature */
#define DOSCYL(cyl,s)	((cyl)+(((s)&0xc0)<<2))
#define DOSSEC(s)	((s)&0x3f)


typedef struct
{
	byte_t		p_flag;		/* bootstrap flags */
	byte_t		p_shd;		/* starting head */
	byte_t		p_ssect;	/* starting sector */
	byte_t		p_scyl;		/* starting cylinder */
	byte_t		p_typ;		/* partition type */
	byte_t		p_ehd;		/* end head */
	byte_t		p_esect;	/* end sector */
	byte_t		p_ecyl;		/* end cylinder */
	uint32_t	p_start;	/* start sector (absolute) */
	uint32_t    p_size;		/* # of sectors */
} dos_part_entry;


typedef struct dos_pt
{
	struct dos_pt	*t_ext;		/* -> extended parttable */
	byte_t		_align[2];
	byte_t		t_boot[DOSPARTOFF];
	dos_part_entry	t_parts[NDOSPARTS];
	uint16_t	t_magic;	/* DOSPTMAGIC */
} dos_part_table;


typedef struct dos_gp
{
	dos_part_entry	g_p[NDOSPARTS];
	struct dos_gp	*g_next;
	s64_t		g_sec;		/* found there */
	unsigned int	g_ext	: 1;	/* extended ptbl */
	unsigned int	g_prim	: 1;	/* primary partition */
	unsigned int	g_log	: 1;	/* logical partition */
	unsigned int	g_inv	: 1;	/* invalid entry */
	unsigned int	g_orph	: 1;	/* orphaned partition */
} dos_guessed_pt;

/*
 * disk description used
 */

typedef struct
{
	char		*d_dev;		/* device name */
	int		d_fd;		/* file descriptor when open */
	ssize_t		d_ssize;	/* sector size */
	byte_t		*d_sbuf;	/* sector buffer */
	s64_t 		d_nsecs;	/* total no of sectors */
	s64_t		d_nsb;		/* # of first sector in sbuf on disk */
	struct disk_geom		/* disk geometry */
	{
		long	d_c;		/* cylinder count */
		long	d_h;		/* heads/cyl */
		long	d_s;		/* sectors/head */
		long	d_rc;		/* real values if the above are */
		long	d_rh;		/* translated */
		long	d_rs;
	} d_dg;
	unsigned int	d_lba	: 1;
	unsigned int	d_dosc	: 1;	/* dos compatible? (g_c < 1024) */
	dos_part_table	d_pt;		/* table of primary partitions */
	dos_part_table	d_gpt;		/* guessed ptbl */
	dos_guessed_pt	*d_gl;		/* list of gathered guesses */
} disk_desc;


struct disk_geom *disk_geometry(disk_desc *);
int reread_partition_table(int);

#define s2mb(d,s)	{ (s)*=(d)->d_ssize; (s)/=1024; (s)/=1024; }
#define align(b,s)	(byte_t *)(((size_t)(b)+(s)-1)&~((s)-1))

#include "gmodules.h"


#endif /* _GPART_H */
