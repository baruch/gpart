/*      
 * gm_xfs.c -- gpart SGI xfs guessing module
 *
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   30.01.2001 <mb@ichabod.han.de>
 * Modified:
 *
 */

#include <string.h>
#include "gpart.h"
#include "gm_xfs.h"

static const char	rcsid[] = "$Id: gm_xfs.c,v 1.1 2001/02/07 18:08:08 mb Exp mb $";


int xfs_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);

	m->m_desc = "SGI XFS filesystem";
	return (512);
}



int xfs_term(disk_desc *d)
{
	return (1);
}



int xfs_gfun(disk_desc *d,g_module *m)
{
	xfs_sb_t		*sb;
	s64_t			size;

	m->m_guess = GM_NO;
	sb = (xfs_sb_t *)d->d_sbuf;

	/*
	 * Sanity checks from xfs_mount.c
	 */

	if (be32(sb->sb_magicnum) != XFS_SB_MAGIC)
		return (1);

	if (be32(sb->sb_blocksize) != getpagesize())
		return (1);

	if ((sb->sb_imax_pct > 100) || (sb->sb_sectsize <= 0))
		return (1);

	if ((be16(sb->sb_inodesize) < XFS_DINODE_MIN_SIZE) ||
	    (be16(sb->sb_inodesize) > XFS_DINODE_MAX_SIZE))
		return (1);

	if (be32(sb->sb_blocksize) != 1 << sb->sb_blocklog)
		return (1);

	size = be64(sb->sb_logstart) ? (s64_t)be32(sb->sb_logblocks) : 0LL;
	size = be64(sb->sb_dblocks) - size;
	size *= be32(sb->sb_blocksize);
	size /= d->d_ssize;

	m->m_guess = GM_YES;
	m->m_part.p_start = d->d_nsb;
	m->m_part.p_size = (unsigned long)size;
	m->m_part.p_typ = 0x83;

	return (1);
}
