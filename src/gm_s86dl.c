/*      
 * gm_s86dl.c -- gpart solaris/x86 disklabel guessing module
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
#include "gm_s86dl.h"

static const char	rcsid[] = "$Id: gm_s86dl.c,v 1.6 2001/02/07 18:08:08 mb Exp mb $";


int s86dl_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);
	m->m_desc = "Solaris/x86 disklabel";
	m->m_notinext = 1;
	return (512 + sizeof(struct solaris_x86_vtoc));
}



int s86dl_term(disk_desc *d)
{
        return (1);
}



int s86dl_gfun(disk_desc *d,g_module *m)
{
	struct solaris_x86_vtoc		*svtoc;
	struct solaris_x86_slice	*ws = 0, *rs = 0;
	int				i;

	m->m_guess = GM_NO;
	svtoc = (struct solaris_x86_vtoc *)(d->d_sbuf + 512);
	if ((svtoc->v_sanity != SOLARIS_X86_VTOC_SANE) ||
	    (svtoc->v_version != SOLARIS_X86_V_VERSION))
		return (1);

	for (i = 0; i < SOLARIS_X86_NUMSLICE; i++)
		switch (svtoc->v_slice[i].s_tag)
		{
			case SOLARIS_X86_V_ROOT :
				rs = &svtoc->v_slice[i];
				break;
			case SOLARIS_X86_V_BACKUP :
				ws = &svtoc->v_slice[i];
				break;
		}

	/*
	 * some simple sanity checks.
	 */

	if ((ws == 0) || (rs == 0))
		return (1);
	if (svtoc->v_sectorsz != d->d_ssize)
		return (1);
	if (d->d_nsb + ws->s_start + ws->s_size > d->d_nsecs)
		return (1);
	if (d->d_nsb + rs->s_start + rs->s_size > d->d_nsecs)
		return (1);
	if ((rs->s_start < ws->s_start) || (rs->s_size > ws->s_size))
		return (1);
	if (ws->s_flag && (ws->s_flag != SOLARIS_X86_V_UNMNT) &&
	    (ws->s_flag != SOLARIS_X86_V_RONLY))
		return (1);
	if (rs->s_flag && (rs->s_flag != SOLARIS_X86_V_UNMNT) &&
	    (rs->s_flag != SOLARIS_X86_V_RONLY))
		return (1);

	/*
	 * If the recognition of the solaris vtoc isn't
	 * enough, I'll have to read a ufs sb, but for
	 * now the vtoc must suffice.
	 */

	m->m_part.p_typ = 0x82;
	m->m_part.p_start = d->d_nsb + le32(ws->s_start);
	m->m_part.p_size = le32(ws->s_size);
	m->m_guess = GM_YES;

	return (1);
}
