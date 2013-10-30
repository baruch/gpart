/*      
 * gm_ntfs.c -- gpart ntfs guessing module
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
 * Modified:  26.02.2000 <mb@ichabod.han.de>
 *            Length of guessed partition incremented to include NT4 backup
 *            boot sector.
 *
 */

#include <stdlib.h>
#include <string.h>
#include "gpart.h"
#include "gm_ntfs.h"

static const char	rcsid[] = "$Id: gm_ntfs.c,v 1.6 2001/02/07 18:08:08 mb Exp mb $";

#define NTFS_SECTSIZE	512

int ntfs_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);

	m->m_desc = "Windows NT/W2K FS";
	m->m_hasptbl = 1;
	return (NTFS_SECTSIZE); /* The ntfs driver in Linux just assumes so */
}



int ntfs_term(disk_desc *d)
{
        return (1);
}



int ntfs_gfun(disk_desc *d,g_module *m)
{
	int	blocksize, clusterfactor, clustersize;
	int	mft_clusters_per_record;
	s64_t	size, ls;
        byte_t	*ubuf, *sbuf;


	m->m_guess = GM_NO;
	if (IS_NTFS_VOLUME(d->d_sbuf))
	{
		/*
		 * ntfs detection is quite weak, should come before
		 * fat or hpfs.
		 */

		if (NTFS_GETU32(d->d_sbuf + 0x40) > 256UL)
			return (1);
		if (NTFS_GETU32(d->d_sbuf + 0x44) > 256UL)
			return (1);

		blocksize = NTFS_GETU16(d->d_sbuf + 0x0B);
		clusterfactor = NTFS_GETU8(d->d_sbuf + 0x0D);
		clustersize = blocksize * clusterfactor;
		mft_clusters_per_record = NTFS_GETS8(d->d_sbuf + 0x40);
		if ((mft_clusters_per_record < 0) && (mft_clusters_per_record != -10))
			return (1);
		size = NTFS_GETU64(d->d_sbuf + 0x28);

		size /= clusterfactor;
		size *= clustersize;
		size /= d->d_ssize;

		/*
		 * look for an additional backup boot sector at the end of
		 * this FS (NT4 puts this backup sector after the FS, this
		 * sector must be counted).
		 */

		ls = d->d_nsb + size; ls *= d->d_ssize;
		if (l64seek(d->d_fd,ls,SEEK_SET) >= 0)
		{
			ubuf = alloc(NTFS_SECTSIZE + getpagesize());
			sbuf = align(ubuf,getpagesize());
			if (read(d->d_fd,sbuf,NTFS_SECTSIZE) != NTFS_SECTSIZE)
				pr(FATAL,"ntfs: cannot read backup boot sector");
			if (memcmp(d->d_sbuf,sbuf,NTFS_SECTSIZE) == 0)
				size += 1;
			free((void *)ubuf);
		}

		m->m_part.p_start = d->d_nsb;
		m->m_part.p_size = (unsigned long)size;
		m->m_part.p_typ = 0x07;
		m->m_guess = GM_YES;
	}
	return (1);
}
