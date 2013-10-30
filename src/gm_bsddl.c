/*      
 * gm_bsddl.c -- gpart BSD disk label guessing module
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
#include "gm_bsddl.h"

static const char	rcsid[] = "$Id: gm_bsddl.c,v 1.6 2001/02/07 18:08:08 mb Exp mb $";


int bsddl_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);
	m->m_desc = "*BSD disklabel";
	m->m_hasptbl = 1; m->m_notinext = 1;
	return (BBSIZE);
}



int bsddl_term(disk_desc *d)
{
	return (1);
}



int bsddl_gfun(disk_desc *d,g_module *m)
{
	struct disklabel	*dl;
	struct partition	*bsdpt;
	unsigned short		*cp, *ep, cs1, cs2;

	m->m_guess = GM_NO;
	dl = (struct disklabel *)(d->d_sbuf + LABELSECTOR * d->d_ssize);
	if ((dl->d_magic == le32(DISKMAGIC)) && (dl->d_magic2 == le32(DISKMAGIC)))
	{
		/*
		 * partition RAW_PART(2) usually denotes the whole disk (slice)
		 */

		if (dl->d_npartitions <= RAW_PART)
			return (1);
		bsdpt = &dl->d_partitions[RAW_PART];

		/*
		 * some sanity checks: disklabel checksum and start
		 * of partition.
		 */

		cs1 = 0; cs2 = le16(dl->d_checksum); dl->d_checksum = 0;
		cp = (unsigned short *)dl;
		ep = (unsigned short *)&dl->d_partitions[dl->d_npartitions];
		for ( ; cp < ep; cp++)
			cs1 ^= le16(*cp);
		dl->d_checksum = le16(cs2);

		if ((le32(bsdpt->p_offset) == d->d_nsb) && (cs1 == cs2))
		{
			m->m_part.p_typ = 0xA5;
			m->m_part.p_start = le32(bsdpt->p_offset);
			m->m_part.p_size = le32(bsdpt->p_size);
			m->m_guess = GM_YES;
		}
	}
	return (1);
}
