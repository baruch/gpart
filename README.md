# gpart

[![Build Status](https://travis-ci.org/baruch/gpart.svg)](https://travis-ci.org/baruch/gpart)

Gpart is a small tool which tries to guess what partitions
are on a PC type, MBR-partitioned hard disk in case the
primary partition table was damaged.

Gpart works by scanning through the device (or file) given on
the command line on a sector basis. Each guessing module is
asked if it thinks a filesystem it knows about could start at
a given sector. Several filesystem guessing modules are built
in.

Consult the manual page for command line options and usage.


## Installation

See file *INSTALL*.

## Currently recognized partitions/filesystems types

   Modname  | Typ  | Description
   :--------|:----:|:------------
   fat      | 0x01 | Primary DOS with 12 bit FAT
            | 0x04 | Primary DOS with 16 bit FAT (<= 32MB)
            | 0x06 | Primary 'big' DOS (> 32MB)
            | 0x0B | DOS or Windows 95 with 32 bit FAT
            | 0x0C | DOS or Windows 95 with 32 bit FAT, LBA
   ntfs     | 0x07 | OS/2 HPFS, NTFS, QNX or Advanced UNIX
   hpfs     | 0x07 | OS/2 HPFS, NTFS, QNX or Advanced UNIX
   ext2     | 0x83 | Linux ext2 filesystem
   lswap    | 0x82 | Linux swap
   bsddl    | 0xA5 | FreeBSD/NetBSD/386BSD
   s86dl    | 0x82 | Solaris/x86 disklabel
   minix    | 0x80 | Minix V1
            | 0x81 | Minix V2
   reiserfs | 0x83 | ReiserFS filesystem
   hmlvm    | 0xFE | Linux LVM physical volumes
   qnx4     | 0x4F | QNX 4.x
   beos     | 0xEB | BeOS fs
   xfs      | 0x83 | SGI XFS filesystem



## Guessing modules

Each guessing module must provide three functions callable from
gpart:

    int xxx_init(disk_desc *d,g_module *m)

>   Initialisation function. Will be called before a scan.
>   It should return the minimum number of bytes it wants
>   to receive for a test. The module should set the
>   description of the filesystem/partition type it handles
>   in `g_module.m_desc`. If the filesystem/partition type
>   included a partition table like first sector (like the
>   \*BSD disklabels do), the flag `m_hasptbl` should be set.
>   Another flag is `m_notinext` which means the tested type
>   cannot reside in a logical partition.

    int xxx_term(disk_desc *d)

>   Termination/cleanup function, called after the scanning
>   of the device has been done.

    int xxx_gfun(disk_desc *d,g_module *m)

>   The actual guessing function, called from within the
>   scan loop. It should test the plausibility of the
>   given sectors, and return its guess in `m->m_guess` (a
>   probability between 0 and 1). See existing modules
>   for examples.
>
>   The given file descriptor `d->d_fd` can be used for seeking
>   and reading (see e.g. *gm_ext2.c* which tries to read
>   the first spare superblock). If a module is convinced
>   that it has found a filesystem/partition start it should
>   fill in the assumed begin and size of the partition.
>
>   The test performed should not be too pedantic, for
>   instance it should not be relied upon that the file-
>   system is clean/was properly unmounted. On the other
>   hand too much tolerance leads to misguided guesses,
>   so a golden middle way must be found.


## Output explanation

Here is a sample `gpart -v` run on my first IDE hard disk
(comments in block-quotes):

    dev(/dev/hda) mss(512) chs(1232/255/63)(LBA) #s(19792080) size(9664mb)

>   `mss` is the medium sector size, `chs` the geometry retrieved
>   from the OS (or from the command line), `#s` is the total
>   sector count.

    Primary partition(1)
       type: 006(0x06)(Primary 'big' DOS (> 32MB)) (BOOT)
       size: 502mb #s(1028097) s(63-1028159)
       chs:  (0/1/1)-(63/254/63)d (0/1/1)-(63/254/63)r
       hex:  80 01 01 00 06 FE 3F 3F 3F 00 00 00 01 B0 0F 00

>   `size`: the size of the partition in megabytes, number of
>   sectors and the sector range.  
>   `chs`: the partition table chs range (`d`) and the real one
>   (`r`). If the number of cylinders is less than 1024, both
>   are identical.  
>   `hex`: the hexadecimal representation of the partition entry
>   as found in the partition table.  

...

    Begin scan...
    Possible partition(DOS FAT), size(502mb), offset(0mb)
       type: 006(0x06)(Primary 'big' DOS (> 32MB))
       size: 502mb #s(1028097) s(63-1028159)
       chs:  (0/1/1)-(63/254/63)d (0/1/1)-(63/254/63)r
       hex:  00 01 01 00 06 FE 3F 3F 3F 00 00 00 01 B0 0F 00

    Possible extended partition at offset(502mb)
        Possible partition(Linux ext2), size(31mb), offset(502mb)
          type: 131(0x83)(Linux ext2 filesystem)
          size: 31mb #s(64196) s(1028223-1092418)
          chs:  (64/1/1)-(67/254/62)d (64/1/1)-(67/254/62)r
          hex:  00 01 01 40 83 FE 3E 43 7F B0 0F 00 C4 FA 00 00

       Possible partition(Linux swap), size(125mb), offset(533mb)
          type: 130(0x82)(Linux swap or Solaris/x86)
          size: 125mb #s(256976) s(1092483-1349458)
          chs:  (68/1/1)-(83/254/62)d (68/1/1)-(83/254/62)r
          hex:  00 01 01 44 82 FE 3E 53 83 AB 10 00 D0 EB 03 00

>   During the scan phase all found partitions are listed by
>   their real type names. The Linux swap partition above is
>   recognized as Linux swap but will get the 0x82 partition
>   identifier which can be both a Solaris disklabel or a
>   Linux swap partition.
>
>   When examining the hex values of the first primary partition
>   it can be seen that they are identical to the values of the
>   actual partition table (good guess) except for the first
>   value (0x80 vs. 0x00). This entry denotes the partition
>   'boot' flag which cannot be guessed.

...

    End scan.

    Checking partitions...
    Partition(Primary 'big' DOS (> 32MB)): primary
       Partition(Linux ext2 filesystem): logical
       Partition(Linux swap or Solaris/x86): logical
       Partition(Linux LVM physical volume): logical
       Partition(Linux ext2 filesystem): logical
       Partition(DOS or Windows 95 with 32 bit FAT, LBA): logical
    Partition(FreeBSD/NetBSD/386BSD): primary
    Partition(Linux LVM physical volume): primary
    Ok.

>   During the scan phase gpart gathers a simple list of possible
>   partitions, the check phase now tries to decide if found
>   extended partitions seem consistent, if partitions do not
>   overlap etc. Overlapping partitions are silently discarded,
>   all remaining ones are given an attribute 'primary', 'logical',
>   'orphaned' or 'invalid'. If gpart is called like `gpart -vv ...`,
>   it also tells why it thinks a partition guess is invalid.
>
>   If any inconsistencies are found, gpart prints the number
>   of remaining inconsistencies, otherwise it says 'Ok.'

    Guessed primary partition table:
    Primary partition(1)
       type: 006(0x06)(Primary 'big' DOS (> 32MB))
       size: 502mb #s(1028097) s(63-1028159)
       chs:  (0/1/1)-(63/254/63)d (0/1/1)-(63/254/63)r
       hex:  00 01 01 00 06 FE 3F 3F 3F 00 00 00 01 B0 0F 00

    Primary partition(2)
       type: 005(0x05)(Extended DOS)
       size: 6157mb #s(12611025) s(1028160-13639184)
       chs:  (64/0/1)-(848/254/63)d (64/0/1)-(848/254/63)r
       hex:  00 00 01 40 05 FE FF 50 40 B0 0F 00 D1 6D C0 00

    Primary partition(3)
       type: 165(0xA5)(FreeBSD/NetBSD/386BSD)
       size: 1396mb #s(2859570) s(13639185-16498754)
       chs:  (849/0/1)-(1023/254/63)d (849/0/1)-(1026/254/63)r
       hex:  00 00 C1 51 A5 FE FF FF 11 1E D0 00 32 A2 2B 00

    Primary partition(4)
       type: 254(0xFE)(Linux LVM physical volume)
       size: 1608mb #s(3293325) s(16498755-19792079)
       chs:  (1023/254/63)-(1023/254/63)d (1027/0/1)-(1231/254/63)r
       hex:  00 FE FF FF FE FE FF FF 43 C0 FB 00 8D 40 32 00

>   This is a resulting primary partition table. Note that
>   the logical partition guesses were only used to create
>   the extended partition entry. Up to now gpart cannot
>   reconstruct a damaged logical partition chain itself.
>
>   If a guessed primary partition table should be written to
>   some file or device the user must specify (via the `-W`
>   option) which partition gets the active (bootable) one.

## Author

gpart README, Aug 1999, Michail Brzitwa <mb@ichabod.han.de>
