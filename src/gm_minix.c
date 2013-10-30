/*      
 * gm_minix.c -- gpart minix guessing module
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
#include "gm_minix.h"

static const char	rcsid[] = "$Id: gm_minix.c,v 1.5 2001/02/07 18:08:08 mb Exp mb $";


int minix_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);
	m->m_desc = "Minix filesystem";
	return (2 * BLOCK_SIZE);
}



int minix_term(disk_desc *d)
{
        return (1);
}



int minix_gfun(disk_desc *d,g_module *m)
{
	long				version = 0;
	struct minix_super_block	*ms;
	byte_t				*p;
	unsigned long			zones, size;

	ms = (struct minix_super_block *)(d->d_sbuf + BLOCK_SIZE);
	m->m_guess = GM_NO;

	if (ms->s_magic == le16(MINIX_SUPER_MAGIC))
		version = MINIX_V1;
	if (ms->s_magic == le16(MINIX_SUPER_MAGIC2))
		version = MINIX_V1;
	if (ms->s_magic == le16(MINIX2_SUPER_MAGIC))
		version = MINIX_V2;
	if (ms->s_magic == le16(MINIX2_SUPER_MAGIC2))
		version = MINIX_V2;

	if (version == 0)
		return (1);
	if ((ms->s_state != MINIX_VALID_FS) && (ms->s_state != MINIX_ERROR_FS))
		return (1);

	/*
	 * the rest of the disk block where the superblock
	 * was found should be zeroed out.
	 */

	for (p = d->d_sbuf + BLOCK_SIZE + sizeof(struct minix_super_block);
		p < d->d_sbuf + 2 * BLOCK_SIZE; p++)
		if (*p)
			return (1);

	zones = (version == MINIX_V2) ? ms->s_zones : ms->s_nzones;
	size = zones << ms->s_log_zone_size; size *= BLOCK_SIZE;

        m->m_guess = GM_YES;
        m->m_part.p_typ = (version == MINIX_V2) ? 0x81 : 0x80;
        m->m_part.p_start = d->d_nsb;
        m->m_part.p_size = size / d->d_ssize;

	return (1);
}
