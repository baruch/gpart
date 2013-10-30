/*      
 * gm_qnx4.c -- gpart qnx4 guessing module
 *
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   04.01.2001 <mb@ichabod.han.de>
 * Modified:
 *
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "gpart.h"
#include "gm_qnx4.h"

#include <stdio.h>

static const char	rcsid[] = "$Id: gm_qnx4.c,v 1.1 2001/01/29 17:11:34 mb Exp mb $";


int qnx4_init(disk_desc *d,g_module *m)
{
	if ((d == 0) || (m == 0))
		return (0);
	m->m_desc = "QNX4 filesystem";
	m->m_notinext = 1;
	return (2 * QNX4_BLOCK_SIZE);
}



int qnx4_term(disk_desc *d)
{
        return (1);
}



int qnx4_gfun(disk_desc *d,g_module *m)
{
	struct qnx4_super_block	*sb;
	struct qnx4_inode_entry	*rootdir, bitmap;
	int			rd, rl, i, j, psize, found;
	s64_t			ls, ofs, size;
	byte_t			*ubuf, *sbuf;

	m->m_guess = GM_NO;

	/*
	 * check QNX signature
	 */

	if (memcmp(d->d_sbuf + 4,QNX4_BOOTSECT_SIG,strlen(QNX4_BOOTSECT_SIG)))
		return (1);
	sb = (struct qnx4_super_block *)(d->d_sbuf + QNX4_BLOCK_SIZE);
	if (*sb->RootDir.di_fname != '/')
		return (1);

	/*
	 * read root directory
	 */

	psize = getpagesize();
	ubuf = alloc(QNX4_BLOCK_SIZE + psize);
	sbuf = align(ubuf,psize);

	found = 0;

	rd = le32(sb->RootDir.di_first_xtnt.xtnt_blk) - 1;
	rl = le32(sb->RootDir.di_first_xtnt.xtnt_size);

	for (j = 0; j < rl; j++)
	{
		if ((ls = l64tell(d->d_fd)) == -1)
			pr(FATAL,"qnx4: cannot seek: %s",strerror(errno));
		ls /= d->d_ssize; ls -= d->d_nsb; ls *= d->d_ssize;
		ofs = rd + j; ofs *= QNX4_BLOCK_SIZE;
		if (l64seek(d->d_fd,ofs - ls,SEEK_CUR) == -1)
			pr(FATAL,"qnx4: cannot seek: %s",strerror(errno));
		if (read(d->d_fd,sbuf,QNX4_BLOCK_SIZE) != QNX4_BLOCK_SIZE)
			pr(FATAL,"qnx4: cannot read root dir entry");

		/*
		 * find the ".bitmap" entry
		 */

		for (i = 0; i < QNX4_INODES_PER_BLOCK; i++)
		{
			rootdir = (struct qnx4_inode_entry *) (sbuf + i * QNX4_DIR_ENTRY_SIZE);
			if (rootdir->di_fname && !strncmp(rootdir->di_fname,QNX4_BITMAP_NAME,strlen(QNX4_BITMAP_NAME)))
			{
				memcpy(&bitmap,rootdir,sizeof(struct qnx4_inode_entry));
				found = 1;
			}
		}
	}

	if (! found)
		return (1);

	size = le32(bitmap.di_size) * 8 - 6;
	size *= QNX4_BLOCK_SIZE;
	size /= d->d_ssize;

	m->m_guess = GM_YES;
	m->m_part.p_typ = 0x4F;
	m->m_part.p_start = d->d_nsb;
	m->m_part.p_size = size;

	free((void *)ubuf);
	return (1);
}
