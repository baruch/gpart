/*      
 * gm_fat.c -- gpart fat guessing module
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

#include "gpart.h"
#include "gm_fat.h"

static const char	rcsid[] = "$Id: gm_fat.c,v 1.6 2001/02/07 18:08:08 mb Exp mb $";


int fat_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);
	m->m_desc = "DOS FAT";
	m->m_align = 'h';
	return (sizeof(struct fat_boot_sector));
}



int fat_term(disk_desc *d)
{
        return (1);
}



int fat_gfun(disk_desc *d,g_module *m)
{
	struct fat_boot_sector	*sb = (struct fat_boot_sector *)d->d_sbuf;
	dos_part_entry		*pt = &m->m_part;
	unsigned long		nsecs = 0;
	unsigned char		ig1, ig2, media;
	int			sectsize, fat32 = 0, fat12 = 0;
	s64_t			size = 0;

	m->m_guess = GM_NO;
	ig1 = sb->ignored[0]; ig2 = sb->ignored[2]; media = sb->media;
	if ((ig1 == 0xeb) && (ig2 == 0x90) && ((media == 0xf8) || (media == 0xfc)))
	{
		if (*((unsigned short *)d->d_sbuf + 255) != le16(DOSPTMAGIC))
			return (1);

		/*
		 * looks like a standard FAT boot sector. Now find out,
		 * which one of the numerous versions this could be.
		 */

		pt->p_start = d->d_nsb;
		nsecs = le16(*(unsigned short *)sb->sectors);
		if (nsecs == 0)
			nsecs = le32(sb->total_sect);
		if (nsecs == 0)
			return (1);
		sectsize = le16(*(unsigned short *)sb->sector_size);
		if ((d->d_sbuf[0x39] == '1') && (d->d_sbuf[0x3a] == '2'))
			fat12 = 1;
		if (sb->fat_length == 0)
			fat32 = 1;
		if (fat32 && (*(d->d_sbuf + 0x26) == 0x29))
			return (1);
		if (!fat32 && (*(d->d_sbuf + 0x26) != 0x29))
			return (1);
		if (fat12 && fat32)
			return (1);
		m->m_guess = GM_YES;


		/*
		 * what happens when the fat sectsize != medium sectsize?
		 * I don't know. I just say no now.
		 */

		if (sectsize != d->d_ssize)
			m->m_guess = GM_NO;
		size = nsecs; size *= sectsize; size /= 1024;
		if (size >= 32768)
		{
			pt->p_typ = 0x06;
			if (fat32)
				pt->p_typ = d->d_lba ? 0x0C : 0x0B;
		}
		else
			pt->p_typ = fat12 ? 0x01 : 0x04;
		pt->p_size = nsecs;
	}
	return (1);
}
