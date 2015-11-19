/*
 * gpart.c -- gpart main
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
 * Modified:  11.06.1999 <mb@ichabod.han.de>
 *            Handle disk read errors.
 *            Minor fixes.
 *
 *            29.06.1999 <mb@ichabod.han.de>
 *            Made every disk read/write buffer aligned to pagesize.
 *
 *            29.08.1999 <mb@ichabod.han.de>
 *            Default scan increment now 's'.
 *            Extended ptbl boundary condition now depends on scan
 *            increment.
 *
 *            26.02.2000 <mb@ichabod.han.de>
 *            Default scan increment 'h' again.
 *            Fixed faulty head boundary condition.
 *            Introduced ptbl entry editing after guess loop.
 *            First scanned sector is no of sects/head, if no start
 *            sector was given.
 *            m_notinext now honoured.
 *            Make a MBR backup.
 *            Interactive mode now somehow works.
 *
 *            14.05.2000 <mb@ichabod.han.de>
 *            Made writing of guessed table also aligned.
 *            Fixed stupid copy&paste bug in the check routine
 *            (found by Bruno Bozza <brunobozza@hotmail.com>.
 *
 *            29.01.2001 <mb@ichabod.han.de>
 *            Extended partition type on an LBA disk now 0x0f instead
 *            of 0x05. Changed some partition types (get_part_type).
 *            When comparing partition types in extptbl links, try
 *            to compare similarity, not equality (is_same_partition_type).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "gpart.h"

static const char *gpart_version = PACKAGE_NAME " v" VERSION;

int f_check = 0, f_verbose = 0, f_dontguess = 0, f_fast = 1;
int f_getgeom = 1, f_interactive = 0, f_quiet = 0, f_testext = 1;
int f_skiperrors = 1, berrno = 0;
int (*boundary_fun)(disk_desc *, s64_t);
unsigned long increment = 's', gc = 0, gh = 0, gs = 0;
s64_t skipsec = 0, maxsec = 0;
FILE *logfile = 0;

void usage()
{
	FILE *fp = stderr;

	fprintf(fp, "Usage: %s [options] device\n", PACKAGE_NAME);
	fprintf(fp, "Options: [-b <backup MBR>][-C c,h,s][-c][-d][-E][-e][-f][-g][-h][-i]\n");
	fprintf(fp, "         [-K <last sector>][-k <# of sectors>][-L][-l <log file>]\n");
	fprintf(fp, "         [-n <increment>][-q][-s <sector-size>]\n");
	fprintf(fp, "         [-V][-v][-W <device>][-w <module-name,weight>]\n");
	fprintf(fp, "%s (c) 1999-2001 Michail Brzitwa <michail@brzitwa.de>.\n", gpart_version);
	fprintf(fp, "Guess PC-type hard disk partitions.\n\n");
	fprintf(fp, "Options:\n");
	fprintf(fp, " -b  Save a backup of the original MBR to specified file.\n");
	fprintf(fp, " -C  Set c/h/s to be used in the scan.\n");
	fprintf(fp, " -c  Check/compare mode.\n");
	fprintf(fp, " -d  Do not start the guessing loop.\n");
	fprintf(fp, " -E  Do not try to identify extended partition tables.\n");
	fprintf(fp, " -e  Do not skip disk read errors.\n");
	fprintf(fp, " -f  Full scan.\n");
	fprintf(fp, " -g  Do not try to get the disk geometry.\n");
	fprintf(fp, " -h  Show this help.\n");
	fprintf(fp, " -i  Run interactively (ask for confirmation).\n");
	fprintf(fp, " -K  Scan only up to given sector.\n");
	fprintf(fp, " -k  Skip sectors before scan.\n");
	fprintf(fp, " -L  List available modules and their weights, then exit.\n");
	fprintf(fp, " -l  Logfile name.\n");
	fprintf(fp, " -n  Scan increment: number or 's' sector, 'h' head, 'c' cylinder.\n");
	fprintf(fp, " -q  Run quiet (however log file is written if specified).\n");
	fprintf(fp, " -s  Sector size to use (disable sector size probing).\n");
	fprintf(fp, " -V  Show version.\n");
	fprintf(fp, " -v  Verbose mode. Can be given more than once.\n");
	fprintf(fp, " -W  Write guessed primary partition table to given device or file.\n");
	fprintf(fp, " -w  Weight factor of module.\n");
	fprintf(fp, "\n");
}

void pr(int type, char *fmt, ...)
{
	va_list vl;
	static char msg[512];

	va_start(vl, fmt);
	vsnprintf(msg, 511, fmt, vl);
	va_end(vl);
	msg[511] = 0;
	switch (type) {
	case FATAL:
		g_mod_deleteall();
		if (!f_quiet)
			fprintf(stderr, EM_FATALERROR, msg);
		if (logfile) {
			fprintf(logfile, EM_FATALERROR, msg);
			fclose(logfile);
		}
		exit(1);
	case ERROR:
		if (!f_quiet)
			fprintf(stderr, EM_SIMPLEERROR, msg);
		if (logfile)
			fprintf(logfile, EM_SIMPLEERROR, msg);
		break;
	case WARN:
		if (!f_quiet)
			fprintf(stderr, EM_WARNING, msg);
		if (logfile)
			fprintf(logfile, EM_WARNING, msg);
		break;
	case MSG:
		if (!f_quiet)
			fputs(msg, stdout);
		fflush(stdout);
		if (logfile)
			fputs(msg, logfile);
		break;
	}
	if (logfile)
		fflush(logfile);
}

byte_t *alloc(ssize_t s)
{
	byte_t *p = (byte_t *)malloc(s);

	if (p == 0)
		pr(FATAL, EM_MALLOCFAILED, s);
	memset(p, 0, s);
	return (p);
}

/*
 * read nsecs blocks of ssize bytes from fd
 */

ssize_t bread(int fd, byte_t *buf, size_t ssize, size_t nsecs)
{
	const size_t total = ssize * nsecs;
	size_t read_bytes = 0;

	while (read_bytes < total) {
		ssize_t ret = read(fd, buf + read_bytes, total - read_bytes);
		if (ret > 0)
			read_bytes += ret;
		else if (ret == 0) {
			return read_bytes;
		} else {
			// ret < 0, an error case
			if (errno == EINTR)
				continue; // Rogue signal interruption, retry
			berrno = errno;
			break;
		}
	}

	return read_bytes ? read_bytes : -1;
}

static int yesno(char *q)
{
	int ch = 0;
	char buf[3];

	pr(MSG, q);
	pr(MSG, " %s : ", DM_YESNO);
	if (fgets(buf, 3, stdin))
		ch = *buf;
	pr(MSG, "\n");
	return (strchr(DM_YES, ch) == 0 ? 0 : 1);
}

static long number_or_quit(char *m, long lo, long up)
{
	char buf[32];
	long num = -1;

	pr(MSG, m);
	pr(MSG, DM_NUMORQUIT, lo, up);
	if (fgets(buf, 32, stdin)) {
		if (strchr(DM_QUIT, *buf))
			return (-1);
		num = strtoul(buf, 0, 0);
		if (errno == ERANGE)
			return (-1);
	}
	return (num);
}

/*
 * get three comma separated strings.
 */

static int get_csep_arg(char *arg, char **p1, char **p2, char **p3)
{
	char *p;

	if (p1)
		*p1 = arg;
	else
		return (0);
	if ((p = strchr(arg, ',')) == 0)
		return (0);
	*p = 0;
	arg = p + 1;
	if (p2)
		*p2 = arg;
	else
		return (1);
	if (p3) {
		if ((p = strchr(arg, ',')) == 0)
			return (0);
		*p = 0;
		*p3 = p + 1;
	}
	return (1);
}

/*
 * partition type list, taken from *BSD i386 fdisk, cfdisk etc.
 */

static char *get_part_type(int type)
{
	int i;
	struct {
		int t;
		char *n;
	} ptypes[] = {{0x00, "unused"},
				  {0x01, "Primary DOS with 12 bit FAT"},
				  {0x02, "XENIX / filesystem"},
				  {0x03, "XENIX /usr filesystem"},
				  {0x04, "Primary DOS with 16 bit FAT (<= 32MB)"},
				  {0x05, "Extended DOS"},
				  {0x06, "Primary 'big' DOS (> 32MB)"},
				  {0x07, "OS/2 HPFS, NTFS, QNX or Advanced UNIX"},
				  {0x08, "AIX filesystem"},
				  {0x09, "AIX boot partition or Coherent"},
				  {0x0A, "OS/2 Boot Manager or OPUS"},
				  {0x0B, "DOS or Windows 95 with 32 bit FAT"},
				  {0x0C, "DOS or Windows 95 with 32 bit FAT, LBA"},
				  {0x0E, "Primary 'big' DOS (> 32MB, LBA)"},
				  {0x0F, "Extended DOS, LBA"},
				  {0x10, "OPUS"},
				  {0x11, "Hidden DOS with 12 bit FAT"},
				  {0x12, "Compaq Diagnostics"},
				  {0x14, "Hidden DOS with 16 bit FAT (<= 32MB)"},
				  {0x16, "Hidden 'big' DOS (> 32MB)"},
				  {0x17, "OS/2 Boot Manager HPFS"},
				  {0x18, "AST special Windows swap file"},
				  {0x24, "NEC MS-DOS 3.x"},
				  {0x3C, "PowerQuest PartitionMagic recovery partition"},
				  {0x40, "VENIX 286"},
				  {0x4D, "QNX4.x"},
				  {0x4E, "QNX4.x 2nd part"},
				  {0x4F, "QNX4.x 3rd part"},
				  {0x50, "DM"},
				  {0x51, "DM"},
				  {0x51, "DM"},
				  {0x52, "CP/M or Microport SysV/AT"},
				  {0x55, "EZ Drive"},
				  {0x56, "GB"},
				  {0x61, "SpeedStor"},
				  {0x63, "ISC UNIX, other System V/386, GNU HURD or Mach"},
				  {0x64, "Novell Netware 2.xx"},
				  {0x65, "Novell Netware 3.xx"},
				  {0x70, "DiskSecure Multi-Boot"},
				  {0x75, "PCIX"},
				  {0x80, "Minix V1"},
				  {0x81, "Minix V2/Linux"},
				  {0x82, "Linux swap or Solaris/x86"},
				  {0x83, "Linux ext2 filesystem"},
				  {0x85, "Extended Linux"},
				  {0x86, "FAT16 volume/stripe set"},
				  {0x8E, "Linux LVM physical volume"},
				  {0x93, "Amoeba filesystem"},
				  {0x94, "Amoeba bad block table"},
				  {0xA5, "FreeBSD/NetBSD/386BSD"},
				  {0xA6, "OpenBSD"},
				  {0xA7, "NEXTSTEP"},
				  {0xB7, "BSDI BSD/386 filesystem"},
				  {0xB8, "BSDI BSD/386 swap"},
				  {0xC7, "Syrinx"},
				  {0xDB, "Concurrent CPM or C.DOS or CTOS"},
				  {0xE1, "SpeedStor 12-bit FAT extended"},
				  {0xE3, "Speed"},
				  {0xE4, "SpeedStor 16-bit FAT"},
				  {0xEB, "BeOS fs"},
				  {0xF1, "SpeedStor"},
				  {0xF2, "DOS 3.3+ Secondary"},
				  {0xF4, "SpeedStor"},
				  {0xFD, "Linux raid autodetect"},
				  {0xFE, "LANstep"},
				  {0xFF, "BBT (Bad Blocks Table)"}};

	for (i = 0; i < sizeof(ptypes) / sizeof(ptypes[0]); i++)
		if (type == ptypes[i].t)
			return (ptypes[i].n);
	return (0);
}

static int is_ext_parttype(dos_part_entry *p)
{
	return (p->p_size && ((p->p_typ == 0x05) || (p->p_typ == 0x0F) || (p->p_typ == 0x85)));
}

static int is_sane_partentry(disk_desc *d, dos_part_entry *p, int c)
{
	if (p->p_start >= d->d_nsecs) {
		if (c)
			pr(WARN, EM_PSTART2BIG, get_part_type(p->p_typ));
		return (0);
	}
	if (p->p_size > d->d_nsecs) {
		if (c)
			pr(WARN, EM_PSIZE2BIG, get_part_type(p->p_typ));
		return (0);
	}
	if (p->p_start + p->p_size > d->d_nsecs) {
		if (c)
			pr(WARN, EM_PEND2BIG, get_part_type(p->p_typ));
		return (0);
	}
	if (p->p_flag && (p->p_flag != DOSPARTACTIVE)) {
		if (c)
			pr(WARN, EM_STRANGEPTYPE, get_part_type(p->p_typ));
		return (0);
	}
	return (1);
}

static int is_real_parttype(dos_part_entry *p) { return (!is_ext_parttype(p) && p->p_typ && get_part_type(p->p_typ)); }

static int no_of_ext_partitions(dos_part_entry *p)
{
	dos_part_entry *t;
	int ne = 0;

	for (t = &p[0]; t <= &p[NDOSPARTS - 1]; t++)
		if (is_ext_parttype(t))
			ne++;
	return (ne);
}

static int no_of_real_partitions(dos_part_entry *p)
{
	dos_part_entry *t;
	int nr = 0;

	for (t = &p[0]; t <= &p[NDOSPARTS - 1]; t++)
		if (is_real_parttype(t))
			nr++;
	return (nr);
}

/*
 * Test similarity of partition types
 */

static int is_same_partition_type(dos_part_entry *p1, dos_part_entry *p2)
{
	int ret = 0;

	switch (p1->p_typ) {
	case 0x01:
	case 0x11:
		ret = (p2->p_typ == 0x06) || (p2->p_typ == 0x0E);
		break;

	case 0x06:
	case 0x0E:
	case 0x16:
		ret = (p2->p_typ == 0x06) || (p2->p_typ == 0x0E) || (p2->p_typ == 0x16);
		break;

	case 0x05:
	case 0x0F:
		ret = (p2->p_typ == 0x05) || (p2->p_typ == 0x0F);
		break;

	case 0x0B:
	case 0x0C:
		ret = (p2->p_typ == 0x0B) || (p2->p_typ == 0x0C);
		break;

	case 0x8E:
	case 0xFE:
		ret = (p2->p_typ == 0x8E) || (p2->p_typ == 0xFE);
		break;

	default:
		ret = p1->p_typ == p2->p_typ;
		break;
	}
	return (ret);
}

/*
 * detecting an extended ptbl isn't unambiguous, the boot code
 * preceding the ptbl should be zeroed but isn't always. The
 * ptbl should in theory contain one 'normal' entry, zero or
 * one link to the next extended ptbl and two or three zeroed
 * entries.
 */

static int is_ext_parttable(disk_desc *d, byte_t *buf)
{
	int r, e;
	byte_t *magic;
	dos_part_entry *p, *t;

	p = (dos_part_entry *)(buf + DOSPARTOFF);
	magic = (byte_t *)&p[NDOSPARTS];
	if (*(unsigned short *)magic != le16(DOSPTMAGIC))
		return (0);

	/*
	 * ptbl sanity checks.
	 */

	for (t = p; t <= &p[NDOSPARTS - 1]; t++)
		if (!is_sane_partentry(d, t, 0))
			return (0);

	/*
	 * one real, zero or one extended and two or three unused
	 * partition entries.
	 */

	r = no_of_real_partitions(p);
	e = no_of_ext_partitions(p);
	return ((r == 1) && ((e == 0) || (e == 1)));
}

static void fillin_dos_chs(disk_desc *d, dos_part_entry *p, s64_t offset)
{
	unsigned long n;

	n = p->p_start;
	if (n > 1023 * d->d_dg.d_h * d->d_dg.d_s) {
		p->p_ssect = d->d_dg.d_s | ((1023 >> 2) & 0xc0);
		p->p_shd = d->d_dg.d_h - 1;
		p->p_scyl = 1023 & 0xff;
	} else {
		p->p_ssect = (n % d->d_dg.d_s) + 1;
		n /= d->d_dg.d_s;
		p->p_shd = n % d->d_dg.d_h;
		n /= d->d_dg.d_h;
		p->p_scyl = n & 0xff;
		p->p_ssect |= (n >> 2) & 0xc0;
	}
	n = p->p_size + p->p_start - 1;
	if (n > 1023 * d->d_dg.d_h * d->d_dg.d_s) {
		p->p_esect = d->d_dg.d_s | ((1023 >> 2) & 0xc0);
		p->p_ehd = d->d_dg.d_h - 1;
		p->p_ecyl = 1023 & 0xff;
	} else {
		p->p_esect = (n % d->d_dg.d_s) + 1;
		n /= d->d_dg.d_s;
		p->p_ehd = n % d->d_dg.d_h;
		n /= d->d_dg.d_h;
		p->p_ecyl = n & 0xff;
		p->p_esect |= (n >> 2) & 0xc0;
	}
}

static void u_to_chs(disk_desc *d, unsigned long u, long *c, long *h, long *s)
{
	struct disk_geom *g = &d->d_dg;

	*c = *h = *s = 0;
	if (g->d_h && g->d_s && u) {
		*c = u / (g->d_h * g->d_s);
		*h = (u / g->d_s) % g->d_h;
		*s = u % g->d_s + 1;
	}
}

static int on_cyl_boundary(disk_desc *d, s64_t sec)
{
	struct disk_geom *g = &d->d_dg;

	if (g->d_h && g->d_s)
		return ((sec % (g->d_h * g->d_s)) == 0);
	return (1);
}

static int on_head_boundary(disk_desc *d, s64_t sec)
{
	struct disk_geom *g = &d->d_dg;

	if (g->d_s)
		return ((sec % g->d_s) == 0);
	return (1);
}

static void print_partition(disk_desc *d, dos_part_entry *p, int inset, s64_t offset)
{
	long i, c = 0, h = 0, s = 0;
	s64_t size;
	char *ptyp = get_part_type(p->p_typ);

#define indent(s)                                                                                                      \
	for (i = 0; i < s; i++)                                                                                            \
	pr(MSG, "   ")

	size = p->p_size;
	s2mb(d, size);
	indent(inset);
	pr(MSG, PM_PT_TYPE, p->p_typ, p->p_typ, ptyp ? ptyp : "UNKNOWN");
	if (p->p_flag == DOSPARTACTIVE)
		pr(MSG, " (BOOT)");
	pr(MSG, "\n");

	indent(inset);
	pr(MSG, PM_PT_SIZE, size, (s64_t)p->p_size);
	size = p->p_start;
	size += offset;
	size += p->p_size;
	if (size)
		size -= 1;
	pr(MSG, " s(%qd-%qd)\n", (s64_t)p->p_start + offset, size);

	indent(inset);
	pr(MSG, PM_PT_CHS, DOSCYL(p->p_scyl, p->p_ssect), p->p_shd, DOSSEC(p->p_ssect), DOSCYL(p->p_ecyl, p->p_esect),
	   p->p_ehd, DOSSEC(p->p_esect));
	if (size)
		u_to_chs(d, p->p_start + offset, &c, &h, &s);
	pr(MSG, " (%ld/%ld/%ld)-", c, h, s);
	if (size)
		u_to_chs(d, p->p_start + offset + p->p_size - 1, &c, &h, &s);
	pr(MSG, "(%ld/%ld/%ld)r\n", c, h, s);

	if (f_verbose > 0) {
		indent(inset);
		pr(MSG, PM_PT_HEX);
		for (i = 0; i < sizeof(dos_part_entry); i++)
			pr(MSG, " %02X", ((byte_t *)p)[i]);
		pr(MSG, "\n");
	}
	pr(MSG, "\n");
}

static void print_ext_partitions(disk_desc *d, s64_t offset)
{
	dos_part_table *pt = d->d_pt.t_ext;
	dos_part_entry *p;
	s64_t extst = 0;

	for (; pt; pt = pt->t_ext) {
		pr(MSG, PM_EXTPART);
		for (p = pt->t_parts; p <= &pt->t_parts[NDOSPARTS - 1]; p++)
			if (is_real_parttype(p))
				print_partition(d, p, 1, offset + extst);

		for (p = pt->t_parts; p <= &pt->t_parts[NDOSPARTS - 1]; p++)
			if (is_ext_parttype(p))
				extst = p->p_start;
	}
}

static void print_ptable(disk_desc *d, dos_part_table *pt, int pr_ext)
{
	int n;

	for (n = 0; n < NDOSPARTS; n++) {
		pr(MSG, PM_PRIMPART, n + 1);
		print_partition(d, &pt->t_parts[n], 0, 0);
		if (pr_ext && is_ext_parttype(&pt->t_parts[n]))
			print_ext_partitions(d, pt->t_parts[n].p_start);
	}
}

static void print_disk_desc(disk_desc *d)
{
	s64_t s;

	pr(MSG, PM_DEVDESC1, d->d_dev, d->d_ssize);
	if (f_getgeom) {
		s = d->d_nsecs;
		s2mb(d, s);
		pr(MSG, PM_DEVDESC2, d->d_dg.d_c, d->d_dg.d_h, d->d_dg.d_s, d->d_lba ? "(LBA) " : " ", d->d_nsecs, s);
	}
	pr(MSG, "\n");
	if (d->d_pt.t_magic != le16(DOSPTMAGIC))
		pr(WARN, EM_STRANGEPTBLMAGIC, d->d_pt.t_magic);
	print_ptable(d, &d->d_pt, 1);
}

static void print_mboot_block(disk_desc *d)
{
	int n, m, cols = 16;
	byte_t *boot = d->d_pt.t_boot;

	pr(MSG, PM_MBRPRINT, d->d_dev);
	for (n = 0; n < DOSPARTOFF - cols; n += cols) {
		pr(MSG, "   %04X:  ", n);
		for (m = n; m < n + cols; m++)
			pr(MSG, " %02x", boot[m]);
		pr(MSG, "\n          ");
		for (m = n; m < n + cols; m++)
			pr(MSG, " %c ", isprint(boot[m]) ? boot[m] : '.');
		pr(MSG, "\n");
	}
}

static void read_part_table(disk_desc *d, s64_t sec, byte_t *where)
{
	ssize_t rd;
	size_t psize;
	byte_t *ubuf, *buf;

	psize = getpagesize();
	ubuf = alloc(MAXSSIZE + psize);
	buf = align(ubuf, psize);
	sec *= d->d_ssize;
	if (l64seek(d->d_fd, sec, SEEK_SET) == -1)
		pr(FATAL, EM_SEEKFAILURE, d->d_dev);

	if (d->d_ssize < 512)
		rd = bread(d->d_fd, buf, d->d_ssize, 512 / d->d_ssize);
	else
		rd = bread(d->d_fd, buf, d->d_ssize, 1);

	if (rd == -1)
		pr(FATAL, EM_PTBLREAD);
	memcpy(where, buf, 512);
	free((void *)ubuf);
}

static void read_ext_part_table(disk_desc *d, dos_part_table *pt)
{
	dos_part_entry *p, *ep;
	s64_t epsize, epstart, epoffset;
	int epcount;

	epsize = epstart = epoffset = epcount = 0;
	while (1) {
		ep = 0;
		for (p = pt->t_parts; p <= &pt->t_parts[NDOSPARTS - 1]; p++)
			if (is_ext_parttype(p)) {
				if (ep == 0) {
					ep = p;
					break;
				}
				pr(ERROR, EM_TOOMANYEXTP);
			}

		if (ep == 0)
			return;
		if (++epcount > 128) /* arbitrary maximum */
		{
			pr(ERROR, EM_TOOMANYLOGP, 128);
			return;
		}
		if (epstart == 0) {
			epstart = ep->p_start;
			epsize = ep->p_size;
			epoffset = 0;
		} else
			epoffset = ep->p_start;
		if (epoffset > epsize) {
			pr(ERROR, EM_EPILLEGALOFS);
			return;
		}

		/*
		 * link in new extended ptbl.
		 */

		pt->t_ext = (dos_part_table *)alloc(sizeof(dos_part_table));
		read_part_table(d, epstart + epoffset, (pt = pt->t_ext)->t_boot);
		if (!is_ext_parttable(d, pt->t_boot)) {
			pr(ERROR, EM_INVXPTBL, epstart + epoffset);
			return;
		}
	}
}

static void free_disk_desc(disk_desc *d)
{
	dos_part_table *pt;
	dos_guessed_pt *pg;
	void *t;

	for (pt = d->d_pt.t_ext; pt;) {
		t = pt->t_ext;
		free((void *)pt);
		pt = t;
	}
	for (pt = d->d_gpt.t_ext; pt;) {
		t = pt->t_ext;
		free((void *)pt);
		pt = t;
	}
	for (pg = d->d_gl; pg;) {
		t = pg->g_next;
		free((void *)pg);
		pg = t;
	}
	free((void *)d);
}

static disk_desc *get_disk_desc(char *dev, int sectsize)
{
	byte_t *ubuf, *buf;
	disk_desc *d;
	int psize, ssize;
	struct disk_geom *dg;

	psize = getpagesize();
	ubuf = alloc(MAXSSIZE + psize);
	buf = align(ubuf, psize);
	d = (disk_desc *)alloc(sizeof(disk_desc));

	/*
	 * I don't care if the given name denotes a block or character
	 * special file or just a regular file.
	 */

	if ((d->d_fd = open(dev, O_RDONLY)) == -1)
		pr(FATAL, EM_OPENFAIL, dev, strerror(errno));

	/*
	 * try to test for sector sizes (doesn't work under many systems).
	 */

	if (sectsize > MAXSSIZE)
		pr(FATAL, EM_WRONGSECTSIZE, MAXSSIZE);

	if (sectsize) {
		ssize = bread(d->d_fd, buf, sectsize, 1);
		if (ssize != sectsize)
			pr(FATAL, EM_FAILSSIZEATTEMPT, sectsize);
		d->d_ssize = sectsize;
	} else {
		for (d->d_ssize = MINSSIZE; d->d_ssize <= MAXSSIZE; d->d_ssize *= 2) {
			if (l64seek(d->d_fd, 0, SEEK_SET) == -1)
				pr(FATAL, EM_SEEKFAILURE, dev);
			ssize = bread(d->d_fd, buf, d->d_ssize, 1);
			if (ssize == d->d_ssize)
				break;
		}
		if (ssize == -1)
			pr(FATAL, EM_CANTGETSSIZE, dev);
	}

	d->d_dev = dev;
	read_part_table(d, 0, d->d_pt.t_boot);
	if (f_getgeom) {
		if ((dg = disk_geometry(d)) == 0)
			pr(FATAL, EM_CANTGETGEOM);
		memcpy(&d->d_dg, dg, sizeof(struct disk_geom));

		d->d_nsecs = dg->d_nsecs;

		/*
		 * command line geometry overrides
		 */

		if (gc)
			d->d_dg.d_c = gc;
		if (gh)
			d->d_dg.d_h = gh;
		if (gs)
			d->d_dg.d_s = gs;
	} else {
		d->d_dg.d_c = gc;
		d->d_dg.d_h = gh;
		d->d_dg.d_s = gs;
	}
	if (d->d_dg.d_c < 1024)
		d->d_dosc = 1;
	if ((d->d_dg.d_h > 16) || (d->d_dg.d_s > 63))
		d->d_lba = 1;

	if (gh && gc && gs) {
		/* Override number of sectors with command line parameters */
		d->d_nsecs = d->d_dg.d_c;
		d->d_nsecs *= d->d_dg.d_h;
		d->d_nsecs *= d->d_dg.d_s;
	}

	read_ext_part_table(d, &d->d_pt);
	close(d->d_fd);
	free((void *)ubuf);
	return (d);
}

static void add_guessed_p(disk_desc *d, dos_part_entry *p, int cnt)
{
	dos_guessed_pt *gpt;

	if (d->d_gl == 0)
		gpt = d->d_gl = (dos_guessed_pt *)alloc(sizeof(dos_guessed_pt));
	else {
		for (gpt = d->d_gl; gpt->g_next; gpt = gpt->g_next)
			;
		gpt->g_next = (dos_guessed_pt *)alloc(sizeof(dos_guessed_pt));
		gpt = gpt->g_next;
	}

	gpt->g_ext = (cnt > 1);
	for (; cnt > 0; cnt--)
		memcpy(&gpt->g_p[cnt - 1], &p[cnt - 1], sizeof(dos_part_entry));
	gpt->g_sec = d->d_nsb;
}

static g_module *get_best_guess(g_module **g, int count)
{
	int mx, i;
	float bestg = 0.0;

	/*
	 * up to now the best guess is simple that one which
	 * reported the largest probability (if there are more
	 * than one, the last one of them).
	 */

	for (mx = i = 0; i < count; i++)
		if (g[i]->m_guess * g[i]->m_weight > bestg)
			bestg = g[mx = i]->m_guess * g[i]->m_weight;

	return ((bestg > 0.0) ? g[mx] : 0);
}

static int mod_is_aligned(disk_desc *d, g_module *m)
{
	s64_t al;

	switch (m->m_align) {
	case 'h':
		return (on_head_boundary(d, d->d_nsb));

	case 'c':
		return (on_cyl_boundary(d, d->d_nsb));

	case 1:
	case 's':
		return (1);
	default:
		if (m->m_align > 0) {
			al = d->d_nsb;
			al %= m->m_align;
			return (al == 0);
		}
		break;
	}
	return (1);
}

/*
 * the main guessing loop.
 */

static void do_guess_loop(disk_desc *d)
{
	g_module *m, **guesses;
	unsigned long incr = 0;
	int nsecs, in_ext = 0, end_of_ext = 0, psize;
	ssize_t rd, bsize = d->d_ssize;
	s64_t bincr, noffset, start;
	byte_t *ubuf;

	if ((d->d_fd = open(d->d_dev, O_RDONLY)) == -1)
		pr(FATAL, EM_OPENFAIL, d->d_dev, strerror(errno));

#if HAVE_POSIX_FADVISE
	posix_fadvise(d->d_fd, 0, 0, POSIX_FADV_SEQUENTIAL);
	posix_fadvise(d->d_fd, 0, 0, POSIX_FADV_WILLNEED);
#endif /* HAVE_POSIX_FADVISE */
	/*
	 * initialize modules. Each should return the minimum
	 * size in bytes it wants to receive for a test.
	 */

	for (m = g_mod_head(); m; m = m->m_next)
		if (m->m_init) {
			int sz;

			if ((sz = (*m->m_init)(d, m)) <= 0)
				pr(ERROR, EM_MINITFAILURE, m->m_name);
			bsize = max(sz, bsize);
		}

	if (bsize % d->d_ssize)
		bsize += d->d_ssize - bsize % d->d_ssize;
	nsecs = bsize / d->d_ssize;
	switch (increment) {
	case 's':
		incr = 1;
		break;
	case 'h':
		incr = d->d_dg.d_s;
		break;
	case 'c':
		incr = d->d_dg.d_s * d->d_dg.d_h;
		break;
	default:
		incr = increment;
		break;
	}
	if (incr == 0)
		incr = 1;

	boundary_fun = (incr == 1) ? on_head_boundary : on_cyl_boundary;
	psize = getpagesize();
	ubuf = alloc(bsize + psize);
	d->d_sbuf = align(ubuf, psize);
	d->d_nsb = 0;
	bincr = incr * d->d_ssize;

	start = skipsec ? skipsec : d->d_dg.d_s;
	d->d_nsb = start - incr;
	start *= d->d_ssize;
	if (l64seek(d->d_fd, start, SEEK_SET) == -1)
		pr(FATAL, EM_SEEKFAILURE, d->d_dev);

	/*
	 * do the work: read blocks, distribute to modules, check
	 * for probable hits.
	 */

	guesses = (g_module **)alloc(g_mod_count() * sizeof(g_module *));
	pr(MSG, DM_STARTSCAN);

scanloop:
	while ((rd = bread(d->d_fd, d->d_sbuf, d->d_ssize, nsecs)) == bsize) {
		int mod, have_ext = 0;
		g_module *bg;
		s64_t sz, ofs, fpos;

		d->d_nsb += incr;
		noffset = 0;
		ofs = d->d_nsb;
		s2mb(d, ofs);
		fpos = d->d_nsb * d->d_ssize + bsize;
		if (maxsec && (d->d_nsb > maxsec))
			break;

		/*
		 * reset modules
		 */

		for (m = g_mod_head(); m; m = m->m_next)
			m->m_skip = 0;

	guessit:
		bg = 0;
		mod = 0;
		for (m = g_mod_head(); m; m = m->m_next) {
			if (m->m_skip || (in_ext && m->m_notinext) || !mod_is_aligned(d, m))
				continue;

			/*
			 * because a gmodule is allowed to seek on
			 * d->d_fd the current file position must be
			 * restored after calling it.
			 */

			memset(&m->m_part, 0, sizeof(dos_part_entry));
			m->m_guess = GM_NO;
			if ((*m->m_gfun)(d, m) && (m->m_guess * m->m_weight >= GM_PERHAPS))
				guesses[mod++] = m;
			l64seek(d->d_fd, fpos, SEEK_SET);
		}

		/*
		 * now fetch the best guess.
		 */

		if (mod && (bg = get_best_guess(guesses, mod))) {
			noffset = bg->m_part.p_size;
			fillin_dos_chs(d, &bg->m_part, 0);
		}

		/*
		 * extended partition begin?
		 */

		if (f_testext && boundary_fun(d, d->d_nsb) && (!bg || !bg->m_hasptbl) && is_ext_parttable(d, d->d_sbuf)) {
			dos_part_entry *p;
			int no_ext;

			p = (dos_part_entry *)(d->d_sbuf + DOSPARTOFF);
			no_ext = no_of_ext_partitions(p);
			if (!in_ext) {
				pr(MSG, PM_POSSIBLEEXTPART, ofs);
				in_ext = 1;
				end_of_ext = 0;
				noffset = 0;
			} else if (no_ext == 0)
				end_of_ext = 1;

			if (in_ext) {
				if (f_interactive) {
					if (yesno(DM_ACCEPTGUESS))
						add_guessed_p(d, p, have_ext = NDOSPARTS);
					else if (mod && bg) {
						bg->m_skip = 1;
						goto guessit;
					}
				} else
					add_guessed_p(d, p, have_ext = NDOSPARTS);
			}
		}

		if (!have_ext && noffset) {
			sz = noffset;
			s2mb(d, sz);
			if (in_ext)
				pr(MSG, "   ");
			pr(MSG, PM_POSSIBLEPART, bg->m_desc ? bg->m_desc : bg->m_name, sz, ofs);
			if (f_verbose)
				print_partition(d, &bg->m_part, in_ext ? 1 : 0, 0);
			if (f_interactive)
				if (!yesno(DM_ACCEPTGUESS)) {
					noffset = 0;
					if (mod && bg) {
						bg->m_skip = 1;
						goto guessit;
					}
				}

			if (noffset) {
				add_guessed_p(d, &bg->m_part, 1);
				if (end_of_ext)
					in_ext = 0;
			}
		}

		/*
		 * seek to next sectors to investigate (may seek
		 * backwards).
		 */

		if (noffset && f_fast) {
			if (noffset % incr)
				noffset += incr - noffset % incr;
			d->d_nsb += noffset - incr;
			noffset -= nsecs;
			noffset *= d->d_ssize;
			if (l64seek(d->d_fd, noffset, SEEK_CUR) == -1)
				pr(FATAL, EM_SEEKFAILURE, d->d_dev);
		} else if (bincr)
			if (l64seek(d->d_fd, bincr - bsize, SEEK_CUR) == -1)
				pr(FATAL, EM_SEEKFAILURE, d->d_dev);
	}

	/*
	 * short read?
	 */

	if ((rd > 0) && (rd < bsize))
		if (d->d_nsb + nsecs + 1 < d->d_nsecs) {
			/*
			 * short read not at end of disk
			 */

			pr(f_skiperrors ? WARN : FATAL, EM_SHORTBREAD, d->d_nsb, rd, bsize);
			noffset = l64tell(d->d_fd);
			noffset /= d->d_ssize;
			noffset *= d->d_ssize;
			if (l64seek(d->d_fd, noffset, SEEK_SET) == -1)
				pr(FATAL, EM_SEEKFAILURE, d->d_dev);
			d->d_nsb = l64tell(d->d_fd) / d->d_ssize - incr;
			goto scanloop;
		}

	if (rd == -1) {
		/*
		 * EIO is ignored (skipping current sector(s))
		 */

		if (f_skiperrors && (berrno == EIO)) {
			pr(WARN, EM_BADREADIO, d->d_nsb);
			noffset = l64tell(d->d_fd);
			noffset /= d->d_ssize;
			noffset += incr;
			noffset *= d->d_ssize;
			if (l64seek(d->d_fd, noffset, SEEK_SET) == -1)
				pr(FATAL, EM_SEEKFAILURE, d->d_dev);
			d->d_nsb = l64tell(d->d_fd) / d->d_ssize - incr;
			goto scanloop;
		}
		pr(FATAL, EM_READERROR, d->d_dev, d->d_nsb, strerror(berrno));
	}

	pr(MSG, DM_ENDSCAN);
	if (guesses)
		free((void *)guesses);

	for (m = g_mod_head(); m; m = m->m_next)
		if (m->m_term)
			(*m->m_term)(d);
	free((void *)ubuf);
	close(d->d_fd);
}

static void edit_partition(disk_desc *d, dos_part_entry *p)
{
	char ans[32];
	int n;
	unsigned long val;

	while (1) {
		pr(MSG, DM_NOCHECKWARNING);
		pr(MSG, PM_EDITITEM1, p->p_start);
		pr(MSG, PM_EDITITEM2, p->p_size);
		pr(MSG, PM_EDITITEM3, p->p_typ, get_part_type(p->p_typ));
		if ((n = number_or_quit(DM_EDITWHICHITEM, 1, 3)) < 0)
			break;
		if ((n < 1) || (n > 3))
			continue;
		pr(MSG, "Enter value for %d : ", n);
		if (fgets(ans, 32, stdin)) {
			val = strtoul(ans, 0, 0);
			switch (n) {
			case 1:
				p->p_start = val;
				fillin_dos_chs(d, p, 0);
				break;
			case 2:
				p->p_size = val;
				fillin_dos_chs(d, p, 0);
				break;
			case 3:
				p->p_typ = val & 0xFF;
				break;
			}
		}
	}
}

static int make_mbr_backup(disk_desc *d, char *bfile)
{
	int fd, ret = 0;

	if ((fd = open(bfile, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
		return (ret);

	if (write(fd, d->d_pt.t_boot, 512) == 512)
		ret = 1;
	close(fd);
	return (ret);
}

static void write_primary_ptbl(disk_desc *d, char *dev)
{
	struct stat sbuf;
	byte_t *ptbl, *uptbl;
	int fd, doesntexist = 0, n;

	uptbl = alloc(d->d_ssize + getpagesize());
	ptbl = align(uptbl, getpagesize());

	if (stat(dev, &sbuf) == -1) {
		if (errno != ENOENT)
			pr(FATAL, EM_STATFAILURE, dev, strerror(errno));
		else
			doesntexist = 1;
	}
	fd = open(dev, O_WRONLY | (doesntexist ? O_CREAT | O_EXCL : 0), 0660);
	if (fd == -1)
		pr(FATAL, EM_OPENFAIL, dev, strerror(errno));
	if (l64seek(fd, 0, SEEK_SET) == -1)
		pr(FATAL, EM_SEEKFAILURE, dev);

	/*
	 * is there a guessed partition table?
	 */

	if (d->d_gpt.t_magic == le16(DOSPTMAGIC)) {
		/*
		 * ask if table should be hand-edited
		 */

		if (yesno(DM_EDITPTBL))
			while (1) {
				if ((n = number_or_quit(DM_EDITWHICHPART, 1, NDOSPARTS)) < 0)
					break;
				if ((n >= 1) && (n <= NDOSPARTS))
					edit_partition(d, &d->d_gpt.t_parts[n - 1]);
				else
					break;
				print_ptable(d, &d->d_gpt, 0);
			}

		/*
		 * ask for the active partition.
		 */

		while (1) {
			if ((n = number_or_quit(DM_ACTWHICHPART, 1, NDOSPARTS)) < 0)
				break;
			if ((n >= 1) && (n <= NDOSPARTS) && get_part_type(d->d_gpt.t_parts[n].p_typ)) {
				d->d_gpt.t_parts[n - 1].p_flag = DOSPARTACTIVE;
				break;
			}
		}

		if (yesno(DM_WRITEIT)) {
			memcpy(ptbl, d->d_pt.t_boot, DOSPARTOFF);
			memcpy(ptbl + DOSPARTOFF, d->d_gpt.t_parts, NDOSPARTS * sizeof(dos_part_entry) + 2);
			if (write(fd, ptbl, d->d_ssize) != d->d_ssize)
				pr(FATAL, EM_PTBLWRITE);

			if (S_ISBLK(sbuf.st_mode) || S_ISCHR(sbuf.st_mode)) {
				sync();
				sleep(1);
				sync();
				reread_partition_table(fd);
				pr(WARN, DM_ASKTOREBOOT);
			}
		} else
			pr(MSG, DM_NOTWRITTEN);
	}
	close(fd);
	free((void *)uptbl);
}

static void warn_invalid(disk_desc *d, dos_part_entry *p, char *w)
{
	if (f_verbose > 1) {
		pr(MSG, EM_PINVALID, w);
		print_partition(d, p, 0, 0);
	}
}

/*
 * after having gathered a list of possible partitions they
 * have to be checked for consistency. This routine must be
 * improved, it's too weird and is mistaken too often.
 */

static int check_partition_list(disk_desc *d)
{
	dos_guessed_pt *gp, *prev;
	dos_part_entry *p, *rp, *ep, *lep;
	int n, npp, epp, maxp, in_ext;
	s64_t size, ofs;

	p = rp = ep = lep = 0;

	pr(MSG, DM_STARTCHECK);

	/*
	 * 1. pass: discard overlapping entries. This means
	 * that the first entry is assumed to be ok.
	 */

	ofs = 0;
	prev = 0;
	npp = 0;
	for (gp = d->d_gl; gp; gp = gp->g_next) {
		if (gp->g_ext || gp->g_inv)
			continue;
		p = &gp->g_p[0];
		if (gp == d->d_gl)
			ofs = p->p_start + p->p_size;
		else {
			if (p->p_start < ofs) {
				/*
				 * overlap. unlink and discard.
				 */

				prev->g_next = gp->g_next;
				free((void *)gp);
				gp = prev;
				npp++;
			} else
				ofs += p->p_size;
		}
		prev = gp;
	}

	if (npp)
		pr(WARN, EM_DISCARDOVLP, npp);

	/*
	 * 2. pass: decide type of every partition,
	 * marking inconsistent ones as invalid.
	 */

	size = 0;
	in_ext = npp = epp = 0;
	maxp = NDOSPARTS;
	for (n = 1, gp = d->d_gl; gp; gp = gp->g_next, n++) {
		if (gp->g_inv)
			continue;
		if (gp->g_ext) {
			if (gp->g_next == 0) {
				/*
				 * ext ptbl without logical p.
				 */

				gp->g_inv = 1;
				warn_invalid(d, p, EM_P_EATEND);
				break;
			}
			if (!in_ext) {
				/*
				 * new extended p. chain.
				 */

				if (no_of_ext_partitions(gp->g_p) == 0) {
					gp->g_inv = 1;
					warn_invalid(d, p, EM_P_EWLP);
					continue;
				}

				in_ext = 1;
				epp++;
				if (maxp >= NDOSPARTS)
					maxp--;

				/*
				 * already had one?
				 */

				if (epp > 1) {
					gp->g_inv = 1;
					warn_invalid(d, p, EM_P_MTOE);
					continue;
				}
			}
			if (no_of_ext_partitions(gp->g_p) == 0)
				in_ext = 0;
			rp = 0;
			for (p = &gp->g_p[0]; p <= &gp->g_p[NDOSPARTS - 1]; p++) {
				if (is_real_parttype(p))
					rp = p;
				else if (is_ext_parttype(p) && (n == 1))
					size = p->p_start;
			}
			gp = gp->g_next;
			if (gp->g_ext) {
				/*
				 * should not happen: a supposedly logical
				 * partition which is an extended p itself.
				 */

				gp->g_inv = 1;
				warn_invalid(d, p, EM_P_LISAE);
				continue;
			} else
				gp->g_log = 1;

			/*
			 * the p. type in the extended ptbl and the following
			 * logical p. type must be identical. Also check size.
			 */

			p = &gp->g_p[0];
			if (is_real_parttype(p) && is_same_partition_type(rp, p) && (rp->p_size >= p->p_size)) {
				if (!is_sane_partentry(d, p, 1))
					gp->g_inv = 1;
				else
					size += gp->g_p[0].p_size + 1;
			} else {
				gp->g_inv = 1;
				warn_invalid(d, p, EM_P_UTS);
			}
		} else if (!in_ext) {
			/*
			 * primary entry.
			 */

			gp->g_prim = 1;
			p = &gp->g_p[0];
			if (n == 1)
				size = p->p_start;
			if (npp++ >= maxp) {
				gp->g_inv = 1;
				warn_invalid(d, p, EM_P_2MANYPP);
			} else {
				if (!is_sane_partentry(d, p, 1)) {
					gp->g_inv = 1;
					warn_invalid(d, p, EM_P_NOTSANE);
				} else
					size += p->p_size;
			}
		} else {
			/*
			 * in_ext && !gp->g_ext. This means the end
			 * of the logical partition chain hasn't been
			 * found. Reset it.
			 */

			in_ext = 0;
			gp->g_inv = 1;
			warn_invalid(d, p, EM_P_ENDNOTF);
		}
	}

	if (epp > 1)
		pr(WARN, EM_TOOMANYXPTS, epp);
	if (npp > maxp)
		pr(WARN, EM_TOOMANYPPTS, maxp, npp);

	/*
	 * 3. pass: check logical partition chain. Logical
	 * partitions which seem ok but are not found in the
	 * link chain are marked orphaned.
	 */

	in_ext = size = ofs = 0;
	lep = 0;
	for (gp = d->d_gl; gp; gp = gp->g_next) {
		if (gp->g_inv)
			continue;
		if (gp->g_ext) {
			if (!in_ext) {
				in_ext = 1;
				ofs = gp->g_sec;
			}
			rp = ep = 0;
			for (p = &gp->g_p[0]; p <= &gp->g_p[NDOSPARTS - 1]; p++) {
				if (is_real_parttype(p))
					rp = p;
				else if (is_ext_parttype(p))
					ep = p;
			}

			if (lep && rp)
				if (gp->g_sec != ofs + lep->p_start)
					gp->g_next->g_orph = 1;
			if (ep)
				lep = ep;
			gp = gp->g_next;
		}
	}

	/*
	 * if the list was consistent the size of the whole
	 * extended ptbl is equal to the end of the last eptbl
	 * link.
	 */

	if (rp && lep)
		size = lep->p_start + lep->p_size;
	else
		size = ofs = 0;

	for (gp = d->d_gl; gp; gp = gp->g_next) {
		if (gp->g_ext)
			continue;
		p = &gp->g_p[0];
		if (gp->g_log)
			pr(MSG, "   ");
		pr(MSG, "Partition(%s): ", get_part_type(p->p_typ));
		if (gp->g_inv)
			pr(MSG, PM_G_INVALID);
		if (gp->g_orph)
			pr(MSG, PM_G_ORPHANED);
		if (gp->g_prim)
			pr(MSG, PM_G_PRIMARY);
		if (gp->g_log)
			pr(MSG, PM_G_LOGICAL);
		pr(MSG, "\n");
		if (f_verbose > 1)
			print_partition(d, p, gp->g_log ? 1 : 0, 0);
	}

	/*
	 * now fill in the guessed primary partition table.
	 */

	in_ext = n = 0;
	memset(&d->d_gpt, 0, sizeof(dos_part_table));
	for (gp = d->d_gl; gp; gp = gp->g_next) {
		if (n >= NDOSPARTS)
			break;
		if (gp->g_inv)
			continue;
		if (gp->g_ext) {
			if (!in_ext) {
				in_ext = 1;
				if (size && ofs) {
					p = &d->d_gpt.t_parts[n++];
					p->p_start = ofs;
					p->p_typ = 0x05;
					p->p_typ = d->d_lba ? 0x0F : 0x05;
					p->p_size = size;
					fillin_dos_chs(d, p, 0);
				}
			}
			gp = gp->g_next;
			continue;
		}
		if (gp->g_prim)
			memcpy(&d->d_gpt.t_parts[n++], &gp->g_p[0], sizeof(dos_part_entry));
	}

	/*
	 * final step: re-check this table. If ok, set the
	 * ptbl magic number which is the indicator for
	 * write_primary_ptbl that it seems to be ok.
	 */

	ep = 0;
	npp = 0;
	for (n = 0; n < NDOSPARTS; n++) {
		p = &d->d_gpt.t_parts[n];
		if (ep && p->p_typ) {
			if ((ep->p_start + ep->p_size > p->p_start) || !is_sane_partentry(d, p, 1)) {
				/*
				 * zis is not funny. Perhaps the p. list
				 * can be re-checked but for now only
				 * inconsistencies are counted.
				 */

				npp++;
				if (f_verbose > 2) {
					pr(WARN, EM_PINCONS);
					print_partition(d, p, 0, 0);
				}
			}
		}
		ep = p;
	}
	if (npp == 0) {
		d->d_gpt.t_magic = le16(DOSPTMAGIC);
		pr(MSG, "Ok.\n");
	} else
		pr(MSG, DM_NOOFINCONS, npp);
	return (npp);
}

/*
 * compare both existing and guessed partition tables.
 * The order of the ptbl entries is not important (the
 * physically first partition on disk can be in the last
 * ptbl slot).
 */

static int compare_parttables(disk_desc *d)
{
	int ret, i, j, diff;
	byte_t *pr, *pg;

	ret = 0;
	for (i = 0; i < NDOSPARTS; i++) {
		pr = (byte_t *)&d->d_pt.t_parts[i];
		for (j = 0; j < NDOSPARTS; j++) {
			pg = (byte_t *)&d->d_gpt.t_parts[j];

			/*
			 * the p_flag entry cannot be included
			 * in the comparison.
			 */

			diff = memcmp(pr + 1, pg + 1, sizeof(dos_part_entry) - 1);
			if (diff == 0)
				break;
		}
		if (diff)
			ret++;
	}
	return (ret);
}

/*
 * main
 */

int main(int ac, char **av)
{
	char *optstr = "b:C:cdEefghiK:k:Ll:n:qs:t:VvW:w:";
	char *p1, *p2, *p3, *odev = 0, *backup = 0;
	int opt, sectsize = 0, no_of_incons = 0;
	disk_desc *d;

	g_mod_addinternals();
	while ((opt = getopt(ac, av, optstr)) != -1)
		switch (opt) {
		case 'b':
			backup = optarg;
			break;
		case 'C':
			if (!get_csep_arg(optarg, &p1, &p2, &p3)) {
				usage();
				return (EXIT_FAILURE);
			}
			gc = strtoul(p1, 0, 0);
			if (errno == ERANGE)
				pr(FATAL, EM_INVVALUE);
			gh = strtoul(p2, 0, 0);
			if (errno == ERANGE)
				pr(FATAL, EM_INVVALUE);
			gs = strtoul(p3, 0, 0);
			if (errno == ERANGE)
				pr(FATAL, EM_INVVALUE);
			break;
		case 'c':
			f_check = 1;
			break;
		case 'd':
			f_dontguess = 1;
			break;
		case 'E':
			f_testext = 0;
			break;
		case 'e':
			f_skiperrors = 0;
			break;
		case 'f':
			f_fast = 0;
			break;
		case 'g':
			f_getgeom = 0;
			break;
		case 'i':
			f_interactive = 1;
			break;
		case 'K':
			maxsec = strtoul(optarg, 0, 0);
			if ((maxsec <= 0) || (errno == ERANGE))
				pr(FATAL, EM_INVVALUE);
			break;
		case 'k':
			/* strtos64? */
			skipsec = strtoul(optarg, 0, 0);
			if (errno == ERANGE)
				pr(FATAL, EM_INVVALUE);
			break;
		case 'n':
			if ((*optarg == 's') || (*optarg == 'h') || (*optarg == 'c'))
				increment = *optarg;
			else {
				increment = strtoul(optarg, 0, 0);
				if (errno == ERANGE)
					pr(FATAL, EM_INVVALUE);
			}
			break;
		case 'l':
			if (logfile)
				fclose(logfile);
			if ((logfile = fopen(optarg, "w")) == 0)
				pr(FATAL, EM_OPENLOG, optarg);
			break;
		case 'L':
			g_mod_list();
			return (EXIT_SUCCESS);
		case 'q':
			f_quiet = 1;
			break;
		case 's':
			if ((sectsize = atoi(optarg)) <= 0)
				pr(FATAL, "sector size must be >= 0");
			break;
		case 'v':
			f_verbose++;
			break;
		case 'V':
			fprintf(stderr, "%s\n", gpart_version);
			return (EXIT_SUCCESS);
		case 'w':
			if (!get_csep_arg(optarg, &p1, &p2, 0)) {
				usage();
				return (EXIT_FAILURE);
			}
			if (!g_mod_setweight(p1, atof(p2)))
				pr(FATAL, EM_NOSUCHMOD, p1);
			break;
		case 'W':
			odev = optarg;
			break;
		case '?':
		case 'h':
		default:
			usage();
			return (EXIT_FAILURE);
		}

	if ((optind + 1) != ac) {
		usage();
		return (EXIT_FAILURE);
	}

	if (f_dontguess)
		f_check = 0;
	if (f_check) {
		f_quiet = 1;
		f_dontguess = 0;
		odev = 0;
	}
	if (f_quiet)
		f_interactive = 0;

	sync();
	d = get_disk_desc(av[optind], sectsize);
	if (f_verbose > 0)
		print_disk_desc(d);
	if (f_verbose > 2)
		print_mboot_block(d);

	if (!f_dontguess) {
		sleep(1);
		sync();
		do_guess_loop(d);
		no_of_incons = check_partition_list(d);
		pr(MSG, DM_GUESSEDPTBL);
		print_ptable(d, &d->d_gpt, 0);

		if ((no_of_incons == 0) && f_check)
			no_of_incons = compare_parttables(d);

		if ((no_of_incons == 0) && odev) {
			if (backup)
				make_mbr_backup(d, backup);
			write_primary_ptbl(d, odev);
		}
	}
	free_disk_desc(d);
	if (logfile)
		fclose(logfile);

	return (f_check ? no_of_incons : 0);
}
