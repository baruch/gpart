/*
 * disku.c -- gpart disk util routines
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
 * Modified:  13.12.2000 <mb@ichabod.han.de>
 *            Calculation of disk cylinder count changed for Linux.
 *
 */


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "gpart.h"

#if defined(__linux__)
#include <sys/mount.h>
#include <linux/hdreg.h>
#endif

#if defined(__FreeBSD__)
#include <sys/disklabel.h>
#endif

#include <unistd.h>


/*
 * get disk geometry. The medium is opened for reading,
 * descriptor in d_fd.
 */

struct disk_geom *disk_geometry(disk_desc *d)
{
	static struct disk_geom	g;
	long			nsects;

#if defined(__linux__)
	struct hd_geometry	hg;
#endif
#if defined(__FreeBSD__)
	struct disklabel	dl;
#endif

	struct stat st;
	int ret;
	long lba;
	ret = stat(d->d_dev, &st);
	if (ret == 0)
	{
		if (S_ISREG(st.st_mode))
		{
			nsects = st.st_size / 512;
			if (nsects == 0)
				pr(FATAL, EM_FATALERROR, "Not a block device image file");
			lba = nsects - 1;
			g.d_h = (lba / 63) % 255;
			g.d_s = lba % 63 + 1;
			g.d_c = lba / (255 * 63);
			return (&g);
		}
	}

#if defined(__linux__)
	if (ioctl(d->d_fd,HDIO_GETGEO,&hg) == -1)
		pr(FATAL,EM_IOCTLFAILED,"HDIO_GETGEO",strerror(errno));
#ifdef BLKGETSIZE
	if (ioctl(d->d_fd,BLKGETSIZE,&nsects) == -1)
		pr(FATAL,EM_IOCTLFAILED,"BLKGETSIZE",strerror(errno));
	g.d_c = nsects / (hg.heads * hg.sectors);
#else
	g.d_c = hg.cylinders;
#endif
	g.d_h = hg.heads;
	g.d_s = hg.sectors;

#endif

#if defined(__FreeBSD__)
	if (ioctl(d->d_fd,DIOCGDINFO,&dl) == -1)
		pr(FATAL,EM_IOCTLFAILED,"DIOCGDINFO",strerror(errno));
	g.d_c = dl.d_ncylinders;
	g.d_h = dl.d_ntracks;
	g.d_s = dl.d_nsectors;
#endif

	return (&g);
}


/*
 * tell the OS to reread a changed partition table. Do
 * nothing if there is no such possibility.
 */

int reread_partition_table(int fd)
{
#if defined(__linux__) && defined(BLKRRPART)
	if (ioctl(fd,BLKRRPART) == -1)
	{
		pr(ERROR,"rereading partition table: %s",strerror(errno));
		return (0);
	}
#endif

	return (1);
}
