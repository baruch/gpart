/*      
 * gm_hpfs.c -- gpart hpfs guessing module
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
 * Modified:  29.06.1999 <mb@ichabod.han.de>
 *            Made every disk read/write buffer aligned to pagesize.
 *
 *
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "gpart.h"
#include "gm_hpfs.h"

static const char	rcsid[] = "$Id: gm_hpfs.c,v 1.8 2001/02/07 18:08:08 mb Exp mb $";

#define OS2SECTSIZE		512



int hpfs_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);

	m->m_desc = "OS/2 HPFS";
	return (OS2SECTSIZE);
}



int hpfs_term(disk_desc *d)
{
        return (1);
}



int hpfs_gfun(disk_desc *d,g_module *m)
{
	struct hpfs_boot_block	*bb = (struct hpfs_boot_block *)d->d_sbuf;
	struct hpfs_super_block	*sb;
	s64_t			s;
	size_t			psize;
	byte_t			*ubuf, *sbuf;

	m->m_guess = GM_NO;
	if (	(bb->sig_28h == 0x28) &&
		(strncmp((char *)bb->sig_hpfs,"HPFS    ",8) == 0) &&
		(bb->magic == le16(0xaa55)) &&
		(*(unsigned short *)bb->bytes_per_sector == le16(OS2SECTSIZE)))
	{
		/*
		 * looks like a hpfs boot sector. Test hpfs superblock
		 * at sector offset 16 (from start of partition).
		 */

		if ((s = l64tell(d->d_fd)) == -1)
			pr(FATAL,"hpfs: cannot seek: %s",strerror(errno));
		s /= d->d_ssize; s -= d->d_nsb; s *= d->d_ssize;
		if (l64seek(d->d_fd,16 * OS2SECTSIZE - s,SEEK_CUR) == -1)
			pr(FATAL,"hpfs: cannot seek: %s",strerror(errno));

		psize = getpagesize();
		ubuf = alloc(OS2SECTSIZE + psize);
		sbuf = align(ubuf,psize);
		if (read(d->d_fd,sbuf,OS2SECTSIZE) != OS2SECTSIZE)
			pr(FATAL,"hpfs: cannot read super block");
		sb = (struct hpfs_super_block *)sbuf;
		if (sb->magic != le32(SB_MAGIC))
			goto out;

		/*
		 * ok, fill in sizes.
		 */

		s = sb->n_sectors;
		s *= OS2SECTSIZE;
		s /= d->d_ssize;
		m->m_part.p_start = d->d_nsb;
		m->m_part.p_size = s;
		m->m_guess = GM_YES;
out:
		free((void *)ubuf);
	}
	return (1);
}
