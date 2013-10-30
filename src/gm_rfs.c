/*      
 * gm_rfs.c -- gpart ReiserFS guessing module
 *
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   21.01.1999 <mb@ichabod.han.de>
 * Modified:  26.12.2000 Francis Devereux <francis@devereux.tc>
 *            Added reiserfs 3.5.28 support.
 *
 */

#include <string.h>
#include <errno.h>
#include "gpart.h"
#include "gm_rfs.h"

static const char	rcsid[] = "$Id: gm_rfs.c,v 1.5 2001/02/07 18:08:08 mb Exp mb $";


int rfs_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);

	m->m_desc = "Reiser filesystem";
	return (REISERFS_FIRST_BLOCK * 1024 + SB_SIZE);
}



int rfs_term(disk_desc *d)
{
	return (1);
}



int rfs_gfun(disk_desc *d,g_module *m)
{
	struct reiserfs_super_block	*sb;
	dos_part_entry			*pt = &m->m_part;
	s64_t				size;

	m->m_guess = GM_NO;
	sb = (struct reiserfs_super_block *)(d->d_sbuf + REISERFS_FIRST_BLOCK * 1024);
	if (strncmp(sb->s_magic,REISERFS_SUPER_MAGIC,12) == 0)
	{
		/*
		 * sanity checks.
		 */

		if (sb->s_block_count < sb->s_free_blocks)
			return (1);

		if (sb->s_block_count < REISERFS_MIN_BLOCK_AMOUNT)
			return (1);

		if ((sb->s_state != REISERFS_VALID_FS) &&
		    (sb->s_state != REISERFS_ERROR_FS))
			return (1);

		if (sb->s_oid_maxsize % 2) /* must be even */
			return (1);

		if (sb->s_oid_maxsize < sb->s_oid_cursize)
			return (1);

		if ((sb->s_blocksize != 4096) && (sb->s_blocksize != 8192))
			return (1);

		/*
		 * ok.
		 */

		m->m_guess = GM_YES;
		pt->p_start = d->d_nsb;
		size = sb->s_block_count; size *= sb->s_blocksize; size /= d->d_ssize;
		pt->p_size = (unsigned long)size;
		pt->p_typ = 0x83;
	}
	return (1);
}
