/*
 * gm_lvm2.c -- gpart Linux LVM2 physical volume guessing module
 *
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   20.11.2015 <mwilck@arcor.de>
 *
 */

#include <string.h>
#include <errno.h>
#include "gpart.h"
#include "gm_lvm2.h"


int lvm2_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);

	m->m_desc = "Linux LVM2 physical volume";
	return SECTOR_SIZE + LABEL_SIZE;
}



int lvm2_term(disk_desc *d)
{
	return (1);
}



int lvm2_gfun(disk_desc *d,g_module *m)
{
	struct label_header *lh;
	struct pv_header *pvh;
	dos_part_entry *pt = &m->m_part;
	s64_t pv_size;
	byte_t *p = d->d_sbuf + SECTOR_SIZE;

	m->m_guess = GM_NO;
	lh = (struct label_header*)p;
	if (strncmp((char*)lh->id, LABEL_ID, sizeof(lh->id)) ||
	    strncmp((char*)lh->type, LVM2_LABEL, sizeof(lh->type)))
		return 1;

	pvh = (struct pv_header*) ((char*)lh + le32toh(lh->offset_xl));
	pv_size = le64toh(pvh->device_size_xl);
	pv_size /=  d->d_ssize;
	if (d->d_nsecs != 0 && pv_size > d->d_nsecs - d->d_nsb)
		return 1;

	m->m_guess = GM_YES;
	pt->p_start = d->d_nsb;
	pt->p_size = pv_size;
	pt->p_typ = 0x8E;

	return 1;
}
