/*      
 * gm_ext2.c -- gpart ext2 guessing module
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
 * Modified:  30.04.1999 <mb@ichabod.han.de>
 *            Added suggestions from Andries.Brouwer@cwi.nl
 *
 *            18.06.1999 <mb@ichabod.han.de>
 *            Fixed buggy ext2 spare superblock location calculation.
 *
 *            29.06.1999 <mb@ichabod.han.de>
 *            Made every disk read/write buffer aligned to pagesize.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "gpart.h"
#include "gm_ext2.h"

static const char	rcsid[] = "$Id: gm_ext2.c,v 1.8 2001/02/07 18:08:08 mb Exp mb $";


int ext2_init(disk_desc *d,g_module *m)
{
	int		bsize = SUPERBLOCK_SIZE;

	if ((d == 0) || (m == 0))
		return (0);

	/*
	 * the medium sector size must either be a multiple
	 * of the superblock size or vice versa.
	 */

	if (((d->d_ssize > bsize) && (d->d_ssize % bsize)) ||
	    ((d->d_ssize < bsize) && (bsize % d->d_ssize)))
	{
		pr(ERROR,"ext2_init: cannot work on that sector size");
		return (0);
	}
	m->m_desc = "Linux ext2";
	return (SUPERBLOCK_OFFSET + SUPERBLOCK_SIZE);
}



int ext2_term(disk_desc *d)
{
	return (1);
}



int ext2_gfun(disk_desc *d,g_module *m)
{
	struct ext2fs_sb	*sb, *sparesb;
	int			psize, bsize = 1024;
	s64_t			ls, ofs;
	dos_part_entry		*pt = &m->m_part;
	byte_t			*ubuf, *sbuf;

	m->m_guess = GM_NO;
	sb = (struct ext2fs_sb *)(d->d_sbuf + SUPERBLOCK_OFFSET);
	if (sb->s_magic != le16(EXT2_SUPER_MAGIC))
		return (1);

	/*
	 * first some plausability checks.
	 */

	if (sb->s_free_blocks_count >= sb->s_blocks_count) return (1);
	if (sb->s_free_inodes_count >= sb->s_inodes_count) return (1);
	if (sb->s_errors &&
	    (sb->s_errors != EXT2_ERRORS_CONTINUE) &&
	    (sb->s_errors != EXT2_ERRORS_RO) &&
	    (sb->s_errors != EXT2_ERRORS_PANIC))
		return (1);
	if (sb->s_state & ~(EXT2_VALID_FS | EXT2_ERROR_FS))
		return (1);

	/*
	 * empty filesystems seem unlikely to me.
	 */

	if (sb->s_blocks_count == 0)
		return (1);

	/*
	 * yet they also shouldn't be too large.
	 */

	if (d->d_nsecs)
	{
		ls = sb->s_blocks_count; ls *= bsize;
		ls /= d->d_ssize; ls += d->d_nsb;
		if (ls > d->d_nsecs)
			return (1);
	}

	/*
	 * ext2fs supports 1024, 2048 and 4096b blocks.
	 */

	switch (sb->s_log_block_size)
	{
		case BSIZE_1024 : bsize = 1024; break;
		case BSIZE_2048 : bsize = 2048; break;
		case BSIZE_4096 : bsize = 4096; break;
		default:          return (1);
	}

	/*
	 * current mount count shouldn't be greater than max+20
	 */

	if (sb->s_mnt_count > sb->s_max_mnt_count + 20)
		return (1);

	/*
	 * up to here this looks like a valid ext2 sb, now try to read
	 * the first spare super block to be sure.
	 */

	if ((ls = l64tell(d->d_fd)) == -1)
		pr(FATAL,"ext2: cannot seek: %s",strerror(errno));
	ls /= d->d_ssize; ls -= d->d_nsb; ls *= d->d_ssize;
	ofs = sb->s_blocks_per_group + sb->s_first_data_block; ofs *= bsize;
	if (l64seek(d->d_fd,ofs - ls,SEEK_CUR) == -1)
		pr(FATAL,"ext2: cannot seek: %s",strerror(errno));

	psize = getpagesize();
	ubuf = alloc(SUPERBLOCK_SIZE + psize);
	sbuf = align(ubuf,psize);
	if (read(d->d_fd,sbuf,SUPERBLOCK_SIZE) != SUPERBLOCK_SIZE)
		pr(FATAL,"ext2: cannot read spare super block");
	sparesb = (struct ext2fs_sb *)sbuf;

	/*
	 * test only some values of the spare sb.
	 */

	if (sparesb->s_magic != le16(EXT2_SUPER_MAGIC))
		goto out;
	if (sparesb->s_log_block_size != sb->s_log_block_size)
		goto out;

	/*
	 * seems ok.
	 */

	m->m_guess = GM_YES;
	pt->p_typ = 0x83;
	pt->p_start = d->d_nsb;
	pt->p_size = bsize / d->d_ssize;
	pt->p_size *= sb->s_blocks_count;

out:
	free((void *)ubuf);
	return (1);
}
