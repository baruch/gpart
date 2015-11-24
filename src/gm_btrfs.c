/*
 * gm_btrfs.c -- gpart Linux Btrfs volume guessing module
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
#include <endian.h>
#include "gpart.h"
#include "gm_btrfs.h"


int btrfs_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);

	m->m_desc = "Btrfs volume";
	return BTRFS_SUPER_INFO_OFFSET + BTRFS_SUPER_INFO_SIZE;
}

int btrfs_term(disk_desc *d)
{
	return (1);
}

int btrfs_gfun(disk_desc *d,g_module *m)
{
	struct btrfs_super_block *sb;
	dos_part_entry *pt = &m->m_part;
	s64_t psize;

	m->m_guess = GM_NO;
	sb = (struct btrfs_super_block*)
		(d->d_sbuf + BTRFS_SUPER_INFO_OFFSET);

	if (le64toh(sb->magic) != BTRFS_MAGIC)
		return 1;

	if (memcmp(sb->fsid, sb->dev_item.fsid, BTRFS_FSID_SIZE))
		return 1;

	psize = le64toh(sb->dev_item.total_bytes);
	if (psize > btrfs_sb_offset(1)) {
		struct btrfs_super_block sb_copy;
		if (l64seek(d->d_fd,
			    d->d_nsb * d->d_ssize + btrfs_sb_offset(1),
			    SEEK_SET) == -1)
			pr(FATAL,"btrfs: cannot seek: %s", strerror(errno));
		read(d->d_fd, &sb_copy, sizeof(sb_copy));
		if (le64toh(sb_copy.magic) != BTRFS_MAGIC ||
		    memcmp(sb->fsid, sb_copy.fsid, BTRFS_FSID_SIZE)) {
			pr(MSG,"btrfs: superblock copy mismatch\n");
			return 1;
		}
	}

	m->m_guess = GM_YES;
	pt->p_start = d->d_nsb;
	pt->p_size = psize / d->d_ssize;
	pt->p_typ = 0x83;

	return 1;
}
