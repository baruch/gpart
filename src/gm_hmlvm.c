/*      
 * gm_hmlvm.c -- gpart Linux LVM physical volume guessing module
 *
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   29.08.1999 <mb@ichabod.han.de>
 * Modified:  29.01.2001 <mb@ichabod.han.de>
 *            Minor update to LVM 0.9.
 *
 */

#include <string.h>
#include <errno.h>
#include "gpart.h"
#include "gm_hmlvm.h"

static const char	rcsid[] = "$Id: gm_hmlvm.c,v 1.3 2001/02/07 18:08:08 mb Exp mb $";


int hmlvm_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);

	m->m_desc = "Linux LVM physical volume";
	return (LVM_PV_DISK_BASE + LVM_PV_DISK_SIZE);
}



int hmlvm_term(disk_desc *d)
{
	return (1);
}



int hmlvm_gfun(disk_desc *d,g_module *m)
{
	pv_disk_t		*pv;
	dos_part_entry		*pt = &m->m_part;
	unsigned int		size;
	s64_t			s;

	m->m_guess = GM_NO;
	pv = (pv_disk_t *)&d->d_sbuf[LVM_PV_DISK_BASE];
	if ((strncmp((char *)pv->id,LVM_ID,sizeof(pv->id)) == 0) &&
	    ((pv->version == 1) || (pv->version == 2)))
	{
		/*
		 * looks like a physical volume header. Do the usual
		 * consistency checks.
		 */

		if (pv->pv_size > LVM_MAX_SIZE)
			return (1);
		if ((pv->pv_status != 0) && (pv->pv_status != PV_ACTIVE))
			return (1);
		if ((pv->pv_allocatable != 0) && (pv->pv_allocatable != PV_ALLOCATABLE))
			return (1);
		if (pv->lv_cur > MAX_LV)
			return (1);
		if (strlen((char *)pv->vg_name) > NAME_LEN / 2)
			return (1);

		size = pv->pe_size / LVM_MIN_PE_SIZE * LVM_MIN_PE_SIZE;
		if ((pv->pe_size != size) ||
		    (pv->pe_size < LVM_MIN_PE_SIZE) ||
		    (pv->pe_size > LVM_MAX_PE_SIZE))
			return (1);

		if (pv->pe_total > ( pv->pe_on_disk.size / sizeof ( disk_pe_t)))
			return (1);
		if (pv->pe_allocated > pv->pe_total)
			return (1);

		/*
		 * Ok.
		 */

		m->m_guess = GM_YES;
		pt->p_start = d->d_nsb;
		s = pv->pv_size; s *= 512; s /= d->d_ssize;
		pt->p_size = s;
		pt->p_typ = 0x8E;
	}

	return (1);
}
