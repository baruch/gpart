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
#include <sys/param.h>
#include <sys/disklabel.h>
#include <sys/disk.h>
#endif

#include <unistd.h>

static void geometry_from_num_sectors(struct disk_geom *g, uint64_t nsects)
{
	const uint64_t lba = nsects - 1;

	g->d_h = (lba / 63) % 255;
	g->d_s = lba % 63 + 1;
	g->d_c = lba / (255 * 63);
	g->d_nsecs = nsects;
}

#if defined(__linux__)
static void os_disk_geometry(disk_desc *d, struct disk_geom *g)
{
	struct hd_geometry hg;
	uint64_t nsects;

	if (ioctl(d->d_fd, HDIO_GETGEO, &hg) == -1)
		pr(FATAL, EM_IOCTLFAILED, "HDIO_GETGEO", strerror(errno));
	else {
		g->d_h = hg.heads;
		g->d_s = hg.sectors;
		g->d_c = hg.cylinders;
	}
#ifdef BLKGETSIZE
	if (ioctl(d->d_fd, BLKGETSIZE, &nsects) == -1)
		pr(FATAL, EM_IOCTLFAILED, "BLKGETSIZE", strerror(errno));
	else {
		if (hg.heads && hg.sectors)
			g->d_c = nsects / (hg.heads * hg.sectors);
		else
			geometry_from_num_sectors(g, nsects);
	}
#endif
}
#elif defined(__FreeBSD__)
static void os_disk_geometry(disk_desc *d, struct disk_geom *g)
{
	struct disklabel dl;
	struct disklabel loclab;
	u_int u;
	off_t o; /* total disk size */

	if (ioctl(d->d_fd, DIOCGFWSECTORS, &u) == 0)
		g.d_s = u;
	else
		pr(FATAL, EM_IOCTLFAILED, "DIOCGFWSECTORS", strerror(errno));
	// loclab.d_nsectors = 63;
	if (ioctl(d->d_fd, DIOCGFWHEADS, &u) == 0)
		g.d_h = u;
	else
		pr(FATAL, EM_IOCTLFAILED, "DIOCGFWHEADS", strerror(errno));
	if (ioctl(d->d_fd, DIOCGSECTORSIZE, &u) == 0)
		if (u != 512)
			pr(FATAL, "sector size not a multiple of 512");
	if (ioctl(d->d_fd, DIOCGMEDIASIZE, &o))
		pr(FATAL, EM_IOCTLFAILED, "DIOCGMEDIASIZE", strerror(errno));

	g.d_nsecs = o / u;
	g.d_c = g.d_nsecs / g.d_h / g.d_s;
}
#else
#error Only Linux and FreeBSD supported
#endif

/*
 * get disk geometry. The medium is opened for reading,
 * descriptor in d_fd.
 */

struct disk_geom *disk_geometry(disk_desc *d)
{
	static struct disk_geom g;
	uint64_t nsects;
	struct stat st;
	int ret;

	memset(&g, 0, sizeof(g));

	ret = stat(d->d_dev, &st);
	if (ret == 0 && S_ISREG(st.st_mode)) {
		// We have something, we'll use it for a first fill of the data
		nsects = st.st_size / 512;
		if (nsects == 0)
			pr(FATAL, EM_FATALERROR, "Not a block device image file");
		geometry_from_num_sectors(&g, nsects);
		return (&g);
	}

	os_disk_geometry(d, &g);
	return (&g);
}

/*
 * tell the OS to reread a changed partition table. Do
 * nothing if there is no such possibility.
 */

int reread_partition_table(int fd)
{
#if defined(__linux__) && defined(BLKRRPART)
	if (ioctl(fd, BLKRRPART) == -1) {
		pr(ERROR, "rereading partition table: %s", strerror(errno));
		return (0);
	}
#endif

	return (1);
}
