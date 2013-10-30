/*      
 * gm_lswap.c -- gpart linux swap guessing module
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
 * Modified:  22.01.1999 <mb@ichabod.han.de>
 *            Calculation of old swap-partition was wrong.
 *
 */

#include <string.h>
#include "gpart.h"


static const char	rcsid[] = "$Id: gm_lswap.c,v 1.8 2001/02/07 18:08:08 mb Exp mb $";
static char		*sigs[] = { "SWAP-SPACE", "SWAPSPACE2" };
static int		pszs[] = { 4096, 8192 };
static int		siglen = 10;



int lswap_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);

	m->m_desc = "Linux swap";

	/*
	 * return the max. pagesize of platforms running Linux.
	 * Seems to be 8k (Alpha).
	 */

	return (8192);
}



int lswap_term(disk_desc *d)
{
        return (1);
}



int lswap_gfun(disk_desc *d,g_module *m)
{
	char		*sig = 0;
	int		i, j, pagesize, vers;
	byte_t		*p, b;
	s64_t		np = 0;
	dos_part_entry	*pt = &m->m_part;

	m->m_guess = GM_NO; pagesize = vers = 0;
	for (i = 0; (pagesize == 0) && (i < sizeof(sigs)/sizeof(char *)); i++)
		for (j = 0; j < sizeof(pszs)/sizeof(int); j++)
		{
			sig = (char *)(d->d_sbuf + pszs[j] - siglen);
			if (strncmp(sig,sigs[i],siglen) == 0)
			{
				pagesize = pszs[j]; vers = i;
				break;
			}
		}

	if (pagesize == 0)
		return (1);

	if (vers == 0) /* old (<128mb) style swap */
	{
		if (*d->d_sbuf != 0xFE)
			return (1);

		for (p = (byte_t *)(sig - 1); p >= d->d_sbuf; p--)
			if (*p)
				break;
		np = (p - d->d_sbuf) * 8;
		for (b = *p; (b & 0x01) == 1; b >>= 1)
			np++;
	}
	else if (vers == 1) /* Linux > 2.2.X swap partitions */
	{
		struct swapinfo
		{
			char		bootbits[1024];
			unsigned int	version;
			unsigned int	last_page;
			unsigned int	nr_badpages;
			unsigned int	padding[125];
			unsigned int	badpages[1];
		} *info = (struct swapinfo *)d->d_sbuf;

		if (info->version != 1)
			return (1);
		np = 1 + info->last_page;
	}
	else
		return (1);

	if (np >= 10) /* mkswap(8) says this */
	{
		np *= pagesize; np /= d->d_ssize;
		m->m_guess = GM_YES; pt->p_typ = 0x82;
		pt->p_start = d->d_nsb; pt->p_size = np;
	}
	return (1);
}
