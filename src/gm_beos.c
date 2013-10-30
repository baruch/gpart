/*      
 * gm_beos.c -- gpart BeOS filesystem guessing module
 *
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   29.01.2001 <mb@ichabod.han.de>
 * Modified:
 *
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "gpart.h"
#include "gm_beos.h"

#include <stdio.h>

static const char	rcsid[] = "$Id: gm_beos.c,v 1.2 2001/02/07 18:08:08 mb Exp mb $";


int beos_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);
	m->m_desc = "BeOS filesystem";
	return (2 * 512);
}



int beos_term(disk_desc *d)
{
        return (1);
}



int beos_gfun(disk_desc *d,g_module *m)
{
	beos_super_block	*sb;
	s64_t			size;

	m->m_guess = GM_NO;

	/*
	 * BeOS superblock without little/big endian conversions
	 */

	sb = (beos_super_block *)(d->d_sbuf + 512);
	if ((sb->magic1 != BEOS_SUPER_BLOCK_MAGIC1) ||
	    (sb->magic2 != BEOS_SUPER_BLOCK_MAGIC2) ||
	    (sb->magic3 != BEOS_SUPER_BLOCK_MAGIC3))
		return (1);

	/*
	 * some consistency checks
	 */

	if ((sb->block_size != 1024) && (sb->block_size != 2048) &&
	    (sb->block_size != 4096) && (sb->block_size != 8192))
		return (1);

	if (sb->block_size != 1 << sb->block_shift)
		return (1);

	if (sb->num_blocks < sb->used_blocks)
		return (1);

	if ((sb->flags != BEOS_CLEAN) && (sb->flags != BEOS_DIRTY))
		return (1);

	/*
	 * I hope this is enough, if not I have to read the root dir
	 * as well later.
	 */

	size = sb->num_blocks; size *= sb->block_size; size /= d->d_ssize;
	m->m_guess = GM_YES;
	m->m_part.p_typ = 0xEB;
	m->m_part.p_start = d->d_nsb;
	m->m_part.p_size = (unsigned long)size;

	return (1);
}
