/*
 * l64seek.c -- gpart signed 64bit seek
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

#include "config.h" /* for large file support */
#include "l64seek.h"


#define OSTACKLEN	16
static struct
{
	s64_t		fpos;
	int		fd;
} ostck[OSTACKLEN];
static int		osptr = -1;



off64_t l64seek(int fd,off64_t offset,int whence)
{
	off64_t		ret = (off64_t)-1;

	ret = lseek(fd,offset,whence);

	return (ret);
}



int l64opush(int fd)
{
	s64_t		fpos;

	if (osptr < OSTACKLEN - 1)
	{
		if ((fpos = l64tell(fd)) >= 0)
		{
			ostck[++osptr].fd = fd;
			ostck[osptr].fpos = fpos;
			return (1);
		}
	}
	return (0);
}


s64_t l64opop(int fd)
{
	if ((osptr >= 0) && (ostck[osptr].fd == fd))
		return (ostck[osptr--].fpos);
	return (-1);
}
